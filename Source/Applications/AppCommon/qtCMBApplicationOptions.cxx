/*=========================================================================

Program:   CMB
Module:    qtCMBApplicationOptions.cxx

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

#include "qtCMBApplicationOptions.h"
#include "ui_qtCMBApplicationOptions.h"

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
#include "pqColorPresetManager.h"
#include "pqColorMapModel.h"
#include "pqColorPresetModel.h"
#include "pqObjectBuilder.h"
#include "pqChartValue.h"
#include "pqSMAdaptor.h"

#include "vtkProcessModule.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLParser.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSessionProxyManager.h"

#include <QMenu>
#include <QDoubleValidator>
#include <QDir>
#include <QTemporaryFile>
#include <vtksys/ios/sstream>

class qtCMBApplicationOptions::pqInternal
  : public Ui::qtCMBApplicationOptions
{
public:
  pqInternal()
    {
    this->ColorTFProxy = NULL;
    this->OpacityTFProxy = NULL;
    this->ColorLookupTableModified = false;
    }

  void updateLODThresholdLabel(int value)
    {
    this->lodThresholdLabel->setText(
      QString("%1").arg(value/10.0, 0, 'f', 2) + " MBytes");
    }
  void updateOutlineThresholdLabel(int value)
    {
    this->outlineThresholdLabel->setText(
      QVariant(value/10.0).toString() + " MCells");
    }

  QPointer<pqSettings> CMBCommonSettings;
  QPointer<pqColorPresetManager> Presets;
  bool ColorLookupTableModified;
  QString CurrentDefaultColorLookupTable;
  vtkSMProxy* ColorTFProxy;
  vtkSMProxy* OpacityTFProxy;
};

//-----------------------------------------------------------------------------
qtCMBApplicationOptions* qtCMBApplicationOptions::Instance = 0;

//-----------------------------------------------------------------------------
qtCMBApplicationOptions* qtCMBApplicationOptions::instance()
{
  return qtCMBApplicationOptions::Instance;
}

//----------------------------------------------------------------------------
qtCMBApplicationOptions::qtCMBApplicationOptions(QWidget *widgetParent)
  : qtCMBOptionsContainer(widgetParent)
{
  // Only 1 qtCMBApplicationOptions instance can be created.
  Q_ASSERT(qtCMBApplicationOptions::Instance == NULL);
  qtCMBApplicationOptions::Instance = this;

  this->Internal = new pqInternal;
  this->Internal->setupUi(this);

  this->Internal->Presets = new pqColorPresetManager(this);

  // Add the color scale presets menu.
  this->loadBuiltinColorPresets();
  this->restorePresetColorTableSettings();

  QIntValidator* validator = new QIntValidator(this->Internal->maxNumberCloudPoints);
  this->Internal->maxNumberCloudPoints->setValidator(validator);

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->Internal->ColorTFProxy = builder->createProxy(
    "lookup_tables", "PVLookupTable",
    core->getActiveServer(), "lookup_tables");
  this->Internal->OpacityTFProxy = builder->createProxy(
    "piecewise_functions", "PiecewiseFunction",
    core->getActiveServer(), "piecewise_functions");

  // start fresh
  this->resetChanges();

  // enable the apply button when things are changed
  QObject::connect(this->Internal->DefaultRepresentationType,
                  SIGNAL(currentIndexChanged(int)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->maxNumberCloudPoints,
                  SIGNAL(textChanged(const QString&)),
                  this, SIGNAL(changesAvailable()));

  QObject::connect(this->Internal->BackgroundColor,
                  SIGNAL(chosenColorChanged(const QColor&)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->SelectionColor,
                  SIGNAL(chosenColorChanged(const QColor&)),
                  this, SIGNAL(changesAvailable()));

  QObject::connect(this->Internal->dirMeshBrowserButton,
    SIGNAL(clicked()), this, SLOT(chooseMeshStorageDir()));
  QObject::connect(this->Internal->dirTempBrowserButton,
    SIGNAL(clicked()), this, SLOT(chooseTempScratchDir()));
  // TO DO: disabled for now.
  // this->Internal->pushButtonColorTable->setVisible(false);
  QObject::connect(this->Internal->pushButtonColorTable,
    SIGNAL(clicked()), this, SLOT(choosePresetColorTable()));

  QObject::connect(this->Internal->lodThreshold,
    SIGNAL(valueChanged(int)), this, SLOT(lodThresholdSliderChanged(int)));

  QObject::connect(this->Internal->outlineThreshold,
    SIGNAL(valueChanged(int)), this, SLOT(outlineThresholdSliderChanged(int)));

  QObject::connect(this->Internal->ResetColorsToDefault,
    SIGNAL(clicked()),
    this, SLOT(resetColorsToDefault()));
}

//-----------------------------------------------------------------------------
qtCMBApplicationOptions::~qtCMBApplicationOptions()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::setPage(const QString &page)
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
QStringList qtCMBApplicationOptions::getPageList()
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
void qtCMBApplicationOptions::applyChanges()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  cmbSettings->setValue("DefaultRepresentationType",
    this->Internal->DefaultRepresentationType->currentText());
  cmbSettings->setValue("MaxNumberOfCloudPoints",
    this->Internal->maxNumberCloudPoints->text().toInt());
  // colors
  cmbSettings->setValue("GlobalProperties/BackgroundColor",
    this->Internal->BackgroundColor->chosenColor());
  cmbSettings->setValue("GlobalProperties/SelectionColor",
    this->Internal->SelectionColor->chosenColor());
  // temp dirs
  cmbSettings->setValue("DefaultTempScratchDirectory",
    this->Internal->dirTempScratchFiles->text());
  cmbSettings->setValue("DefaultMeshStorageDirectory",
    this->Internal->dirStoreMeshes->text());

  // set the colors to pv settings
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue("GlobalProperties/BackgroundColor",
    this->Internal->BackgroundColor->chosenColor());
  settings->setValue("GlobalProperties/SelectionColor",
    this->Internal->SelectionColor->chosenColor());

  // This needs to be out of beginGroup()/endGroup().
//  pqPipelineRepresentation::setUnstructuredGridOutlineThreshold(
//    this->Internal->outlineThreshold->value()/10.0);
  settings->beginGroup("renderModule");
  settings->setValue("LODThreshold", this->Internal->lodThreshold->value() / 10.0);
  settings->endGroup();
  // also cache this in cmb settings
  cmbSettings->setValue("LODThreshold",
    this->Internal->lodThreshold->value() / 10.0);
  cmbSettings->setValue("UnstructuredGridOutlineThreshold",
    this->Internal->outlineThreshold->value() / 10.0);

  // apply preset color lookup table settings
  vtkSMProxy *lookupTable = this->Internal->ColorTFProxy;
  vtkSMProxy *opacityProxy = this->Internal->OpacityTFProxy;
  /*
  pqApplicationCore* core = pqApplicationCore::instance();
  pqLookupTableManager* lut_mgr = core->getLookupTableManager();
  if (lut_mgr && this->Internal->ColorLookupTableModified)
    {
    pqScalarsToColors* pqColormap =
      new pqScalarsToColors(lookupTable->GetXMLGroup(),
      lookupTable->GetXMLName(), lookupTable, core->getActiveServer(), this);
    lut_mgr->saveLUTAsDefault(pqColormap);
    delete pqColormap;
    pqScalarOpacityFunction* pqOpacity =
      new pqScalarOpacityFunction(opacityProxy->GetXMLGroup(),
      opacityProxy->GetXMLName(), opacityProxy, core->getActiveServer(), this);
    lut_mgr->saveOpacityFunctionAsDefault(pqOpacity);
    delete pqOpacity;

    // Now we need to copy this change to cmb settings
    pqSettings *settings = pqApplicationCore::instance()->settings();
    pqSettings* cmbSettings = this->cmbAppSettings();
    cmbSettings->setValue(pqPQLookupTableManager::DEFAULT_LOOKUPTABLE_SETTING_KEY(),
      settings->value(pqPQLookupTableManager::DEFAULT_LOOKUPTABLE_SETTING_KEY()));
    cmbSettings->setValue(pqPQLookupTableManager::DEFAULT_OPACITYFUNCTION_SETTING_KEY(),
      settings->value(pqPQLookupTableManager::DEFAULT_OPACITYFUNCTION_SETTING_KEY()));
    cmbSettings->setValue("/lookupTable/DefaultPresetLUT",
      this->Internal->CurrentDefaultColorLookupTable);
    }
*/
  this->loadGlobalPropertiesFromSettings();

  // render all views.
  pqApplicationCore::instance()->render();
}
//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::loadGlobalPropertiesFromSettings()
{
//  pqApplicationCore::instance()->loadGlobalPropertiesFromSettings();

  pqView* view = pqActiveObjects::instance().activeView();
  pqRenderView* renderView = qobject_cast<pqRenderView*>(view);
  if (renderView)
    {
//    renderView->restoreSettings(true);
    vtkSMRenderViewProxy* rm = renderView->getRenderViewProxy();
    rm->UpdateVTKObjects();
    rm->Update();
    rm->ResetCamera();
    renderView->forceRender();
    }
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::resetChanges()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  QString curView = this->defaultRepresentationType().c_str();
  int index = this->Internal->DefaultRepresentationType->findText(curView);
  index = (index==-1)? 0 : index;
  this->Internal->DefaultRepresentationType->setCurrentIndex(index);
  int maxNumOfPtsCloud = this->maxNumberOfCloudPoints();
  this->Internal->maxNumberCloudPoints->setText(
    QString::number(maxNumOfPtsCloud));

  this->Internal->dirStoreMeshes->setText(
    this->defaultMeshStorageDirectory().c_str());
  this->Internal->dirTempScratchFiles->setText(
    this->defaultTempScratchDirectory().c_str());

  // LOD threshold for both cmb and pv
  pqSettings* settings = pqApplicationCore::instance()->settings();
  QVariant val = cmbSettings->value("LODThreshold", 50); // default to 50 MBytes
  val = val.toDouble() >= VTK_FLOAT_MAX ? 50 : val;
  this->Internal->lodThreshold->setValue(static_cast<int>(val.toDouble()*10));
  this->Internal->updateLODThresholdLabel(this->Internal->lodThreshold->value());
  cmbSettings->value("LODThreshold",
    this->Internal->lodThreshold->value() / 10.0);
  settings->beginGroup("renderModule");
  settings->setValue("LODThreshold", this->Internal->lodThreshold->value() / 10.0);
  settings->endGroup();

  // outline threshold for both cmb and pv
  // default to 5M cells
  val = cmbSettings->value("UnstructuredGridOutlineThreshold", 5);
  this->Internal->outlineThreshold->setValue(static_cast<int>(val.toDouble()*10));
  this->Internal->updateOutlineThresholdLabel(this->Internal->outlineThreshold->value());
//  pqPipelineRepresentation::setUnstructuredGridOutlineThreshold(
//    this->Internal->outlineThreshold->value()/10.0);

  this->resetColorsToDefault();
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::resetColorsToDefault()
{
  // cmb Setting and pv setting should be consistent here
  // pqSettings* settings = pqApplicationCore::instance()->settings();
  pqSettings* cmbSettings = this->cmbAppSettings();
  QColor bcolor = cmbSettings->value("GlobalProperties/BackgroundColor",
   QColor::fromRgbF(0.32, 0.34, 0.43)).value<QColor>();
  this->Internal->BackgroundColor->setChosenColor(bcolor);
  QColor scolor = cmbSettings->value("GlobalProperties/SelectionColor",
    QColor::fromRgbF(1, 0, 1)).value<QColor>();
  this->Internal->SelectionColor->setChosenColor(scolor);

  // set the colors to pv settings
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue("GlobalProperties/BackgroundColor",
    this->Internal->BackgroundColor->chosenColor());
  settings->setValue("GlobalProperties/SelectionColor",
    this->Internal->SelectionColor->chosenColor());

  this->loadGlobalPropertiesFromSettings();
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::lodThresholdSliderChanged(int value)
{
  this->Internal->updateLODThresholdLabel(value);
  emit this->changesAvailable();
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::outlineThresholdSliderChanged(int value)
{
  this->Internal->updateOutlineThresholdLabel(value);
  emit this->changesAvailable();
}

//-----------------------------------------------------------------------------
pqSettings* qtCMBApplicationOptions::cmbAppSettings()
{
  if ( !this->Internal->CMBCommonSettings )
    {
    pqOptions* options = pqOptions::SafeDownCast(
      vtkProcessModule::GetProcessModule()->GetOptions());
    if (options && options->GetDisableRegistry())
      {
      QTemporaryFile tFile;
      tFile.open();
      this->Internal->CMBCommonSettings = new pqSettings(
        tFile.fileName() + ".ini", true, this);
      this->Internal->CMBCommonSettings->clear();
      }
    else
      {
      this->Internal->CMBCommonSettings = new pqSettings(
        QApplication::organizationName(),
        QString("CmbAppsCommon") + QApplication::applicationVersion(),
        this);
      }
    }
  return this->Internal->CMBCommonSettings;
}
//-----------------------------------------------------------------------------
int qtCMBApplicationOptions::maxNumberOfCloudPoints()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  // default to 0.1M points
  return cmbSettings->value(
    "MaxNumberOfCloudPoints",100000).toInt();

}
//-----------------------------------------------------------------------------
std::string qtCMBApplicationOptions::defaultMeshStorageDirectory()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  return cmbSettings->value(
    "DefaultMeshStorageDirectory",QDir::tempPath()).toString().toStdString();

}
//-----------------------------------------------------------------------------
std::string qtCMBApplicationOptions::defaultTempScratchDirectory()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  return cmbSettings->value(
    "DefaultTempScratchDirectory",QDir::tempPath()).toString().toStdString();
}
//-----------------------------------------------------------------------------
std::string qtCMBApplicationOptions::defaultRepresentationType()
{
  pqSettings* cmbSettings = this->cmbAppSettings();
  // default to surface
  return cmbSettings->value(
    "DefaultRepresentationType","Surface").toString().toStdString();
}
//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::chooseMeshStorageDir()
{
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
    this,"Select Mesh Storage Directory",
    this->defaultMeshStorageDirectory().c_str());
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
    {
    //update the line-edit with the new folder
    this->Internal->dirStoreMeshes->setText(dialog.getSelectedFiles()[0]);
    emit this->changesAvailable();
    }
}
//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::chooseTempScratchDir()
{
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
    this,"Select Temp Scratch Directory",
    this->defaultTempScratchDirectory().c_str());
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
    {
    //update the line-edit with the new folder
    this->Internal->dirTempScratchFiles->setText(dialog.getSelectedFiles()[0]);
    emit this->changesAvailable();
    }
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::loadBuiltinColorPresets()
{
  pqColorMapModel colorMap;
  pqColorPresetModel *model = this->Internal->Presets->getModel();
  colorMap.setColorSpace(pqColorMapModel::DivergingSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor( 59, 76, 192), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(180,  4,  38), 1.0);
  colorMap.setNanColor(QColor(63, 0, 0));
  model->addBuiltinColorMap(colorMap, "Cool to Warm");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::HsvSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(  0, 0, 255), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(255, 0,   0), 0.0);
  colorMap.setNanColor(QColor(127, 127, 127));
  model->addBuiltinColorMap(colorMap, "Blue to Red Rainbow");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::HsvSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(255, 0,   0), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(  0, 0, 255), 1.0);
  colorMap.setNanColor(QColor(127, 127, 127));
  model->addBuiltinColorMap(colorMap, "Red to Blue Rainbow");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(  0,   0,   0), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(255, 255, 255), 1.0);
  colorMap.setNanColor(QColor(255, 0, 0));
  model->addBuiltinColorMap(colorMap, "Grayscale");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(255, 255, 255 ), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(  0,   0,    0), 1.0);
  colorMap.setNanColor(QColor(255, 0, 0));
  model->addBuiltinColorMap(colorMap, "X Ray");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor( 10,  10, 242), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(242, 242,  10), 1.0);
  colorMap.setNanColor(QColor(255, 0, 0));
  model->addBuiltinColorMap(colorMap, "Blue to Yellow");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(  0,   0, 0  ), 0.0);
  colorMap.addPoint(pqChartValue(0.4), QColor(230,   0, 0  ), 0.4);
  colorMap.addPoint(pqChartValue(0.8), QColor(230, 230, 0  ), 0.8);
  colorMap.addPoint(pqChartValue(1.0), QColor(255, 255, 255), 1.0);
  colorMap.setNanColor(QColor(0, 127, 255));
  model->addBuiltinColorMap(colorMap, "Black-Body Radiation");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::LabSpace);
  colorMap.addPoint(pqChartValue(0.0), QColor(0, 153, 191), 0.0);
  colorMap.addPoint(pqChartValue(1.0), QColor(196, 119, 87),1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "CIELab Blue to Red");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),   QColor(  0,   0,   0), 0.0);
  colorMap.addPoint(pqChartValue(0.333), QColor(  0,   0, 128), 0.333);
  colorMap.addPoint(pqChartValue(0.666), QColor(  0, 128, 255), 0.666);
  colorMap.addPoint(pqChartValue(1.0),   QColor(255, 255, 255), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Black, Blue and White");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),   QColor(  0,   0, 0  ), 0.0);
  colorMap.addPoint(pqChartValue(0.333), QColor(128,   0, 0  ), 0.333);
  colorMap.addPoint(pqChartValue(0.666), QColor(255, 128, 0  ), 0.666);
  colorMap.addPoint(pqChartValue(1.0),   QColor(255, 255, 255), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Black, Orange and White");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),  QColor(  0, 255, 255 ), 0.0);
  colorMap.addPoint(pqChartValue(0.45), QColor(  0,   0, 255 ), 0.45);
  colorMap.addPoint(pqChartValue(0.5),  QColor(  0,   0, 128 ), 0.5);
  colorMap.addPoint(pqChartValue(0.55), QColor(255,   0,   0 ), 0.55);
  colorMap.addPoint(pqChartValue(1.0),  QColor(255, 255,   0 ), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Cold and Hot");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),   QColor( 71,  71, 219 ), 0.0);
  colorMap.addPoint(pqChartValue(0.143), QColor(  0,   0,  92 ), 0.143);
  colorMap.addPoint(pqChartValue(0.285), QColor(  0, 255, 255 ), 0.285);
  colorMap.addPoint(pqChartValue(0.429), QColor(  0, 128,   0 ), 0.429);
  colorMap.addPoint(pqChartValue(0.571), QColor(255, 255,   0 ), 0.571);
  colorMap.addPoint(pqChartValue(0.714), QColor(255,  97,   0 ), 0.714);
  colorMap.addPoint(pqChartValue(0.857), QColor(107,   0,   0 ), 0.857);
  colorMap.addPoint(pqChartValue(1.0),   QColor(224,  77,  77 ), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Rainbow Desaturated");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),  QColor(255, 255, 255 ), 0.0);
  colorMap.addPoint(pqChartValue(0.17), QColor(  0,   0, 255 ), 0.17);
  colorMap.addPoint(pqChartValue(0.34), QColor(  0, 255, 255 ), 0.34);
  colorMap.addPoint(pqChartValue(0.50), QColor(  0, 255,   0 ), 0.50);
  colorMap.addPoint(pqChartValue(0.67), QColor(255, 255,   0 ), 0.67);
  colorMap.addPoint(pqChartValue(0.84), QColor(255,   0,   0 ), 0.84);
  colorMap.addPoint(pqChartValue(1.0),  QColor(224,   0, 255 ), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Rainbow Blended White");

  colorMap.removeAllPoints();
  colorMap.setColorSpace(pqColorMapModel::RgbSpace);
  colorMap.addPoint(pqChartValue(0.0),  QColor( 81,  87, 110 ), 0.0);
  colorMap.addPoint(pqChartValue(0.17), QColor(  0,   0, 255 ), 0.17);
  colorMap.addPoint(pqChartValue(0.34), QColor(  0, 255, 255 ), 0.34);
  colorMap.addPoint(pqChartValue(0.50), QColor(  0, 255,   0 ), 0.50);
  colorMap.addPoint(pqChartValue(0.67), QColor(255, 255,   0 ), 0.67);
  colorMap.addPoint(pqChartValue(0.84), QColor(255,   0,   0 ), 0.84);
  colorMap.addPoint(pqChartValue(1.0),  QColor(224,   0, 255 ), 1.0);
  colorMap.setNanColor(QColor(255, 255, 0));
  model->addBuiltinColorMap(colorMap, "Rainbow Blended Grey");
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::choosePresetColorTable()
{
  this->Internal->Presets->setUsingCloseButton(false);
  this->Internal->ColorLookupTableModified = false;
  if(this->Internal->Presets->exec() == QDialog::Accepted)
    {
    // Get the color map from the selection.
    QItemSelectionModel *selection = this->Internal->Presets->getSelectionModel();
    QModelIndex index = selection->currentIndex();
    const pqColorMapModel *colorMap =
        this->Internal->Presets->getModel()->getColorMap(index.row());
    if(colorMap)
      {
      QModelIndex textIndex = index.sibling(index.row(), 0);
      this->Internal->CurrentDefaultColorLookupTable =
        textIndex.data(Qt::DisplayRole).toString();
      this->Internal->labelDefaultColorLUTName->setText(
        this->Internal->CurrentDefaultColorLookupTable);

      //QModelIndex lutIndex = index.sibling(index.row(), 0);
      //this->Internal->labelDefaultColorLUT->setPixmap(
      //  lutIndex.data(Qt::DecorationRole).value<QPixmap>());

      int colorSpace = colorMap->getColorSpaceAsInt();
      //this->Internal->labelDef
      QColor color;
      pqChartValue value, opacity;
      pqColorMapModel temp(*colorMap);
      // Update the displayed range.
      temp.getValueRange(value, opacity);
      bool singleScalar = (value.getDoubleValue()==opacity.getDoubleValue());
      int numPoints = colorMap->getNumberOfPoints();
      if(numPoints == 0)
        {
        return;
        }

      QList<QVariant> colors, opacities;
      if(numPoints > 0)
        {
        if(singleScalar)
          {
          if(numPoints>1)
            {
            temp.getPointColor(numPoints-1, color);
            colors << value.getDoubleValue() << color.redF()
              << color.greenF() << color.blueF();
            }
          temp.getPointColor(0, color);
          colors << value.getDoubleValue() << color.redF()
            << color.greenF() << color.blueF();
          }
        else
          {
          for(int i = 0; i < numPoints; i++)
            {
            temp.getPointColor(i, color);
            temp.getPointValue(i, value);
            colors << value.getDoubleValue() << color.redF()
              << color.greenF() << color.blueF();
            temp.getPointOpacity(i, opacity);
            opacities << value.getDoubleValue() << opacity.getDoubleValue()
              << 0.5 << 0.0;
            }
          }
        }

      // Set the property on the lookup table.
      int wrap = colorSpace == 2 ? 1 : 0;
      if(colorSpace >= 2)
        {
        colorSpace--;
        }

      vtkSMProxy *lookupTable = this->Internal->ColorTFProxy;
      pqSMAdaptor::setMultipleElementProperty(
        lookupTable->GetProperty("RGBPoints"), colors);
      lookupTable->UpdateVTKObjects();

      pqSMAdaptor::setElementProperty(
          lookupTable->GetProperty("ColorSpace"), colorSpace);
      pqSMAdaptor::setElementProperty(
          lookupTable->GetProperty("HSVWrap"), wrap);

      // Update the NaN color.
      QColor nanColor;
      colorMap->getNanColor(nanColor);
      // Set the property on the lookup table.
      QList<QVariant> values;
      values << nanColor.redF() << nanColor.greenF() << nanColor.blueF();
      pqSMAdaptor::setMultipleElementProperty(
                                  lookupTable->GetProperty("NanColor"), values);

      vtkSMProxy *opacityProxy = this->Internal->OpacityTFProxy;
      vtkSMDoubleVectorProperty* smProp = vtkSMDoubleVectorProperty::SafeDownCast(
        opacityProxy->GetProperty("Points"));
      pqSMAdaptor::setMultipleElementProperty(smProp, opacities);
      opacityProxy->UpdateVTKObjects();

      this->savePresetColorTableSettings();
      this->Internal->ColorLookupTableModified = true;

      emit this->changesAvailable();
      }
    }
}
//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::savePresetColorTableSettings()
{
  this->Internal->Presets->saveSettings();

  // Now copy over the settings saved to common cmb settings
  pqSettings* cmbSettings = this->cmbAppSettings();
  cmbSettings->beginGroup("ColorMapPresets");
  cmbSettings->remove("");
  pqSettings *settings = pqApplicationCore::instance()->settings();
  settings->beginGroup("ColorMapPresets");
  QStringList keys = settings->childKeys();
  for(QStringList::Iterator key = keys.begin(); key != keys.end(); ++key)
    {
    // Get the color map xml from the settings.
    QString text = settings->value(*key).toString();
    if(text.isEmpty())
      {
      continue;
      }
    cmbSettings->setValue(*key, QVariant(text));
    }
  cmbSettings->endGroup();
  settings->endGroup();
}

//-----------------------------------------------------------------------------
void qtCMBApplicationOptions::restorePresetColorTableSettings()
{
  // Now copy the common cmb settings, if it exists, to app settings
  pqSettings *settings = pqApplicationCore::instance()->settings();
  settings->beginGroup("ColorMapPresets");
  settings->remove("");
  pqSettings* cmbSettings = this->cmbAppSettings();
  cmbSettings->beginGroup("ColorMapPresets");
  QStringList keys = cmbSettings->childKeys();
  bool isCMBElevationFound = false;
  QString cmbElevationLUT("CMB General Elevation");
  for(QStringList::Iterator key = keys.begin(); key != keys.end(); ++key)
    {
    // Get the color map xml from the settings.
    QString text = cmbSettings->value(*key).toString();
    if(text.isEmpty())
      {
      continue;
      }
    if(text.contains(cmbElevationLUT))
      {
      isCMBElevationFound = true;
      }
    settings->setValue(*key, QVariant(text));
    }

  settings->endGroup();
  cmbSettings->endGroup();
/*
  // Default lookup table settings
  const char* defaultLutKey =
    pqPQLookupTableManager::DEFAULT_LOOKUPTABLE_SETTING_KEY();
  const char* defaultOpKey =
    pqPQLookupTableManager::DEFAULT_OPACITYFUNCTION_SETTING_KEY();
  if(cmbSettings->contains(defaultLutKey))
    {
    settings->setValue(defaultLutKey, cmbSettings->value(defaultLutKey));
    }
  if(cmbSettings->contains(defaultOpKey))
    {
    settings->setValue(defaultOpKey, cmbSettings->value(defaultOpKey));
    }
*/
  this->Internal->Presets->restoreSettings();

  // Bob's elevation color table
  if(!isCMBElevationFound)
    {
    pqColorMapModel colorMap;
    pqColorPresetModel *model = this->Internal->Presets->getModel();
    colorMap.removeAllPoints();
    colorMap.setColorSpace(pqColorMapModel::RgbSpace);
    colorMap.addPoint(pqChartValue(-1000.0),  QColor::fromRgbF( 0.0,  0.0, 0.498 ));
    colorMap.addPoint(pqChartValue(-100.0), QColor::fromRgbF(  0.0,   0.0, 1.0));
    colorMap.addPoint(pqChartValue(0.0), QColor::fromRgbF(  0.0, 1.0, 1.0 ));
    colorMap.addPoint(pqChartValue(1.0), QColor::fromRgbF(  0.333, 1.0,   0.0 ));
    colorMap.addPoint(pqChartValue(2.0), QColor::fromRgbF(0.1216, 0.3725,   0.0 ));
    colorMap.addPoint(pqChartValue(250.0), QColor::fromRgbF(1.0,   1.0,   0.0 ));
    colorMap.addPoint(pqChartValue(750.0),  QColor::fromRgbF(1.0,   0.333, 0.0 ));
    colorMap.setNanColor(QColor(255, 255, 0));
    model->addColorMap(colorMap, cmbElevationLUT);
    // Need a save setting here, so that Paraview's color table will get it.
    this->Internal->Presets->saveSettings();
    }

  if(cmbSettings->contains("/lookupTable/DefaultPresetLUT"))
    {
    this->Internal->CurrentDefaultColorLookupTable =
    cmbSettings->value("/lookupTable/DefaultPresetLUT").toString();
    this->Internal->labelDefaultColorLUTName->setText(
      this->Internal->CurrentDefaultColorLookupTable);
    }
}
