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

#include <QObject>
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtRootView.h"
#include "cmbSystemConfig.h"

#include <QMap>
#include <QList>
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
  pqSimBuilderUIManager(const char *topViewName="SimBuilder");
  virtual ~pqSimBuilderUIManager();
  smtk::attribute::SystemPtr attSystem() const
    {return this->AttSystem;}
  smtk::attribute::qtUIManager* qtManager() const
    {return this->qtAttSystem;}

  void setServer(pqServer* s)
  { this->ActiveServer = s; }
  pqServer *server()
  { return this->ActiveServer; }
  void setRenderView(pqRenderView* view)
  { this->RenderView = view; }

  pqRenderView *renderView()
  { return this->RenderView; }

  smtk::attribute::qtRootView* rootView();
  void initializeUI(QWidget* parentWidget, SimBuilderCore* sbCore);
  smtk::model::ManagerPtr attModelManager() const;
  void setModelManager(smtk::model::ManagerPtr);
  void setModelPanel(pqSMTKModelPanel*);

  void getAttributeDefinitions(
           QMap<QString, QList<smtk::attribute::DefinitionPtr> > &attDefMap);

signals:
  void numOfAttriubtesChanged();
  void attColorChanged();
  void attAssociationChanged();

public slots:

protected slots:
  void onFileItemCreated(smtk::attribute::qtFileItem*);
  void onLaunchFileBrowser();
  void createFunctionWithExpression(
    QString& expression, double initVal,
    double deltaVal, int numVals);
  void onModelEntityItemCreated(
  smtk::attribute::qtModelEntityItem* entItem);
  void onRequestEntityAssociation();
  void onRequestEntitySelection(const smtk::common::UUIDs& uuids);

protected:
  pqServer* ActiveServer;
  pqRenderView* RenderView;
  smtk::attribute::SystemPtr AttSystem;
  smtk::attribute::qtUIManager* qtAttSystem;
  QPointer<pqSMTKModelPanel> m_ModelPanel;

private:
  static pqSimBuilderUIManager* Instance;
  pqSimBuilderUIManagerInternals *Internals;

};

#endif
