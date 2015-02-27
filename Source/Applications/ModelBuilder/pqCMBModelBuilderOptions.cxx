/*=========================================================================

Program:   CMB
Module:    pqCMBModelBuilderOptions.cxx

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

#include "pqCMBModelBuilderOptions.h"
#include "ui_pqCMBModelBuilderOptions.h"

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

class pqCMBModelBuilderOptions::pqInternal
  : public Ui::pqCMBModelBuilderOptions
{
public:

};

//-----------------------------------------------------------------------------
pqCMBModelBuilderOptions* pqCMBModelBuilderOptions::Instance = 0;

//-----------------------------------------------------------------------------
pqCMBModelBuilderOptions* pqCMBModelBuilderOptions::instance()
{
  return pqCMBModelBuilderOptions::Instance;
}

//----------------------------------------------------------------------------
pqCMBModelBuilderOptions::pqCMBModelBuilderOptions(QWidget *widgetParent)
  : qtCMBOptionsContainer(widgetParent)
{
  // Only 1 pqCMBModelBuilderOptions instance can be created.
  Q_ASSERT(pqCMBModelBuilderOptions::Instance == NULL);
  pqCMBModelBuilderOptions::Instance = this;

  this->Internal = new pqInternal;
  this->Internal->setupUi(this);

  // start fresh
  this->resetChanges();

  // enable the apply button when things are changed
  QObject::connect(this->Internal->Default3DFaceColorMode,
                  SIGNAL(currentIndexChanged(int)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->Default2DFaceColorMode,
                  SIGNAL(currentIndexChanged(int)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->Default2DEdgeColorMode,
                  SIGNAL(currentIndexChanged(int)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->DefaultEdgeColor,
                  SIGNAL(chosenColorChanged(const QColor&)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->DefaultPolygonColor,
                  SIGNAL(chosenColorChanged(const QColor&)),
                  this, SIGNAL(changesAvailable()));

  QObject::connect(this->Internal->dirTemplateBrowserButton,
    SIGNAL(clicked()), this, SLOT(chooseSimBuilderTemplateDirectory()));
}

//-----------------------------------------------------------------------------
pqCMBModelBuilderOptions::~pqCMBModelBuilderOptions()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderOptions::setPage(const QString &page)
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
QStringList pqCMBModelBuilderOptions::getPageList()
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
void pqCMBModelBuilderOptions::applyChanges()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue(
    "SimBuilder/DefaultTemplateDirectory",
    this->Internal->dirSBTemplates->text());
  // default to none
  settings->setValue(
    "ModelBuilder/Default3DModelFaceColorMode",
    this->Internal->Default3DFaceColorMode->currentText());
  // default to none
  settings->setValue(
    "ModelBuilder/Default2DModelFaceColorMode",
    this->Internal->Default2DFaceColorMode->currentText());
  // default to none
  settings->setValue(
    "ModelBuilder/Default2DModelEdgeColorMode",
    this->Internal->Default2DEdgeColorMode->currentText());
  // colors
  settings->setValue("ModelBuilder/EdgeColor",
    this->Internal->DefaultEdgeColor->chosenColor());
  settings->setValue("ModelBuilder/PolygonColor",
    this->Internal->DefaultPolygonColor->chosenColor());
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderOptions::resetChanges()
{
  QString str3dmodelfacecolor = this->default3DModelFaceColorMode().c_str();
  int index = this->Internal->Default3DFaceColorMode->findText(str3dmodelfacecolor);
  index = (index==-1) ? 0 : index;
  this->Internal->Default3DFaceColorMode->setCurrentIndex(index);
  QString str2dmodelfacecolor = this->default2DModelFaceColorMode().c_str();
  index = this->Internal->Default2DFaceColorMode->findText(str2dmodelfacecolor);
  index = (index==-1) ? 0 : index;
  this->Internal->Default2DFaceColorMode->setCurrentIndex(index);
  QString str2dmodeledgecolor = this->default2DModelEdgeColorMode().c_str();
  index = this->Internal->Default2DEdgeColorMode->findText(str2dmodeledgecolor);
  index = (index==-1) ? 0 : index;
  this->Internal->Default2DEdgeColorMode->setCurrentIndex(index);

  QColor ecolor = this->defaultEdgeColor();
  this->Internal->DefaultEdgeColor->setChosenColor(ecolor);
  QColor pcolor = this->defaultPolygonColor();
  this->Internal->DefaultPolygonColor->setChosenColor(pcolor);

  this->Internal->dirSBTemplates->setText(
    this->defaultSimBuilderTemplateDirectory().c_str());
}

//-----------------------------------------------------------------------------
std::string pqCMBModelBuilderOptions::defaultSimBuilderTemplateDirectory()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  return settings->value(
    "SimBuilder/DefaultTemplateDirectory",QApplication::applicationDirPath())
    .toString().toStdString();

}
//-----------------------------------------------------------------------------
std::string pqCMBModelBuilderOptions::default3DModelFaceColorMode()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  // default to none
  return settings->value(
    "ModelBuilder/Default3DModelFaceColorMode","None").toString().toStdString();
}
//-----------------------------------------------------------------------------
std::string pqCMBModelBuilderOptions::default2DModelFaceColorMode()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  // default to none
  return settings->value(
    "ModelBuilder/Default2DModelFaceColorMode","None").toString().toStdString();
}
//-----------------------------------------------------------------------------
std::string pqCMBModelBuilderOptions::default2DModelEdgeColorMode()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  // default to none
  return settings->value(
    "ModelBuilder/Default2DModelEdgeColorMode","None").toString().toStdString();
}
//-----------------------------------------------------------------------------
QColor pqCMBModelBuilderOptions::defaultPolygonColor()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  // default to white
  return settings->value("ModelBuilder/PolygonColor",
   QColor::fromRgbF(1.0, 1.0, 1.0)).value<QColor>();
}
//-----------------------------------------------------------------------------
QColor  pqCMBModelBuilderOptions::defaultEdgeColor()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  // default to white
  return settings->value("ModelBuilder/EdgeColor",
   QColor::fromRgbF(1.0, 1.0, 1.0)).value<QColor>();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderOptions::chooseSimBuilderTemplateDirectory()
{
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
    this,"Select Default SimBuilder Template Directory",
    this->defaultSimBuilderTemplateDirectory().c_str());
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
    {
    //update the line-edit with the new folder
    this->Internal->dirSBTemplates->setText(dialog.getSelectedFiles()[0]);
    emit this->changesAvailable();
    }
}
