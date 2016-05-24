//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqSimBuilderUIManager.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"
#include "smtk/model/Manager.h"

#include "vtkDoubleArray.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkSBFunctionParser.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxy.h"

#include "SimBuilderCore.h"

#include "pqSMTKUIHelper.h"
#include "pqSMTKModelPanel.h"

#include <QLineEdit>
#include <QStringList>
#include <QMessageBox>

//----------------------------------------------------------------------------
class pqSimBuilderUIManagerInternals
{
  public:

  smtk::model::WeakManagerPtr ModelMgr;
  typedef QMap<QString, QList<smtk::attribute::DefinitionPtr> > DefMap;
  typedef QMap<QString, QList<smtk::attribute::DefinitionPtr> >::const_iterator DefMapIt;
};

//----------------------------------------------------------------------------
pqSimBuilderUIManager::pqSimBuilderUIManager(QObject* parent)
  : QObject(parent)
{
  this->ActiveServer = NULL;
  this->RenderView = NULL;
  this->m_AttSystem = smtk::attribute::SystemPtr(new smtk::attribute::System());
  this->m_attUIManager = new smtk::extension::qtUIManager(*(this->m_AttSystem));
  this->Internals = new pqSimBuilderUIManagerInternals;
 }

//----------------------------------------------------------------------------
pqSimBuilderUIManager::~pqSimBuilderUIManager()
{
  delete this->Internals;

  this->m_AttSystem = smtk::attribute::SystemPtr();
  if (this->m_attUIManager != NULL)
    {
    delete this->m_attUIManager;
    }
}

//----------------------------------------------------------------------------
smtk::extension::qtBaseView* pqSimBuilderUIManager::topView()
{
  return this->m_attUIManager->topView();
}

//----------------------------------------------------------------------------
smtk::model::ManagerPtr pqSimBuilderUIManager::attModelManager() const
{
  return this->Internals->ModelMgr.lock();
}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::setModelManager(smtk::model::ManagerPtr refModelMgr)
{
  smtk::model::ManagerPtr curManager = this->Internals->ModelMgr.lock();
  if (curManager != refModelMgr)
    {
    this->Internals->ModelMgr = refModelMgr;
    }
  this->m_AttSystem->setRefModelManager(refModelMgr);
}
//-----------------------------------------------------------------------------
void pqSimBuilderUIManager::setModelPanel(pqSMTKModelPanel* panel)
{
  this->m_ModelPanel = panel;
}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::setSMTKView(smtk::common::ViewPtr view,
                                        QWidget* parentWidget, SimBuilderCore* sbCore)
{
  if (!view)
    {
    QMessageBox::warning(parentWidget,
      tr("No Views Warning"),
      tr("There is no top view specified in the template file!"),
      QMessageBox::Ok);
    return;
    }

  this->m_attUIManager->disconnect();
  QObject::connect(this->m_attUIManager, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)),
    this, SLOT(onFileItemCreated(smtk::extension::qtFileItem*)));
  QObject::connect(this->m_attUIManager,
    SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)),
    this, SLOT(onModelEntityItemCreated(smtk::extension::qtModelEntityItem*)));
  QObject::connect(this->m_attUIManager, SIGNAL(entitiesSelected(const smtk::common::UUIDs&)),
    this, SLOT(onRequestEntitySelection(const smtk::common::UUIDs&)));

  this->m_attUIManager->setSMTKView(view, parentWidget);
  if (!this->topView())
    {
    QMessageBox::warning(parentWidget,
      tr("No Views Warning"),
      tr("There is no top level view created in the UI."),
      QMessageBox::Ok);
    return;
    }

  smtk::extension::qtGroupView* qTopGroupV =
    qobject_cast<smtk::extension::qtGroupView*>(this->topView());
  if (!qTopGroupV)
    {
    QMessageBox::warning(parentWidget,
      tr("No Root View Warning"),
      tr("There is no root view created in the UI."),
      QMessageBox::Ok);
    return;
    }

  // callbacks from Expressions sections
  QList<smtk::extension::qtBaseView*> expressions;
  qTopGroupV->getChildView("SimpleExpression", expressions);
  foreach(smtk::extension::qtBaseView* sec, expressions)
    {
    smtk::extension::qtSimpleExpressionView* simpleExpSec =
      qobject_cast<smtk::extension::qtSimpleExpressionView*>(sec);
    if(simpleExpSec)
      {
      QObject::connect(simpleExpSec, SIGNAL(onCreateFunctionWithExpression(
        QString&, double, double, int)), this,
        SLOT(createFunctionWithExpression(QString&, double, double, int)));
      }
    }

  // callbacks from Attributes sections
  QList<smtk::extension::qtBaseView*> attViews;
  qTopGroupV->getChildView("Attribute", attViews);
  foreach(smtk::extension::qtBaseView* sec, attViews)
    {
    smtk::extension::qtAttributeView* attSec =
      qobject_cast<smtk::extension::qtAttributeView*>(sec);
    if(attSec)
      {
      QObject::connect(attSec, SIGNAL(attColorChanged()), this,
        SIGNAL(attColorChanged()));
      QObject::connect(attSec, SIGNAL(attAssociationChanged()), this,
        SIGNAL(attAssociationChanged()));
      QObject::connect(attSec, SIGNAL(numOfAttriubtesChanged()), this,
        SIGNAL(numOfAttriubtesChanged()));
      }
    }

}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::onFileItemCreated(smtk::extension::qtFileItem* fileItem)
{
  if(fileItem)
    {
    QObject::connect(fileItem, SIGNAL(launchFileBrowser()),
      this, SLOT(onLaunchFileBrowser()));
    }
}
//----------------------------------------------------------------------------
void pqSimBuilderUIManager::onLaunchFileBrowser()
{
  smtk::extension::qtFileItem* const fileItem =
    qobject_cast<smtk::extension::qtFileItem*>(QObject::sender());
  if(!fileItem || !this->topView())
    {
    return;
    }
  pqSMTKUIHelper::process_smtkFileItemRequest(
    fileItem, this->ActiveServer, this->topView()->parentWidget());
}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::onModelEntityItemCreated(
  smtk::extension::qtModelEntityItem* entItem)
{
  if(entItem)
    {
    QObject::connect(entItem, SIGNAL(requestEntityAssociation()),
      this, SLOT(onRequestEntityAssociation()));
    QObject::connect(entItem, SIGNAL(entityListHighlighted(const smtk::common::UUIDs&)),
      this, SLOT(onRequestEntitySelection(const smtk::common::UUIDs&)));
    }
}
//----------------------------------------------------------------------------
void pqSimBuilderUIManager::onRequestEntityAssociation()
{
  smtk::extension::qtModelEntityItem* const entItem =
    qobject_cast<smtk::extension::qtModelEntityItem*>(QObject::sender());
  if(!entItem || !this->m_ModelPanel)
    {
    return;
    }

  this->m_ModelPanel->requestEntityAssociation(entItem);
}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::onRequestEntitySelection(const smtk::common::UUIDs& uuids)
{
  if(this->m_ModelPanel)
    this->m_ModelPanel->requestEntitySelection(uuids);
}

//----------------------------------------------------------------------------
void pqSimBuilderUIManager::createFunctionWithExpression(
  QString& funcExpr, double initVal,
  double deltaVal, int numValues)
{
  smtk::extension::qtSimpleExpressionView* const expressionView =
    qobject_cast<smtk::extension::qtSimpleExpressionView*>(
    QObject::sender());
  if(!expressionView)
    {
    return;
    }

  int errorPos = -1;
  std::string errorMsg;
  vtkNew<vtkSBFunctionParser> FunctionParser;
  FunctionParser->SetFunction(funcExpr.toStdString());
  FunctionParser->CheckExpression(errorPos, errorMsg);
  if(errorPos != -1 && !errorMsg.empty())
    {
    expressionView->displayExpressionError(errorMsg, errorPos);
    return;
    }

  FunctionParser->SetNumberOfValues(numValues);
  FunctionParser->SetInitialValue(initVal);
  FunctionParser->SetDelta(deltaVal);
  vtkDoubleArray* result = FunctionParser->GetResult();
  if(result)
    {
    int numberOfComponents = result->GetNumberOfComponents();
    std::vector<double> values(numberOfComponents);
    QStringList strVals;
    for(vtkIdType i=0,j=0;i<result->GetNumberOfTuples();i++, j+=numberOfComponents)
      {
      result->GetTupleValue(i, &values[0]);
      for(int c=0; c<numberOfComponents-1; c++)
        {
        strVals << QString::number(values[c]) <<"\t";
        }
      strVals << QString::number(values[numberOfComponents-1]);
      strVals << LINE_BREAKER_STRING;
      }

    QString valuesText = strVals.join(" ");
    expressionView->buildSimpleExpression(
      funcExpr, valuesText, numberOfComponents);
    }
}

//-----------------------------------------------------------------------------
void pqSimBuilderUIManager::getAttributeDefinitions(
    QMap<QString, QList<smtk::attribute::DefinitionPtr> > &outDefMap)
{
  if(!this->topView())
    {
    return;
    }
  smtk::extension::qtGroupView* qTopGroupV =
    qobject_cast<smtk::extension::qtGroupView*>(this->topView());
  if (!qTopGroupV)
    {
    return;
    }
  QList<smtk::extension::qtBaseView*> attsections;
  qTopGroupV->getChildView("Attribute", attsections);

  foreach(smtk::extension::qtBaseView* sec, attsections)
    {
    smtk::extension::qtAttributeView* attSec =
      qobject_cast<smtk::extension::qtAttributeView*>(sec);
    if(attSec)
      {
      const pqSimBuilderUIManagerInternals::DefMap &attDefMap = attSec->attDefinitionMap();
      pqSimBuilderUIManagerInternals::DefMapIt dit = attDefMap.begin();
      for(; dit != attDefMap.end(); ++dit)
        {
        outDefMap[dit.key()].append(dit.value());
        }
      }
    }
}
