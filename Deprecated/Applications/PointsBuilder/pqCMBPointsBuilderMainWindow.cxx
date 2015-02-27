/*=========================================================================

  Program:   CMB
  Module:    pqCMBPointsBuilderMainWindow.cxx

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
#include "pqCMBPointsBuilderMainWindow.h"

#include "pqCMBPointsBuilderMainWindowCore.h"
#include "cmbLIDARConfig.h"
#include "qtCMBLIDARPanelWidget.h"
#include "pqCMBLIDARPieceObject.h"
#include "pqCMBLIDARPieceTable.h"
#include "pqCMBLIDARTerrainExtractionManager.h"
#include "ui_qtLIDARPanel.h"
#include "ui_qtCMBMainWindow.h"

#include <QDir>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QtDebug>
#include <QToolButton>
#include <QHeaderView>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>

#include "qtCMBAboutDialog.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqProxyWidget.h"
#include "pqDisplayColorWidget.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqPropertyLinks.h"
#include "pqRenderView.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqCMBRubberBandHelper.h"
#include "pqScalarBarRepresentation.h"

#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqWaitCursor.h"

#include "vtkCellData.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkConvertSelection.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkFacesConnectivityFilter.h"
#include "vtkHydroModelPolySource.h"
#include "vtkIdTypeArray.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkMapper.h"
#include "vtkMergeFacesFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkSelection.h"
#include "vtkSelectionSource.h"
#include "vtkSmartPointer.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVariant.h"
#include "qtCMBHelpDialog.h"

#include "cmbLoadDataReaction.h"
#include "cmbFileExtensions.h"

class pqCMBPointsBuilderMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/)
    {
    this->LastSelectedPort = NULL;
    }

  ~vtkInternal()
    {
    }
  pqOutputPort* LastSelectedPort;
  QPointer<QAction> LoadContourAction;
  QPointer<QAction> SaveContourAction;

};

//----------------------------------------------------------------------------
pqCMBPointsBuilderMainWindow::pqCMBPointsBuilderMainWindow():
  Internal(new vtkInternal(this))
{
  this->initializeApplication();
  this->setupZoomToBox();
  this->setupMenuActions();
  this->updateEnableState();
  this->initProjectManager();
  this->MainWindowCore->applyAppSettings();
}

//----------------------------------------------------------------------------
pqCMBPointsBuilderMainWindow::~pqCMBPointsBuilderMainWindow()
{
  delete this->Internal;
  delete this->MainWindowCore;
  this->MainWindowCore = NULL;
}

//----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::initializeApplication()
{
  this->MainWindowCore = new pqCMBPointsBuilderMainWindowCore(this);
  this->setWindowIcon( QIcon(QString::fromUtf8(":/cmb/PointsBuilderIcon.png")) );
  this->initMainWindowCore();

 // only Single selection is allowed currently
  this->MainWindowCore->activeRenderView()->
    setUseMultipleRepresentationSelection(false);
  QObject::connect(this->MainWindowCore->activeRenderView(),
                   SIGNAL(selected(pqOutputPort*)),
                   this, SLOT(onViewSelected(pqOutputPort*)));

  this->getThisCore()->setupControlPanel(
    this->getMainDialog()->dockWidgetContents_2->layout()->parentWidget());
  this->Superclass::addControlPanel(this->getThisCore()->getControlPanel());

  this->getThisCore()->setupProgressBar(
    this->getMainDialog()->statusbar);

  QObject::connect(this->getMainDialog()->action_Select,
    SIGNAL(triggered(bool)),
    this->getThisCore(), SLOT(onRubberBandSelect(bool)));

//  QObject::connect(this->getThisCore()->renderViewSelectionHelper(),
//    SIGNAL(selectionModeChanged(int)),
//    this, SLOT(onSelectionModeChanged(int)));

  QObject::connect(this->getThisCore(),
    SIGNAL(newDataLoaded()),  this, SLOT(onDataLoaded()));

  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  QObject::connect(this->getMainDialog()->actionConvert_from_Lat_Long,
    SIGNAL(triggered(bool)),
    this->getThisCore(), SLOT(onConvertFromLatLong(bool)));

  pqCMBPointsBuilderMainWindowCore* mainWinCore =
    qobject_cast<pqCMBPointsBuilderMainWindowCore*>(this->MainWindowCore);

  // TerrainExtractionManager setup during setupControlPanel
  QObject::connect(
    mainWinCore->getTerrainExtractionManager(), SIGNAL(enableMenuItems(bool)),
    this, SLOT(onEnableMenuItems(bool)));

  this->getMainDialog()->actionServerConnect->setEnabled(0);
  this->getMainDialog()->actionServerDisconnect->setEnabled(0);

  QObject::connect(this->getThisCore(),
    SIGNAL(openMoreFiles()),  this, SLOT(onOpenMoreFiles()));

  QString filters = cmbFileExtensions::PointsBuilder_FileTypes();
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(
    cmbFileExtensions::PointsBuilder_ReadersMap());
  this->loadDataReaction()->setMultiFiles(true);
}

//----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::setupMenuActions()
{
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_Select,
    this->getMainDialog()->actionConvert_from_Lat_Long);

  this->Internal->LoadContourAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->LoadContourAction->setObjectName(QString::fromUtf8("action_loadContour"));
  this->Internal->LoadContourAction->setText(QString::fromUtf8("Load Contours"));
  QObject::connect(this->Internal->LoadContourAction, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onLoadContour()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->LoadContourAction);

  this->Internal->SaveContourAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->SaveContourAction->setObjectName(QString::fromUtf8("action_saveContour"));
  this->Internal->SaveContourAction->setText(QString::fromUtf8("Save Contours"));
  QObject::connect(this->Internal->SaveContourAction, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onSaveContour()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->SaveContourAction);

  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);
}

//----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::updateEnableState()
{
  bool dataLoaded = this->getThisCore()->IsDataLoaded();
  this->getMainDialog()->action_Select->setEnabled(dataLoaded);
  this->getMainDialog()->actionConvert_from_Lat_Long->setEnabled(dataLoaded);
  this->Superclass::updateEnableState(dataLoaded);
}

//-----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(QApplication::translate("Points Builder AboutDialog",
                                               "About Points Builder",
                                               0, QApplication::UnicodeUTF8));
  dialog->setPixmap(QPixmap(QString(":/cmb/PointsBuilderSplashAbout.png")));
  dialog->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(LIDAR_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onHelpHelp()
{
this->showHelpPage("qthelp://paraview.org/cmbsuite/PointsBuilder_README.html");
}

//----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onViewSelected(pqOutputPort* selPort)
{
  this->Internal->LastSelectedPort = selPort;
}

//----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onEnableMenuItems(bool state)
{
  // File menu
  this->getMainDialog()->action_Open_File->setEnabled(state);
  this->getMainDialog()->action_Close->setEnabled(state);
  this->getMainDialog()->action_Save_Data->setEnabled(state);
  this->getMainDialog()->action_Save_As->setEnabled(state);
  this->getMainDialog()->menuRecentFiles->setEnabled(state);

  // Edit menu
  this->getMainDialog()->action_Select->setEnabled(state);

  // View menu
  this->getMainDialog()->actionConvert_from_Lat_Long->setEnabled(state);
}

//-----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::updateSelection()
{
  this->getThisCore()->updateSelection(this->Internal->LastSelectedPort);
}

//-----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onDataLoaded()
{
  this->updateEnableState();
  this->getMainDialog()->actionConvert_from_Lat_Long->blockSignals(true);
  this->getMainDialog()->actionConvert_from_Lat_Long->setChecked(0);
  this->getMainDialog()->actionConvert_from_Lat_Long->blockSignals(false);
  bool dataLoaded = this->getThisCore()->IsDataLoaded();
  if(dataLoaded)
    {
    QFileInfo fInfo(this->getThisCore()->getLIDARFileTitle());
    this->appendDatasetNameToTitle(fInfo.fileName());
    }
  else
    {
    this->appendDatasetNameToTitle("");
    }
}

//-----------------------------------------------------------------------------
void pqCMBPointsBuilderMainWindow::onOpenMoreFiles()
{
  this->getThisCore()->onOpenMoreFiles(this->loadDataReaction());
}

//-----------------------------------------------------------------------------
pqCMBPointsBuilderMainWindowCore* pqCMBPointsBuilderMainWindow::getThisCore()
{
  return qobject_cast<pqCMBPointsBuilderMainWindowCore*>(this->MainWindowCore);
}
