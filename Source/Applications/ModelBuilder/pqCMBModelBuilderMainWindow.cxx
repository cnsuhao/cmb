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

#include "cmbModelBuilderConfig.h"
#include "ui_qtCMBMainWindow.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCheckableComboPopupEventPlayer.h"
#include "pqCheckableComboPopupEventTranslator.h"
#include "pqColorToolbar.h"
#include "pqContextMenuEventTranslator.h"
#include "pqDataRepresentation.h"
#include "pqEditColorMapReaction.h"
#include "pqModelTreeViewEventPlayer.h"
#include "pqModelTreeViewEventTranslator.h"
#include "pqObjectBuilder.h"
#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqPropertyLinks.h"
#include "pqProxyWidget.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqScalarBarVisibilityReaction.h"
#include "pqSearchBox.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqTestUtility.h"
#include "pqWaitCursor.h"
#include "smtk/extension/qt/qtSelectionManager.h"

#ifdef ENABLE_JOBS_PANEL
#include "qtJobsPanel.h"
#endif

#include "vtkCollection.h"
#include "vtkPVGeneralSettings.h"
#include "vtkProcessModule.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringArray.h"
#include "vtkVariant.h"

#include "qtCMBAboutDialog.h"
#include "qtCMBBathymetryDialog.h"
#include "qtCMBCreateSimpleGeometry.h"
#include "qtCMBHelpDialog.h"
#include "qtCMBTreeWidget.h"

#include "pqCMBColorMapWidget.h"
#include "pqCMBFileExtensions.h"
#include "pqCMBLoadDataReaction.h"
#include "pqCMBModelBuilderMainWindowCore.h"
#include "pqCMBModelBuilderOptions.h"
#include "pqCMBModelManager.h"
#include "pqCMBRubberBandHelper.h"
#include "pqCMBSceneObjectBase.h"
#include "pqCMBSceneTree.h"
#include "pqPlanarTextureRegistrationDialog.h"
#include "pqSMTKInfoPanel.h"
#include "pqSMTKMeshPanel.h"
#include "pqSMTKModelInfo.h"
#include "pqSMTKModelPanel.h"

#include "SimBuilder/SimBuilderCore.h"
#include "SimBuilder/pqSimBuilderUIManager.h"
#include "SimBuilder/qtSimBuilderUIPanel.h"

#include <vtksys/SystemTools.hxx>

#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtSelectionManager.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Manager.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/StringData.h"
#include "smtk/model/StringData.h"
#include <vtksys/SystemTools.hxx>

#include "vtkPVPlugin.h"

#include <QComboBox>
#include <QDockWidget>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPalette>
#include <QPixmap>
#include <QScrollArea>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QSplitter>
#include <QToolButton>
#include <QtDebug>

class pqCMBModelBuilderMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/) { this->SceneGeoTree = 0; }

  ~vtkInternal()
  {
    if (this->SceneGeoTree)
    {
      delete this->SceneGeoTree;
    }
  }

  QPointer<QSettings> SplitterSettings;
  QPointer<QToolBar> Model2DToolbar;
  QPointer<QToolBar> ColorByToolbar;
  QPointer<QComboBox> ColorByArrayBox;

  QPointer<QAction> LoadScenarioAction;
  QPointer<QAction> SaveScenarioAction;
  QPointer<QAction> CreateSimpleModelAction;
  QPointer<QAction> CreateModelEdgesAction;

  QPointer<QMenu> NewModelSessionMenu;
  QPointer<QSignalMapper> NewModelSignalMapper;
  std::vector<QPointer<QAction> > NewModelMenus;

  pqPropertyLinks LineResolutionLinks;
  pqCMBSceneTree* SceneGeoTree;

  // cache toolBar_Selection status
  std::vector<bool> toolBar_Selection;
  // Texture related
  QStringList TextureFiles;
  QPointer<QAction> ChangeTextureAction;
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*> CurrentDockWidgets;

  template <typename T, typename U>
  void createSessionCentricMenus(T core, U self)
  {
    // Add both sets of menu items and hide some of them...

    // I. Session-centric modeling menus (you create new sessions and populate them as desired)

    // Add "New Session Action", which will show all available sessions
    this->NewModelSessionMenu = new QMenu(self->getMainDialog()->menu_File);
    this->NewModelSessionMenu->setObjectName(QString::fromUtf8("menu_newsession"));
    this->NewModelSessionMenu->setTitle(QString::fromUtf8("New Session..."));
    self->getMainDialog()->menu_File->insertMenu(
      self->getMainDialog()->action_Open_File, this->NewModelSessionMenu);

    // Add sessions to the "New Session..." menu
    smtk::model::StringList newBnames = core->modelManager()->managerProxy()->sessionNames();
    for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
    {
      self->addNewSession((*it).c_str());
    }

    // II. Single-session, model-based UI:

    // Find sessions with a "create model" operator
    this->NewModelSignalMapper = new QSignalMapper(self);
    smtk::model::StringList newSessionNames = core->modelManager()->managerProxy()->sessionNames();
    for (smtk::model::StringList::iterator it = newSessionNames.begin();
         it != newSessionNames.end(); ++it)
    {
      std::set<std::string> operatorNames =
        smtk::model::SessionRegistrar::sessionOperatorNames(*it);
      if (operatorNames.find("create model") != operatorNames.end())
      {
        std::ostringstream os;
        os << "New " << *it << " model";
        QAction* act = new QAction(QString::fromUtf8(os.str().c_str()), self);
        //self->getMainDialog()->menu_File->addAction(act);
        self->getMainDialog()->menu_File->insertAction(
          self->getMainDialog()->action_Open_File, act);
        act->setVisible(
          false); // Hide these by default since the session-centric menu appears by default.
        QObject::connect(act, SIGNAL(triggered()), this->NewModelSignalMapper, SLOT(map()));
        this->NewModelSignalMapper->setMapping(act, QString::fromUtf8(it->c_str()));
        this->NewModelMenus.push_back(act);
      }
    }
    QObject::connect(this->NewModelSignalMapper, SIGNAL(mapped(QString)), self,
      SLOT(onCreateNewModel(const QString&)));
  }

  template <typename T>
  void switchSessionCentricMenus(T self, bool sessionCentricModeling)
  {
    (void)self;
    this->NewModelSessionMenu->menuAction()->setVisible(sessionCentricModeling);

    for (auto it = this->NewModelMenus.begin(); it != this->NewModelMenus.end(); ++it)
    {
      (*it)->setVisible(!sessionCentricModeling);
    }
  }
};

pqCMBModelBuilderMainWindow::pqCMBModelBuilderMainWindow()
  : Internal(new vtkInternal(this))
{
  this->initializeApplication();
  this->resetUIPanels();

// Jobs panel is optional
// If enabled (via cmake), jobs panel instantiated once
#ifdef ENABLE_JOBS_PANEL
  QDockWidget* dw = this->initUIPanel(qtCMBPanelsManager::JOBS);
  dw->setVisible(false);
#endif

  this->setupToolbars();
  this->setupMenuActions();
  this->updateEnableState();
  this->initProjectManager();
  this->MainWindowCore->applyAppSettings();
}

pqCMBModelBuilderMainWindow::~pqCMBModelBuilderMainWindow()
{
  this->m_isExiting = true;
  delete this->MainWindowCore;
  delete this->Internal;
  this->MainWindowCore = NULL;
}

bool pqCMBModelBuilderMainWindow::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonPress)
  { // ctrl + left mouse button
    if (Qt::CTRL == QApplication::keyboardModifiers())
    {
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent)
      {
        if (mouseEvent->button() == Qt::LeftButton)
        {
          this->getThisCore()->smtkSelectionManager()->setSelectionModifierToSubtraction();
          this->getMainDialog()->action_Select->setChecked(true);
          this->onSurfaceRubberBandSelect(true);
        }
      }
    }
    else if (Qt::SHIFT == QApplication::keyboardModifiers())
    { // Shfit + left mouse button
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent)
      {
        if (mouseEvent->button() == Qt::LeftButton)
        {
          this->getThisCore()->smtkSelectionManager()->setSelectionModifierToAddition();
          this->getMainDialog()->action_Select->setChecked(true);
          this->onSurfaceRubberBandSelect(true);
        }
      }
    }
  }
  // continue PV mouse event filter
  return pqCMBCommonMainWindow::eventFilter(watched, event);
}

void pqCMBModelBuilderMainWindow::initializeApplication()
{
  //  vtkPVGeneralSettings* gsettings = vtkPVGeneralSettings::GetInstance();
  //  gsettings->SetScalarBarMode(vtkPVGeneralSettings::MANUAL_SCALAR_BARS);

  this->MainWindowCore = new pqCMBModelBuilderMainWindowCore(this);
  this->setWindowIcon(QIcon(QString::fromUtf8(":/cmb/ModelBuilderIcon.png")));
  this->initMainWindowCore();

  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());
  // add selection filter button into ModelBuilder
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByMeshes);
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByModels);
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByVolumes);
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByFaces);
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByEdges);
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->SelectByVertices);

  // connect selection filter signals
  QObject::connect(this->getMainDialog()->SelectByMeshes, SIGNAL(toggled(bool)),
    qtActiveObjects::instance().smtkSelectionManager().get(), SLOT(filterMeshes(bool)));
  QObject::connect(this->getMainDialog()->SelectByFaces, SIGNAL(toggled(bool)),
    qtActiveObjects::instance().smtkSelectionManager().get(), SLOT(filterFaces(bool)));
  QObject::connect(this->getMainDialog()->SelectByEdges, SIGNAL(toggled(bool)),
    qtActiveObjects::instance().smtkSelectionManager().get(), SLOT(filterEdges(bool)));
  QObject::connect(this->getMainDialog()->SelectByVertices, SIGNAL(toggled(bool)),
    qtActiveObjects::instance().smtkSelectionManager().get(), SLOT(filterVertices(bool)));

  QObject::connect(this->getMainDialog()->actionLoad_Simulation_Template, SIGNAL(triggered()), this,
    SLOT(loadSimulationTemplate()));
  QObject::connect(this->getMainDialog()->actionLoad_Simulation, SIGNAL(triggered()), this,
    SLOT(loadSimulation()));
  QObject::connect(this->getMainDialog()->actionSave_Simulation, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onSaveSimulation()));
  QObject::connect(this->getMainDialog()->actionExport_Simulation_File, SIGNAL(triggered()),
    this->getThisCore(), SLOT(onExportSimFile()));
  this->getMainDialog()->actionExport_Simulation_File->setEnabled(false);

  QObject::connect(this->getMainDialog()->action_Select, SIGNAL(triggered(bool)), this,
    SLOT(onSurfaceRubberBandSelect(bool)));

  QObject::connect(this->getThisCore()->cmbRenderViewSelectionHelper(),
    SIGNAL(selectionFinished(int, int, int, int)), this, SLOT(onSelectionFinished()));

  QObject::connect(this->getThisCore(), SIGNAL(newModelCreated()), this, SLOT(onNewModelCreated()),
    Qt::QueuedConnection);
  QObject::connect(this->getThisCore(), SIGNAL(newMeshCreated()), this, SLOT(onNewMeshCreated()),
    Qt::QueuedConnection);

  QObject::connect(
    this->getMainDialog()->action_Close_Session, SIGNAL(triggered()), this, SLOT(onCloseSession()));

  this->initSimBuilder();

  this->Internal->SplitterSettings = new QSettings(this);

  QObject::connect(this->getThisCore(), SIGNAL(newSceneLoaded()), this, SLOT(onSceneFileLoaded()));
  QObject::connect(
    this->getMainDialog()->action_MB_Load_Scene, SIGNAL(triggered()), this, SLOT(onLoadScene()));
  QObject::connect(this->getMainDialog()->action_MB_Unload_Scene, SIGNAL(triggered()), this,
    SLOT(onUnloadScene()));

  this->getMainDialog()->faceParametersDock->setVisible(false);

  //this->getMainDialog()->actionServerConnect->setEnabled(0);
  //this->getMainDialog()->actionServerDisconnect->setEnabled(0);
  QString filters = pqCMBFileExtensions::ModelBuilder_FileTypes();
  std::set<std::string> modelFileTypes = this->getThisCore()->modelManager()->supportedFileTypes();
  QStringList modelFileExts;
  for (std::set<std::string>::iterator it = modelFileTypes.begin(); it != modelFileTypes.end();
       ++it)
  {
    modelFileExts << (*it).c_str();
  }

  this->loadDataReaction()->addSpecialExtensions(modelFileExts);
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(pqCMBFileExtensions::ModelBuilder_ReadersMap());
  QObject::connect(this->loadDataReaction(), SIGNAL(filesSelected(const QStringList&)),
    this->getThisCore(), SLOT(onFileOpen(const QStringList&)));

  this->Internal->createSessionCentricMenus(this->getThisCore(), this);
  QObject::connect(this->getThisCore(), SIGNAL(sessionCentricModelingPreferenceChanged(bool)), this,
    SLOT(toggleSessionCentricMenus(bool)));

  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(operationLog(const smtk::io::Logger&)), this, SLOT(updateLog(const smtk::io::Logger&)));
  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(newSessionLoaded(const QStringList&)), this, SLOT(addNewSessions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(newSessionLoaded(const QStringList&)), this, SLOT(addNewSessions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(sessionIsNowEmpty(const smtk::model::SessionRef&)), this,
    SLOT(autoCloseSession(const smtk::model::SessionRef&)));
  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(newFileTypesAdded(const QStringList&)), this->loadDataReaction(),
    SLOT(addSpecialExtensions(const QStringList&)));
  QObject::connect(this->getThisCore()->modelManager(), SIGNAL(currentModelCleared()), this,
    SLOT(onCMBModelCleared()));
  QObject::connect(&pqActiveObjects::instance(),
    SIGNAL(representationChanged(pqDataRepresentation*)), this,
    SLOT(onActiveRepresentationChanged(pqDataRepresentation*)));
  QObject::connect(this->getThisCore()->modelManager(),
    SIGNAL(modelRepresentationAdded(pqDataRepresentation*)), this,
    SLOT(onModelRepresentationAdded(pqDataRepresentation*)));

  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqModelTreeViewEventTranslator(pqApplicationCore::instance()->testUtility()));
  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqContextMenuEventTranslator(pqApplicationCore::instance()->testUtility()));
  pqApplicationCore::instance()->testUtility()->eventTranslator()->addWidgetEventTranslator(
    new pqCheckableComboPopupEventTranslator(pqApplicationCore::instance()->testUtility()));

  pqApplicationCore::instance()->testUtility()->eventPlayer()->addWidgetEventPlayer(
    new pqModelTreeViewEventPlayer(pqApplicationCore::instance()->testUtility()));
  pqApplicationCore::instance()->testUtility()->eventPlayer()->addWidgetEventPlayer(
    new pqCheckableComboPopupEventPlayer(pqApplicationCore::instance()->testUtility()));

  //launch a local meshing server and monitor so that we can submit jobs
  //any time
  this->MainWindowCore->launchLocalMeshingService();
}

void pqCMBModelBuilderMainWindow::addNewSession(const QString& brname)
{
  QAction* act = this->Internal->NewModelSessionMenu->addAction(brname);
  QObject::connect(act, SIGNAL(triggered()), this, SLOT(onCreateNewSession()));
}

void pqCMBModelBuilderMainWindow::addNewSessions(const QStringList& brnames)
{
  foreach (QString brname, brnames)
  {
    this->addNewSession(brname);
  }
}

void pqCMBModelBuilderMainWindow::onCreateNewSession()
{
  QAction* const action = qobject_cast<QAction*>(QObject::sender());
  if (!action)
  {
    return;
  }
  std::string brName = action->text().toStdString();
  bool started = this->getThisCore()->startNewSession(brName);
  if (started)
  {
    this->getThisCore()->modelPanel()->resetUI();
    this->onNewModelCreated();
  }
}

void pqCMBModelBuilderMainWindow::onCreateNewModel(const QString& sessionType)
{
  this->getThisCore()->startNewSession(sessionType.toUtf8().constData(), true, true);
}

void pqCMBModelBuilderMainWindow::onViewChanged()
{
  if (this->MainWindowCore && this->MainWindowCore->activeRenderView())
  {
    this->Superclass::onViewChanged();
    // The immediateMode is turned on for the new model mapper,
    // because the mapper has to cache its own display list for individual
    // model entity display property, and update its display list when and only
    // when necessary (such as some model entity is turned off).
    //    pqServer::setGlobalImmediateModeRenderingSetting(true);
  }
}

void pqCMBModelBuilderMainWindow::onLoadScene()
{
  this->getThisCore()->setpqCMBSceneTree(this->getpqCMBSceneTree());
  this->getThisCore()->onLoadScene();
}

void pqCMBModelBuilderMainWindow::onUnloadScene()
{
  this->getThisCore()->clearpqCMBSceneTree();
  this->getThisCore()->activeRenderView()->resetCamera();
  this->getThisCore()->activeRenderView()->render();

  this->updateEnableState();
}

void pqCMBModelBuilderMainWindow::setupToolbars()
{
  this->Internal->Model2DToolbar = NULL;
  //  QToolBar* colorToolbar = new pqColorToolbar(this)
  //    << pqSetName("variableToolbar");
  QToolBar* colorToolbar = new QToolBar("Color By", this);
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
  colorbyBox->setCurrentIndex(1);
  QObject::connect(colorbyBox, SIGNAL(currentIndexChanged(const QString&)), this->getThisCore(),
    SLOT(onColorByModeChanged(const QString&)));

  colorToolbar->addAction(this->getMainDialog()->actionScalarBarVisibility);
  new pqScalarBarVisibilityReaction(this->getMainDialog()->actionScalarBarVisibility);

  //  colorToolbar->addAction(this->getMainDialog()->actionEdit_Color_Map);
  //  new pqEditColorMapReaction(
  //    this->getMainDialog()->actionEdit_Color_Map);

  this->getThisCore()->setupColorByAttributeToolbar(colorToolbar);
  this->getThisCore()->setupColorByComboBox(colorbyBox);

  this->addToolBar(Qt::TopToolBarArea, colorToolbar);
  this->insertToolBarBreak(colorToolbar);
  this->Internal->ColorByToolbar = colorToolbar;
  this->Internal->ColorByArrayBox = colorbyBox;
}

void pqCMBModelBuilderMainWindow::setupMenuActions()
{
  // Add actions to "File" menu.
  // Scene File actions
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->action_MB_Load_Scene);
  this->getMainDialog()->action_MB_Load_Scene->setEnabled(true);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->action_MB_Unload_Scene);

  this->getMainDialog()->menu_File->insertSeparator(this->getMainDialog()->action_Exit);

  // SimBuilder file actions
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->actionLoad_Simulation_Template);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->actionLoad_Simulation);
  this->getMainDialog()->menu_File->insertSeparator(this->getMainDialog()->action_Exit);

  this->Internal->LoadScenarioAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->LoadScenarioAction->setObjectName(QString::fromUtf8("action_loadScenario"));
  this->Internal->LoadScenarioAction->setText(QString::fromUtf8("Load Simulation Scenario"));
  QObject::connect(
    this->Internal->LoadScenarioAction, SIGNAL(triggered()), this, SLOT(loadSimulationScenario()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->Internal->LoadScenarioAction);

  this->Internal->SaveScenarioAction = new QAction(this->getMainDialog()->menu_File);
  this->Internal->SaveScenarioAction->setObjectName(QString::fromUtf8("action_saveScenario"));
  this->Internal->SaveScenarioAction->setText(QString::fromUtf8("Save Simulation Scenario"));
  QObject::connect(
    this->Internal->SaveScenarioAction, SIGNAL(triggered()), this, SLOT(saveSimulationScenario()));
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->Internal->SaveScenarioAction);

  this->getMainDialog()->menu_File->insertSeparator(this->getMainDialog()->action_Exit);

  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->actionSave_Simulation);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Exit, this->getMainDialog()->actionExport_Simulation_File);
  this->getMainDialog()->menu_File->insertSeparator(this->getMainDialog()->action_Exit);

  // The "Save" should go through "write" operation of sessions
  this->getMainDialog()->action_Save_Data->setVisible(false);
  this->getMainDialog()->action_Save_As->setVisible(false);

  this->getMainDialog()->action_Close_Session->setEnabled(false);
}

void pqCMBModelBuilderMainWindow::updateEnableState()
{
  bool sessions_open = this->getThisCore()->modelManager()->numberOfRemoteSessions() > 0;
  bool model_loaded = this->getThisCore()->modelManager()->numberOfModels() > 0;
  this->updateEnableState(sessions_open);
  this->getMainDialog()->action_Close_Session->setEnabled(sessions_open);

  this->getMainDialog()->action_Select->setEnabled(model_loaded);
  this->getMainDialog()->action_Select->setChecked(false);

  // this->getMainDialog()->action_MB_Load_Scene->setEnabled(data_loaded);
  this->getMainDialog()->action_MB_Unload_Scene->setEnabled(
    this->Internal->SceneGeoTree && !this->Internal->SceneGeoTree->isEmpty());

  // SimBuilder related UI

  // if there is a SimBuilder model loaded, the left panel needs to be enabled.
  bool isSimLoaded = this->getThisCore()->getSimBuilder()->isSimModelLoaded();
  this->getMainDialog()->actionSave_Simulation->setEnabled(isSimLoaded);
  this->getMainDialog()->actionExport_Simulation_File->setEnabled(isSimLoaded);
  this->Internal->SaveScenarioAction->setEnabled(isSimLoaded);
  if (isSimLoaded)
  {
    this->getMainDialog()->faceParametersDock->setEnabled(true);
    this->getMainDialog()->action_Close->setEnabled(true);
  }
}

void pqCMBModelBuilderMainWindow::onCMBModelModified()
{
  this->UpdateInfoTable();
}

void pqCMBModelBuilderMainWindow::onCMBModelCleared()
{
  this->clearGUI();
}

void pqCMBModelBuilderMainWindow::UpdateInfoTable()
{
  this->updateDataInfo();
}

void pqCMBModelBuilderMainWindow::updateDataInfo()
{
}

void pqCMBModelBuilderMainWindow::clearSelectedPorts()
{
  for (int i = 0; i < this->getLastSelectionPorts().count(); i++)
  {
    pqOutputPort* selPort = this->getLastSelectionPorts().value(i);
    pqPipelineSource* source = selPort ? selPort->getSource() : NULL;
    if (!source)
    {
      continue;
    }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
    if (!smSource)
    {
      continue;
    }
    smSource->SetSelectionInput(0, NULL, 0);
  }
}

bool pqCMBModelBuilderMainWindow::multipleCellsSelected()
{
  int numSel = this->getLastSelectionPorts().count();
  if (numSel > 1)
  {
    int numSelCells = 0;
    for (int i = 0; i < numSel; i++)
    {
      pqOutputPort* selPort = this->getLastSelectionPorts().value(i);
      numSelCells += this->getNumberOfSelectedCells(selPort);
      if (numSelCells > 1)
      {
        return true;
      }
    }
  }
  else if (numSel == 1)
  {
    return (this->getNumberOfSelectedCells(this->getLastSelectionPorts()[0]) > 1);
  }

  return false;
}

int pqCMBModelBuilderMainWindow::getNumberOfSelectedCells(pqOutputPort* selPort)
{
  pqPipelineSource* source = selPort ? selPort->getSource() : NULL;
  if (!source)
  {
    return 0;
  }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  if (!smSource || !smSource->GetSelectionInput(0))
  {
    return 0;
  }

  vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
  vtkSMVectorProperty* vp = vtkSMVectorProperty::SafeDownCast(selSource->GetProperty("IDs"));
  QList<QVariant> ids = pqSMAdaptor::getMultipleElementProperty(vp);
  int numElemsPerCommand = vp->GetNumberOfElementsPerCommand();
  return (numElemsPerCommand > 0) ? (ids.size() / numElemsPerCommand) : 0;
}

void pqCMBModelBuilderMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(
    QApplication::translate("Model Builder AboutDialog", "About Model Builder", 0
#if QT_VERSION < 0x050000
      ,
      QApplication::UnicodeUTF8
#endif
      ));
  dialog->setPixmap(QPixmap(QString(":/cmb/ModelBuilderSplashAbout.png")));
  dialog->setVersionText(QString("<html><b>Version: <i>%1</i></b></html>").arg(CMB_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

void pqCMBModelBuilderMainWindow::onHelpHelp()
{

  this->showHelpPage("qthelp://paraview.org/cmbsuite/ModelBuilder_README.html");
}

void pqCMBModelBuilderMainWindow::clearGUI()
{
  // clear the new panels (except for JOBS)
  QDockWidget* jobsPanel = NULL;
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*>::iterator it;
  for (it = this->Internal->CurrentDockWidgets.begin();
       it != this->Internal->CurrentDockWidgets.end(); ++it)
  {
    if (it.key() == qtCMBPanelsManager::JOBS)
    {
      jobsPanel = it.value();
      continue;
    }

    if (it.value())
    {
      //      it.value()->setWidget(NULL);
      this->removeDockWidget(it.value());
      it.value()->setParent(0);
      //      delete it.value();
    }
  }
  pqApplicationCore::instance()->unRegisterManager("COLOR_EDITOR_PANEL");
  this->Internal->CurrentDockWidgets.clear();
  if (jobsPanel)
  {
    this->Internal->CurrentDockWidgets[qtCMBPanelsManager::JOBS] = jobsPanel;
  }
  this->updateEnableState();
  this->appendDatasetNameToTitle("");

  // Need to explicitly clear the render view
  this->getThisCore()->activeRenderView()->resetCamera();
}

void pqCMBModelBuilderMainWindow::updateSelectionUI(bool disable)
{
  if (disable)
  {
    if (this->getMainDialog()->action_Select->isChecked())
    {
      this->getMainDialog()->action_Select->setChecked(false);
    }
  }

  if (this->getMainDialog()->actionZoomToBox->isChecked())
  {
    this->getMainDialog()->actionZoomToBox->setChecked(false);
  }
  //  this->getMainDialog()->actionZoomToBox->setEnabled(!disable);
  //  this->getMainDialog()->toolBar_Selection->setEnabled(!disable);
}

void pqCMBModelBuilderMainWindow::onSelectionFinished()
{
  if (this->getMainDialog()->action_Select->isChecked())
  {
    this->getThisCore()->cmbRenderViewSelectionHelper()->endSelection();
    this->updateSMTKSelection();
    this->getMainDialog()->action_Select->setChecked(false);
  }
  else if (this->getMainDialog()->actionZoomToBox->isChecked())
  {
    this->getMainDialog()->actionZoomToBox->setChecked(0);
    this->getThisCore()->cmbRenderViewSelectionHelper()->endZoom();
  }
  else
  {
    this->getThisCore()->modelPanel()->startMeshSelectionOperation(this->getLastSelectionPorts());

    //    this->clearSelectedPorts();
  }
}

void pqCMBModelBuilderMainWindow::onSurfaceRubberBandSelect(bool checked)
{
  this->getThisCore()->onRubberBandSelect(checked);
}

void pqCMBModelBuilderMainWindow::meshSelectionFinished()
{
  // finish up grow, accept or cancel
  this->getThisCore()->modelPanel()->startMeshSelectionOperation(this->getLastSelectionPorts());

  this->getThisCore()->modelPanel()->setCurrentMeshSelectionItem(NULL);

  this->updateSelectionUI(false);
  this->getThisCore()->onRubberBandSelect(false);
  this->getThisCore()->modelManager()->clearModelSelections();
}

void pqCMBModelBuilderMainWindow::onAskedToExit()
{
  if (!this->getThisCore()->abortActionForUnsavedWork("quit ModelBuilder"))
  {
    emit userAcceptsExit();
  }
}

void pqCMBModelBuilderMainWindow::onShowCenterAxisChanged(bool enabled)
{
  this->getMainDialog()->actionShowCenterAxes->setEnabled(enabled);
  this->getMainDialog()->actionShowCenterAxes->blockSignals(true);
  pqRenderView* renView = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  this->getMainDialog()->actionShowCenterAxes->setChecked(
    renView ? renView->getCenterAxesVisibility() : false);
  this->getMainDialog()->actionShowCenterAxes->blockSignals(false);
}

void pqCMBModelBuilderMainWindow::setToolbarEnableState(QToolBar* toolbar, bool enabled)
{
  for (int i = 0; i < toolbar->actions().count(); i++)
  {
    toolbar->actions().value(i)->setEnabled(enabled);
  }
  toolbar->setEnabled(enabled);
}

pqCMBModelBuilderMainWindowCore* pqCMBModelBuilderMainWindow::getThisCore()
{
  return qobject_cast<pqCMBModelBuilderMainWindowCore*>(this->MainWindowCore);
}

void pqCMBModelBuilderMainWindow::initSimBuilder()
{
  //this->getThisCore()->getSimBuilder()->getMeshManager()->setUIPanel(
  //  this->Internal->GUIPanel);
  //QObject::connect(this->getThisCore()->getSimBuilder(),
  //  SIGNAL(newSimFileLoaded()), this, SLOT(updateEnableState()));
  QObject::connect(this->getThisCore()->getSimBuilder(), SIGNAL(newSimFileLoaded(const char*)),
    this, SLOT(onSimFileLoaded(const char*)));
}

pqCMBSceneTree* pqCMBModelBuilderMainWindow::getpqCMBSceneTree()
{
  if (!this->Internal->SceneGeoTree)
  {
    QPixmap pix(":/cmb/pqEyeball16.png");
    QPixmap pixd(":/cmb/pqEyeballd16.png");
    QPixmap pixs(":/cmb/snapIcon.png");
    QPixmap pixl(":/cmb/lockIcon.png");

    qtCMBTreeWidget* trWidget = new qtCMBTreeWidget(this);
    trWidget->setColumnCount(3);
    trWidget->setHeaderLabels(QStringList() << tr("Scene") << tr("Visibility") << tr("Locked"));

    this->Internal->SceneGeoTree = new pqCMBSceneTree(&pix, &pixd, &pixs, &pixl, trWidget);
    this->Internal->SceneGeoTree->getWidget()->setContextMenuPolicy(Qt::NoContextMenu);
    this->Internal->SceneGeoTree->getWidget()->setSelectionMode(QAbstractItemView::NoSelection);
    this->Internal->SceneGeoTree->getWidget()->setColumnHidden(2, true);
    this->Internal->SceneGeoTree->setDataObjectPickable(false);
  }

  return this->Internal->SceneGeoTree;
}

void pqCMBModelBuilderMainWindow::onNewModelCreated()
{
  auto mpanel = this->getThisCore()->modelPanel();
  if (!mpanel->modelView())
  {
    mpanel->resetUI();
  }
  // legacy slots, should be updated later with new smtk model slots
  // SMTK model loaded
  this->updateSelectionUI(false);
  this->getMainDialog()->action_Select->setEnabled(true);

  QObject::connect(this->getThisCore()->modelPanel()->modelView(),
    SIGNAL(meshSelectionItemCreated(
      smtk::extension::qtMeshSelectionItem*, const std::string&, const smtk::common::UUID&)),
    this, SLOT(onMeshSelectionItemCreated(
            smtk::extension::qtMeshSelectionItem*, const std::string&, const smtk::common::UUID&)),
    Qt::UniqueConnection);

  // If there is no dock panel yet, this is the first time, so init
  // default panels
  this->initUIPanel(qtCMBPanelsManager::MODEL);
  this->initUIPanel(qtCMBPanelsManager::MESH);
  this->initUIPanel(qtCMBPanelsManager::INFO);
  this->initUIPanel(qtCMBPanelsManager::DISPLAY);
  this->initUIPanel(qtCMBPanelsManager::COLORMAP);

  this->updateEnableState();
}

void pqCMBModelBuilderMainWindow::onModelRepresentationAdded(pqDataRepresentation* rep)
{
  (void)rep;
  this->initUIPanel(qtCMBPanelsManager::DISPLAY, false);
  this->initUIPanel(qtCMBPanelsManager::COLORMAP, false);
}

void pqCMBModelBuilderMainWindow::onNewMeshCreated()
{
  this->getThisCore()->modelPanel()->show();
  this->getThisCore()->modelPanel()->raise();
}

bool pqCMBModelBuilderMainWindow::onCloseSession()
{
  smtk::common::UUIDs uids;
  if (this->getThisCore()->smtkSelectionManager())
  {
    this->getThisCore()->smtkSelectionManager()->getSelectedEntities(uids);
  }
  auto cmbModelMgr = this->getThisCore()->modelManager();
  auto modelMgrPxy = cmbModelMgr ? cmbModelMgr->managerProxy() : NULL;
  smtk::model::Manager::Ptr mgr = modelMgrPxy->modelManager();
  if (!mgr)
  {
    std::cerr << "Could not close session: no model manager!\n\n";
    return false;
  }
  smtk::model::SessionRef sref;
  for (auto uid : uids)
  {
    smtk::model::EntityRef ent(mgr, uid);
    if (ent.isSessionRef())
    {
      sref = ent;
      break;
    }
    if (ent.isValid() && (sref = ent.owningSession()).isValid())
    {
      break;
    }
  }

  if (!sref.isValid())
  {
    smtkWarningMacro(mgr->log(), "You must select a session you wish to close in the model panel.");
    return false;
  }

  if (this->getThisCore()->abortActionForUnsavedWork("close session", sref))
  {
    return false;
  }

  return cmbModelMgr->closeSession(sref);
}

/// A slot to close a particular session, called when a
/// sessionIsNowEmpty signal is emitted by pqCMBModelManager.
/// This will only close the session when session-centric modeling
/// is turned off, as it could be annoying when sessions manually
/// created by the user would be automatically closed after any
/// subsequent operation.
bool pqCMBModelBuilderMainWindow::autoCloseSession(const smtk::model::SessionRef& sref)
{
  if (!this->getThisCore()->userPreferences()->sessionCentricModeling())
  {
    auto cmbModelMgr = this->getThisCore()->modelManager();
    return cmbModelMgr->closeSession(sref);
  }
  return false;
}

void pqCMBModelBuilderMainWindow::loadSimulation()
{
  this->getThisCore()->onLoadSimulation();
}

void pqCMBModelBuilderMainWindow::loadSimulationTemplate()
{
  this->getThisCore()->onLoadSimulationTemplate();
}

void pqCMBModelBuilderMainWindow::loadSimulationScenario()
{
  this->getThisCore()->onLoadScenario();
}

void pqCMBModelBuilderMainWindow::saveSimulationScenario()
{
  this->getThisCore()->onSaveScenario();
}

void pqCMBModelBuilderMainWindow::onSimFileLoaded(const char* vtkNotUsed(filename))
{
  // if there is a SimBuilder model loaded, the left panel needs to be updated.
  SimBuilderCore* sbCore = this->getThisCore()->getSimBuilder();
  this->initUIPanel(qtCMBPanelsManager::ATTRIBUTE);

  bool isNewScenario =
    sbCore->isSimModelLoaded() && sbCore->isLoadingScenario() && sbCore->hasScenarioModelEntities();
  if (isNewScenario)
  {
    // this->initTreeWidgets();
  }
  this->UpdateInfoTable();
  this->updateEnableState();
}

void pqCMBModelBuilderMainWindow::onSceneFileLoaded()
{
  this->updateEnableState();
  // If there is no dock panel yet, this is the first time, so init
  // default panels
  this->initUIPanel(qtCMBPanelsManager::SCENE);
  //  this->initUIPanel(qtCMBPanelsManager::INFO);
  //  this->initUIPanel(qtCMBPanelsManager::DISPLAY);
}

const QStringList& pqCMBModelBuilderMainWindow::getTextureFileNames()
{
  return this->Internal->TextureFiles;
}

void pqCMBModelBuilderMainWindow::addTextureFileName(const char* filename)
{
  if (filename && !this->Internal->TextureFiles.contains(filename))
  {
    this->Internal->TextureFiles.append(filename);
  }
}

void pqCMBModelBuilderMainWindow::toggleSessionCentricMenus(bool sessionCentric)
{
  this->Internal->switchSessionCentricMenus(this, sessionCentric);
}

void pqCMBModelBuilderMainWindow::updateSMTKSelection()
{
  this->getThisCore()->updateSMTKSelection();
}

void pqCMBModelBuilderMainWindow::resetUIPanels()
{
  QMap<qtCMBPanelsManager::PanelType, QDockWidget*>::iterator it;
  for (it = this->Internal->CurrentDockWidgets.begin();
       it != this->Internal->CurrentDockWidgets.end(); ++it)
  {
    if (it.value())
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
  paneltypes << qtCMBPanelsManager::MODEL << qtCMBPanelsManager::ATTRIBUTE
             << qtCMBPanelsManager::MESH << qtCMBPanelsManager::SCENE << qtCMBPanelsManager::INFO
             << qtCMBPanelsManager::DISPLAY << qtCMBPanelsManager::COLORMAP
             << qtCMBPanelsManager::JOBS;
  //           << qtCMBPanelsManager::PROPERTIES
  //           << qtCMBPanelsManager::RENDER
  //           << qtCMBPanelsManager::USER_DEFINED;

  panelManager->setPanelTypes(paneltypes);
  this->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);

  //  QDockWidget* dw = panelManager->createDockWidget(this,
  //    this->colorMapEditor(), "Color Map",
  //    Qt::RightDockWidgetArea, NULL);
  //  dw->hide();
  //  pqApplicationCore::instance()->registerManager(
  //    "COLOR_EDITOR_PANEL", dw);
}

inline bool internal_checkRep(pqDataRepresentation* rep, pqCMBModelManager* cmbModelMgr)
{
  if (!rep)
  {
    pqSMTKModelInfo* minfo = cmbModelMgr->activateModelRepresentation();
    if (minfo)
      rep = minfo->Representation;
  }
  return rep != NULL;
}

QDockWidget* pqCMBModelBuilderMainWindow::initUIPanel(
  qtCMBPanelsManager::PanelType enType, bool recreate)
{
  if (this->Internal->CurrentDockWidgets.contains(enType))
  {
    if (!recreate)
    {
      return NULL;
    }
    else if (QDockWidget* existingDoc = this->Internal->CurrentDockWidgets[enType])
    {
      this->removeDockWidget(existingDoc);
      existingDoc->setParent(0);
    }
  }
  int numDocks = this->Internal->CurrentDockWidgets.count();
  QDockWidget* lastdw = numDocks > 0
    ? this->Internal->CurrentDockWidgets[this->Internal->CurrentDockWidgets.keys()[numDocks - 1]]
    : NULL;
  QDockWidget* dw = NULL;
  qtCMBPanelsManager* panelManager = this->panelsManager();
  pqCMBModelManager* cmbModelMgr = this->getThisCore()->modelManager();
  switch (enType)
  {
    case qtCMBPanelsManager::ATTRIBUTE:
      // The qtSimBuilderUIPanel
      {
        dw = this->getThisCore()->getSimBuilder()->GetUIPanel();
        //uip->uiManager()->initializeUI(
        //  uip->panelWidget(), this->getThisCore()->getSimBuilder());
        dw->setParent(this);
        dw->setWindowTitle(qtCMBPanelsManager::type2String(enType).c_str());
        this->addDockWidget(Qt::RightDockWidgetArea, dw);
        if (lastdw)
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
      if (lastdw)
      {
        this->tabifyDockWidget(lastdw, dw);
      }
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
    case qtCMBPanelsManager::DISPLAY:
    case qtCMBPanelsManager::PROPERTIES:
    {
      pqDataRepresentation* rep = pqActiveObjects::instance().activeRepresentation();
      if (internal_checkRep(rep, cmbModelMgr))
      {
        pqProxyWidget* pwidget = this->displayPanel(rep->getProxy());
        pqSearchBox* searchBox = this->createSearchBox();
        QWidget* panel = new QWidget(this);
        panel->setLayout(new QVBoxLayout());
        panel->layout()->addWidget(searchBox);
        panel->layout()->addWidget(pwidget);

        dw = panelManager->createDockWidget(
          this, panel, qtCMBPanelsManager::type2String(enType), Qt::RightDockWidgetArea, lastdw);

        pwidget->filterWidgets(false);
        dw->show();
        //pwidget->show(dw);
        this->Internal->CurrentDockWidgets[enType] = dw;
      }
      break;
    }
    case qtCMBPanelsManager::INFO:
    {
      dw = panelManager->createDockWidget(this, this->getThisCore()->infoPanel(),
        qtCMBPanelsManager::type2String(enType), Qt::RightDockWidgetArea, lastdw);
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;

      pqDataRepresentation* rep = pqActiveObjects::instance().activeRepresentation();
      internal_checkRep(rep, cmbModelMgr);
      pqOutputPort* actPort = rep ? rep->getOutputPortFromInput() : NULL;
      if (this->getThisCore()->infoPanel() &&
        this->getThisCore()->infoPanel()->getOutputPort() != actPort)
      {
        this->getThisCore()->infoPanel()->setOutputPort(actPort);
      }
      break;
    }
    case qtCMBPanelsManager::SCENE:
      dw = panelManager->createDockWidget(this, this->getpqCMBSceneTree()->getWidget(),
        qtCMBPanelsManager::type2String(enType), Qt::RightDockWidgetArea, lastdw);
      dw->show();
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
    case qtCMBPanelsManager::COLORMAP:
    {
      pqDataRepresentation* rep = pqActiveObjects::instance().activeRepresentation();
      if (internal_checkRep(rep, cmbModelMgr))
      {
        pqCMBColorMapWidget* colorWidget = this->colorEditor(this);
        colorWidget->setDataRepresentation(rep);
        dw = panelManager->createDockWidget(this, colorWidget,
          qtCMBPanelsManager::type2String(enType), Qt::RightDockWidgetArea, lastdw);
        dw->show();
        pqApplicationCore::instance()->unRegisterManager("COLOR_EDITOR_PANEL");
        pqApplicationCore::instance()->registerManager("COLOR_EDITOR_PANEL", dw);
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
      if (lastdw)
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
#ifdef ENABLE_JOBS_PANEL
    case qtCMBPanelsManager::JOBS:
      dw = new qtJobsPanel(this);
      dw->setWindowTitle(qtCMBPanelsManager::type2String(enType).c_str());
      this->addDockWidget(Qt::RightDockWidgetArea, dw);
      if (lastdw)
      {
        this->tabifyDockWidget(lastdw, dw);
      }
      this->Internal->CurrentDockWidgets[enType] = dw;
      break;
#endif
    default:
      break;
  }
  return dw;
}

void pqCMBModelBuilderMainWindow::onActiveRepresentationChanged(pqDataRepresentation* acitveRep)
{
  foreach (qtCMBPanelsManager::PanelType enType, this->Internal->CurrentDockWidgets.keys())
  {
    QDockWidget* existingDoc = this->Internal->CurrentDockWidgets[enType];
    switch (enType)
    {
      case qtCMBPanelsManager::DISPLAY:
      case qtCMBPanelsManager::PROPERTIES:
      {
        if (existingDoc && existingDoc->widget())
        {
          delete existingDoc->widget();
        }

        if (acitveRep && existingDoc)
        {
          pqProxyWidget* pwidget = this->displayPanel(acitveRep->getProxy());
          pwidget->filterWidgets(false);
          pqSearchBox* searchBox = this->createSearchBox();
          QWidget* container = new QWidget();
          container->setObjectName("dockscrollWidget");
          container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

          QScrollArea* s = new QScrollArea(existingDoc);
          s->setWidgetResizable(true);
          s->setFrameShape(QFrame::NoFrame);
          s->setObjectName("scrollArea");
          s->setWidget(container);

          QVBoxLayout* vboxlayout = new QVBoxLayout(container);
          vboxlayout->setMargin(0);
          vboxlayout->addWidget(searchBox);
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

  if (acitveRep && this->Internal->ColorByArrayBox)
  {
    vtkSMPropertyHelper colorArrayHelper(acitveRep->getProxy(), "ColorArrayName", true);
    const char* arrayName = colorArrayHelper.GetInputArrayNameToProcess();
    this->Internal->ColorByArrayBox->blockSignals(true);
    int currIdx = -1;
    if (arrayName != NULL)
      for (int i = 0; i < this->Internal->ColorByArrayBox->count(); ++i)
      {
        if (this->Internal->ColorByArrayBox->itemText(i) == arrayName)
        {
          currIdx = i;
          break;
        }
      }
    this->Internal->ColorByArrayBox->setCurrentIndex(currIdx);
    this->Internal->ColorByArrayBox->blockSignals(false);
  }

  if (this->m_isExiting)
  {
    return;
  }
  this->getThisCore()->modelManager()->setActiveModelRepresentation(acitveRep);
}

void pqCMBModelBuilderMainWindow::onRequestMeshSelection()
{
  smtk::extension::qtMeshSelectionItem* const qMeshItem =
    qobject_cast<smtk::extension::qtMeshSelectionItem*>(QObject::sender());
  this->getThisCore()->modelPanel()->setCurrentMeshSelectionItem(qMeshItem);
  if (!qMeshItem)
  {
    return;
  }
  smtk::attribute::MeshSelectionItemPtr meshItem =
    smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(qMeshItem->getObject());
  if (!meshItem)
  {
    return;
  }

  this->updateSelectionUI(true);

  const smtk::attribute::MeshSelectionItemDefinition* itemDef =
    dynamic_cast<const smtk::attribute::MeshSelectionItemDefinition*>(meshItem->definition().get());
  smtk::model::BitFlags masked = itemDef->membershipMask();
  if (masked == smtk::model::FACE)
    this->onRequestMeshCellSelection(meshItem);
  else if (masked == (smtk::model::EDGE | smtk::model::VERTEX) || masked == smtk::model::EDGE ||
    masked == smtk::model::VERTEX)
    this->onRequestMeshEdgePointSelection(meshItem);
}

void pqCMBModelBuilderMainWindow::onRequestMeshCellSelection(
  const smtk::attribute::MeshSelectionItemPtr& meshSelectItem)
{

  switch (meshSelectItem->modifyMode())
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

void pqCMBModelBuilderMainWindow::onRequestMeshEdgePointSelection(
  const smtk::attribute::MeshSelectionItemPtr& meshSelectItem)
{
  /*
  // HACK: trigger the vtkSMHardwareSelector to clear buffer. Otherwise,
  // the SelectSurface Points sometime will not work.
  this->RenderView->getRenderViewProxy()->GetActiveCamera()->Modified();
  this->RenderView->forceRender();
*/
  switch (meshSelectItem->modifyMode())
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

void pqCMBModelBuilderMainWindow::onMeshSelectionItemCreated(
  smtk::extension::qtMeshSelectionItem* meshItem, const std::string& opName,
  const smtk::common::UUID& uuid)
{
  if (meshItem)
  {
    QObject::connect(meshItem, SIGNAL(requestMeshSelection(smtk::attribute::ModelEntityItemPtr)),
      this, SLOT(onRequestMeshSelection()));
    this->getThisCore()->modelPanel()->addMeshSelectionOperation(meshItem, opName, uuid);
  }
}

void pqCMBModelBuilderMainWindow::initInspectorDock()
{
  if (this->getMainDialog()->faceParametersDock)
  {
    this->removeDockWidget(this->getMainDialog()->faceParametersDock);
    this->getMainDialog()->faceParametersDock->setParent(NULL);
  }
}

pqSearchBox* pqCMBModelBuilderMainWindow::createSearchBox()
{
  pqSearchBox* searchBox = new pqSearchBox();
  searchBox->setAdvancedSearchEnabled(true);
  QObject::connect(
    searchBox, SIGNAL(advancedSearchActivated(bool)), this, SLOT(filterDisplayPanel()));
  QObject::connect(searchBox, SIGNAL(textChanged(QString)), this, SLOT(filterDisplayPanel()));
  return searchBox;
}

void pqCMBModelBuilderMainWindow::closeEvent(QCloseEvent* event)
{
  pqOptions* opts = pqApplicationCore::instance()->getOptions();
  if (!opts || opts->GetTestScripts().empty())
  {
    if (this->getThisCore()->abortActionForUnsavedWork("close main window"))
    {
      event->ignore();
      return;
    }
  }

  return QMainWindow::closeEvent(event);
}

void pqCMBModelBuilderMainWindow::filterDisplayPanel()
{
  pqSearchBox* const searchBox = qobject_cast<pqSearchBox*>(QObject::sender());
  if (!searchBox)
  {
    return;
  }
  QDockWidget* existingDoc = this->Internal->CurrentDockWidgets[qtCMBPanelsManager::DISPLAY];
  if (existingDoc && existingDoc->widget())
  {
    QList<pqProxyWidget*> pxyWidgets = existingDoc->widget()->findChildren<pqProxyWidget*>();
    if (pxyWidgets.count())
    {
      pxyWidgets[0]->filterWidgets(searchBox->isAdvancedSearchActive(), searchBox->text());
    }
  }
}

void pqCMBModelBuilderMainWindow::updateToolBar_Selection(bool checked)
{
  if (checked)
  { // cache the toolBar_Selection status
    this->Internal->toolBar_Selection.push_back(this->getMainDialog()->SelectByModels->isChecked());
    this->Internal->toolBar_Selection.push_back(
      this->getMainDialog()->SelectByVolumes->isChecked());
    this->Internal->toolBar_Selection.push_back(this->getMainDialog()->SelectByFaces->isChecked());
    this->Internal->toolBar_Selection.push_back(this->getMainDialog()->SelectByEdges->isChecked());
    this->Internal->toolBar_Selection.push_back(
      this->getMainDialog()->SelectByVertices->isChecked());
    // reset all to false since selection by entity would do nothing
    this->getMainDialog()->SelectByModels->setChecked(false);
    this->getMainDialog()->SelectByVolumes->setChecked(false);
    this->getMainDialog()->SelectByFaces->setChecked(false);
    this->getMainDialog()->SelectByEdges->setChecked(false);
    this->getMainDialog()->SelectByVertices->setChecked(false);
  }
  else
  { // reset to initial state
    if (this->Internal->toolBar_Selection.size() == 5)
    {
      this->getMainDialog()->SelectByModels->setChecked(this->Internal->toolBar_Selection[0]);
      this->getMainDialog()->SelectByVolumes->setChecked(this->Internal->toolBar_Selection[1]);
      this->getMainDialog()->SelectByFaces->setChecked(this->Internal->toolBar_Selection[2]);
      this->getMainDialog()->SelectByEdges->setChecked(this->Internal->toolBar_Selection[3]);
      this->getMainDialog()->SelectByVertices->setChecked(this->Internal->toolBar_Selection[4]);
      this->Internal->toolBar_Selection.clear();
    }
  }
}
