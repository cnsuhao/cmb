/*=========================================================================

  Module:    smtkUIManager.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "smtkUIManager.h"

#include "smtkModel.h"

#include "smtk/view/Base.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/Qt/qtItem.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtFileItem.h"
#include "smtk/Qt/qtRootView.h"
#include "smtk/Qt/qtAttributeView.h"
#include "smtk/Qt/qtBaseView.h"
#include "smtk/Qt/qtSimpleExpressionView.h"

#include "vtkDiscreteModel.h"
#include "vtkDoubleArray.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkSBFunctionParser.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxy.h"

#include "SimBuilderCore.h"
#include "../pqCMBModel.h"

#include "pqFileDialog.h"

#include <QLineEdit>
#include <QStringList>

//----------------------------------------------------------------------------
class smtkUIManagerInternals
{
  public:

    vtkSmartPointer<vtkEventQtSlotConnect> ModelConnect;
    smtk::model::ModelPtr AttModel;

  typedef QMap<QString, QList<smtk::attribute::DefinitionPtr> > DefMap;
  typedef QMap<QString, QList<smtk::attribute::DefinitionPtr> >::const_iterator DefMapIt;
};

//----------------------------------------------------------------------------
smtkUIManager::smtkUIManager()
{
  this->ActiveServer = NULL;
  this->RenderView = NULL;
  this->AttManager = smtk::attribute::ManagerPtr(new smtk::attribute::Manager());
  this->qtAttManager = new smtk::attribute::qtUIManager(*(this->AttManager));
  this->Internals = new smtkUIManagerInternals;
  this->Internals->AttModel = smtk::model::ModelPtr(new smtkModel());
  this->AttManager->setRefModel(this->Internals->AttModel);

}

//----------------------------------------------------------------------------
smtkUIManager::~smtkUIManager()
{
  this->setupModelConnection(NULL, NULL);

  delete this->Internals;

  this->AttManager = smtk::attribute::ManagerPtr();
  if (this->qtAttManager != NULL)
    {
    delete this->qtAttManager;
    }
}
//----------------------------------------------------------------------------
smtk::attribute::qtRootView* smtkUIManager::rootView()
{
  return this->qtAttManager->rootView();
}

//----------------------------------------------------------------------------
smtkModel* smtkUIManager::attModel()
{
  return dynamic_cast<smtkModel*>(this->Internals->AttModel.get());
}

//----------------------------------------------------------------------------
void smtkUIManager::initializeUI(QWidget* parentWidget, SimBuilderCore* sbCore)
{
  this->qtManager()->disconnect();
  QObject::connect(this->qtAttManager, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SLOT(onFileItemCreated(smtk::attribute::qtFileItem*)));

  this->qtManager()->initializeUI(parentWidget);
  // callbacks from Expressions sections
  QList<smtk::attribute::qtBaseView*> expressions;
  this->qtManager()->rootView()->getChildView(
    smtk::view::Base::SIMPLE_EXPRESSION, expressions);
  foreach(smtk::attribute::qtBaseView* sec, expressions)
    {
    smtk::attribute::qtSimpleExpressionView* simpleExpSec =
      qobject_cast<smtk::attribute::qtSimpleExpressionView*>(sec);
    if(simpleExpSec)
      {
      QObject::connect(simpleExpSec, SIGNAL(onCreateFunctionWithExpression(
        QString&, double, double, int)), this,
        SLOT(createFunctionWithExpression(QString&, double, double, int)));
      }
    }

  // callbacks from Attributes sections
  QList<smtk::attribute::qtBaseView*> attViews;
  this->qtManager()->rootView()->getChildView(
    smtk::view::Base::ATTRIBUTE, attViews);
  foreach(smtk::attribute::qtBaseView* sec, attViews)
    {
    smtk::attribute::qtAttributeView* attSec =
      qobject_cast<smtk::attribute::qtAttributeView*>(sec);
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

  if (sbCore)
    {
    pqCMBModel* model = sbCore->getCMBModel();
    QObject::connect(model, SIGNAL(modelEntityNameChanged(vtkModelEntity*)),
      this, SLOT(updateModelItems()));
    }
}
//----------------------------------------------------------------------------
void smtkUIManager::setupModelConnection(vtkDiscreteModel* model,
  vtkSMProxy* modelwrapper)
{
  if(this->attModel()->discreteModel() == model &&
    this->attModel()->modelWrapper() == modelwrapper)
    {
    return;
    }
  this->attModel()->setDiscreteModel(model);
  this->attModel()->setModelWrapper(modelwrapper);

  if(model)
    {
    if(this->Internals->ModelConnect)
      {
      this->Internals->ModelConnect->Disconnect();
      }
    else
      {
      this->Internals->ModelConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
      }
    this->Internals->ModelConnect->Connect(model, DomainSetCreated,
      this, SLOT(modelCallback(vtkObject* , unsigned long , void* ,
      void*)));
    this->Internals->ModelConnect->Connect(model, DomainSetDestroyed,
      this, SLOT(modelCallback(vtkObject* , unsigned long , void* ,
      void*)));
    this->Internals->ModelConnect->Connect(model, ModelEntityGroupCreated,
      this, SLOT(modelCallback(vtkObject* , unsigned long , void* , void* )));
    this->Internals->ModelConnect->Connect(model, ModelEntityGroupDestroyed,
      this, SLOT(modelCallback(vtkObject* , unsigned long , void* , void* )));
    }
  else if(this->Internals->ModelConnect)
    {
    this->Internals->ModelConnect->Disconnect();
    }
}

//----------------------------------------------------------------------------
void smtkUIManager::onFileItemCreated(smtk::attribute::qtFileItem* fileItem)
{
  if(fileItem)
    {
    QObject::connect(fileItem, SIGNAL(launchFileBrowser()),
      this, SLOT(onLaunchFileBrowser()));
    }
}
//----------------------------------------------------------------------------
void smtkUIManager::onLaunchFileBrowser()
{
  smtk::attribute::qtFileItem* const fileItem =
    qobject_cast<smtk::attribute::qtFileItem*>(QObject::sender());
  if(!fileItem)
    {
    return;
    }
  QLineEdit* lineEdit =  static_cast<QLineEdit*>(
    fileItem->property("DataItem").value<void *>());
  if(!lineEdit)
    {
    return;
    }

  QString filters;
  bool existingFile=true;
  if (!fileItem->isDirectory())
    {
    smtk::attribute::FileItemPtr fItem =
      smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(fileItem->getObject());
    const smtk::attribute::FileItemDefinition *fItemDef =
      dynamic_cast<const smtk::attribute::FileItemDefinition*>(fItem->definition().get());
    filters = fItemDef->getFileFilters().c_str();
    existingFile = fItemDef->shouldExist();
    }
  else
    {
    smtk::attribute::DirectoryItemPtr dItem =
      smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(fileItem->getObject());
    const smtk::attribute::DirectoryItemDefinition *dItemDef =
      dynamic_cast<const smtk::attribute::DirectoryItemDefinition*>(dItem->definition().get());
    existingFile = dItemDef->shouldExist();
    }

  QString title = fileItem->isDirectory() ? tr("Select Directory:") :
    tr("Select File:");
  pqFileDialog file_dialog(this->ActiveServer,
     this->rootView()->parentWidget(),
     title,
     QString(),
     filters);

  if (fileItem->isDirectory())
    {
    file_dialog.setFileMode(pqFileDialog::Directory);
    }
  else if (existingFile)
    {
    file_dialog.setFileMode(pqFileDialog::ExistingFile);
    }
  else
    {
    file_dialog.setFileMode(pqFileDialog::AnyFile);
    }
  file_dialog.setObjectName("SimBuilder Select File Dialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    lineEdit->setText(files[0]);
    fileItem->onInputValueChanged();
    }
}
//----------------------------------------------------------------------------
void smtkUIManager::createFunctionWithExpression(
  QString& funcExpr, double initVal,
  double deltaVal, int numValues)
{
  smtk::attribute::qtSimpleExpressionView* const expressionView =
    qobject_cast<smtk::attribute::qtSimpleExpressionView*>(
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
void smtkUIManager::loadModelItems(int groupType)
{
  this->attModel()->loadGroupItems(groupType);
}

//-----------------------------------------------------------------------------
void smtkUIManager::updateModelItems(int groupType)
{
  this->attModel()->updateGroupItems(groupType);
}

//-----------------------------------------------------------------------------
void smtkUIManager::clearModelItems()
{
  this->attModel()->clearItems();
  this->qtManager()->updateModelViews();
}

//-----------------------------------------------------------------------------
void smtkUIManager::updateModelItems()
{
  this->attModel()->updateGroupItems(vtkModelMaterialType);
  this->attModel()->updateGroupItems(vtkDiscreteModelEntityGroupType);
  this->qtManager()->updateModelViews();
}

//-----------------------------------------------------------------------------
void smtkUIManager::modelCallback(
  vtkObject* object, unsigned long e, void* /*clientData*/, void* callData)
{
  vtkIdType entityId = *reinterpret_cast<vtkIdType*>(callData);
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(object);
  if(!model || model != this->attModel()->discreteModel())
    {
    return;
    }
  int grouptype = -1;
  switch(e)
    {
    case DomainSetCreated:
    case DomainSetDestroyed:
      grouptype = vtkModelMaterialType;
      break;
    case ModelEntityGroupCreated:
    case ModelEntityGroupDestroyed:
      grouptype = vtkDiscreteModelEntityGroupType;
      break;
    default:
      break;
    }
  if(grouptype == vtkModelMaterialType ||
     grouptype == vtkDiscreteModelEntityGroupType)
    {
    if(e == DomainSetDestroyed || e == ModelEntityGroupDestroyed)
      {
      this->attModel()->removeModelItem(entityId);
      }
    else
      {
      this->attModel()->createModelGroupFromCMBGroup(entityId);
      }
    this->qtManager()->updateModelViews();
    }
}
//-----------------------------------------------------------------------------
void smtkUIManager::getAttributeDefinitions(
    QMap<QString, QList<smtk::attribute::DefinitionPtr> > &outDefMap)
{
  QList<smtk::attribute::qtBaseView*> attsections;
  this->qtManager()->rootView()->getChildView(
    smtk::view::Base::ATTRIBUTE, attsections);

  foreach(smtk::attribute::qtBaseView* sec, attsections)
    {
    smtk::attribute::qtAttributeView* attSec =
      qobject_cast<smtk::attribute::qtAttributeView*>(sec);
    if(attSec)
      {
      const smtkUIManagerInternals::DefMap &attDefMap = attSec->attDefinitionMap();
      smtkUIManagerInternals::DefMapIt dit = attDefMap.begin();
      for(; dit != attDefMap.end(); ++dit)
        {
        outDefMap[dit.key()].append(dit.value());
        }
      }
    }
}
