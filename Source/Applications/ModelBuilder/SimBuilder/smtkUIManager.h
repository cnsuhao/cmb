/*=========================================================================

  Module:    smtkUIManager.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME smtkUIManager - a user interface manager.
// .SECTION Description
// .SECTION See Also

#ifndef __smtkUIManager_h
#define __smtkUIManager_h

#include <QObject>
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "cmbSystemConfig.h"

#include <QMap>
#include <QList>

class pqServer;
class pqRenderView;
class vtkObject;
class smtkUIManagerInternals;
class vtkSMProxy;
class SimBuilderCore;

class smtkUIManager : public QObject
{

Q_OBJECT

public:
  smtkUIManager();
  virtual ~smtkUIManager();
  smtk::attribute::ManagerPtr attManager() const
    {return this->AttManager;}
  smtk::attribute::qtUIManager* qtManager() const
    {return this->qtAttManager;}

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
  smtk::model::ManagerPtr attModel() const;

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

protected:
  pqServer* ActiveServer;
  pqRenderView* RenderView;
  smtk::attribute::ManagerPtr AttManager;
  smtk::attribute::qtUIManager* qtAttManager;

private:
  static smtkUIManager* Instance;
  smtkUIManagerInternals *Internals;

};

#endif
