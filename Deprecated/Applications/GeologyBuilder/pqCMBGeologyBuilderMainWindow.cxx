/*=========================================================================

  Program:   CMB
  Module:    pqCMBGeologyBuilderMainWindow.cxx

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
#include "pqCMBGeologyBuilderMainWindow.h"

#include "pqCMBGeologyBuilderMainWindowCore.h"
#include "cmbGeologyBuilderConfig.h"
#include "qtCMBGeologyBuilderPanelWidget.h"
#include "ui_qtCMBGeologyBuilderPanel.h"
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
#include "vtkPVArrayInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkSelection.h"
#include "vtkSelectionSource.h"
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
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneNodeIterator.h"
#include "pqCMBGlyphObject.h"
#include "cmbLoadDataReaction.h"
#include "cmbFileExtensions.h"

class pqCMBGeologyBuilderMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/)
    {
    this->ScenePanel = 0;
    }

  ~vtkInternal()
    {
    if(this->ScenePanel)
      {
      delete this->ScenePanel;
      }
    }

  qtCMBGeologyBuilderPanelWidget* ScenePanel;

  pqPropertyLinks GUILinker;
  QLabel *SelectionLabel;
  QComboBox *SelectionMenu;
  QList<QAction*> EditMenuActions;

  QPointer<QToolBar> GeoToolbar;
};

//----------------------------------------------------------------------------
pqCMBGeologyBuilderMainWindow::pqCMBGeologyBuilderMainWindow():
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
pqCMBGeologyBuilderMainWindow::~pqCMBGeologyBuilderMainWindow()
{
  delete this->Tree;
  delete this->Internal;
  delete this->MainWindowCore;
  this->MainWindowCore = NULL;
}

//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::initializeApplication()
{
  this->MainWindowCore = new pqCMBGeologyBuilderMainWindowCore(this);
  this->setWindowIcon( QIcon(QString::fromUtf8(":/cmb/GeologyBuilderIcon.png")) );
  this->initMainWindowCore();

  this->Internal->ScenePanel = new qtCMBGeologyBuilderPanelWidget(
    this->getMainDialog()->dockWidgetContents_2->layout()->parentWidget());
  this->Superclass::addControlPanel(this->Internal->ScenePanel);

  this->MainWindowCore->setupProcessBar(this->statusBar());
  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  QPixmap pix(":/cmb/pqEyeball16.png");
  QPixmap pixd(":/cmb/pqEyeballd16.png");
  QPixmap pixs(":/cmb/snapIcon.png");
  QPixmap pixl(":/cmb/lockIcon.png");
  this->Tree = new pqCMBSceneTree(&pix, &pixd, &pixs, &pixl,
                             this->Internal->ScenePanel->getGUIPanel()->SceneGraph,
                             this->Internal->ScenePanel->getGUIPanel()->InfoGraph);
  this->Tree->setUndoRedoActions(this->getMainDialog()->actionUndo,
                                 this->getMainDialog()->actionRedo);
  this->Tree->setImportObjectAction(this->getMainDialog()->actionImport_Object_File);
  this->Tree->setDuplicateNodeAction(this->getMainDialog()->actionDuplicateNode,
                                     this->getMainDialog()->actionDuplicate_Node_Randomly);
// Create Actions
  this->Tree->setCreateArcNodeAction(this->getMainDialog()->actionContour_Widget);
  this->Tree->setConicalNodeActions(this->getMainDialog()->actionCreateConicalSolid,
                                   this->getMainDialog()->actionEditConicalSolid);
  this->Tree->setGroundPlaneActions(this->getMainDialog()->actionCreateGroundPlane,
                                   this->getMainDialog()->actionEditGroundPlane);
  this->Tree->setInsertNodeAction(this->getMainDialog()->actionAddNode);
  this->Tree->setCreateLineNodeAction(this->getMainDialog()->actionCreate_Line);
  this->Tree->setCreatePolygonAction(this->getMainDialog()->actionCreatePolygon);
  this->Tree->setCreateVOINodeAction(this->getMainDialog()->actionCreate_VOI);

  this->Tree->setSelectSnapNodeAction(this->getMainDialog()->actionSelectSnapTarget,
                                      this->getMainDialog()->actionUnselectSnapTarget);
  this->Tree->setSnapObjectAction(this->getMainDialog()->actionSnap_Object);
  this->Tree->setColorActions(this->getMainDialog()->actionAssign_Color_to_Node,
                              this->getMainDialog()->actionUnassign_Color_from_Node);

  this->Tree->setTextureAction(this->getMainDialog()->actionEditTextureMap);
  this->Tree->setApplyBathymetryAction(this->getMainDialog()->actionApplyBathymetry);

  this->Tree->setTINStackAction(this->getMainDialog()->actionCreateStackTIN);
  this->Tree->setTINStitchAction(this->getMainDialog()->actionCreate_Solid_from_TINs);
  this->Tree->setGenerateArcsAction(this->getMainDialog()->actionGenerateContours);

 this->Tree->setDefineVOIAction(this->getMainDialog()->actionDefineVOI);

 //setup all the actions that relate to arcs/contours
 this->Tree->setArcActions(
  this->getMainDialog()->actionEditContourNode,
  this->getMainDialog()->actionArcSnapping,
  this->getMainDialog()->actionMergeArcs,
  this->getMainDialog()->actionGrowArcSelection,
  this->getMainDialog()->actionAutoConnectArcs);

  this->Tree->setChangeNumberOfPointsLoadedAction(
    this->getMainDialog()->actionChangeNumberOfPointsLoaded);

  this->Tree->setChangeUserDefineObjectTypeAction(
    this->getMainDialog()->actionChangeObjectsType);

  //setup the delete key as it relates to the scene tree
  QShortcut *deleteShortcut = new QShortcut(Qt::Key_Delete,this->Tree->getWidget());
  deleteShortcut->setContext(Qt::WidgetShortcut); //only fire when the tree has focus
  QObject::connect(deleteShortcut,SIGNAL(activated()),
    this->getMainDialog()->actionDeleteNode,SLOT(trigger()));

#if defined(__APPLE__ ) || defined(APPLE)
  //setup the backspace key to the delete key as the apple
  //is weird and names the backspace key the delete key
  QShortcut *backSpaceShortcut = new QShortcut(Qt::Key_Backspace,this->Tree->getWidget());
  backSpaceShortcut->setContext(Qt::WidgetShortcut); //only fire when the tree has focus
  QObject::connect(backSpaceShortcut,SIGNAL(activated()),
    this->getMainDialog()->actionDeleteNode,SLOT(trigger()));
#endif

 // Set the delete action to be last
  this->Tree->setDeleteNodeAction(this->getMainDialog()->actionDeleteNode);

  pqCMBGeologyBuilderMainWindowCore* mainWinCore =
    qobject_cast<pqCMBGeologyBuilderMainWindowCore*>(this->MainWindowCore);
  mainWinCore->setpqCMBSceneTree(this->Tree);

  pqApplicationCore* core = pqApplicationCore::instance();
  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->ClearSelectionButton,
    SIGNAL(clicked()), this->Tree, SLOT(clearSelection()));

  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->tabWidget,
    SIGNAL(currentChanged(int)),this->Tree,SLOT(collapseAllDataInfo()));

  QObject::connect(this->Internal->ScenePanel->getGUIPanel()->ZoomButton,
    SIGNAL(clicked()), mainWinCore, SLOT(zoomOnSelection()));

  QObject::connect(
    mainWinCore, SIGNAL(newSceneLoaded()),
    this, SLOT(onSceneLoaded()));

  QObject::connect(
    mainWinCore, SIGNAL(sceneSaved()),
    this, SLOT(onSceneSaved()));

  QObject::connect(this->getMainDialog()->action_Select,
    SIGNAL(triggered(bool)),
    this, SLOT(onRubberBandSelect(bool)));
  //
  //QObject::connect(mainWinCore->renderViewSelectionHelper(),
  //  SIGNAL(selectionModeChanged(int)),
  //  this, SLOT(onSelectionModeChanged(int)));

  QObject::connect(this->getMainDialog()->actionNew,
    SIGNAL(triggered()), mainWinCore, SLOT(newScene()));

  QObject::connect(this->Tree,
    SIGNAL(lockedNodesChanged()), mainWinCore, SLOT(updateBoxWidget()));

  QObject::connect(this->Tree,
    SIGNAL(firstDataObjectAdded()), mainWinCore, SLOT(resetCamera()),
    Qt::QueuedConnection);

  mainWinCore->setupAppearanceEditor(
    this->Internal->ScenePanel->getGUIPanel()->appearanceFrame);

  QObject::connect(this->Tree,
                   SIGNAL(nodeNameChanged(pqCMBSceneNode *)),
                   this, SLOT(onSceneNodeNameChanged(pqCMBSceneNode*)),
                   Qt::QueuedConnection);

  QObject::connect(this->Tree,
                   SIGNAL(selectionUpdated(const QList<pqCMBSceneNode *> *,
                                           const QList<pqCMBSceneNode *> *)),
                   this,
                   SLOT(onSceneNodeSelected(const QList<pqCMBSceneNode *> *,
                                            const QList<pqCMBSceneNode *> *)),
                   Qt::QueuedConnection);

  QObject::connect(this->Tree,SIGNAL(enableMenuItems(bool)),
    this,SLOT(onEnableMenuItems(bool)));

  //connect the contour manager to control which tab is active
  QObject::connect(this->Tree,SIGNAL(focusOnSceneTab()),
    this->Internal->ScenePanel,SLOT(focusOnSceneTab()));
  QObject::connect(this->Tree,SIGNAL(focusOnDisplayTab()),
    this->Internal->ScenePanel,SLOT(focusOnDisplayTab()));

  // disable until Scene loaded (what is the minimum requirement?)

  this->getMainDialog()->actionNew->setEnabled(true);
  this->getMainDialog()->actionImport_Object_File->setEnabled(false);
  this->Tree->clearSelection();
  this->initGeoToolbar();

  QString filters = cmbFileExtensions::GeologyBuilder_FileTypes();
  this->loadDataReaction()->setSupportedFileTypes(filters);
}
//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::initGeoToolbar()
{
  // Grow toolbar
  this->Internal->GeoToolbar = new QToolBar("Boreholes Display", this);
  this->Internal->GeoToolbar->setObjectName("geo_Toolbar");
  QAction* bhEditColorAction = this->getMainDialog()->actionEdit_Color_Map;
  this->Internal->GeoToolbar->addAction(bhEditColorAction);
  bhEditColorAction->setText("Edit Bore Hole Color Map");
  bhEditColorAction->setToolTip("Edit Bore Hole Color Map");

  QObject::connect(bhEditColorAction, SIGNAL(triggered(bool)),
    this->getThisCore(), SLOT(onBHEditColorMap()));

  this->getThisCore()->setupGeoToolbar(this->Internal->GeoToolbar);
  this->insertToolBar(
    this->getMainDialog()->toolBar_Selection,this->Internal->GeoToolbar);
  QAction* csEditColorAction = new QAction(this->Internal->GeoToolbar);
  QPixmap pixIcon(":/cmb/pqEditColor24.png");
  csEditColorAction->setIcon(pixIcon);
  csEditColorAction->setObjectName("actionEdit_CSColor_Map");
  csEditColorAction->setText("Edit Cross Section Color Map");
  csEditColorAction->setToolTip("Edit Cross Section Color Map");
  this->Internal->GeoToolbar->addAction(csEditColorAction);
  QObject::connect(csEditColorAction, SIGNAL(triggered(bool)),
    this->getThisCore(), SLOT(onCSEditColorMap()));
}
//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::setupMenuActions()
{
  //this->Internal->ScenePanel->getGUIPanel()->setupUi(this);
  // First create a Create and Properties Submenu
  QMenuBar *menubar = this->menuBar();
  QMenu *createMenu = new QMenu("Create", menubar);
  createMenu->setObjectName("menu_Create");
  QMenu *propertiesMenu = new QMenu("Properties", menubar);
  propertiesMenu->setObjectName("menu_Properties");

  QAction *createAction =
    menubar->insertMenu(this->getMainDialog()->menu_View->menuAction(),
                        createMenu);
  QAction *propertiesAction =
    menubar->insertMenu(this->getMainDialog()->menu_View->menuAction(),
                        propertiesMenu);

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
  propertiesMenu->addAction(this->getMainDialog()->actionEditTextureMap);
  propertiesMenu->addAction(this->getMainDialog()->actionApplyBathymetry);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionUndo);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionRedo);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionClearUndoRedoStack);

  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDuplicateNode);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionDuplicate_Node_Randomly);
  this->Internal->EditMenuActions.append(this->getMainDialog()->actionSnap_Object);

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
    this->getMainDialog()->action_Open_File,
    this->getMainDialog()->actionImport_Object_File);

  // Add actions to "Edit" menu.
  this->getMainDialog()->menuEdit->addActions(this->Internal->EditMenuActions);
  this->getMainDialog()->menuEdit->insertSeparator(this->Internal->EditMenuActions.value(0));
  this->getMainDialog()->menuEdit->insertSeparator(this->getMainDialog()->actionDeleteNode);
  this->getMainDialog()->menuEdit->insertSeparator(this->getMainDialog()->actionSnap_Object);


  for(int i=0; i<this->Internal->EditMenuActions.count(); i++)
    {
    this->Internal->ScenePanel->getGUIPanel()->SceneGraph->addAction(
      this->Internal->EditMenuActions.value(i));
    }
  // Add actions to "Tools" menu.
  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionCreateStackTIN);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionCreate_Solid_from_TINs);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionGenerateContours);

  this->getMainDialog()->menu_Tools->insertSeparator(
    this->getMainDialog()->actionGenerateContours);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionArcSnapping);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionMergeArcs);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionGrowArcSelection);

  this->getMainDialog()->menu_Tools->insertAction(
    this->getMainDialog()->actionLock_View_Size,
    this->getMainDialog()->actionAutoConnectArcs);

  this->getMainDialog()->menu_Tools->insertSeparator(
    this->getMainDialog()->actionLock_View_Size);

  // For the time being lets not allow saving
  this->getMainDialog()->action_Save_Data->setVisible(false);
  this->getMainDialog()->action_Save_As->setVisible(false);
}

//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::onSceneLoaded()
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
void pqCMBGeologyBuilderMainWindow::onSceneSaved()
{
  this->updateEnableState();
}

//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::updateEnableState()
{
  bool data_loaded = !this->Tree->isEmpty();
  this->Superclass::updateEnableState(data_loaded);
}

//-----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(QApplication::translate("Geology Builder AboutDialog",
                                               "About Geology Builder",
                                               0, QApplication::UnicodeUTF8));
  dialog->setPixmap(QPixmap(QString(":/cmb/GeologyBuilderSplashAbout.png")));
  dialog->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(GeologyBuilder_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::onHelpHelp()
{
  this->showHelpPage("qthelp://paraview.org/cmbsuite/GeologyBuilder_README.html");
}


//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::onEnableExternalProcesses(bool /*state*/)
{
}


//-----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::clearGUI()
{
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
// For processing selections via the Scene Tree's nodes
void pqCMBGeologyBuilderMainWindow::onSceneNodeSelected(const QList<pqCMBSceneNode *> *,
                                                 const QList<pqCMBSceneNode *> *)
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
void pqCMBGeologyBuilderMainWindow::onSceneNodeNameChanged(pqCMBSceneNode *node)
{
  if (node->isTypeNode()  || (!node->isSelected()))
    {
    // Type nodes are currently not allowed to be selected
    return;
    }

  QString text("Selected Object: ");
  text += node->getName();
  this->getThisCore()->showStatusMessage(text);
}

//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::updateSelection()
{
  int isShiftKeyDown = this->MainWindowCore->activeRenderView()->
    getRenderViewProxy()->GetInteractor()->GetShiftKey();
  if (!isShiftKeyDown)
    {
    // Clear all existing selections
    this->Tree->clearSelection();
    this->getThisCore()->clearSelectedGlyphList();
    }

  int i, n = this->getLastSelectionPorts().count();
  pqOutputPort* opPort;
  pqCMBSceneNode *node;
  QTreeWidgetItem* objItem;

  if(this->getMainDialog()->action_Select->isChecked())
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
  else if(this->getMainDialog()->action_SelectPoints->isChecked())
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
      if(node->getDataObject()->getType() != pqCMBSceneObjectBase::Glyph)
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
void pqCMBGeologyBuilderMainWindow::onSelectGlyph(bool checked)
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
void pqCMBGeologyBuilderMainWindow::onRubberBandSelect(bool checked)
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

//----------------------------------------------------------------------------
// Update "Select" action with the status of the rubber band.
void pqCMBGeologyBuilderMainWindow::onSelectionModeChanged(int mode)
{
  if (mode == pqCMBRubberBandHelper::INTERACT)
    {
    if(this->getMainDialog()->action_SelectPoints->isChecked())
      {
      this->updateSelection();
      this->getMainDialog()->action_SelectPoints->setChecked(false);
      }
    else if(this->getMainDialog()->action_Select->isChecked())
      {
      this->updateSelection();
      this->getMainDialog()->action_Select->setChecked(false);
      }
    }
  else if(mode != pqCMBRubberBandHelper::ZOOM)
    {
    if(mode == pqCMBRubberBandHelper::SELECT_POINTS)
      {
      this->getMainDialog()->action_SelectPoints->setChecked(true);
      }
    else
      {
      this->getMainDialog()->action_Select->setChecked(true);
      }
    }

  this->getMainDialog()->actionZoomToBox->setChecked(
    mode == pqCMBRubberBandHelper::ZOOM);

}

//-----------------------------------------------------------------------------
pqCMBGeologyBuilderMainWindowCore* pqCMBGeologyBuilderMainWindow::getThisCore()
{
  return qobject_cast<pqCMBGeologyBuilderMainWindowCore*>(this->MainWindowCore);
}

//----------------------------------------------------------------------------
void pqCMBGeologyBuilderMainWindow::onEnableMenuItems(bool state)
{
  this->Superclass::onEnableMenuItems(state);
  this->getMainDialog()->menuEdit->setEnabled(state);
  this->getMainDialog()->menu_File->setEnabled(state);
}
