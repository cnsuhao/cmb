/*=========================================================================

Program:   CMB
Module:    qtSceneBuilderOptions.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "qtSceneBuilderOptions.h"
#include "ui_qtSceneBuilderOptions.h"
#include "pqCMBAppCommonConfig.h" // for safe including of vtkPVConfig

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
#include "vtkProcessModule.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"

#include <QMenu>
#include <QDoubleValidator>
#include <QDir>
#include <QTemporaryFile>

class qtSceneBuilderOptions::pqInternal
  : public Ui::qtSceneBuilderOptions
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
qtSceneBuilderOptions::qtSceneBuilderOptions(QWidget *widgetParent)
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
    SIGNAL(chosenColorChanged(const QColor&)),
    this, SIGNAL(changesAvailable()));
}

//-----------------------------------------------------------------------------
qtSceneBuilderOptions::~qtSceneBuilderOptions()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qtSceneBuilderOptions::setPage(const QString &page)
{
  int count = this->Internal->stackedWidget->count();
  for(int i=0; i<count; i++)
    {
    if(this->Internal->stackedWidget->widget(i)->objectName() == page)
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
  for(int i=0; i<count; i++)
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
    "SceneBuilder/InitialSceneObjectColor",
    this->Internal->InitialSceneObjectColor->chosenColor());
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
  return settings->value("SceneBuilder/InitialSceneObjectColor",
                         QColor::fromRgbF(1, 1, 1)).value<QColor>();
}
