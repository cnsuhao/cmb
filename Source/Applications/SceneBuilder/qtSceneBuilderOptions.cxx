//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtSceneBuilderOptions.h"
#include "pqCMBAppCommonConfig.h" // for safe including of vtkPVConfig
#include "ui_qtSceneBuilderOptions.h"

#include "pqActiveObjects.h"
#include "pqAnimationScene.h"
#include "pqApplicationCore.h"
#include "pqFileDialog.h"
#include "pqInterfaceTracker.h"
#include "pqOptions.h"
#include "pqPipelineRepresentation.h"
#include "pqPluginManager.h"
#include "pqRenderView.h"
#include "pqScalarsToColors.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSettings.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"

#include <QDir>
#include <QDoubleValidator>
#include <QMenu>
#include <QTemporaryFile>

class qtSceneBuilderOptions::pqInternal : public Ui::qtSceneBuilderOptions
{
public:
};

//-----------------------------------------------------------------------------
qtSceneBuilderOptions* qtSceneBuilderOptions::Instance = 0;

//-----------------------------------------------------------------------------
qtSceneBuilderOptions* qtSceneBuilderOptions::instance()
{
  return qtSceneBuilderOptions::Instance;
}

//----------------------------------------------------------------------------
qtSceneBuilderOptions::qtSceneBuilderOptions(QWidget* widgetParent)
  : qtCMBOptionsContainer(widgetParent)
{
  // Only 1 qtSceneBuilderOptions instance can be created.
  Q_ASSERT(qtSceneBuilderOptions::Instance == NULL);
  qtSceneBuilderOptions::Instance = this;

  this->Internal = new pqInternal;
  this->Internal->setupUi(this);

  // start fresh
  this->resetChanges();

  // enable the apply button when things are changed
  QObject::connect(this->Internal->InitialSceneObjectColor,
    SIGNAL(chosenColorChanged(const QColor&)), this, SIGNAL(changesAvailable()));
}

//-----------------------------------------------------------------------------
qtSceneBuilderOptions::~qtSceneBuilderOptions()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qtSceneBuilderOptions::setPage(const QString& page)
{
  int count = this->Internal->stackedWidget->count();
  for (int i = 0; i < count; i++)
  {
    if (this->Internal->stackedWidget->widget(i)->objectName() == page)
    {
      this->Internal->stackedWidget->setCurrentIndex(i);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
QStringList qtSceneBuilderOptions::getPageList()
{
  QStringList pages;

  int count = this->Internal->stackedWidget->count();
  for (int i = 0; i < count; i++)
  {
    pages << this->Internal->stackedWidget->widget(i)->objectName();
  }
  return pages;
}

//-----------------------------------------------------------------------------
void qtSceneBuilderOptions::applyChanges()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue(
    "SceneBuilder/InitialSceneObjectColor", this->Internal->InitialSceneObjectColor->chosenColor());
}

//-----------------------------------------------------------------------------
void qtSceneBuilderOptions::resetChanges()
{
  QColor objcolor = this->initialNewObjectColor();
  this->Internal->InitialSceneObjectColor->setChosenColor(objcolor);
}
//-----------------------------------------------------------------------------
QColor qtSceneBuilderOptions::initialNewObjectColor()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  return settings->value("SceneBuilder/InitialSceneObjectColor", QColor::fromRgbF(1, 1, 1))
    .value<QColor>();
}
