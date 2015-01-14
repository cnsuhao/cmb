/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelBuilderMainWindow.cxx

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
#include "pqCMBModelBuilderMainWindow.h"
//#include "ui_qtCMBMainWindow.h"

#include "pqCMBModelBuilderMainWindowCore.h"
#include "pqCMBRubberBandHelper.h"
#include "ui_qtCMBMainWindow.h"
#include "pqProxyInformationWidget.h"

#include "cmbModelBuilderConfig.h"

#include <QDir>
#include <QScrollArea>
#include <QShortcut>
#include <QtDebug>
#include <QToolButton>

#include <QMenu>
#include <QDoubleSpinBox>
#include <QDropEvent>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPalette>
#include <QPixmap>
#include <QSplitter>
#include <QSettings>

#include "qtCMBAboutDialog.h"
#include "pqApplicationCore.h"
#include "pqColorChooserButton.h"
#include "pqDisplayColorWidget.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqPropertyLinks.h"
#include "pqRenderView.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqScalarBarRepresentation.h"
#include "pqScalarsToColors.h"
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
#include "vtkMapper.h"
#include "vtkMergeFacesFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkPVMultiBlockRootObjectInfo.h"
#include "vtkSelection.h"
#include "vtkSelectionSource.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVariant.h"
#include "vtkCollection.h"

#include "qtCMBCreateSimpleGeometry.h"
#include "qtCMBHelpDialog.h"

#include "SimBuilder/SimBuilderCore.h"
#include "SimBuilder/smtkUIManager.h"

#include "qtCMBBathymetryDialog.h"
#include "pqCMBSceneObjectBase.h"
#include "pqPlanarTextureRegistrationDialog.h"

#include "cmbLoadDataReaction.h"
#include "vtkSMSessionProxyManager.h"
#include "cmbFileExtensions.h"
// panels requied includes
#include "pqActiveObjects.h"
#include "pqProxyWidget.h"
#include "SimBuilder/SimBuilderUIPanel.h"
#include "pqCMBSceneTree.h"
#include <QDockWidget>
#include "qtSMTKModelPanel.h"
#include "ModelManager.h"
#include "pqCMBColorMapWidget.h"
#include "qtCMBTreeWidget.h"
#include "vtkSMModelManagerProxy.h"

#include <vtksys/SystemTools.hxx>
#include "smtk/model/StringData.h"

class pqCMBModelBuilderMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/)
    {
    this->SceneGeoTree = 0;
    }

  ~vtkInternal()
    {
    if(this->SceneGeoTree)
      {
      delete this->SceneGeoTree;
      }
    }

  QPointer<pqProxyInformationWidget> InformationWidget;

  QPointer<pqColorChooserButton> ColorButton;
  QPointer<QDoubleSpinBox> GrowAngleBox;

  QPointer<QSettings> SplitterSettings;
  QPointer<QToolBar> GrowToolbar;
  QPointer<QToolBar> Model2DToolbar;
  QPointer<QToolBar> VariableToolbar;

  QPointer<QAction> LoadScenarioAction;
  QPointer<QAction> SaveScenarioAction;
  QPointer<QAction> SaveMeshInfoAction;
  QPointer<QAction> LoadMeshInfoAction;
  QPointer<QAction> CreateSimpleModelAction;
  QPointer<QAction> CreateModelEdgesAction;

  QPointer<QMenu> NewModelBridgeMenu;

  pqPropertyLinks LineResolutionLinks;
  pqCMBSceneTree* SceneGeoTree;

  // Texture related
  QStringList TextureFiles;
  QPointer<QAction> ChangeTextureAction;
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*> CurrentDockWidgets;
};

//----------------------------------------------------------------------------
pqCMBModelBuilderMainWindow::pqCMBModelBuilderMainWindow():
Internal(new vtkInternal(this))
{
  this->initializeApplication();
  this->initUIPanels();

  this->setupToolbars();
  this->setupZoomToBox();
  this->setupMenuActions();
  this->updateEnableState();
  this->initProjectManager();
  this->MainWindowCore->applyAppSettings();
}

//----------------------------------------------------------------------------
pqCMBModelBuilderMainWindow::~pqCMBModelBuilderMainWindow()
{
  delete this->MainWindowCore;
  delete this->Internal;
  this->MainWindowCore = NULL;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::initializeApplication()
{
  this->MainWindowCore = new pqCMBModelBuilderMainWindowCore(this);
  this->setWindowIcon( QIcon(QString::fromUtf8(":/cmb/ModelBuilderIcon.png")) );
  this->initMainWindowCore();

  this->MainWindowCore->setupProcessBar(this->statusBar());
  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  QObject::connect(this->getMainDialog()->actionLoad_Simulation_Template,
    SIGNAL(triggered()), this, SLOT(loadSimulationTemplate()));
  QObject::connect(this->getMainDialog()->actionLoad_Simulation,
    SIGNAL(triggered()), this, SLOT(loadSimulation()));
  QObject::connect(this->getMainDialog()->actionSave_Simulation,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onSaveSimulation()));
  QObject::connect(this->getMainDialog()->actionExport_Simulation_File,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onExportSimFile()));

  QObject::connect(this->getMainDialog()->action_Generate_Omicron_Input,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onGenerateOmicronInput()));

  QObject::connect(this->getMainDialog()->action_Save_BCSs,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onSaveBCSs()));

  QObject::connect(this->getMainDialog()->actionSpawn_Surface_Mesher,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onSpawnSurfaceMesher()));

  QObject::connect(this->getMainDialog()->actionSpawn_Volume_Mesher,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onSpawnVolumeMesher()));

//  QObject::connect(this->getMainDialog()->actionConvert_from_Lat_Long,
//    SIGNAL(triggered(bool)),
//    this, SLOT(onConvertLatLong(bool)));
  QObject::connect(this->getMainDialog()->action_Select,
    SIGNAL(triggered(bool)),
    this, SLOT(onSurfaceRubberBandSelect(bool)));

  QObject::connect(this->getMainDialog()->actionGrow_Cell,
    SIGNAL(triggered(bool)),
    this, SLOT(onGrowFromCell(bool)));
  QObject::connect(this->getMainDialog()->actionGrowPlus,
    SIGNAL(triggered(bool)),
    this, SLOT(onGrowAndMerge(bool)));
  QObject::connect(this->getMainDialog()->actionGrowMinus,
    SIGNAL(triggered(bool)),
    this, SLOT(onGrowAndRemove(bool)));
  QObject::connect(this->getMainDialog()->actionGrow_Clear,
    SIGNAL(triggered()),this, SLOT(onClearGrowResult()));
  QObject::connect(this->getMainDialog()->actionGrow_Accept,
    SIGNAL(triggered()),this, SLOT(onAcceptGrowFacets()));

  QObject::connect(this->getThisCore()->cmbRenderViewSelectionHelper(),
    SIGNAL(selectionFinished(int, int, int, int)),
    this, SLOT(onSelectionFinished()));

  QObject::connect(this->getThisCore(),
    SIGNAL(rubberSelectionModeChanged()),
    this, SLOT(onClearSelection()));

  QObject::connect(this->getThisCore(),
    SIGNAL(newModelCreated()),
    this, SLOT(onNewModelCreated()));

  this->initSimBuilder();

  this->getThisCore()->setupSelectionRepresentationToolbar(
    this->getMainDialog()->toolBar_Selection);

  this->Internal->SplitterSettings = new QSettings();

  QObject::connect(this->getThisCore(),
    SIGNAL(newSceneLoaded()), this, SLOT(onSceneFileLoaded()));
  QObject::connect(this->getMainDialog()->action_MB_Load_Scene,
    SIGNAL(triggered()), this, SLOT(onLoadScene()));
  QObject::connect(this->getMainDialog()->action_MB_Unload_Scene,
    SIGNAL(triggered()), this, SLOT(onUnloadScene()));

//  QObject::connect(this->getMainDialog()->actionApplyBathymetry,
//    SIGNAL(triggered()), this, SLOT(onDisplayBathymetryDialog()));

 this->getMainDialog()->faceParametersDock->setParent(0);
  this->getMainDialog()->faceParametersDock->setVisible(false);

  //this->getMainDialog()->actionServerConnect->setEnabled(0);
  //this->getMainDialog()->actionServerDisconnect->setEnabled(0);
  QString filters = cmbFileExtensions::ModelBuilder_FileTypes();
  std::set<std::string> modelFileTypes = this->getThisCore()->modelManager()->supportedFileTypes();
  QStringList modelFileExts;
  for (std::set<std::string>::iterator it = modelFileTypes.begin(); it != modelFileTypes.end(); ++it)
    {
    modelFileExts <<  (*it).c_str();
    }

  this->loadDataReaction()->addSpecialExtensions(modelFileExts);
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(
    cmbFileExtensions::ModelBuilder_ReadersMap());
  QObject::connect(this->loadDataReaction(), SIGNAL(filesSelected(const QStringList&)),
      this->getThisCore(), SLOT(onFileOpen(const QStringList&)));

  // Add "New Bridge Action", which will show all available bridges
  this->Internal->NewModelBridgeMenu = new QMenu(this->getMainDialog()->menu_File);
  this->Internal->NewModelBridgeMenu->setObjectName(QString::fromUtf8("menu_newbridge"));
  this->Internal->NewModelBridgeMenu->setTitle(QString::fromUtf8("New Session..."));
  this->getMainDialog()->menu_File->insertMenu(
    this->getMainDialog()->action_Open_File,
    this->Internal->NewModelBridgeMenu);

  // adding bridges to the "New Session..." menu
  smtk::model::StringList newBnames = this->getThisCore()->modelManager()->managerProxy()->bridgeNames();
  for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
    {
    this->addNewBridge((*it).c_str());
    }

  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(newBridgeLoaded(const QStringList&)),
      this, SLOT(addNewBridges(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(newFileTypesAdded(const QStringList&)),
      this->loadDataReaction(), SLOT(addSpecialExtensions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(currentModelCleared()),
      this, SLOT(onCMBModelCleared()));
  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(representationChanged(pqDataRepresentation*)),
    this, SLOT(onActiveRepresentationChanged(pqDataRepresentation*)));
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::SetCheckBoxStateQuiet(QCheckBox* box, bool state)
{
  // Modify Qcheckbox status without emit signal
  box->blockSignals(true);
  box->setChecked(state);
  box->blockSignals(false);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::addNewBridge(const QString& brname)
{
  QAction* act = this->Internal->NewModelBridgeMenu->addAction(brname);
  QObject::connect(act, SIGNAL(triggered()), this, SLOT(onCreateNewBridge()));
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::addNewBridges(const QStringList& brnames)
{
  foreach(QString brname, brnames)
    {
    this->addNewBridge(brname);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onCreateNewBridge()
{
  QAction* const action = qobject_cast<QAction*>(
    QObject::sender());
  if(!action)
    {
    return;
    }
  std::string brName = action->text().toStdString();
  bool started = this->getThisCore()->modelManager()->startSession(brName);
  if(started)
    {
    this->onNewModelCreated();
    this->getThisCore()->modelPanel()->onDataUpdated();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onViewChanged()
{
  if(this->MainWindowCore && this->MainWindowCore->activeRenderView())
    {
    this->Superclass::onViewChanged();
    // The immediateMode is turned on for the new model mapper,
    // because the mapper has to cache its own display list for individual
    // model entity display property, and update its display list when and only
    // when necessary (such as some model entity is turned off).
//    pqServer::setGlobalImmediateModeRenderingSetting(true);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onLoadScene()
{
  this->getThisCore()->setpqCMBSceneTree(this->getpqCMBSceneTree());
  this->getThisCore()->onLoadScene();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onUnloadScene()
{
  this->getThisCore()->clearpqCMBSceneTree();
  this->getThisCore()->activeRenderView()->resetCamera();
  this->getThisCore()->activeRenderView()->render();

  this->updateEnableState();
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setupToolbars()
{
/*
  // Variable toolbar
  this->Internal->VariableToolbar = new QToolBar("Active Variable Controls", this);
  this->Internal->VariableToolbar->setObjectName("variableToolbar");

  this->getThisCore()->setupVariableToolbar(this->Internal->VariableToolbar);
  //add the color button to the variable toolbar
  this->Internal->ColorButton = new pqColorChooserButton(
    this->Internal->VariableToolbar)<< pqSetName("displayColorButton");
  this->Internal->ColorButton->setToolTip("Set color on selected objects");
  this->Internal->ColorButton->setChosenColor(Qt::white);
  this->Internal->ColorButton->setIconRadiusHeightRatio(0.5);
  this->Internal->VariableToolbar->addWidget(this->Internal->ColorButton);

  QObject::connect(this->Internal->ColorButton,
                   SIGNAL(validColorChosen(const QColor&)),
                   this,
                   SLOT(setSolidColorOnSelections(const QColor&)));

  this->insertToolBar(this->getMainDialog()->toolBar_Selection,this->Internal->VariableToolbar);

  // Grow toolbar
  this->Internal->GrowToolbar = new QToolBar("Grow Operations", this);
  this->Internal->GrowToolbar->setObjectName("growPick_Toolbar");
  this->Internal->GrowToolbar->addAction(
    this->getMainDialog()->actionGrow_Cell);
  this->Internal->GrowToolbar->addAction(
    this->getMainDialog()->actionGrowPlus);
  this->Internal->GrowToolbar->addAction(
    this->getMainDialog()->actionGrowMinus);
  this->Internal->GrowToolbar->addAction(
    this->getMainDialog()->actionGrow_Clear);
  this->Internal->GrowToolbar->addAction(
    this->getMainDialog()->actionGrow_Accept);
//  this->Internal->GrowToolbar->addAction(
//    this->getMainDialog()->actionGrow_AcceptNG);
  this->Internal->GrowToolbar->insertSeparator(
    this->getMainDialog()->actionGrow_Clear);

  this->insertToolBar(this->Internal->VariableToolbar,this->Internal->GrowToolbar);
*/
  this->Internal->Model2DToolbar = NULL;

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setupMenuActions()
{
    // Add actions to "File" menu.
  // Scene File actions
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->action_MB_Load_Scene);
  this->getMainDialog()->action_MB_Load_Scene->setEnabled(true);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->action_MB_Unload_Scene);

  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);

  // SimBuilder file actions
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->actionLoad_Simulation_Template);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->actionLoad_Simulation);
  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);

  this->Internal->LoadScenarioAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->LoadScenarioAction->setObjectName(QString::fromUtf8("action_loadScenario"));
  this->Internal->LoadScenarioAction->setText(QString::fromUtf8("Load Simulation Scenario"));
  QObject::connect(this->Internal->LoadScenarioAction, SIGNAL(triggered()),
    this, SLOT(loadSimulationScenario()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->LoadScenarioAction);

  this->Internal->SaveScenarioAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->SaveScenarioAction->setObjectName(QString::fromUtf8("action_saveScenario"));
  this->Internal->SaveScenarioAction->setText(QString::fromUtf8("Save Simulation Scenario"));
  QObject::connect(this->Internal->SaveScenarioAction, SIGNAL(triggered()),
    this, SLOT(saveSimulationScenario()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->SaveScenarioAction);

  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);


  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->actionSave_Simulation);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->actionExport_Simulation_File);
  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);

  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->action_Save_BCSs);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->getMainDialog()->action_Generate_Omicron_Input);
  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);

  // Load and Save Analysis Mesh Info Actions
  this->Internal->LoadMeshInfoAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->LoadMeshInfoAction->setObjectName(QString::fromUtf8("action_loadmeshinfo"));
  this->Internal->LoadMeshInfoAction->setText(QString::fromUtf8("Load Analysis Mesh Info"));
  QObject::connect(this->Internal->LoadMeshInfoAction, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onLoadMeshToModelInfo()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->LoadMeshInfoAction);
  this->Internal->SaveMeshInfoAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->SaveMeshInfoAction->setObjectName(QString::fromUtf8("action_savemeshinfo"));
  this->Internal->SaveMeshInfoAction->setText(QString::fromUtf8("Save Analysis Mesh Info"));
  QObject::connect(this->Internal->SaveMeshInfoAction, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onSaveMeshToModelInfo()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->SaveMeshInfoAction);

  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);
/*
  // Create Simple Model Action
  this->Internal->CreateSimpleModelAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->CreateSimpleModelAction->setObjectName(QString::fromUtf8("action_createsimplemodel"));
  this->Internal->CreateSimpleModelAction->setText(QString::fromUtf8("Create Simple Model"));
  QObject::connect(this->Internal->CreateSimpleModelAction, SIGNAL(triggered()),
    this, SLOT(onCreateSimpleModel()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit,
    this->Internal->CreateSimpleModelAction);

  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->action_Exit);
*/
  // Add actions to "Edit" menu.
//  this->getMainDialog()->menuEdit->insertAction(
//    this->getMainDialog()->action_Select,
//    this->getMainDialog()->actionConvert_from_Lat_Long);
//  this->getMainDialog()->menuEdit->insertAction(
//    this->getMainDialog()->action_Select,
//    this->getMainDialog()->actionApplyBathymetry);
//  this->getMainDialog()->menuEdit->insertSeparator(
//    this->getMainDialog()->action_Select);
/*
  this->getMainDialog()->menuEdit->addActions(
    this->Internal->GrowToolbar->actions());
  this->getMainDialog()->menuEdit->insertSeparator(
    this->Internal->GrowToolbar->actions().value(0));

  this->Internal->CreateModelEdgesAction = new QAction(this->getMainDialog()->menuEdit);
  this->Internal->CreateModelEdgesAction->setObjectName(QString::fromUtf8("action_createModelEdges"));
  this->Internal->CreateModelEdgesAction->setText(QString::fromUtf8("Create Model Edges"));
//  QObject::connect(this->Internal->CreateModelEdgesAction, SIGNAL(triggered()),
//    this, SLOT(createModelEdges()));
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->actionConvert_from_Lat_Long,
    this->Internal->CreateModelEdgesAction);
  this->getMainDialog()->menuEdit->insertSeparator(
    this->getMainDialog()->actionConvert_from_Lat_Long);
  this->Internal->CreateModelEdgesAction->setEnabled(false);
*/

/*
  this->Internal->ChangeTextureAction = new QAction(this->getMainDialog()->menuEdit);
  this->Internal->ChangeTextureAction->setObjectName(QString::fromUtf8("action_editTexture"));
  this->Internal->ChangeTextureAction->setText(QString::fromUtf8("Edit Texture Map"));
  QObject::connect(this->Internal->ChangeTextureAction, SIGNAL(triggered()),
    this, SLOT(editTexture()));
  this->getMainDialog()->menuEdit->insertAction(
    this->Internal->CreateModelEdgesAction,
    this->Internal->ChangeTextureAction);
  this->Internal->ChangeTextureAction->setEnabled(false);

  // add a spin box for the Grow-Angle in the grow toolbar
  this->Internal->GrowAngleBox = new QDoubleSpinBox(this->Internal->GrowToolbar);
  this->Internal->GrowAngleBox->setMinimum(0.0);
  this->Internal->GrowAngleBox->setMaximum(180.0);
  this->Internal->GrowAngleBox->setValue(30.0);
  this->Internal->GrowAngleBox->setObjectName("growAngleBox");
  this->Internal->GrowAngleBox->setToolTip("Feature angle for grow selection");
  this->Internal->GrowToolbar->addWidget(this->Internal->GrowAngleBox);
*/
  // Add actions to "Tools" menu.
  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionSpawn_Volume_Mesher);
  this->getMainDialog()->menu_Tools->insertSeparator(
    this->getMainDialog()->actionLock_View_Size);

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateEnableState()
{
  bool data_loaded = false; /*(this->getThisCore()->getCMBModel()->
    GetCurrentModelEntityMap().keys().count() > 0) ? true : false;*/

  bool model_loaded = this->getThisCore()->modelManager()
    ->numberOfModels() > 0;
  this->Superclass::updateEnableState(model_loaded);

  this->getMainDialog()->action_Save_BCSs->setEnabled(model_loaded);
  this->getMainDialog()->action_Select->setEnabled(model_loaded);
  this->getMainDialog()->actionGrow_Cell->setEnabled(data_loaded);
  this->getMainDialog()->actionGrowPlus->setEnabled(data_loaded);
  this->getMainDialog()->actionGrowPlus->setEnabled(data_loaded);
  this->getMainDialog()->action_Select->setChecked(false);
  this->getMainDialog()->actionConvert_from_Lat_Long->setChecked(false);
  this->getMainDialog()->actionConvert_from_Lat_Long->setEnabled(model_loaded);
  this->setGrowButtonsState(false);

//  this->getMainDialog()->actionConvert_from_Lat_Long->setEnabled(model_loaded);
//  this->getMainDialog()->actionApplyBathymetry->setEnabled(model_loaded);
//  this->Internal->ChangeTextureAction->setEnabled(model_loaded);

  //this->Internal->ColorWidget->setEnabled(data_loaded);
  //this->Internal->RepresentationWidget->setEnabled(data_loaded);
//  this->setToolbarEnableState(this->Internal->VariableToolbar, model_loaded);
//  this->setToolbarEnableState(this->Internal->GrowToolbar, data_loaded);

  //this->Internal->VariableToolbar->setEnabled(data_loaded);
  //this->Internal->GrowToolbar->setEnabled(data_loaded);

  // The state could be changed in updateUIByDimension()
  this->getMainDialog()->action_Generate_Omicron_Input->setEnabled( model_loaded );
  this->getMainDialog()->actionSpawn_Volume_Mesher->setEnabled( model_loaded );

  // always disabled for now
  this->getMainDialog()->actionSpawn_Surface_Mesher->setEnabled( false );

  this->Internal->SaveMeshInfoAction->setEnabled(model_loaded);
  this->Internal->LoadMeshInfoAction->setEnabled(model_loaded);

  // this->getMainDialog()->action_MB_Load_Scene->setEnabled(data_loaded);
  this->getMainDialog()->action_MB_Unload_Scene->setEnabled(
    this->Internal->SceneGeoTree &&
    !this->Internal->SceneGeoTree->isEmpty());

  // SimBuilder related UI

  // if there is a SimBuilder model loaded, the left panel needs to be enabled.
  bool isSimLoaded = this->getThisCore()->getSimBuilder()->isSimModelLoaded();
  this->getMainDialog()->actionSave_Simulation->setEnabled(isSimLoaded);
  this->Internal->SaveScenarioAction->setEnabled(isSimLoaded);
  if(isSimLoaded)
    {
    this->getMainDialog()->faceParametersDock->setEnabled(true);
    this->getMainDialog()->action_Close->setEnabled(true);
    }

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateUIByDimension()
{
  bool dim2D = false; /* this->getThisCore()->getCMBModel()->getModelDimension() == 2 ?
    true : false; */
  bool has2dEdges = false; //this->getThisCore()->getCMBModel()->has2DEdges();
  if(dim2D)
    {
    this->getMainDialog()->actionSpawn_Volume_Mesher->setEnabled(0);
    this->getMainDialog()->action_Generate_Omicron_Input->setEnabled( 0 );
//    this->Internal->CreateModelEdgesAction->setEnabled(false);
    }
  else
    {
    this->getMainDialog()->actionSpawn_Volume_Mesher->setEnabled(1);
    this->getMainDialog()->action_Generate_Omicron_Input->setEnabled( 1 );
//    this->Internal->CreateModelEdgesAction->setEnabled(!has2dEdges);
    }

  if(has2dEdges)
    {
    if(!this->Internal->Model2DToolbar)
      {
      this->Internal->Model2DToolbar = new QToolBar("Model 2D Operations", this);
      this->Internal->Model2DToolbar->setObjectName("model2D_Toolbar");
      this->Internal->Model2DToolbar->addAction(
        this->getMainDialog()->action_SelectPoints);

      QObject::connect(this->getMainDialog()->action_SelectPoints,
        SIGNAL(triggered(bool)),
        this, SLOT(onConvertArcNodes(bool)));

//      this->insertToolBar(this->Internal->VariableToolbar,this->Internal->Model2DToolbar);
      }
    else
      {
      this->Internal->Model2DToolbar->setVisible(1);
      }
    }
  else
    {
    if(this->Internal->Model2DToolbar)
      {
      this->Internal->Model2DToolbar->setVisible(0);
      }
    }

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onCMBModelModified()
{
  this->UpdateInfoTable();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onCMBModelCleared()
{
  this->clearGUI();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::UpdateInfoTable()
{
  this->updateDataInfo();
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateDataInfo()
{
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::clearSelectedPorts()
{
  for(int i=0; i<this->getLastSelectionPorts().count(); i++)
    {
    pqOutputPort* selPort = this->getLastSelectionPorts().value(i);
    pqPipelineSource *source = selPort? selPort->getSource() : NULL;
    if(!source)
      {
      continue;
      }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
    if(!smSource)
      {
      continue;
      }
    smSource->SetSelectionInput(0, NULL, 0);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setupZoomToBox()
{
  QObject::connect(this->getThisCore()->cmbRenderViewSelectionHelper(),
    SIGNAL(selectionModeChanged(int)),
    this, SLOT(onZoomModeChanged(int)));
  QObject::connect(
    this->getThisCore()->cmbRenderViewSelectionHelper(),
    SIGNAL(enableZoom(bool)),
    this->getMainDialog()->actionZoomToBox, SLOT(setEnabled(bool)));
  QObject::connect(
    this->getMainDialog()->actionZoomToBox, SIGNAL(triggered()),
    this->getThisCore()->cmbRenderViewSelectionHelper(), SLOT(beginZoom()));
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onZoomModeChanged(int mode)
{
  this->getMainDialog()->actionZoomToBox->setChecked(
    mode==pqCMBRubberBandHelper::ZOOM);
}

//----------------------------------------------------------------------------
// Since render view always creates an id based selection, the role of this
// method is to convert it to a "region id" selection so that we select faces.
void pqCMBModelBuilderMainWindow::updateCMBSelection()
{
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateCellGrowSelection()
{
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::mergeCellGrowSelection()
{
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::removeCellGrowSelection()
{
}

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindow::multipleCellsSelected()
{
  int numSel = this->getLastSelectionPorts().count();
  if(numSel>1)
    {
    int numSelCells = 0;
    for(int i=0; i<numSel; i++)
      {
      pqOutputPort* selPort = this->getLastSelectionPorts().value(i);
      numSelCells += this->getNumberOfSelectedCells(selPort);
      if(numSelCells > 1)
        {
        return true;
        }
      }
    }
  else if(numSel==1)
    {
    return (this->getNumberOfSelectedCells(this->getLastSelectionPorts()[0])>1);
    }

  return false;
}

//----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindow::getNumberOfSelectedCells(pqOutputPort* selPort)
{
  pqPipelineSource *source = selPort? selPort->getSource() : NULL;
  if(!source)
    {
    return 0;
    }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  if(!smSource || !smSource->GetSelectionInput(0))
    {
    return 0;
    }

  vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
  vtkSMVectorProperty* vp = vtkSMVectorProperty::SafeDownCast(
    selSource->GetProperty("IDs"));
  QList<QVariant> ids = pqSMAdaptor::getMultipleElementProperty(vp);
  int numElemsPerCommand = vp->GetNumberOfElementsPerCommand();
  return (numElemsPerCommand>0) ? (ids.size()/numElemsPerCommand) : 0;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::selectModelEntitiesByMode(
  QList<vtkIdType>& faces, int clearSelFirst)
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(QApplication::translate("Model Builder AboutDialog",
                                               "About Model Builder",
                                               0, QApplication::UnicodeUTF8));
  dialog->setPixmap(QPixmap(QString(":/cmb/ModelBuilderSplashAbout.png")));
  dialog->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(CMB_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onHelpHelp()
{

  this->showHelpPage("qthelp://paraview.org/cmbsuite/ModelBuilder_README.html");
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::clearGUI()
{
  // clear the new panels
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*>::iterator it;
  for(it = this->Internal->CurrentDockWidgets.begin();
    it != this->Internal->CurrentDockWidgets.end(); ++it)
    {
    if(it.value())
      {
//      it.value()->setWidget(NULL);
      this->removeDockWidget(it.value());
      it.value()->setParent(0);
//      delete it.value();
      }
    }
  pqApplicationCore::instance()->unRegisterManager(
    "COLOR_EDITOR_PANEL");
  this->Internal->CurrentDockWidgets.clear();
  this->updateEnableState();
  this->appendDatasetNameToTitle("");
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSaveState()
{
  this->UpdateModelState(1);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onResetState()
{
  this->UpdateModelState(0);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::UpdateModelState(int accepted)
{
  pqWaitCursor cursor;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateSelectionUI(bool disable)
{
  if(disable)
    {
    if(this->getMainDialog()->action_Select->isChecked())
      {
      this->getMainDialog()->action_Select->setChecked(false);
      }
    }
  else
    {
    }
  if(this->getMainDialog()->actionZoomToBox->isChecked())
    {
    this->getMainDialog()->actionZoomToBox->setChecked(false);
    }
  this->getMainDialog()->actionZoomToBox->setEnabled(!disable);
  this->getMainDialog()->toolBar_Selection->setEnabled(!disable);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onClearSelection()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setSolidColorOnSelections(
  const QColor& setColor)
{
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSelectionFinished()
{
  if(this->getMainDialog()->action_Select->isChecked())
    {
    this->getThisCore()->cmbRenderViewSelectionHelper()->endSelection();
    //this->updateCMBSelection();
    this->updateSMTKSelection();
    this->getMainDialog()->action_Select->setChecked(false);
    }
  else if(this->getMainDialog()->actionZoomToBox->isChecked())
    {
    this->getMainDialog()->actionZoomToBox->setChecked(0);
    this->getThisCore()->cmbRenderViewSelectionHelper()->endZoom();
    }
  else
    {
    if(this->getMainDialog()->actionGrow_Cell->isChecked())
      {
      this->updateCellGrowSelection();
      //this->getMainDialog()->actionGrow_Cell->setChecked(false);
      }
    else if(this->getMainDialog()->actionGrowPlus->isChecked())
      {
      this->mergeCellGrowSelection();
      //this->getMainDialog()->actionGrowPlus->setChecked(false);
      }
    else if(this->getMainDialog()->actionGrowMinus->isChecked())
      {
      this->removeCellGrowSelection();
      //this->getMainDialog()->actionGrowMinus->setChecked(false);
      }
    this->clearSelectedPorts();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSurfaceRubberBandSelect(bool checked)
{
  if(checked)
    {
    this->setGrowButtonsState(false);
    }
  this->getThisCore()->onRubberBandSelect(checked);
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onConvertLatLong(bool checked)
{
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::growFinished()
{
  this->updateGrowGUI(false);
  if(!this->getMainDialog()->action_Select->isChecked())
    {
    this->getThisCore()->onRubberBandSelectCells(false);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onGrowFromCell(bool checked)
{
  if(checked)
   {
   //this->getMainDialog()->actionGrow_Cell->setChecked(false);
   this->getMainDialog()->actionGrowPlus->setChecked(false);
   this->getMainDialog()->actionGrowMinus->setChecked(false);
   this->updateGrowGUI(true);
   }
  this->getThisCore()->onRubberBandSelectCells(checked);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onConvertArcNodes(bool checked)
{
  this->updateSelectionUI(checked);
  this->onClearSelection();
  this->getThisCore()->onRubberBandSelectPoints(checked);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onGrowAndMerge(bool checked)
{
  if(checked)
   {
   this->getMainDialog()->actionGrow_Cell->setChecked(false);
   //this->getMainDialog()->actionGrowPlus->setChecked(false);
   this->getMainDialog()->actionGrowMinus->setChecked(false);
   this->updateGrowGUI(checked);
   }
  this->getThisCore()->onRubberBandSelectCells(checked);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onGrowAndRemove(bool checked)
{
  if(checked)
   {
   this->getMainDialog()->actionGrow_Cell->setChecked(false);
   this->getMainDialog()->actionGrowPlus->setChecked(false);
   //this->getMainDialog()->actionGrowMinus->setChecked(false);
   this->updateGrowGUI(checked);
   }
  this->getThisCore()->onRubberBandSelectCells(checked);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onAcceptGrowFacets()
{
  this->growFinished();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onClearGrowResult()
{
  this->growFinished();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onEnableExternalProcesses(bool state)
{
//  this->getMainDialog()->actionSpawn_Surface_Mesher->setEnabled(state);
  this->getMainDialog()->actionSpawn_Volume_Mesher->setEnabled(state);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateGrowGUI(bool doGrow)
{
  this->updateSelectionUI(doGrow);
  if(doGrow)
    {
    this->onClearSelection();
    }
  else
    {
    this->setGrowButtonsState(doGrow);
    }
  this->getMainDialog()->actionGrow_Clear->setEnabled(doGrow);
  this->getMainDialog()->actionGrow_Accept->setEnabled(doGrow);
//  this->getMainDialog()->actionGrow_AcceptNG->setEnabled(doGrow);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setGrowButtonsState(bool checked)
{
  if(this->getMainDialog()->actionGrow_Cell->isChecked() != checked)
    {
    this->getMainDialog()->actionGrow_Cell->setChecked(checked);
    }
  if(this->getMainDialog()->actionGrowPlus->isChecked() != checked)
    {
    this->getMainDialog()->actionGrowPlus->setChecked(checked);
    }
  if(this->getMainDialog()->actionGrowMinus->isChecked() != checked)
    {
    this->getMainDialog()->actionGrowMinus->setChecked(checked);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onShowCenterAxisChanged(bool enabled)
{
  this->getMainDialog()->actionShowCenterAxes->setEnabled(enabled);
  this->getMainDialog()->actionShowCenterAxes->blockSignals(true);
  pqRenderView* renView = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  this->getMainDialog()->actionShowCenterAxes->setChecked(
    renView ? renView->getCenterAxesVisibility() : false);
  this->getMainDialog()->actionShowCenterAxes->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setToolbarEnableState(QToolBar* toolbar, bool enabled)
{
  for(int i=0; i<toolbar->actions().count(); i++)
    {
    toolbar->actions().value(i)->setEnabled(enabled);
    }
  toolbar->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
pqCMBModelBuilderMainWindowCore* pqCMBModelBuilderMainWindow::getThisCore()
{
  return qobject_cast<pqCMBModelBuilderMainWindowCore*>(this->MainWindowCore);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::initSimBuilder()
{
  //this->getThisCore()->getSimBuilder()->getMeshManager()->setUIPanel(
  //  this->Internal->GUIPanel);
  //QObject::connect(this->getThisCore()->getSimBuilder(),
  //  SIGNAL(newSimFileLoaded()), this, SLOT(updateEnableState()));
  QObject::connect(this->getThisCore()->getSimBuilder(),
    SIGNAL(newSimFileLoaded(const char*)), this, SLOT(onSimFileLoaded(const char*)));
  QObject::connect(this->getThisCore()->getSimBuilder()->attributeUIManager(),
    SIGNAL(attColorChanged()),
    this->getThisCore(), SLOT(onColorByAttribute()));
  QObject::connect(this->getThisCore()->getSimBuilder()->attributeUIManager(),
    SIGNAL(numOfAttriubtesChanged()),
    this->getThisCore(), SLOT(onNumOfAttriubtesChanged()));
}

//-----------------------------------------------------------------------------
pqCMBSceneTree* pqCMBModelBuilderMainWindow::getpqCMBSceneTree()
{
  if(!this->Internal->SceneGeoTree)
    {
    QPixmap pix(":/cmb/pqEyeball16.png");
    QPixmap pixd(":/cmb/pqEyeballd16.png");
    QPixmap pixs(":/cmb/snapIcon.png");
    QPixmap pixl(":/cmb/lockIcon.png");

    qtCMBTreeWidget* trWidget = new qtCMBTreeWidget(this);
    this->Internal->SceneGeoTree = new pqCMBSceneTree(
      &pix, &pixd, &pixs, &pixl, trWidget);
    this->Internal->SceneGeoTree->getWidget()->setContextMenuPolicy(Qt::NoContextMenu);
    this->Internal->SceneGeoTree->getWidget()->setSelectionMode(QAbstractItemView::NoSelection);
    this->Internal->SceneGeoTree->getWidget()->setColumnHidden(2, true);
    this->Internal->SceneGeoTree->setDataObjectPickable(false);
    }

  return this->Internal->SceneGeoTree;
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onNewModelCreated()
{
  // legacy slots, should be updated later with new smtk model slots
 // SMTK model loaded
  this->getMainDialog()->faceParametersDock->setVisible(false);
  this->updateSelectionUI(false);
  this->getMainDialog()->action_Select->setEnabled(true);
  // If there is no dock panel yet, this is the first time, so init
  // default panels
  this->initUIPanel(qtCMBPanelsManager::MODEL);
  this->initUIPanel(qtCMBPanelsManager::INFO);
  this->initUIPanel(qtCMBPanelsManager::DISPLAY);
  this->initUIPanel(qtCMBPanelsManager::COLORMAP);

  this->updateEnableState();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::loadSimulation()
{
  this->getThisCore()->onLoadSimulation();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::loadSimulationTemplate()
{
  this->getThisCore()->onLoadSimulationTemplate();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::loadSimulationScenario()
{
  this->getThisCore()->onLoadScenario();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::saveSimulationScenario()
{
  this->getThisCore()->onSaveScenario();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSimFileLoaded(const char* vtkNotUsed(filename))
{
  // if there is a SimBuilder model loaded, the left panel needs to be updated.
  SimBuilderCore* sbCore = this->getThisCore()->getSimBuilder();
  this->initUIPanel(qtCMBPanelsManager::ATTRIBUTE);

  bool isNewScenario = sbCore->isSimModelLoaded()
    && sbCore->isLoadingScenario() && sbCore->hasScenarioModelEntities();
  if(isNewScenario)
    {
    // this->initTreeWidgets();
    }
  this->UpdateInfoTable();
  this->updateEnableState();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onCreateSimpleModel()
{
  qtCMBCreateSimpleGeometry createSimpleGeometry(this);
  createSimpleGeometry.exec();
  if(createSimpleGeometry.result() == QDialog::Accepted)
    {
    switch(createSimpleGeometry.getGeometryType())
      {
      case 0:
        {
        std::vector<double> boundingBox;
        createSimpleGeometry.getGeometryValues(boundingBox);
        std::vector<int> resolution;
        createSimpleGeometry.getResolutionValues(resolution);
        this->getThisCore()->createRectangleModel(
          &boundingBox[0], resolution[0], resolution[1]);
        break;
        }
      case 1:
        {
        std::vector<double> values;
        createSimpleGeometry.getGeometryValues(values);
        std::vector<int> resolution;
        createSimpleGeometry.getResolutionValues(resolution);
        this->getThisCore()->createEllipseModel(&values[0], resolution[0]);
        break;
        }
      default:
        std::cerr << "pqCMBModelBuilderMainWindow: unknown geometry type\n";
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onHideSharedEntities(bool hideShared)
{
  this->getThisCore()->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onHideModelFaces(bool hide)
{
  this->getThisCore()->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onHideOuterModelBoundary(bool hide)
{
  this->getThisCore()->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSceneFileLoaded()
{
  this->updateEnableState();
  // If there is no dock panel yet, this is the first time, so init
  // default panels
  this->initUIPanel(qtCMBPanelsManager::SCENE);
//  this->initUIPanel(qtCMBPanelsManager::INFO);
//  this->initUIPanel(qtCMBPanelsManager::DISPLAY);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onDisplayBathymetryDialog()
{
  this->getpqCMBSceneTree()->blockSignals(true);
  this->getpqCMBSceneTree()->getWidget()->blockSignals(true);
  qtCMBBathymetryDialog importer(this->getpqCMBSceneTree());
  importer.setMeshAndModelMode(true);
  int dlgStatus = importer.exec();
  if(dlgStatus == QDialog::Accepted)
    {
    }
  else if (dlgStatus == 2) // Remove bathymetry
    {
    }
  this->getpqCMBSceneTree()->getWidget()->blockSignals(false);
  this->getpqCMBSceneTree()->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::editTexture()
{
}
//-----------------------------------------------------------------------------
const QStringList &pqCMBModelBuilderMainWindow::getTextureFileNames()
{
  return this->Internal->TextureFiles;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::unsetTextureMap()
{
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::setTextureMap(const QString& filename, int numberOfRegistrationPoints,
                                       double *points)
{
  this->addTextureFileName(filename.toAscii().data());
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::addTextureFileName(const char *filename)
{
  if (filename && !this->Internal->TextureFiles.contains(filename))
    {
    this->Internal->TextureFiles.append(filename);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateSMTKSelection()
{
  this->getThisCore()->updateSMTKSelection();
}

//----------------------------------------------------------------------------
pqProxyInformationWidget* pqCMBModelBuilderMainWindow::getInfoWidget()
{
  if(!this->Internal->InformationWidget)
    {
    this->Internal->InformationWidget = new pqProxyInformationWidget();
//    pqActiveObjects::instance().disconnect(this->Internal->InformationWidget);
    }
  return this->Internal->InformationWidget;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::initUIPanels()
{
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*>::iterator it;
  for(it = this->Internal->CurrentDockWidgets.begin();
    it != this->Internal->CurrentDockWidgets.end(); ++it)
    {
    if(it.value())
      {
//      it.value()->setWidget(NULL);
      this->removeDockWidget(it.value());
      it.value()->setParent(0);
//      delete it.value();
      }
    }
  this->Internal->CurrentDockWidgets.clear();
  qtCMBPanelsManager* panelManager = this->panelsManager();
  QList<qtCMBPanelsManager::PanelType> paneltypes;
  paneltypes << qtCMBPanelsManager::MODEL
             << qtCMBPanelsManager::ATTRIBUTE
             << qtCMBPanelsManager::MESH
             << qtCMBPanelsManager::SCENE
             << qtCMBPanelsManager::INFO
             << qtCMBPanelsManager::DISPLAY
             << qtCMBPanelsManager::COLORMAP;
  //           << qtCMBPanelsManager::PROPERTIES
  //           << qtCMBPanelsManager::RENDER
  //           << qtCMBPanelsManager::USER_DEFINED;

  panelManager->setPanelTypes(paneltypes);
  this->setTabPosition(Qt::RightDockWidgetArea,QTabWidget::North);

//  QDockWidget* dw = panelManager->createDockWidget(this,
//    this->colorMapEditor(), "Color Map",
//    Qt::RightDockWidgetArea, NULL);
//  dw->hide();
//  pqApplicationCore::instance()->registerManager(
//    "COLOR_EDITOR_PANEL", dw);

}

//----------------------------------------------------------------------------
QDockWidget* pqCMBModelBuilderMainWindow::initUIPanel(
  qtCMBPanelsManager::PanelType enType, bool recreate)
{
  if(this->Internal->CurrentDockWidgets.contains(enType))
    {
    if(!recreate)
      {
      return NULL;
      }
    else if(QDockWidget* existingDoc = this->Internal->CurrentDockWidgets[enType])
      {
      this->removeDockWidget(existingDoc);
      existingDoc->setParent(0);
      }
    }
  int numDocks = this->Internal->CurrentDockWidgets.count();
  QDockWidget* lastdw = numDocks > 0 ?
    this->Internal->CurrentDockWidgets[this->Internal->CurrentDockWidgets.keys()[numDocks - 1] ]
    : NULL;
  QDockWidget* dw = NULL;
  qtCMBPanelsManager* panelManager = this->panelsManager();
  switch(enType)
    {
    case qtCMBPanelsManager::ATTRIBUTE:
      // The SimBuilderUIPanel
      {
      dw = this->getThisCore()->getSimBuilder()->GetUIPanel();
      //uip->uiManager()->initializeUI(
      //  uip->panelWidget(), this->getThisCore()->getSimBuilder());
      dw->setParent(this);
      dw->setWindowTitle(qtCMBPanelsManager::type2String(enType).c_str());
      this->addDockWidget(Qt::RightDockWidgetArea,dw);
      if(lastdw)
        {
        this->tabifyDockWidget(lastdw, dw);
        }
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
      }
    case qtCMBPanelsManager::MODEL:
      // The ModelDock in pqCMBModelBuilderMainWindowCore
      dw = this->getThisCore()->modelPanel();
      dw->setParent(this);
      dw->setWindowTitle(qtCMBPanelsManager::type2String(enType).c_str());
      this->addDockWidget(Qt::RightDockWidgetArea, dw);
      if(lastdw)
        {
        this->tabifyDockWidget(lastdw, dw);
        }
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
    case qtCMBPanelsManager::DISPLAY:
    case qtCMBPanelsManager::PROPERTIES:
      {
      pqDataRepresentation* rep = this->getThisCore()->modelManager()->activeModelRepresentation();
      if(rep)
        {
        //this->displayPanel()->setVisible(true);
        pqProxyWidget* pwidget = this->displayPanel(rep->getProxy());
        dw = panelManager->createDockWidget(this,
          pwidget, qtCMBPanelsManager::type2String(enType),
          Qt::RightDockWidgetArea, lastdw);
        pwidget->filterWidgets(true);
        pwidget->setApplyChangesImmediately(true);
        dw->show();
        //pwidget->show(dw);
        this->Internal->CurrentDockWidgets[enType] = dw;
        }
      break;
      }
    case qtCMBPanelsManager::INFO:
      dw = panelManager->createDockWidget(this,
        this->getInfoWidget(), qtCMBPanelsManager::type2String(enType),
        Qt::RightDockWidgetArea, lastdw);
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
    case qtCMBPanelsManager::SCENE:
      dw = panelManager->createDockWidget(this,
        this->getpqCMBSceneTree()->getWidget(), qtCMBPanelsManager::type2String(enType),
        Qt::RightDockWidgetArea, lastdw);
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
    case qtCMBPanelsManager::COLORMAP:
      {
      pqDataRepresentation* rep = this->getThisCore()->modelManager()->activeModelRepresentation();
      if(rep)
        {
        pqCMBColorMapWidget* colorWidget = this->colorEditor(this);
        colorWidget->setDataRepresentation(rep);
        dw = panelManager->createDockWidget(this,
          colorWidget, qtCMBPanelsManager::type2String(enType),
          Qt::RightDockWidgetArea, lastdw);
        dw->show();
        pqApplicationCore::instance()->registerManager(
          "COLOR_EDITOR_PANEL", dw);
        this->Internal->CurrentDockWidgets[enType] = dw;
        }
      break;
      }
    case qtCMBPanelsManager::MESH:
      // meshing panel
    case qtCMBPanelsManager::RENDER:
      // in the future, we may define different render view layout
/*
      if(pqProxyPanle->widget())
        {
        lastdw = this->createDockWidget(
          pqProxyProxy->widget(), qtCMBPanelsManager::type2String(enType),
          Qt::RightDockWidgetArea, lastdw);
        lastdw->show();
        this->Internal->CurrentDockWidgets[enType] = lastdw;
        }
*/
      break;
    default:
      break;
    }
  return dw;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onActiveRepresentationChanged(
  pqDataRepresentation* acitveRep)
{
  foreach(qtCMBPanelsManager::PanelType enType,
    this->Internal->CurrentDockWidgets.keys())
    {
    QDockWidget* existingDoc = this->Internal->CurrentDockWidgets[enType];
    switch(enType)
      {
      case qtCMBPanelsManager::DISPLAY:
      case qtCMBPanelsManager::PROPERTIES:
        {
        if(existingDoc && existingDoc->widget())
          {
          delete existingDoc->widget();
          }

        if(acitveRep && existingDoc)
          {
          //this->displayPanel()->setVisible(true);
          pqProxyWidget* pwidget = this->displayPanel(acitveRep->getProxy());
          pwidget->filterWidgets(true);
          pwidget->setApplyChangesImmediately(true);

          QWidget* container = new QWidget();
          container->setObjectName("dockscrollWidget");
          container->setSizePolicy(QSizePolicy::Preferred,
            QSizePolicy::Expanding);

          QScrollArea* s = new QScrollArea(existingDoc);
          s->setWidgetResizable(true);
          s->setFrameShape(QFrame::NoFrame);
          s->setObjectName("scrollArea");
          s->setWidget(container);

          QVBoxLayout* vboxlayout = new QVBoxLayout(container);
          vboxlayout->setMargin(0);
          vboxlayout->addWidget(pwidget);

          existingDoc->setWidget(s);
          existingDoc->show();
          }
        break;
        }
      case qtCMBPanelsManager::COLORMAP:
        {
        pqCMBColorMapWidget* colorWidget = this->colorEditor(this);
        colorWidget->setDataRepresentation(acitveRep);
        break;
        }
      case qtCMBPanelsManager::ATTRIBUTE:
      case qtCMBPanelsManager::MODEL:
      // The info widget is handling the active representation internally
      case qtCMBPanelsManager::INFO:
      case qtCMBPanelsManager::SCENE:
      case qtCMBPanelsManager::MESH:
      case qtCMBPanelsManager::RENDER:
        break;
      default:
        break;
      }
    }
}
