//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBSceneBuilderMainWindow.h"

#include "cmbSceneBuilderConfig.h"
#include "pqCMBSceneBuilderMainWindowCore.h"
#include "qtCMBSceneBuilderPanelWidget.h"
#include "ui_qtCMBMainWindow.h"
#include "ui_qtCMBSceneBuilderPanel.h"

#include <QComboBox>
#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCMBRubberBandHelper.h"
#include "pqDataRepresentation.h"
#include "pqDisplayColorWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqPropertyLinks.h"
#include "pqProxyWidget.h"
#include "pqRenderView.h"
#include "pqScalarBarRepresentation.h"
#include "qtCMBAboutDialog.h"

#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqWaitCursor.h"

#include "pqCMBGlyphObject.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneNodeIterator.h"
#include "pqCMBSceneTree.h"
#include "qtCMBHelpDialog.h"
#include "vtkCellData.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkConvertSelection.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkFacesConnectivityFilter.h"
#include "vtkHydroModelPolySource.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkMapper.h"
#include "vtkMergeFacesFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSelection.h"
#include "vtkSelectionSource.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVariant.h"

#include "pqCMBFileExtensions.h"
#include "pqCMBLoadDataReaction.h"

class pqCMBSceneBuilderMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/) { this->ScenePanel = 0; }

  ~vtkInternal()
  {
    if (this->ScenePanel)
    {
      delete this->ScenePanel;
    }
  }

  qtCMBSceneBuilderPanelWidget* ScenePanel;

  pqPropertyLinks GUILinker;
  QLabel* SelectionLabel;
  QComboBox* SelectionMenu;
  QList<QAction*> EditMenuActions;
};

//----------------------------------------------------------------------------
pqCMBSceneBuilderMainWindow::pqCMBSceneBuilderMainWindow()
  : Internal(new vtkInternal(this))
{
  this->initializeApplication();
  this->setupMenuActions();
  this->updateEnableState();
  this->initProjectManager();
  this->MainWindowCore->applyAppSettings();
}

//----------------------------------------------------------------------------
pqCMBSceneBuilderMainWindow::~pqCMBSceneBuilderMainWindow()
{
  delete this->Tree;
  delete this->Internal;
  delete this->MainWindowCore;
  this->MainWindowCore = NULL;
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::initializeApplication()
{
  this->MainWindowCore = new pqCMBSceneBuilderMainWindowCore(this);
  this->setWindowIcon(QIcon(QString::fromUtf8(":/cmb/SceneBuilderIcon.png")));
  this->initMainWindowCore();

  this->Internal->ScenePanel = new qtCMBSceneBuilderPanelWidget(
    this->getMainDialog()->dockWidgetContents_2->layout()->parentWidget());
  this->Superclass::addControlPanel(this->Internal->ScenePanel);

  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  QPixmap pix(":/cmb/pqEyeball16.png");
  QPixmap pixd(":/cmb/pqEyeballd16.png");
  QPixmap pixs(":/cmb/snapIcon.png");
  QPixmap pixl(":/cmb/lockIcon.png");
  this->Tree = new pqCMBSceneTree(&pix, &pixd, &pixs, &pixl,
    this->Internal->ScenePanel->getGUIPanel()->SceneGraph,
    this->Internal->ScenePanel->getGUIPanel()->InfoGraph);
  this->Tree->setUndoRedoActions(
    this->getMainDialog()->actionUndo, this->getMainDialog()->actionRedo);
  this->Tree->setImportObjectAction(this->getMainDialog()->actionImport_Object_File);
  this->Tree->setDuplicateNodeAction(this->getMainDialog()->actionDuplicateNode,
    this->getMainDialog()->actionDuplicate_Node_Randomly);
  // Create Actions
  this->Tree->setCreateArcNodeAction(this->getMainDialog()->actionContour_Widget);
  this->Tree->setConicalNodeActions(
    this->getMainDialog()->actionCreateConicalSolid, this->getMainDialog()->actionEditConicalSolid);
  this->Tree->setGroundPlaneActions(
    this->getMainDialog()->actionCreateGroundPlane, this->getMainDialog()->actionEditGroundPlane);
  this->Tree->setInsertNodeAction(this->getMainDialog()->actionAddNode);
  this->Tree->setCreateLineNodeAction(this->getMainDialog()->actionCreate_Line);
  this->Tree->setCreatePolygonAction(this->getMainDialog()->actionCreatePolygon);
  this->Tree->setCreateVOINodeAction(this->getMainDialog()->actionCreate_VOI);

  this->Tree->setSelectSnapNodeAction(
    this->getMainDialog()->actionSelectSnapTarget, this->getMainDialog()->actionUnselectSnapTarget);
  this->Tree->setSnapObjectAction(this->getMainDialog()->actionSnap_Object);
  this->Tree->setColorActions(this->getMainDialog()->actionAssign_Color_to_Node,
    this->getMainDialog()->actionUnassign_Color_from_Node);

  this->Tree->setTextureAction(this->getMainDialog()->actionEditTextureMap);
  this->Tree->setElevationAction(this->getMainDialog()->actionShowElevation);
  this->Tree->setApplyBathymetryAction(this->getMainDialog()->actionApplyBathymetry);

  this->Tree->setTINStackAction(this->getMainDialog()->actionCreateStackTIN);
  this->Tree->setTINStitchAction(this->getMainDialog()->actionCreate_Solid_from_TINs);
  this->Tree->setGenerateArcsAction(this->getMainDialog()->actionGenerateContours);
  this->Tree->setGenerateImageMeshAction(this->getMainDialog()->actionCreateDEMMesh);
  this->Tree->setExportSolidsAction(this->getMainDialog()->actionExportSelectedSolids);
  this->Tree->setExportPolygonsAction(this->getMainDialog()->actionExportSelectedPolygons);

  this->Tree->setDefineVOIAction(this->getMainDialog()->actionDefineVOI);

  //setup all the actions that relate to arcs/contours
  this->Tree->setArcActions(this->getMainDialog()->actionEditContourNode,
    this->getMainDialog()->actionArcSnapping, this->getMainDialog()->actionMergeArcs,
    this->getMainDialog()->actionGrowArcSelection, this->getMainDialog()->actionAutoConnectArcs);

  this->Tree->setChangeNumberOfPointsLoadedAction(
    this->getMainDialog()->actionChangeNumberOfPointsLoaded);

  this->Tree->setChangeUserDefineObjectTypeAction(this->getMainDialog()->actionChangeObjectsType);

  this->Tree->setConvertToGlyphAction(this->getMainDialog()->actionConvertToGlyphs);

  //setup the delete key as it relates to the scene tree
  QShortcut* deleteShortcut = new QShortcut(Qt::Key_Delete, this->Tree->getWidget());
  deleteShortcut->setContext(Qt::WidgetShortcut); //only fire when the tree has focus
  QObject::connect(
    deleteShortcut, SIGNAL(activated()), this->getMainDialog()->actionDeleteNode, SLOT(trigger()));

#if defined(__APPLE__) || defined(APPLE)
  //setup the backspace key to the delete key as the apple
  //is weird and names the backspace key the delete key
  QShortcut* backSpaceShortcut = new QShortcut(Qt::Key_Backspace, this->Tree->getWidget());
  backSpaceShortcut->setContext(Qt::WidgetShortcut); //only fire when the tree has focus
  QObject::connect(backSpaceShortcut, SIGNAL(activated()), this->getMainDialog()->actionDeleteNode,
    SLOT(trigger()));
#endif

  // Set the delete action to be last
  this->Tree->setDeleteNodeAction(this->getMainDialog()->actionDeleteNode);

  pqCMBSceneBuilderMainWindowCore* mainWinCore =
    qobject_cast<pqCMBSceneBuilderMainWindowCore*>(this->MainWindowCore);
  mainWinCore->setpqCMBSceneTree(this->Tree);

  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->ClearSelectionButton,
    SIGNAL(clicked()), this->Tree, SLOT(clearSelection()));

  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->tabWidget,
    SIGNAL(currentChanged(int)), this->Tree, SLOT(collapseAllDataInfo()));

  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->ZoomButton, SIGNAL(clicked()),
    mainWinCore, SLOT(zoomOnSelection()));

  QObject::connect(mainWinCore, SIGNAL(newSceneLoaded()), this, SLOT(onSceneLoaded()));

  QObject::connect(mainWinCore, SIGNAL(sceneSaved()), this, SLOT(onSceneSaved()));

  QObject::connect(this->getMainDialog()->action_Select, SIGNAL(triggered(bool)), this,
    SLOT(onRubberBandSelect(bool)));
  //
  //QObject::connect(mainWinCore->renderViewSelectionHelper(),
  //  SIGNAL(selectionModeChanged(int)),
  //  this, SLOT(onSelectionModeChanged(int)));

  QObject::connect(
    this->getMainDialog()->actionNew, SIGNAL(triggered()), mainWinCore, SLOT(newScene()));

  QObject::connect(this->getMainDialog()->actionCreate_Omicron_Input, SIGNAL(triggered()),
    mainWinCore, SLOT(onCreateOmicronInput()));
  QObject::connect(this->getMainDialog()->actionPackageScene, SIGNAL(triggered()), mainWinCore,
    SLOT(onPackageScene()));
  QObject::connect(this->getMainDialog()->actionExport_2D_BCS_File, SIGNAL(triggered()),
    mainWinCore, SLOT(onExport2DBCSFile()));

  QObject::connect(this->getMainDialog()->actionCreate_CMB_Model, SIGNAL(triggered()), mainWinCore,
    SLOT(onSpawnModelMesher()));

  QObject::connect(this->getMainDialog()->actionCreate_Surface_Mesh, SIGNAL(triggered()),
    mainWinCore, SLOT(onSpawnSurfaceMesher()));

  QObject::connect(this->getMainDialog()->actionModify_With_Arc_Functions, SIGNAL(triggered()),
    mainWinCore, SLOT(onSpawnArcModifier()), Qt::UniqueConnection);

  QObject::connect(this->Tree, SIGNAL(lockedNodesChanged()), mainWinCore, SLOT(updateBoxWidget()));

  QObject::connect(this->Tree, SIGNAL(firstDataObjectAdded()), mainWinCore, SLOT(resetCamera()),
    Qt::QueuedConnection);

  mainWinCore->setupAppearanceEditor(this->Internal->ScenePanel->getGUIPanel()->appearanceFrame);

  QObject::connect(this->Tree, SIGNAL(nodeNameChanged(pqCMBSceneNode*)), this,
    SLOT(onSceneNodeNameChanged(pqCMBSceneNode*)), Qt::QueuedConnection);

  QObject::connect(this->Tree,
    SIGNAL(selectionUpdated(const QList<pqCMBSceneNode*>*, const QList<pqCMBSceneNode*>*)), this,
    SLOT(onSceneNodeSelected(const QList<pqCMBSceneNode*>*, const QList<pqCMBSceneNode*>*)),
    Qt::QueuedConnection);

  QObject::connect(this->Tree, SIGNAL(enableMenuItems(bool)), this, SLOT(onEnableMenuItems(bool)));

  //connect the contour manager to control which tab is active
  QObject::connect(
    this->Tree, SIGNAL(focusOnSceneTab()), this->Internal->ScenePanel, SLOT(focusOnSceneTab()));
  QObject::connect(
    this->Tree, SIGNAL(focusOnDisplayTab()), this->Internal->ScenePanel, SLOT(focusOnDisplayTab()));

  // Setup the selection widget
  QActionGroup* group = new QActionGroup(this->getMainDialog()->toolBar_Selection);
  group->addAction(this->getMainDialog()->action_Select);
  group->addAction(this->getMainDialog()->action_SelectPoints);
  this->getMainDialog()->action_SelectPoints->setText("Select Glyph Points");
  this->getMainDialog()->action_SelectPoints->setToolTip("Select Glyph Points");
  this->getMainDialog()->toolBar_Selection->addAction(this->getMainDialog()->action_SelectPoints);

  // Setup the Undo/Redo Actions
  this->getMainDialog()->toolBar->addSeparator();
  this->getMainDialog()->toolBar->addAction(this->getMainDialog()->actionUndo);
  this->getMainDialog()->toolBar->addAction(this->getMainDialog()->actionRedo);
  QObject::connect(this->getMainDialog()->actionClearUndoRedoStack, SIGNAL(triggered()), this->Tree,
    SLOT(emptyEventList()));

  QObject::connect(this->getMainDialog()->action_SelectPoints, SIGNAL(triggered(bool)), this,
    SLOT(onSelectGlyph(bool)));

  this->Internal->SelectionLabel =
    new QLabel("Selection Style:", this->getMainDialog()->toolBar_Selection);
  this->Internal->SelectionMenu = new QComboBox(this->getMainDialog()->toolBar_Selection);
  this->getMainDialog()->toolBar_Selection->addWidget(this->Internal->SelectionLabel);
  this->getMainDialog()->toolBar_Selection->addWidget(this->Internal->SelectionMenu);
  QStringList list;
  list << "Outline"
       << "Points"
       << "Wireframe"
       << "Surface";
  this->Internal->SelectionMenu->addItems(list);
  QObject::connect(this->Internal->SelectionMenu, SIGNAL(currentIndexChanged(int)), mainWinCore,
    SLOT(setSelectionMode(int)));
  this->Internal->SelectionMenu->setCurrentIndex(0);

  // disable until Scene loaded (what is the minimum requirement?)
  this->getMainDialog()->actionCreate_Surface_Mesh->setEnabled(false);
  this->getMainDialog()->actionModify_With_Arc_Functions->setEnabled(false);
  this->getMainDialog()->actionGlyph_Playback->setEnabled(false);
  this->getMainDialog()->actionCreate_CMB_Model->setEnabled(false);
  this->getMainDialog()->actionCreate_Omicron_Input->setEnabled(false);
  this->getMainDialog()->actionExport_2D_BCS_File->setEnabled(false);

  this->getMainDialog()->actionPackageScene->setEnabled(false);
  this->getMainDialog()->actionNew->setEnabled(true);
  this->getMainDialog()->actionImport_Object_File->setEnabled(false);
  this->Tree->clearSelection();

  QObject::connect(this->getMainDialog()->actionGlyph_Playback, SIGNAL(toggled(bool)), this->Tree,
    SLOT(updateUseGlyphPlayback(bool)));

  //Connect Tree signal for scene manipulation mode
  QObject::connect(
    this->Tree, SIGNAL(set2DCameraMode()), mainWinCore, SLOT(set2DCameraInteraction()));
  QObject::connect(
    this->Tree, SIGNAL(pushCameraMode()), mainWinCore, SLOT(pushCameraInteraction()));
  QObject::connect(this->Tree, SIGNAL(popCameraMode()), mainWinCore, SLOT(popCameraInteraction()));
  QObject::connect(this->Tree, SIGNAL(enableToolbars(bool)), this, SLOT(setToolbarsEnabled(bool)));
  QObject::connect(this->Tree,
    SIGNAL(resetViewDirection(double, double, double, double, double, double)), mainWinCore,
    SLOT(resetViewDirection(double, double, double, double, double, double)));

  QString filters = pqCMBFileExtensions::SceneBuilder_FileTypes();
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(pqCMBFileExtensions::SceneBuilder_ReadersMap());
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::setupMenuActions()
{
  //this->Internal->ScenePanel->getGUIPanel()->setupUi(this);
  // First create a Create and Properties Submenu
  QMenuBar* menubar = this->menuBar();
  QMenu* createMenu = new QMenu("Create", menubar);
  createMenu->setObjectName("menu_Create");
  QMenu* propertiesMenu = new QMenu("Properties", menubar);
  propertiesMenu->setObjectName("menu_Properties");

  menubar->insertMenu(this->getMainDialog()->menu_View->menuAction(), createMenu);
  menubar->insertMenu(this->getMainDialog()->menu_View->menuAction(), propertiesMenu);

  // Create Actions
  createMenu->addAction(this->getMainDialog()->actionContour_Widget);
  createMenu->addAction(this->getMainDialog()->actionCreateConicalSolid);
  createMenu->addAction(this->getMainDialog()->actionCreateGroundPlane);
  createMenu->addAction(this->getMainDialog()->actionAddNode);
  createMenu->addAction(this->getMainDialog()->actionCreate_Line);
  createMenu->addAction(this->getMainDialog()->actionCreatePolygon);
  createMenu->addAction(this->getMainDialog()->actionCreate_VOI);

  // Property Actions
  propertiesMenu->addAction(this->getMainDialog()->actionSelectSnapTarget);
  propertiesMenu->addAction(this->getMainDialog()->actionUnselectSnapTarget);
  propertiesMenu->addAction(this->getMainDialog()->actionAssign_Color_to_Node);
  propertiesMenu->addAction(this->getMainDialog()->actionUnassign_Color_from_Node);
  propertiesMenu->addAction(this->getMainDialog()->actionChangeObjectsType);
  propertiesMenu->addAction(this->getMainDialog()->actionShowElevation);
  propertiesMenu->addAction(this->getMainDialog()->actionEditTextureMap);
  propertiesMenu->addAction(this->getMainDialog()->actionApplyBathymetry);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionUndo);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionRedo);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionClearUndoRedoStack);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDuplicateNode);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDuplicate_Node_Randomly);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionSnap_Object);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionConvertToGlyphs);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDeleteNode);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDefineVOI);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionEditConicalSolid);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionEditGroundPlane);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionEditContourNode);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionChangeNumberOfPointsLoaded);

  // Add actions to "File" menu.
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Open_File, this->getMainDialog()->actionNew);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Open_File, this->getMainDialog()->actionImport_Object_File);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Close, this->getMainDialog()->actionCreate_Omicron_Input);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Close, this->getMainDialog()->actionExportSelectedSolids);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->actionCreate_Omicron_Input, this->getMainDialog()->actionPackageScene);
  this->getMainDialog()->menu_File->insertSeparator(
    this->getMainDialog()->actionExportSelectedSolids);
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Close, this->getMainDialog()->actionExport_2D_BCS_File);
  this->getMainDialog()->menu_File->insertAction(this->getMainDialog()->actionExport_2D_BCS_File,
    this->getMainDialog()->actionExportSelectedPolygons);

  // Add actions to "Edit" menu.
  this->getMainDialog()->menuEdit->addActions(this->Internal->EditMenuActions);
  this->getMainDialog()->menuEdit->insertSeparator(this->Internal->EditMenuActions.value(0));
  this->getMainDialog()->menuEdit->insertSeparator(this->getMainDialog()->actionDeleteNode);
  this->getMainDialog()->menuEdit->insertSeparator(this->getMainDialog()->actionSnap_Object);

  for (int i = 0; i < this->Internal->EditMenuActions.count(); i++)
  {
    this->Internal->ScenePanel->getGUIPanel()->SceneGraph->addAction(
      this->Internal->EditMenuActions.value(i));
  }
  this->Internal->ScenePanel->getGUIPanel()->SceneGraph->addAction(
    this->getMainDialog()->actionExportSelectedSolids);

  // Add actions to "Tools" menu.
  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionCreateStackTIN);

  this->getMainDialog()->menu_Tools->insertAction(this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionCreate_Solid_from_TINs);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionGenerateContours);

  this->getMainDialog()->menu_Tools->insertSeparator(this->getMainDialog()->actionGenerateContours);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionGenerateContours, this->getMainDialog()->actionCreateDEMMesh);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionArcSnapping);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionMergeArcs);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionGrowArcSelection);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionAutoConnectArcs);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionCreate_Surface_Mesh);

  this->getMainDialog()->menu_Tools->insertSeparator(
    this->getMainDialog()->actionCreate_Surface_Mesh);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size, this->getMainDialog()->actionCreate_CMB_Model);

  this->getMainDialog()->menu_Tools->insertSeparator(this->getMainDialog()->actionLock_View_Size);

  this->getMainDialog()->menu_Tools->insertAction(this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionModify_With_Arc_Functions);
  this->getMainDialog()->menu_Tools->insertSeparator(this->getMainDialog()->actionLock_View_Size);

  this->getMainDialog()->menu_Tools->insertAction(0, this->getMainDialog()->actionGlyph_Playback);

  this->getMainDialog()->menu_Tools->insertSeparator(this->getMainDialog()->actionGlyph_Playback);
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onSceneLoaded()
{
  if (this->Tree->isEmpty())
  {
    this->clearGUI();
  }
  else
  {
    this->updateEnableState();
    this->getMainDialog()->faceParametersDock->setEnabled(true);
  }
  QString fname = this->getThisCore()->getOutputFileName();
  if (fname == "")
  {
    this->appendDatasetNameToTitle("Untitled");
  }
  else
  {
    QFileInfo finfo(fname);
    this->appendDatasetNameToTitle(finfo.fileName());
  }
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onSceneSaved()
{
  QFileInfo finfo(this->getThisCore()->getOutputFileName());
  this->appendDatasetNameToTitle(finfo.fileName());
  this->updateEnableState();
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::updateEnableState()
{
  bool data_loaded = !this->Tree->isEmpty();
  this->updateEnableState(data_loaded);

  if (data_loaded &&
    (static_cast<pqCMBSceneBuilderMainWindowCore*>(this->MainWindowCore)->getOutputFileName() !=
        ""))
  {
    this->getMainDialog()->actionPackageScene->setEnabled(true);
  }
  else
  {
    this->getMainDialog()->actionPackageScene->setEnabled(false);
  }

  this->getMainDialog()->actionCreate_Surface_Mesh->setEnabled(data_loaded);
  this->getMainDialog()->actionModify_With_Arc_Functions->setEnabled(data_loaded);
  this->getMainDialog()->actionGlyph_Playback->setEnabled(data_loaded);
  this->getMainDialog()->actionCreate_CMB_Model->setEnabled(data_loaded);
  this->getMainDialog()->actionCreate_Omicron_Input->setEnabled(data_loaded);
  this->getMainDialog()->actionExport_2D_BCS_File->setEnabled(data_loaded);
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::setToolbarsEnabled(bool enable)
{
  this->getMainDialog()->toolBar->setEnabled(enable);
  this->getMainDialog()->toolBar_View->setEnabled(enable);
  this->getMainDialog()->toolBar_Selection->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(
    QApplication::translate("Scene Builder AboutDialog", "About Scene Builder", 0
#if QT_VERSION < 0x050000
      ,
      QApplication::UnicodeUTF8
#endif
      ));
  dialog->setPixmap(QPixmap(QString(":/cmb/SceneBuilderSplashAbout.png")));
  dialog->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(SceneGen_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onHelpHelp()
{
  this->showHelpPage("qthelp://paraview.org/cmbsuite/SceneBuilder_README.html");
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onEnableExternalProcesses(bool state)
{
  this->getMainDialog()->actionCreate_CMB_Model->setEnabled(state);
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::clearGUI()
{
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
// For processing selections via the Scene Tree's nodes
void pqCMBSceneBuilderMainWindow::onSceneNodeSelected(
  const QList<pqCMBSceneNode*>*, const QList<pqCMBSceneNode*>*)
{
  int n = static_cast<int>(this->Tree->getSelected().size());
  this->getLastSelectionPorts().clear();
  if (!n)
  {
    this->Internal->ScenePanel->getGUIPanel()->ClearSelectionButton->setEnabled(false);
    this->Internal->ScenePanel->getGUIPanel()->ZoomButton->setEnabled(false);
    this->getThisCore()->showStatusMessage("Selected Object: None");
    return;
  }

  this->Internal->ScenePanel->getGUIPanel()->ClearSelectionButton->setEnabled(true);

  if (n != 1)
  {
    this->getThisCore()->showStatusMessage("Selected Object: Multiple");
  }
  else
  {
    QString text("Selected Object: ");
    text += this->Tree->getSelected()[0]->getName();
    this->getThisCore()->showStatusMessage(text);
  }
  this->Internal->ScenePanel->getGUIPanel()->ZoomButton->setEnabled(true);
}
//-----------------------------------------------------------------------------
// For processing name changes via the Scene Tree's nodes
void pqCMBSceneBuilderMainWindow::onSceneNodeNameChanged(pqCMBSceneNode* node)
{
  if (node->isTypeNode() || (!node->isSelected()))
  {
    // Type nodes are currently not allowed to be selected
    return;
  }

  QString text("Selected Object: ");
  text += node->getName();
  this->getThisCore()->showStatusMessage(text);
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::updateSelection()
{
  int isShiftKeyDown =
    this->MainWindowCore->activeRenderView()->getRenderViewProxy()->GetInteractor()->GetShiftKey();
  if (!isShiftKeyDown)
  {
    // Clear all existing selections
    this->Tree->clearSelection();
    this->getThisCore()->clearSelectedGlyphList();
  }

  int i, n = this->getLastSelectionPorts().count();
  pqOutputPort* opPort;
  pqCMBSceneNode* node;
  QTreeWidgetItem* objItem;

  if (this->getMainDialog()->action_Select->isChecked())
  {

    this->Tree->getWidget()->blockSignals(true);
    for (i = 0; i < n; i++)
    {
      opPort = this->getLastSelectionPorts().at(i);
      if (!opPort)
      {
        continue;
      }
      node = this->Tree->findNode(opPort->getSource());
      if ((!node) || node->isAncestorWidgetSelected())
      {
        continue;
      }
      objItem = node->getWidget();
      objItem->setSelected(true);
    }

    this->Tree->getWidget()->blockSignals(false);
    this->Tree->nodesSelected();
  }
  else if (this->getMainDialog()->action_SelectPoints->isChecked())
  {
    this->Tree->getWidget()->blockSignals(true);
    QList<pqCMBGlyphObject*> glyphList;
    for (i = 0; i < n; i++)
    {
      opPort = this->getLastSelectionPorts().at(i);
      if (!opPort)
      {
        continue;
      }
      node = this->Tree->findNode(opPort->getSource());
      if (!node || !node->getDataObject())
      {
        continue;
      }
      if (node->getDataObject()->getType() != pqCMBSceneObjectBase::Glyph)
      {
        opPort->setSelectionInput(NULL, 0);
      }
      else
      {
        glyphList.append(static_cast<pqCMBGlyphObject*>(node->getDataObject()));
        node->select();
      }
    }
    this->getThisCore()->selectGlyph(glyphList);
    this->Tree->getWidget()->blockSignals(false);
    // The signal is blocked so that the selection of tree item won't
    // invoke selecting the whole glyph object.
    this->Tree->blockSignals(true);
    this->Tree->nodesSelected();
    this->Tree->blockSignals(false);
  }
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onSelectGlyph(bool checked)
{
  //if(checked)
  //  {
  //  this->Tree->getWidget()->setSelectionMode(
  //    QAbstractItemView::NoSelection);
  //  }
  //else
  //  {
  //  this->Tree->getWidget()->setSelectionMode(
  //    QAbstractItemView::ExtendedSelection);
  //  }

  this->getThisCore()->onSelectGlyph(checked);
}
//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onRubberBandSelect(bool checked)
{
  //if(checked)
  //  {
  //  this->Tree->getWidget()->setSelectionMode(
  //    QAbstractItemView::NoSelection);
  //  }
  //else
  //  {
  //  this->Tree->getWidget()->setSelectionMode(
  //    QAbstractItemView::ExtendedSelection);
  //  }

  this->getThisCore()->onRubberBandSelect(checked);
}

//-----------------------------------------------------------------------------
pqCMBSceneBuilderMainWindowCore* pqCMBSceneBuilderMainWindow::getThisCore()
{
  return qobject_cast<pqCMBSceneBuilderMainWindowCore*>(this->MainWindowCore);
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindow::onEnableMenuItems(bool state)
{
  this->Superclass::onEnableMenuItems(state);
  this->getMainDialog()->menuEdit->setEnabled(state);
  this->getMainDialog()->menu_File->setEnabled(state);
}
