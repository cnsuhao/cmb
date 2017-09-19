//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqSimBuilderUIManager - a user interface manager.
// .SECTION Description
// .SECTION See Also

#ifndef __pqSimBuilderUIManager_h
#define __pqSimBuilderUIManager_h

#include "cmbSystemConfig.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtUIManager.h"
#include <QObject>

#include <QList>
#include <QMap>
#include <QPointer>

class pqServer;
class pqRenderView;
class vtkObject;
class pqSimBuilderUIManagerInternals;
class vtkSMProxy;
class SimBuilderCore;
class pqSMTKModelPanel;

class pqSimBuilderUIManager : public QObject
{

  Q_OBJECT

public:
  pqSimBuilderUIManager(QObject* parent = NULL);
  ~pqSimBuilderUIManager() override;
  smtk::attribute::SystemPtr attributeSystem() const { return this->m_AttSystem; }
  smtk::extension::qtUIManager* attributeUIManager() const { return this->m_attUIManager; }

  void setServer(pqServer* s) { this->ActiveServer = s; }
  pqServer* server() { return this->ActiveServer; }
  void setRenderView(pqRenderView* view) { this->RenderView = view; }

  pqRenderView* renderView() { return this->RenderView; }

  smtk::extension::qtBaseView* topView();
  void setSMTKView(smtk::common::ViewPtr topView, QWidget* parentWidget);
  smtk::model::ManagerPtr attModelManager() const;
  void setModelManager(smtk::model::ManagerPtr);
  void setModelPanel(pqSMTKModelPanel*);

  void getAttributeDefinitions(QMap<QString, QList<smtk::attribute::DefinitionPtr> >& attDefMap);

signals:
  void numOfAttributesChanged();
  void attColorChanged();
  void attAssociationChanged();

protected slots:
  void onFileItemCreated(smtk::extension::qtFileItem*);
  void onLaunchFileBrowser();
  void createFunctionWithExpression(
    QString& expression, double initVal, double deltaVal, int numVals);
  void onModelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
  void onRequestEntityAssociation();
  void onRequestEntitySelection(const smtk::common::UUIDs& uuids);

protected:
  pqServer* ActiveServer;
  pqRenderView* RenderView;
  smtk::attribute::SystemPtr m_AttSystem;
  smtk::extension::qtUIManager* m_attUIManager;
  QPointer<pqSMTKModelPanel> m_ModelPanel;

private:
  static pqSimBuilderUIManager* Instance;
  pqSimBuilderUIManagerInternals* Internals;
};

#endif
