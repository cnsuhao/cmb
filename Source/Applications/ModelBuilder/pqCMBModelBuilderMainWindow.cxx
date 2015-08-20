//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBModelBuilderMainWindow.h"
//#include "ui_qtCMBMainWindow.h"

#include "ui_qtCMBMainWindow.h"
#include "cmbModelBuilderConfig.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqColorToolbar.h"
#include "pqEditColorMapReaction.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqPropertyLinks.h"
#include "pqProxyInformationWidget.h"
#include "pqProxyWidget.h"
#include "pqRenderView.h"
#include "pqScalarBarVisibilityReaction.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqWaitCursor.h"
#include "pqTestUtility.h"
#include "pqCheckableComboPopupEventPlayer.h"
#include "pqCheckableComboPopupEventTranslator.h"
#include "pqContextMenuEventTranslator.h"
#include "pqModelTreeViewEventPlayer.h"
#include "pqModelTreeViewEventTranslator.h"

#include "vtkProcessModule.h"
#include "vtkPVGeneralSettings.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkStringArray.h"
#include "vtkVariant.h"
#include "vtkCollection.h"
#include "vtkSMModelManagerProxy.h"

#include "qtCMBCreateSimpleGeometry.h"
#include "qtCMBHelpDialog.h"
#include "qtCMBBathymetryDialog.h"
#include "qtCMBAboutDialog.h"
#include "qtCMBTreeWidget.h"

#include "pqCMBSceneObjectBase.h"
#include "pqPlanarTextureRegistrationDialog.h"
#include "pqCMBLoadDataReaction.h"
#include "pqCMBFileExtensions.h"
#include "pqCMBSceneTree.h"
#include "pqSMTKMeshPanel.h"
#include "pqSMTKModelPanel.h"
#include "pqCMBModelManager.h"
#include "pqCMBColorMapWidget.h"
#include "pqCMBModelBuilderMainWindowCore.h"
#include "pqCMBRubberBandHelper.h"

#include "SimBuilder/SimBuilderCore.h"
#include "SimBuilder/pqSimBuilderUIManager.h"
#include "SimBuilder/qtSimBuilderUIPanel.h"

#include <vtksys/SystemTools.hxx>
#include "smtk/model/StringData.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/vtk/vtkModelMultiBlockSource.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"

#include "vtkPVPlugin.h"

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QScrollArea>
#include <QShortcut>
#include <QtDebug>
#include <QToolButton>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QPalette>
#include <QPixmap>
#include <QSplitter>
#include <QSettings>

#include "cmb_plugins.h"

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

  QPointer<QSettings> SplitterSettings;
  QPointer<QToolBar> Model2DToolbar;
  QPointer<QToolBar> ColorByToolbar;
  QPointer<QComboBox> ColorByArrayBox;

  QPointer<QAction> LoadScenarioAction;
  QPointer<QAction> SaveScenarioAction;
  QPointer<QAction> CreateSimpleModelAction;
  QPointer<QAction> CreateModelEdgesAction;

  QPointer<QMenu> NewModelSessionMenu;

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
  this->initializePlugins();
  this->resetUIPanels();

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
//  vtkPVGeneralSettings* gsettings = vtkPVGeneralSettings::GetInstance();
//  gsettings->SetScalarBarMode(vtkPVGeneralSettings::MANUAL_SCALAR_BARS);

  this->MainWindowCore = new pqCMBModelBuilderMainWindowCore(this);
  this->setWindowIcon( QIcon(QString::fromUtf8(":/cmb/ModelBuilderIcon.png")) );
  this->initMainWindowCore();

  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  QObject::connect(this->getMainDialog()->actionLoad_Simulation_Template,
    SIGNAL(triggered()), this, SLOT(loadSimulationTemplate()));
  QObject::connect(this->getMainDialog()->actionLoad_Simulation,
    SIGNAL(triggered()), this, SLOT(loadSimulation()));
  QObject::connect(this->getMainDialog()->actionSave_Simulation,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onSaveSimulation()));
  QObject::connect(this->getMainDialog()->actionExport_Simulation_File,
    SIGNAL(triggered()), this->getThisCore(), SLOT(onExportSimFile()));

  QObject::connect(this->getMainDialog()->action_Select,
    SIGNAL(triggered(bool)),
    this, SLOT(onSurfaceRubberBandSelect(bool)));

  QObject::connect(this->getThisCore()->cmbRenderViewSelectionHelper(),
    SIGNAL(selectionFinished(int, int, int, int)),
    this, SLOT(onSelectionFinished()));

  QObject::connect(this->getThisCore(),
    SIGNAL(newModelCreated()),
    this, SLOT(onNewModelCreated()));

  this->initSimBuilder();

  this->Internal->SplitterSettings = new QSettings();

  QObject::connect(this->getThisCore(),
    SIGNAL(newSceneLoaded()), this, SLOT(onSceneFileLoaded()));
  QObject::connect(this->getMainDialog()->action_MB_Load_Scene,
    SIGNAL(triggered()), this, SLOT(onLoadScene()));
  QObject::connect(this->getMainDialog()->action_MB_Unload_Scene,
    SIGNAL(triggered()), this, SLOT(onUnloadScene()));

 this->getMainDialog()->faceParametersDock->setParent(0);
  this->getMainDialog()->faceParametersDock->setVisible(false);

  //this->getMainDialog()->actionServerConnect->setEnabled(0);
  //this->getMainDialog()->actionServerDisconnect->setEnabled(0);
  QString filters = pqCMBFileExtensions::ModelBuilder_FileTypes();
  std::set<std::string> modelFileTypes = this->getThisCore()->modelManager()->supportedFileTypes();
  QStringList modelFileExts;
  for (std::set<std::string>::iterator it = modelFileTypes.begin(); it != modelFileTypes.end(); ++it)
    {
    modelFileExts <<  (*it).c_str();
    }

  this->loadDataReaction()->addSpecialExtensions(modelFileExts);
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(
    pqCMBFileExtensions::ModelBuilder_ReadersMap());
  QObject::connect(this->loadDataReaction(), SIGNAL(filesSelected(const QStringList&)),
      this->getThisCore(), SLOT(onFileOpen(const QStringList&)));

  // Add "New Session Action", which will show all available sessions
  this->Internal->NewModelSessionMenu = new QMenu(this->getMainDialog()->menu_File);
  this->Internal->NewModelSessionMenu->setObjectName(QString::fromUtf8("menu_newsession"));
  this->Internal->NewModelSessionMenu->setTitle(QString::fromUtf8("New Session..."));
  this->getMainDialog()->menu_File->insertMenu(
    this->getMainDialog()->action_Open_File,
    this->Internal->NewModelSessionMenu);

  // adding sessions to the "New Session..." menu
  smtk::model::StringList newBnames = this->getThisCore()->modelManager()->managerProxy()->sessionNames();
  for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
    {
    this->addNewSession((*it).c_str());
    }

  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(newSessionLoaded(const QStringList&)),
      this, SLOT(addNewSessions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(newFileTypesAdded(const QStringList&)),
      this->loadDataReaction(), SLOT(addSpecialExtensions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
      SIGNAL(currentModelCleared()),
      this, SLOT(onCMBModelCleared()));
  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(representationChanged(pqDataRepresentation*)),
    this, SLOT(onActiveRepresentationChanged(pqDataRepresentation*)));

  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqModelTreeViewEventTranslator(
                      pqApplicationCore::instance()->testUtility() ));
  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqContextMenuEventTranslator(
                      pqApplicationCore::instance()->testUtility() ));
  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqCheckableComboPopupEventTranslator(
                      pqApplicationCore::instance()->testUtility() ));


  pqApplicationCore::instance()->testUtility()->eventPlayer()->addWidgetEventPlayer(
    new pqModelTreeViewEventPlayer(
                      pqApplicationCore::instance()->testUtility() ));
  pqApplicationCore::instance()->testUtility()->eventPlayer()->addWidgetEventPlayer(
    new pqCheckableComboPopupEventPlayer(
                      pqApplicationCore::instance()->testUtility() ));

  //launch a local meshing server and monitor so that we can submit jobs
  //any time
  this->MainWindowCore->launchLocalMeshingService();
}

cmb_plugins(PV_PLUGIN_IMPORT_INIT)
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::initializePlugins()
{
    cmb_plugins(PV_PLUGIN_IMPORT)
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::addNewSession(const QString& brname)
{
  QAction* act = this->Internal->NewModelSessionMenu->addAction(brname);
  QObject::connect(act, SIGNAL(triggered()), this, SLOT(onCreateNewSession()));
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::addNewSessions(const QStringList& brnames)
{
  foreach(QString brname, brnames)
    {
    this->addNewSession(brname);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onCreateNewSession()
{
  QAction* const action = qobject_cast<QAction*>(
    QObject::sender());
  if(!action)
    {
    return;
    }
  std::string brName = action->text().toStdString();
  bool started = this->getThisCore()->modelManager()->startNewSession(brName);
  if(started)
    {
    this->onNewModelCreated();
    this->getThisCore()->modelPanel()->resetUI();
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
  this->Internal->Model2DToolbar = NULL;
//  QToolBar* colorToolbar = new pqColorToolbar(this)
//    << pqSetName("variableToolbar");
  QToolBar* colorToolbar = new QToolBar(this);
  colorToolbar->setObjectName("colorByToolbar");
  QLabel* label = new QLabel("Color By ", colorToolbar);
  colorToolbar->layout()->setSpacing(0);
  colorToolbar->addWidget(label);

  QComboBox* colorbyBox = new QComboBox(colorToolbar);
  colorbyBox->setObjectName("colorEntityByBox");
  //toolbar->addWidget(SelectionLabel);
  colorToolbar->addWidget(colorbyBox);
  QStringList list;
  this->getThisCore()->modelManager()->supportedColorByModes(list);

  colorbyBox->addItems(list);
  colorbyBox->setCurrentIndex(0);
  QObject::connect(
      colorbyBox, SIGNAL(currentIndexChanged(const QString &)),
      this->getThisCore(), SLOT(onColorByModeChanged(const QString &)));

  colorToolbar->addAction(this->getMainDialog()->actionScalarBarVisibility);
  new pqScalarBarVisibilityReaction(
    this->getMainDialog()->actionScalarBarVisibility);

//  colorToolbar->addAction(this->getMainDialog()->actionEdit_Color_Map);
//  new pqEditColorMapReaction(
//    this->getMainDialog()->actionEdit_Color_Map);

  this->getThisCore()->setupColorByAttributeToolbar(colorToolbar);

  this->addToolBar(Qt::TopToolBarArea, colorToolbar);
  this->insertToolBarBreak(colorToolbar);
  this->Internal->ColorByToolbar = colorToolbar;
  this->Internal->ColorByArrayBox = colorbyBox;
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

  // The "Save" should go through "write" operation of sessions
  this->getMainDialog()->action_Save_Data->setVisible(false);
  this->getMainDialog()->action_Save_As->setVisible(false);

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::updateEnableState()
{
  bool data_loaded = false; /*(this->getThisCore()->getCMBModel()->
    GetCurrentModelEntityMap().keys().count() > 0) ? true : false;*/

  bool model_loaded = this->getThisCore()->modelManager()
    ->numberOfModels() > 0;
  this->Superclass::updateEnableState(model_loaded);

  this->getMainDialog()->action_Select->setEnabled(model_loaded);
  this->getMainDialog()->action_Select->setChecked(false);

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

  if(this->getMainDialog()->actionZoomToBox->isChecked())
    {
    this->getMainDialog()->actionZoomToBox->setChecked(false);
    }
//  this->getMainDialog()->actionZoomToBox->setEnabled(!disable);
//  this->getMainDialog()->toolBar_Selection->setEnabled(!disable);
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
    this->getThisCore()->modelPanel()->startMeshSelectionOperation(
      this->getLastSelectionPorts());

//    this->clearSelectedPorts();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onSurfaceRubberBandSelect(bool checked)
{
  this->getThisCore()->onRubberBandSelect(checked);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::meshSelectionFinished()
{
  // finish up grow, accept or cancel
  this->getThisCore()->modelPanel()->startMeshSelectionOperation(
    this->getLastSelectionPorts());

  this->getThisCore()->modelPanel()->setCurrentMeshSelectionItem(NULL);

  this->updateSelectionUI(false);
  this->getThisCore()->onRubberBandSelect(false);
  this->getThisCore()->modelManager()->clearModelSelections();
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
    trWidget->setColumnCount(3);
    trWidget->setHeaderLabels(
     QStringList() << tr("Scene") << tr("Visibility") << tr("Locked"));

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

  QObject::connect(this->getThisCore()->modelPanel()->modelView(),
    SIGNAL(meshSelectionItemCreated(smtk::attribute::qtMeshSelectionItem*,
           const std::string&, const smtk::common::UUID&)),
    this, SLOT(onMeshSelectionItemCreated(smtk::attribute::qtMeshSelectionItem*,
               const std::string&, const smtk::common::UUID&)));

  // If there is no dock panel yet, this is the first time, so init
  // default panels
  this->initUIPanel(qtCMBPanelsManager::MODEL);
  this->initUIPanel(qtCMBPanelsManager::MESH);
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
const QStringList &pqCMBModelBuilderMainWindow::getTextureFileNames()
{
  return this->Internal->TextureFiles;
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
void pqCMBModelBuilderMainWindow::resetUIPanels()
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
      // The qtSimBuilderUIPanel
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
        // hiding color related components
        for (size_t index = 0; index < rep->getProxy()->GetNumberOfPropertyGroups(); index++)
          {
          int group_tag  = static_cast<int>(index);
          vtkSMPropertyGroup *group = rep->getProxy()->GetPropertyGroup(index);
          if(!group)
            continue;
          QString grplabel = group->GetXMLLabel();
          if (grplabel == "Coloring" || grplabel == "Scalar Coloring")
            {
//            for (size_t j = 0; j < group->GetNumberOfProperties(); j++)
//              {
//              group->GetProperty(static_cast<unsigned int>(j))->SetPanelVisibility("never");
//              }
            // hide the group.
            group->SetPanelVisibility("never");
            }
          }
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
      {
      dw = panelManager->createDockWidget(this,
        this->getInfoWidget(), qtCMBPanelsManager::type2String(enType),
        Qt::RightDockWidgetArea, lastdw);
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;

      pqDataRepresentation* rep = this->getThisCore()->modelManager()->activeModelRepresentation();
      pqOutputPort* actPort = rep ? rep->getOutputPortFromInput() : NULL;
      if(this->Internal->InformationWidget &&
         this->Internal->InformationWidget->getOutputPort() != actPort)
        {
        this->Internal->InformationWidget->setOutputPort(actPort);
        }
      break;
      }
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
      // The Mesh in pqCMBModelBuilderMainWindowCore
      dw = this->getThisCore()->meshPanel();
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
      // The info widget is connected with the active representation internally
      case qtCMBPanelsManager::INFO:
      case qtCMBPanelsManager::ATTRIBUTE:
      case qtCMBPanelsManager::MODEL:
      case qtCMBPanelsManager::SCENE:
      case qtCMBPanelsManager::MESH:
      case qtCMBPanelsManager::RENDER:
        break;
      default:
        break;
      }
    }

  if(acitveRep && this->Internal->ColorByArrayBox)
    {
    vtkSMPropertyHelper colorArrayHelper(acitveRep->getProxy(), "ColorArrayName", true);
    const char* arrayName = colorArrayHelper.GetInputArrayNameToProcess();
    this->Internal->ColorByArrayBox->blockSignals(true);
    int currIdx = -1;
    if(arrayName != NULL)
    for(int i=0; i<this->Internal->ColorByArrayBox->count(); ++i)
      {
      if(this->Internal->ColorByArrayBox->itemText(i) == arrayName)
        {
        currIdx = i;
        break;
        }
      }
    this->Internal->ColorByArrayBox->setCurrentIndex(currIdx);
    this->Internal->ColorByArrayBox->blockSignals(false);
    }

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onRequestMeshSelection()
{
  smtk::attribute::qtMeshSelectionItem* const qMeshItem = qobject_cast<
  smtk::attribute::qtMeshSelectionItem*>(QObject::sender());
  this->getThisCore()->modelPanel()->setCurrentMeshSelectionItem(
    qMeshItem);
  if(!qMeshItem)
    {
    return;
    }
  smtk::attribute::MeshSelectionItemPtr meshItem =
    smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(
    qMeshItem->getObject());
  if(!meshItem)
    {
    return;
    }

  this->updateSelectionUI(true);

  const smtk::attribute::MeshSelectionItemDefinition *itemDef =
    dynamic_cast<const smtk::attribute::MeshSelectionItemDefinition*>(
    meshItem->definition().get());
  smtk::model::BitFlags masked = itemDef->membershipMask();
  if (masked == smtk::model::FACE)
    this->onRequestMeshCellSelection(meshItem);
  else if(masked == (smtk::model::EDGE | smtk::model::VERTEX) ||
          masked == smtk::model::EDGE || masked == smtk::model::VERTEX)
    this->onRequestMeshEdgePointSelection(meshItem);

}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onRequestMeshCellSelection(
  const smtk::attribute::MeshSelectionItemPtr& meshSelectItem)
{

  switch(meshSelectItem->modifyMode())
  {
    case smtk::attribute::ACCEPT:
      this->meshSelectionFinished();
      break;
    case smtk::attribute::NONE:
      this->clearSelectedPorts();
      this->meshSelectionFinished();
      this->getThisCore()->activeRenderView()->render();
      break;
    case smtk::attribute::RESET:
      this->getThisCore()->modelManager()->clearModelSelections();
      this->getThisCore()->onRubberBandSelectCells(true);
      break;
    case smtk::attribute::MERGE:
    case smtk::attribute::SUBTRACT:
      this->getThisCore()->onRubberBandSelectCells(true);
      break;
    default:
      std::cerr << "ERROR: Unrecognized MeshModifyMode!" << std::endl;
      break;
  }
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onRequestMeshEdgePointSelection(
  const smtk::attribute::MeshSelectionItemPtr& meshSelectItem)
{
/*
  // HACK: trigger the vtkSMHardwareSelector to clear buffer. Otherwise,
  // the SelectSurface Points sometime will not work.
  this->RenderView->getRenderViewProxy()->GetActiveCamera()->Modified();
  this->RenderView->forceRender();
*/
  switch(meshSelectItem->modifyMode())
  {
    case smtk::attribute::ACCEPT:
      this->getThisCore()->onRubberBandSelectPoints(true);
      break;
    case smtk::attribute::NONE:
      this->clearSelectedPorts();
      this->meshSelectionFinished();
      this->getThisCore()->activeRenderView()->render();
      break;
    default:
      std::cerr << "ERROR: Unrecognized MeshModifyMode!" << std::endl;
      break;
  }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindow::onMeshSelectionItemCreated(
  smtk::attribute::qtMeshSelectionItem* meshItem,
  const std::string& opName, const smtk::common::UUID& uuid)
{
  if(meshItem)
    {
    QObject::connect(meshItem, SIGNAL(requestMeshSelection(
                      smtk::attribute::ModelEntityItemPtr)),
                    this, SLOT(onRequestMeshSelection()));
    this->getThisCore()->modelPanel()->addMeshSelectionOperation(
      meshItem, opName, uuid);
    }
}
