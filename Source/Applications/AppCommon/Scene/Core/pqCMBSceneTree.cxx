//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include "qtCMBSceneObjectImporter.h"
#include "pqCMBSceneNodeIterator.h"
#include "cmbSceneNodeReplaceEvent.h"
#include "pqCMBConicalRegion.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBLine.h"
#include "pqCMBPlane.h"
#include "pqCMBPoints.h"
#include "pqCMBPolygon.h"
#include "pqCMBArc.h"
#include "pqCMBVOI.h"
#include "qtCMBColorDialog.h"
#include <QAction>
#include <QFileInfo>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QIcon>
#include <QStringBuilder>
#include <QProgressDialog>
#include "pqCMBLineWidget.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSMAdaptor.h"
#include "pqActiveObjects.h"

#include "vtkDoubleArray.h"
#include "vtkPVArcInfo.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkImplicitSelectionLoop.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkProcessModule.h"
#include "vtkSmartPointer.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMSceneContourSourceProxy.h"

#include "vtkNew.h"
#include "vtkCMBArcMergeArcsClientOperator.h"
#include "vtkCMBArcSnapClientOperator.h"
#include "vtkCMBArcGrowClientOperator.h"
#include "vtkIdTypeArray.h"

#include "qtCMBGenerateContoursDialog.h"
#include "qtCMBSceneObjectDuplicateDialog.h"
#include "qtCMBStackedTINDialog.h"
#include "qtCMBVOIDialog.h"
#include "qtCMBGroundPlaneDialog.h"
#include "pqPlanarTextureRegistrationDialog.h"
#include "qtCMBUserTypeDialog.h"
#include "qtCMBConeNodeDialog.h"
#include "qtCMBArcWidgetManager.h"
#include "qtCMBTreeWidget.h"
#include "ui_qtLIDARNumberOfPointsDialog.h"
#include "ui_qtCMBSceneArcSnappingDialog.h"
#include "ui_qtCMBSceneArcPolygonMeshingDialog.h"
#include "qtCMBBathymetryDialog.h"
#include <vtksys/SystemTools.hxx>
#include "pqCMBBoreHole.h"
#include "pqCMBCrossSection.h"
#include "pqCMBUniformGrid.h"

#include <fstream>

#define MAX_RANDOM_PLACEMENT_TRY 100

//-----------------------------------------------------------------------------
pqCMBSceneTree::pqCMBSceneTree(QPixmap *visPixMap, QPixmap *ivisPixMap,
                               QPixmap *snapPixMap,
                               QPixmap *lockPixMap,
                               QTreeWidget *widget,
                               QTreeWidget *infoWidget) :
    CurrentUndoIndex(0), MaxUndoRedoLimit(25), RecordEvents(true)
{
  this->IconLocked = new QIcon(*lockPixMap);
  this->IconVisible = new QIcon(*visPixMap);
  this->IconInvisible = new QIcon(*ivisPixMap);
  this->IconNULL = new QIcon();
  this->IconSnap = new QIcon(*snapPixMap);
  this->EditMode = false;
  this->TreeWidget = widget;
  this->InfoTreeWidget = infoWidget;
  this->Root = NULL;
  this->SnapTarget = NULL;
  this->ConvertToGlyphAction = NULL;
  this->ImportAction = NULL;
  this->InsertAction = NULL;
  this->DuplicateAction = NULL;
  this->DuplicateRandomlyAction = NULL;
  this->DeleteAction = NULL;
  this->UnsetSnapTargetAction = NULL;
  this->SetSnapTargetAction = NULL;
  this->SetSnapObjectAction = NULL;
  this->SetNodeColorAction = NULL;
  this->UnsetNodeColorAction = NULL;
  this->ChangeTextureAction = NULL;
  this->ApplyBathymetryAction = NULL;
  this->VOIAction = NULL;
  this->LineAction = NULL;
  this->ArcAction = NULL;
  this->EditArcAction = NULL;
  this->ArcSnappingAction = NULL;
  this->MergeArcsAction = NULL;
  this->GrowArcSelectionAction = NULL;
  this->AutoConnectArcsAction = NULL;
  this->CreatePolygonAction = NULL;
  this->CurrentServer = NULL;
  this->CurrentView = NULL;
  this->ChangeNumberOfPointsLoadedAction = NULL;
  this->TINStitchAction = NULL;
  this->TINStackAction = NULL;
  this->GenerateArcsAction = NULL;
  this->ExportSolidsAction = NULL;
  this->ExportPolygonsAction = NULL;
  this->DefineVOIAction = NULL;
  this->ConeCreateAction = NULL;
  this->ConeEditAction = NULL;
  this->CreateGroundPlaneAction = NULL;
  this->ElevationAction = NULL;
  this->EditGroundPlaneAction = NULL;
  this->ChangeUserDefineObjectTypeAction = NULL;
  this->UndoAction = NULL;
  this->RedoAction = NULL;
  this->ArcWidgetManager = NULL;
  this->GenerateImageMesh = NULL;

  this->createArcWidgetManager();

  // Set up Context Menu Structure
  this->ContextMenu = new QMenu(widget);
  this->ContextMenu->setTitle("Scene Menu");
  this->FileMenu = this->ContextMenu->addMenu("File");
  this->CreateMenu = this->ContextMenu->addMenu("Create");
  this->EditMenu = this->ContextMenu->addMenu("Edit");
  this->PropertiesMenu = this->ContextMenu->addMenu("Properties");
  this->ToolsMenu = this->ContextMenu->addMenu("Tools");

  //set up QObject names for the context menu.
  //this is done so that we can use the context menu in tests
  this->ContextMenu->setObjectName("SceneContextMenu");
  this->FileMenu->setObjectName("SceneContextFileMenu");
  this->CreateMenu->setObjectName("SceneContextCreateMenu");
  this->EditMenu->setObjectName("SceneContextEditMenu");
  this->PropertiesMenu->setObjectName("SceneContextPropertiesMeenu");
  this->ToolsMenu->setObjectName("SceneContextToolsMenu");


  this->Units = cmbSceneUnits::Unknown;

  this->TreeWidget->header()->setResizeMode(pqCMBSceneNode::getNameColumn(),
                                            QHeaderView::ResizeToContents);
  this->TreeWidget->header()->setResizeMode(pqCMBSceneNode::getVisibilityColumn(),
                                            QHeaderView::ResizeToContents);
  this->TreeWidget->header()->setResizeMode(pqCMBSceneNode::getLockColumn(),
                                            QHeaderView::ResizeToContents);
  this->TreeWidget->setColumnWidth(pqCMBSceneNode::getLockColumn(),7);
  this->TreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->TreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  if (this->InfoTreeWidget)
    {
    QObject::connect(this->InfoTreeWidget,
                 SIGNAL(itemExpanded(QTreeWidgetItem*)),
                 this, SLOT(recomputeInfo(QTreeWidgetItem*)));
    }

  //cast to qtCMBTreeWidget to grab all the signals
  qtCMBTreeWidget* sceneTree =
    dynamic_cast<qtCMBTreeWidget*>(widget);
  if ( sceneTree )
    {
    QObject::connect(sceneTree,
                     SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                     this, SLOT(nodeChanged(QTreeWidgetItem*, int)),
                     Qt::QueuedConnection);

    QObject::connect(sceneTree,
                     SIGNAL(itemLeftButtonClicked(QTreeWidgetItem*, int)),
                     this, SLOT(nodeClicked(QTreeWidgetItem*, int)),
                     Qt::QueuedConnection);

    //the user can select nodes by also using the keyboard arrow keys
    QObject::connect(sceneTree,
                     SIGNAL(itemActivated(QTreeWidgetItem*, int)),
                     this, SLOT(nodeClicked(QTreeWidgetItem*, int)),
                     Qt::QueuedConnection);

    QObject::connect(sceneTree,
                     SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(showContextMenu(const QPoint &)));

    QObject::connect(sceneTree,
                     SIGNAL(itemSelectionChanged()),
                     this, SLOT(nodesSelected()),
                     Qt::QueuedConnection);
    }

  //setting the edit trigger to be only double click no matter the OS
  this->TreeWidget->setEditTriggers( QAbstractItemView::DoubleClicked );

  this->UserObjectTypes.append("-Unknown");
  this->UserObjectTypes.append("-LIDAR");
  this->UserObjectTypes.append("-Solid");
  this->UserObjectTypes.append("-TIN");
  this->UserObjectTypes.append("-VOI");
  this->UserObjectTypes.append("-Line");
  this->UserObjectTypes.append("-Contour");
  this->UserObjectTypes.append("-GroundPlane");
  this->UserObjectTypes.append("-UniformGrid");
  this->UserObjectTypes.append("-Cone");
  this->UserObjectTypes.append("-SolidMesh");
  this->UserObjectTypes.append("-Other");
  this->UserObjectTypes.sort();

  this->DataObjectPickable = true;
  this->InitialSceneObjectColor = Qt::white;
  this->useGlyphPlayback = false;
}

//-----------------------------------------------------------------------------
pqCMBSceneTree::~pqCMBSceneTree()
{
  this->empty();
  delete this->IconVisible;
  delete this->IconInvisible;
  delete this->IconSnap;
  delete this->IconLocked;
  delete this->IconNULL;
  if ( this->ArcWidgetManager )
    {
    delete this->ArcWidgetManager;
    this->ArcWidgetManager = NULL;
    }

}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::emptyEventList()
{
  // Need to empty the UndoRedo List
  int i, n = this->UndoRedoList.size();
  for (i = 0; i < n; i++)
    {
    delete this->UndoRedoList[i];
    }
  this->UndoRedoList.clear();
  this->CurrentUndoIndex = 0;
  // Turn off both Undo and Redo Actions
  if (this->UndoAction)
    {
    this->UndoAction->setEnabled(false);
    }
  if (this->RedoAction)
    {
    this->RedoAction->setEnabled(false);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::deleteUndoEvents()
{
  // Need to delete all undone events
  int i, n = this->UndoRedoList.size();
  if (!n)
      {
      return;
      }

  for (i = n-1; i > -1; i--)
    {
    if (this->UndoRedoList[i]->isApplied())
        {
        // We are done
        break;
        }
    delete this->UndoRedoList[i];
    this->UndoRedoList.removeLast();
    }
  this->CurrentUndoIndex = this->UndoRedoList.size();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::undo()
{
    // Nothing to undo if the curret index is 0
    if (!this->CurrentUndoIndex)
        {
        return;
        }
    // Undo the most recent event
    this->CurrentUndoIndex--;
    this->UndoRedoList[this->CurrentUndoIndex]->undo();
    // Turn on Redo Action
    if (this->RedoAction)
      {
      this->RedoAction->setEnabled(true);
      }

    if (!this->CurrentUndoIndex)
        {
        // There is nothing to be undone
        if (this->UndoAction)
          {
          this->UndoAction->setEnabled(false);
          }
        }
    // Since this could cause a change in what has been selected
   // this->nodesSelected(true);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::redo()
{
    // Nothing to redo if the curret index == the size of the list
    int n = this->UndoRedoList.size();
    if (this->CurrentUndoIndex == n)
        {
        return;
        }
    // Redo the most recent event
    this->UndoRedoList[this->CurrentUndoIndex]->redo();
    this->CurrentUndoIndex++;
    // Turn on Undo Action
    if (this->UndoAction)
      {
      this->UndoAction->setEnabled(true);
      }
    if (this->CurrentUndoIndex==n)
        {
        // Turn off Redo Action!;
        if (this->RedoAction)
          {
          this->RedoAction->setEnabled(false);
          }
        }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::insertEvent(cmbEvent *event)
{
    //First delete all undo events
    this->deleteUndoEvents();
    this->UndoRedoList.append(event);
    this->CurrentUndoIndex++;
    // Set Undo Action to be enabled
    if (this->UndoAction)
      {
      this->UndoAction->setEnabled(true);
      }
    // Set Redo Action to be disabled
    if (this->RedoAction)
      {
      this->RedoAction->setEnabled(false);
      }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::empty()
{
    this->emptyEventList();

//  pqActiveObjects::instance().setActiveSource(NULL);
  if(this->isEmpty())
    {
    return;
    }
  this->TreeWidget->blockSignals(true);
  if (this->Selected.size())
    {
    this->nodesSelected(true);
    }
  // Need to delete those nodes that depending on other nodes
  // For examples, polygons that have bathymetry applied to them.
  std::vector<pqCMBSceneNode *> bathymetries;
  SceneObjectNodeIterator iter(this->Root);
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::Polygon);
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::Faceted);
  pqCMBSceneNode *node;
  while((node = iter.next()))
    {
    pqCMBTexturedObject *tobj = dynamic_cast<pqCMBTexturedObject*>(
      node->getDataObject());
    if(tobj && tobj->getBathymetrySource())
      {
      tobj->unApplyBathymetry();
      delete node;
      }
    }

  if (this->Root)
    {
    delete this->Root;
    this->Root = NULL;
    }
  this->SnapTarget = NULL;
  this->NameMap.clear();
  this->ObjectMap.clear();
  this->SourceMap.clear();
  this->TreeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
std::string pqCMBSceneTree::createUniqueName(const char *name) const
{
  if (this->NameMap.find(name) == this->NameMap.end())
    {
    return std::string(name);
    }
  int i = 1;
  QString base(name);
  base += "_";
  QString newName;
  QString suffix;
  while(1)
    {
    suffix.setNum(i);
    newName = base + suffix;
    if (this->NameMap.find(newName.toStdString()) == this->NameMap.end())
      {
      return newName.toStdString();
      }
    ++i;
    }
  return "";
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::createNode(const char *name, pqCMBSceneNode *parent,
                                         pqCMBSceneObjectBase *obj,
                                         cmbSceneNodeReplaceEvent *event)
{
  pqCMBSceneNode *node;
  // If we are adding a node with a scene object attached lets see
  // if we contain other scene objects (needed to know if we need
  // to emit the first data object signal
  bool needToSendSignal = false;
  if (obj)
    {
    needToSendSignal = !this->containsDataObjects();
    }
  std::string nname = this->createUniqueName(name);
  if (parent)
    {
    node = new pqCMBSceneNode(nname.c_str(), parent, obj);
    }
  else
    {
    node = new pqCMBSceneNode(nname.c_str(), this);
    }

  this->attachNode(node);

  if (this->RecordEvents)
    {
    if (!event)
      {
      cmbSceneNodeReplaceEvent *cevent = new cmbSceneNodeReplaceEvent(1,0);
      cevent->addCreatedNode(node);
      this->insertEvent(cevent);
      }
    else
      {
      event->addCreatedNode(node);
      }
    }

  if (obj && needToSendSignal)
    {
    emit firstDataObjectAdded();
    }

  return node;
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::attachNode(pqCMBSceneNode *node)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  // Is the node marked for deletion?
  if (node->isMarkedForDeletion())
    {
    node->unsetMarkedForDeletion();
    }

  if (obj)
    {
    this->ObjectMap[obj] = node;
    this->SourceMap[obj->getSelectionSource()] = node;
    obj->setPickable(this->DataObjectPickable);
    }
  this->NameMap[node->getName()] = node;
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::detachNode(pqCMBSceneNode *node)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  // Is the node marked for deletion?
  if (node->isMarkedForDeletion())
    {
    return;
    }

  //Unselect the node, which hides the InfoObject as well
  node->setSelected(false);

  if (this->SnapTarget == node)
    {
    this->SnapTarget = NULL;
    }

  node->setMarkedForDeletion();

  if (obj)
    {
    this->ObjectMap.erase(node->getDataObject());
    }

  this->NameMap.erase(node->getName());
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::createRoot(const char *name)
{
  if (this->Root)
    {
    // Should be an error
    return NULL;
    }

  // Creating the root should not be undoable
  bool recordingState = this->RecordEvents;
  this->RecordEvents = false;
  this->Root = this->createNode(name, NULL, NULL, NULL);
  this->RecordEvents = recordingState;
  this->TreeWidget->header()->setResizeMode(pqCMBSceneNode::getNameColumn(),
                                            QHeaderView::ResizeToContents);

  this->Root->setExplicitColor(this->InitialSceneObjectColor);
  this->Root->getWidget()->setSelected(true);
  this->nodesSelected();
  return this->Root;
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::findNode(const char *name) const
{
  std::map<std::string, pqCMBSceneNode *>::const_iterator i
    = this->NameMap.find(name);
  if (i == this->NameMap.end())
    {
    return NULL;
    }
  else
    {
    return i->second;
    }
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::findNode(pqCMBSceneObjectBase *obj) const
 {
  std::map<pqCMBSceneObjectBase *, pqCMBSceneNode *>::const_iterator i
    = this->ObjectMap.find(obj);
  if (i == this->ObjectMap.end())
    {
    return NULL;
    }
  else
    {
    return i->second;
    }
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::findNode(pqPipelineSource *obj) const
 {
  std::map<pqPipelineSource *, pqCMBSceneNode *>::const_iterator i
    = this->SourceMap.find(obj);
  if (i == this->SourceMap.end())
    {
    return NULL;
    }
  else
    {
    return i->second;
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::sceneObjectChanged()
{
  if (!this->EditMode)
    {
    emit requestSceneUpdate();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::nodesSelected(bool clearSelection/*=false*/)
{
  // if we're already blocking signals, we want to maintain that block
  // for this entire function call (caller will unblock)
  bool signalsAlreadyBlocked = this->TreeWidget->signalsBlocked();
  if (!signalsAlreadyBlocked)
    {
    this->TreeWidget->blockSignals(true);
    }

  // First unselect all nodes whose widgets are no longer selected
  // or whose has an ancestor that is
  int i, n = static_cast<int>(this->Selected.size());
  pqCMBSceneNode *node;
  QTreeWidgetItem *item;
  QList<pqCMBSceneNode *> unselected, newlySelected;

  for (i = 0; i < n; i++)
    {
    node = this->Selected[i];
    if (node->isSelected())
      {
      node->setSelected(false);
      unselected.push_back(node);
      //std::cerr << "Node unselected: " << node->getName() << "\n";
      continue;
      }

    // Is the node's ancestor's widget selected
    if  (node->isAncestorWidgetSelected())
      {
      node->setSelected(false);
      node->getWidget()->setSelected(false);
      unselected.push_back(node);
      //std::cerr << "Node unselected via Ancestor: " << node->getName() << "\n";
      continue;
      }
    }

  // OK We have processes what was currently selected - now we need
  // to process the new list
  this->Selected.clear();

  if (clearSelection)
    {
    this->TreeWidget->clearSelection();
    }

  QList<QTreeWidgetItem*> selItems = this->TreeWidget->selectedItems();

  n = selItems.size();
  for (i = 0; i < n; i++)
    {
    item = selItems.at(i);
    // If the item is no longer selected then it was due to the fact that
    // the list also contained the node's ancestor and was deselected in
    // the above for loop
    if (!item->isSelected())
      {
      //std::cerr << "Node no longer selected: " << node->getName() << "\n";
      continue;
      }

    node = pqCMBSceneNode::getNodeFromWidget(item);
    // If the node is currently selected there is nothing to be done
    if (node->isSelected())
      {
      this->Selected.push_back(node);
      //std::cerr << "Node still selected: " << node->getName() << "\n";
      continue;
      }

    // Does this node also have an ancestor that is also marked for selection?
    // If so we can skip it
    if  (node->isAncestorWidgetSelected())
      {
      item->setSelected(false);
      //std::cerr << "Node and Ancestor selected: " << node->getName() << "\n";
      continue;
      }

    // OK mark the node for selection
    node->setSelected(true);
    this->Selected.push_back(node);
    newlySelected.push_back(node);
    //std::cerr << "Node newly selected: " << node->getName() << "\n";
    }


  if (!signalsAlreadyBlocked)
    {
    this->TreeWidget->blockSignals(false);
    }

  bool enabled;
  if (this->InsertAction)
    {
    this->InsertAction->setEnabled((this->Selected.size() == 1));
    }

  if (this->ImportAction)
    {
    this->ImportAction->setEnabled((this->Selected.size() == 1));
    }

  if (this->ConvertToGlyphAction && this->TreeWidget->isEnabled())
    {
    this->ConvertToGlyphAction->setEnabled(this->Selected.size() > 0);
    }
  if (this->DeleteAction && this->TreeWidget->isEnabled())
    {
    this->DeleteAction->setEnabled(this->Selected.size() > 0);
    }
  if (this->ExportSolidsAction)
    {
    this->ExportSolidsAction->setEnabled((this->Selected.size()));
    }
  if (this->ExportPolygonsAction)
    {
    this->ExportPolygonsAction->setEnabled((this->Selected.size()));
    }

  if (this->DuplicateAction)
    {
    this->DuplicateAction->setEnabled(this->Selected.size() == 1 && (!this->Selected[0]->isArcNode()));
    }

  if (this->DuplicateRandomlyAction)
    {
    this->DuplicateRandomlyAction->setEnabled((this->Selected.size() == 1)
                                              && this->SnapTarget && (!this->Selected[0]->isArcNode()));
    }

  if (this->ConeCreateAction)
    {
    this->ConeCreateAction->setEnabled((this->Selected.size() == 1));
    }

  if (this->VOIAction)
    {
    this->VOIAction->setEnabled((this->Selected.size() == 1));
    }
  if (this->LineAction)
    {
    this->LineAction->setEnabled((this->Selected.size() == 1));
    }
  if (this->ArcAction)
    {
    enabled = !(( this->ArcWidgetManager && this->ArcWidgetManager->hasActiveNode() )
      || this->Selected.size() == 0 );
    this->ArcAction->setEnabled(enabled);
    }
  if(this->CreatePolygonAction)
    {
    enabled = !(( this->ArcWidgetManager && this->ArcWidgetManager->hasActiveNode() )
      || this->Selected.size() == 0 );
    this->CreatePolygonAction->setEnabled(enabled);
    }

  if (this->ChangeUserDefineObjectTypeAction)
    {
    this->ChangeUserDefineObjectTypeAction->setEnabled((this->Selected.size() == 1)
                                                       && (!this->Selected[0]->isTypeNode()));
    }

  if (this->ChangeNumberOfPointsLoadedAction)
    {
    enabled = false;
    if (this->Selected.size() == 1 && this->Selected[0]->getDataObject())
      {
      pqCMBPoints *pobj = dynamic_cast<pqCMBPoints*>(this->Selected[0]->getDataObject());

      if (pobj && pobj->getReaderSource())
        {
        enabled = true;
        }
      }
    this->ChangeNumberOfPointsLoadedAction->setVisible( enabled );
    }

  this->updateSnapOptions();
  this->updateSelectedColorMode();
  this->updateTexturedObjectOptions();
  this->updateBathymetryOptions();
  this->updateTINStitchOptions();
  this->updateTINStackOptions();
  this->updateGenerateArcsOptions();
  this->updateDefineVOIOption();
  this->updateEditConeOption();
  this->updateGroundPlaneOptions();
  this->updateArcOptions();
  emit selectionUpdated(&unselected,&newlySelected);
  this->sceneObjectChanged();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateSelectedColorMode()
{
  if (!this->SetNodeColorAction || !this->UnsetNodeColorAction)
    {
    return;
    }

  size_t i,n = this->Selected.size();
  bool state=false;
  for (i = 0; i < n; ++i)
    {
    if (this->Selected[i]->hasExplicitColor())
      {
      state = true;
      break;
      }
    }


  //if we have a color already set we can use that for
  //for the icon on the action.
  if (state)
    {
    //convert the color to a pixmap, and set that pixmap
    //to a QIcon
    double color[4];

    //we will use the first node that has a valid color
    this->Selected[i]->getColor(color);
    this->setColorNodeIcon(color);
    }
  else
    {
    this->clearColorNodeIcon();
    }
  this->SetNodeColorAction->setVisible(n>0);
  this->UnsetNodeColorAction->setVisible(state);
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setColorNodeIcon(double color[4])
{
  if (color[0] < 0)
    {
    //clear the icon.
    this->clearColorNodeIcon();
    return;
    }

  QColor c;c.setRgbF(color[0],color[1],color[2],color[3]);
  QPixmap px(128,128);
  px.fill(c);
  this->SetNodeColorAction->setIcon( QIcon(px) );
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::clearColorNodeIcon()
{
  this->SetNodeColorAction->setIcon(QIcon());
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateTexturedObjectOptions()
{
  bool enabled0 = false;
  bool showingElevation = false;
  if ((this->Selected.size() == 1) && (!this->Selected[0]->isTypeNode()))
    {
    pqCMBTexturedObject *tobj =
      dynamic_cast<pqCMBTexturedObject*>(this->Selected[0]->getDataObject());
    if (tobj)
      {
      enabled0 = true;
      showingElevation = tobj->showingElevation();
      }
    }
  if (this->ChangeTextureAction)
    {
    this->ChangeTextureAction->setEnabled(enabled0);
    }
  if (this->ElevationAction)
    {
    this->ElevationAction->setEnabled(enabled0);
    this->ElevationAction->setChecked(showingElevation);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateBathymetryOptions()
  {
  pqCMBTexturedObject *tobj = NULL;
  bool enabled0 = false;
  if ((this->Selected.size() == 1) && (!this->Selected[0]->isTypeNode()))
    {
    tobj = dynamic_cast<pqCMBTexturedObject*>(
      this->Selected[0]->getDataObject());
    if (tobj)
      {
      enabled0 = true;
      }
    }
  if (this->ApplyBathymetryAction)
    {
    this->ApplyBathymetryAction->setEnabled(enabled0);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateDefineVOIOption()
{
  if (this->DefineVOIAction)
    {
    if ((this->Selected.size() == 1) &&
        (!(this->Selected[0]->isTypeNode())) &&
        (this->Selected[0]->getDataObject()->getType() ==
         pqCMBSceneObjectBase::VOI))
      {
      this->DefineVOIAction->setVisible(true);
      }
    else
      {
      this->DefineVOIAction->setVisible(false);
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateEditConeOption()
{
  if (this->ConeEditAction)
    {
    if ((this->Selected.size() == 1) &&
        (!(this->Selected[0]->isTypeNode())) &&
        (this->Selected[0]->getDataObject()->getType() ==
         pqCMBSceneObjectBase::GeneralCone))
      {
      this->ConeEditAction->setVisible(true);
      }
    else
      {
      this->ConeEditAction->setVisible(false);
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateGroundPlaneOptions()
{
  if(!this->CreateGroundPlaneAction)
    {
    return;
    }
  this->CreateGroundPlaneAction->setEnabled(this->Selected.size() == 1);

  if (this->EditGroundPlaneAction)
    {
    if ((this->Selected.size() == 1) &&
        (!(this->Selected[0]->isTypeNode())) &&
        (this->Selected[0]->getDataObject()->getType() ==
         pqCMBSceneObjectBase::GroundPlane))
      {
      this->EditGroundPlaneAction->setVisible(true);
      }
    else
      {
      this->EditGroundPlaneAction->setVisible(false);
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateArcOptions()
{
  bool managerGood = (this->ArcWidgetManager &&
    this->ArcWidgetManager->getActiveNode()==NULL);
  size_t size = this->Selected.size();

  bool allAreArcs = true;
  for (size_t i=0; i<this->Selected.size()&&allAreArcs; ++i)
    {
    allAreArcs = this->Selected[i]->isArcNode();
    }
  this->ArcSnappingAction->setVisible(managerGood);

  bool en = managerGood && allAreArcs && size == 2;
  this->MergeArcsAction->setVisible(en);
  this->AutoConnectArcsAction->setVisible(en);

  en = managerGood && allAreArcs && size == 1;
  this->EditArcAction->setVisible(en);

  en = managerGood && allAreArcs && size >= 1;
  this->GrowArcSelectionAction->setVisible(en);
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateTINStitchOptions()
{
  // have to have at least 2 selected, and all must be of type TIN
  if (this->TINStitchAction)
    {
    size_t numTINs = 0;
    // Right now we will ignore Glyphs
// TODO: Glyph Support
    pqCMBFacetedObject *fobj;
    for (unsigned int i = 0; i < this->Selected.size(); i++)
      {
      fobj = dynamic_cast<pqCMBFacetedObject *>(this->Selected[i]->getDataObject());

      if (fobj && fobj->getSurfaceType() == pqCMBSceneObjectBase::TIN)
        {
        numTINs++;
        }
      else
        {
        break;
        }
      }
    this->TINStitchAction->setVisible(numTINs == this->Selected.size() && numTINs > 1);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateTINStackOptions()
{
  // have to have 1 TIN selected
  if (this->TINStackAction)
    {
    bool state = false;
    if (this->Selected.size() == 1)
      {
      pqCMBFacetedObject *fobj =
        dynamic_cast<pqCMBFacetedObject *>(this->Selected[0]->getDataObject());
      if (fobj && fobj->getSurfaceType() == pqCMBSceneObjectBase::TIN)
        {
        state = true;
        }
      }
    this->TINStackAction->setVisible(state);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateGenerateArcsOptions()
{
  if (this->GenerateArcsAction && this->GenerateImageMesh)
    {
    if ((this->Selected.size() == 1) &&
        (!(this->Selected[0]->isTypeNode())) &&
        (this->Selected[0]->getDataObject()->getType() ==
         pqCMBSceneObjectBase::UniformGrid))
      {
      this->GenerateArcsAction->setVisible(true);
      this->GenerateImageMesh->setVisible(true);
      }
    else
      {
      this->GenerateArcsAction->setVisible(false);
      this->GenerateImageMesh->setVisible(false);
      }

    }}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::nodeChanged(QTreeWidgetItem*item, int col)
{
  if (this->isEmpty())
    {
    return;
    }
  int nameColumn = pqCMBSceneNode::getNameColumn();
  if (col != nameColumn)
    {
    return;
    }
  pqCMBSceneNode *node = pqCMBSceneNode::getNodeFromWidget(item);
  this->changeName(node, item->text(nameColumn).toStdString().c_str());
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::deleteNode(pqCMBSceneNode *node,
                                cmbSceneNodeReplaceEvent *event)
{
  // Do not allow the Root to be deleted this way
  if (node == this->Root)
    {
    return;
    }

  this->detachNode(node);

  if (this->RecordEvents)
    {
    if (!event)
      {
      cmbSceneNodeReplaceEvent *devent = new cmbSceneNodeReplaceEvent(0,1);
      devent->addDeletedNode(node);
      this->insertEvent(devent);
      }
    else
      {
      event->addDeletedNode(node);
      }
    }
  else //Not caching delete
    {
    delete node;
    }
}

//-----------------------------------------------------------------------------
bool pqCMBSceneTree::getGlyphPoint(double p[3],
                                   std::deque<double> *glyphPoints,
                                   int glyphPlaybackOption,
                                   double bounds[6])
{
  if(glyphPlaybackOption == 0 || glyphPlaybackOption == -1)
    {
    p[0] = vtkMath::Random(bounds[0], bounds[1]);
    p[1] = vtkMath::Random(bounds[2], bounds[3]);
    p[2] = vtkMath::Random(bounds[4], bounds[5]);
    if(glyphPoints)
      {
      glyphPoints->push_back(p[0]);
      glyphPoints->push_back(p[1]);
      glyphPoints->push_back(p[2]);
      }
    }
  else
    {
    if(!glyphPoints || glyphPoints->empty())
      {
      //No points to import
      return false;
      }
    p[0] = glyphPoints->front();
    glyphPoints->pop_front();
    p[1] = glyphPoints->front();
    glyphPoints->pop_front();
    p[2] = glyphPoints->front();
    glyphPoints->pop_front();
    }
  return true;
}

//-----------------------------------------------------------------------------
void
pqCMBSceneTree::addPointsToGlyph(pqCMBSceneNode *node,
                                 int count,
                                 double *scaling,
                                 std::deque<double> *glyphPoints,
                                 bool repositionOriginal,
                                 QMap<pqCMBSceneNode *, int> *constraints,
                                 bool useTextureConstraint,
                                 int glyphPlaybackOption)
{
  pqCMBSceneObjectBase *target = NULL;
  pqCMBGlyphObject *gobj =
      dynamic_cast<pqCMBGlyphObject*>(node->getDataObject());
  // target will be set only if we are randomly placing objects and
  // there is a target surface to snap too
  pqCMBTexturedObject *textureObject = 0;
  if (this->SnapTarget != NULL)
    {
    target = this->SnapTarget->getDataObject();
    if (useTextureConstraint)
      {
      textureObject = dynamic_cast<pqCMBTexturedObject*>(target);
      }
    }
  double bounds[6];
  double p[3], sp[3];

  if (target)
    {
    target->getBounds(bounds);
    }

  int i;
  vtkIdType numPointsInGlyph = gobj->getNumberOfPoints();
  int numberOfFailedAttempts = 0, maxNumberOfFailedAttempts = count * 100;
  bool constraintPtFound = false;
  bool originalPointRepositioned = false;

  for (i = 0; i < count; i++)
    {
    if (target)
      {
      if (constraints && constraints->count()>0)
        {
        constraintPtFound = this->getRandomConstraintPoint(p, constraints, glyphPlaybackOption, glyphPoints);
        if(!constraintPtFound)
          {
          // No constraint point found
          break;
          }
        }
      else
        {
        if(!this->getGlyphPoint(p, glyphPoints, glyphPlaybackOption, bounds))
          {
          break;
          }
        }
      target->getClosestPoint(p, sp);
      if (textureObject)
        {
        if (vtkMath::Random(0.0, 1.0) < textureObject->getTextureIntensityAtPoint(sp))
          {
          if (++numberOfFailedAttempts >= maxNumberOfFailedAttempts)
            {
            break;
            }
          i--; // don't place at this point, try again
          continue;
          }
        }
      }
    else
      {
      p[0] = p[1] = p[2] = 0.0;
      }
    //If importing points, re-position first point in the glyph scene object
    if(i == 0 && (glyphPlaybackOption == 1) && repositionOriginal)
      {
      gobj->setPoint(0, sp);
      originalPointRepositioned = true;
      }
    else
      {
      gobj->insertNextPoint(sp);
      gobj->setScale(numPointsInGlyph, scaling);
      ++numPointsInGlyph;
      }
    }

  //Do not re-position original again if importing points
  if (target && repositionOriginal && (glyphPlaybackOption != 1))
    {
    // Add 100 to the max number of tries to take into consideration
    // we are moving one more object
    maxNumberOfFailedAttempts+= 100;
    while (numberOfFailedAttempts < maxNumberOfFailedAttempts)
      {
      if (constraints && constraints->count()>0)
        {
        this->getRandomConstraintPoint(p, constraints, glyphPlaybackOption, glyphPoints);
        }
      else
        {
        if(!this->getGlyphPoint(p, glyphPoints, glyphPlaybackOption, bounds))
          {
          break;
          }
        }
      target->getClosestPoint(p, sp);
      if ( textureObject &&
           vtkMath::Random(0.0, 1.0) < textureObject->getTextureIntensityAtPoint(sp) )
        {
        numberOfFailedAttempts++;
        continue;
        }
      gobj->setPoint(0, sp);
      break; // success
      }
    }
  //If no points from the playback file suffice the conditions,
  //replace the glyph scene object with a duplicate without the original point.
  if(glyphPlaybackOption == 1 && repositionOriginal && !originalPointRepositioned)
    {
    QMessageBox::critical(this->TreeWidget, "No node created", "None of the points in the playback file suffice the constraint conditions.");
    cmbSceneNodeReplaceEvent *event = new cmbSceneNodeReplaceEvent(0,1);
    this->deleteNode(node, event);
    this->nodesSelected();
    delete event;
    return;
    }

  // Force a render to make sure bounds are correct
  this->getCurrentView()->forceRender();

  // Do we have to update the parent node's selection info?
  if (node->getParent()->isSelected())
    {
    node->getParent()->unselect();
    this->nodesSelected();
    node->getParent()->select();
    }
}

//-----------------------------------------------------------------------------
std::vector<pqCMBSceneNode *>
pqCMBSceneTree::duplicateNode(pqCMBSceneNode *node,
                              std::deque<double> *glyphPoints,
                              int count,
                              bool randomPlacement,
                              bool repositionOriginal,
                              QMap<pqCMBSceneNode *, int> *constraints,
                              bool useGlyphs,
                              bool useTextureConstraint,
                              int glyphPlaybackOption)
{
  std::vector<pqCMBSceneNode *> nodes;
  // We don't allow for non-leaf nodes to be duplicated
  if (node->isTypeNode())
    {
    return nodes;
    }

  if (useGlyphs)
    {
    nodes.resize(1, NULL);
    }
  else if((glyphPlaybackOption == 1) && repositionOriginal)
    {
    nodes.resize(count-1, NULL);
    }
  else
    {
    nodes.resize(count, NULL);
    }

  pqCMBSceneObjectBase *nobj, *orig, *target = NULL;
  // target will be set only if we are randomly placing objects and
  // there is a target surface to snap too
  pqCMBTexturedObject *textureObject = 0;
  if (randomPlacement && (this->SnapTarget != NULL))
    {
    target = this->SnapTarget->getDataObject();
    if (useTextureConstraint)
      {
      textureObject = dynamic_cast<pqCMBTexturedObject*>(target);
      }
    }

  orig = node->getDataObject();

  double bounds[6];
  double p[3], sp[3];
  std::string name;

  if (target)
    {
    target->getBounds(bounds);
    }

  int i;
  double d[3];
  int numberOfFailedAttempts = 0, maxNumberOfFailedAttempts = count * 100;

  // If we are using glyphs we are only going to create 1 node not several
  pqCMBGlyphObject *gorig = dynamic_cast<pqCMBGlyphObject*>(orig);
  pqCMBFacetedObject *forig = dynamic_cast<pqCMBFacetedObject*>(orig);
  if (useGlyphs && (gorig || forig))
    {
    pqCMBGlyphObject *gobj;
    pqPipelineSource *gsource;
    std::string gname;
    if (gorig != NULL)
      {
      gsource = gorig->duplicateGlyphPipelineSource(this->CurrentServer);
      gname = gorig->getGlyphFileName();
      }
    else
      {
      gsource = forig->duplicatePipelineSource(this->CurrentServer);
      gname = forig->getFileName();
      }
    gobj = new pqCMBGlyphObject(gsource,
                                   this->CurrentView,this->CurrentServer,
                                   gname.c_str(), false);
    bool constraintPtFound = false;
    int numPointsImported = 0;
    for (i = 0; i < count; i++)
      {
      if (target)
        {
        if (constraints && constraints->count()>0)
          {
          constraintPtFound = this->getRandomConstraintPoint(p, constraints, glyphPlaybackOption, glyphPoints);
          if(!constraintPtFound)
            {
            break;
            }
          }
        else
          {
          if(!this->getGlyphPoint(p, glyphPoints, glyphPlaybackOption, bounds))
            {
            break;
            }
          }
        target->getClosestPoint(p, sp);
        if (textureObject)
          {
          if (vtkMath::Random(0.0, 1.0) < textureObject->getTextureIntensityAtPoint(sp))
            {
            if (++numberOfFailedAttempts >= maxNumberOfFailedAttempts)
              {
              break;
              }
            i--; // don't place at this point, try again
            continue;
            }
          }
        }
      else
        {
        p[0] = p[1] = p[2] = 0.0;
        }
      gobj->insertNextPoint(sp);
      numPointsImported++;
      if(gorig != NULL)
        {
        gorig->getScale(i,d);
        gobj->setScale(i,d);
        gorig->getOrientation(i,d);
        gobj->setOrientation(i,d);
        }
      }
    gobj->copyAttributes(orig);
    name = node->getName();
    name += " Copy";
    //If no points from the playback file suffice the conditions,
    //replace the glyph scene object with a duplicate without the original point.
    if(glyphPlaybackOption == 1 && numPointsImported == 0)
      {
      QMessageBox::critical(this->TreeWidget, "No node created", "None of the points in the playback file suffice the constraint conditions.");
      delete gobj;
      nodes.clear();
      }
    else
      {
      nodes[0] = this->createNode(name.c_str(), node->getParent(), gobj, NULL);
      }
    if(!nodes.empty())
      {
      if (node->hasExplicitColor())
        {
        double color[4];
        node->getColor(color);
        nodes[0]->setExplicitColor(color);
        }
      nodes[0]->getDataObject()->updateRepresentation();
      }
    }
  else
    {
    bool hasExplicitColor = node->hasExplicitColor();
    // Stuff needed in the case of glyphs
    double aveGpoint[3], gpoint[3];
    pqCMBGlyphObject *ngobj;
    double color[4];
    if (gorig)
      {
      gorig->getAveragePoint(aveGpoint);
      }
   if (hasExplicitColor)
      {
      node->getColor(color);
      }

    cmbSceneNodeReplaceEvent *cevent = NULL;
    if (this->RecordEvents)
      {
      cevent = new cmbSceneNodeReplaceEvent(count, 0);
      this->insertEvent(cevent);
      }

    bool constraintPtFound = false;
    int numPointsImported = 0;
    for (i = 0; i < count; i++)
      {
      if (target)
        {
        if (constraints && constraints->count()>0)
          {
          constraintPtFound = this->getRandomConstraintPoint(p, constraints, glyphPlaybackOption, glyphPoints);
          if(!constraintPtFound)
            {
            break;
            }
          }
        else
          {
          if(!this->getGlyphPoint(p, glyphPoints, glyphPlaybackOption, bounds))
            {
            break;
            }
          }
        target->getClosestPoint(p, sp);
        if (textureObject)
          {
          if (vtkMath::Random(0.0, 1.0) < textureObject->getTextureIntensityAtPoint(sp))
            {
            if (++numberOfFailedAttempts >= maxNumberOfFailedAttempts)
              {
              break;
              }
            i--; // don't place at this point, try again
            continue;
            }
          }
        }
      numPointsImported++;
      nobj = orig->duplicate(this->CurrentServer, this->CurrentView);
      if(nobj->getType() == pqCMBSceneObjectBase::Line)
        {
        this->setLineWidgetCallbacks(static_cast<pqCMBLine*>(nobj));
        }
      if (target)
        {
        if(gorig)
          {
          ngobj = dynamic_cast<pqCMBGlyphObject*>(nobj);
          int j, gcount = ngobj->getNumberOfPoints();
          for (j = 0; j < gcount; j++)
            {
            ngobj->getPoint(j, p);
            // calulate the delta between the average point of the
            // entire glyph and the new random point and apply
            // the translation to the glyph point and then snap it
            gpoint[0] = p[0] + sp[0] - aveGpoint[0];
            gpoint[1] = p[1] + sp[1] - aveGpoint[1];
            gpoint[2] = p[2] + sp[2] - aveGpoint[2];
            target->getClosestPoint(gpoint, p);
            ngobj->setPoint(j, p);
            }
          }
        else
          {
          nobj->setPosition(sp);
          }
        }
      if((glyphPlaybackOption == 1) && (i == 0) && repositionOriginal)
        {
        nobj->getPosition(sp);
        orig->setPosition(sp);
        delete nobj;
        }
      else
        {
        int l = (glyphPlaybackOption == 1) && repositionOriginal ? i-1 : i;
        name = node->getName();
        name += " Copy";
        nodes[l] = this->createNode(name.c_str(), node->getParent(), nobj, cevent);
        if (hasExplicitColor)
          {
          nodes[l]->setExplicitColor(color);
          }
        }
      }
    //Need to handle case when importing points - no point sufficed the constraint conditions
    if((glyphPlaybackOption == 1) && (numPointsImported == 0) && repositionOriginal)
      {
      QMessageBox::critical(this->TreeWidget, "No node created", "None of the points in the playback file suffice the constraint conditions.");
      cmbSceneNodeReplaceEvent *event = new cmbSceneNodeReplaceEvent(0,1);
      this->deleteNode(node, event);
      this->nodesSelected();
      delete event;
      return nodes;
      }
    }

  if (target && repositionOriginal && (glyphPlaybackOption != 1))
    {
    // Add 100 to the max number of tries to take into consideration
    // we are moving one more object
    maxNumberOfFailedAttempts+= 100;
    while (numberOfFailedAttempts < maxNumberOfFailedAttempts)
      {
      if (constraints && constraints->count()>0)
        {
        this->getRandomConstraintPoint(p, constraints, glyphPlaybackOption, glyphPoints);
        }
      else
        {
        if(!this->getGlyphPoint(p, glyphPoints, glyphPlaybackOption, bounds))
          {
          break;
          }
        }
      target->getClosestPoint(p, sp);
      if ( textureObject &&
        vtkMath::Random(0.0, 1.0) < textureObject->getTextureIntensityAtPoint(sp) )
        {
        numberOfFailedAttempts++;
        continue;
        }
      orig->setPosition(sp);
      break; // success
      }
    }

  // Force a render to make sure bounds are correct
  this->getCurrentView()->forceRender();

  // Do we have to update the parent node's selection info?
  if (node->getParent()->isSelected())
    {
    node->getParent()->unselect();
    this->nodesSelected();
    node->getParent()->select();
    }
  return nodes;
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *pqCMBSceneTree::duplicateNode(pqCMBSceneNode *node,
                                    double zOffset)
{
  // We don't allow for non-leaf nodes to be duplicated
  if (node->isTypeNode())
    {
    return NULL;
    }

  pqCMBSceneObjectBase *nobj, *orig;
  pqCMBSceneNode *copy;
  double p[3];
  std::string name;

  orig = node->getDataObject();
  orig->getPosition(p);
  //Apply the offset
  p[2] += zOffset;

  nobj = orig->duplicate(this->CurrentServer, this->CurrentView);
  if(nobj->getType() == pqCMBSceneObjectBase::Line)
    {
    this->setLineWidgetCallbacks(static_cast<pqCMBLine*>(nobj));
    }
  nobj->setPosition(p);
  name = node->getName();
  name += " Copy";
  copy = this->createNode(name.c_str(), node->getParent(), nobj, NULL);
  return copy;
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::changeName(pqCMBSceneNode *node, const char *newName)
{
  if (node->getName() == newName)
    {
    return;
    }
  this->NameMap.erase(node->getName());
  node->changeName(newName);
  this->NameMap[node->getName()] = node;
  emit nodeNameChanged(node);
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::nodeClicked(QTreeWidgetItem*item, int col)
{
  if (col == pqCMBSceneNode::getNameColumn())
    {
    return;
    }
  pqCMBSceneNode *node = pqCMBSceneNode::getNodeFromWidget(item);
  if (col == pqCMBSceneNode::getVisibilityColumn())
    {
    node->toggleVisibility();
    if (!node->isVisible())
      {
      node->unselect();
      }
    else
      {
      pqCMBLine* lineObj = dynamic_cast<pqCMBLine*>(
        node->getDataObject());
      if(lineObj && lineObj->getLineWidget()
         && lineObj->getLineWidget()->widgetVisible())
        {
        lineObj->select();
        }
      }
    }
  else if (col == pqCMBSceneNode::getLockColumn())
    {
    node->toggleLocked();
    emit lockedNodesChanged();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::deleteSelected()
{

  // Since deleting Arcs can cause Polygons to be deleted - lets go through the
  // list once and move the arc nodes to a different list
  std::vector<pqCMBSceneNode *>arcs;
  pqCMBSceneObjectBase *obj;
  const int n = static_cast<int>(this->Selected.size());
  cmbSceneNodeReplaceEvent *event = new cmbSceneNodeReplaceEvent(0,n);
  this->insertEvent(event);
  arcs.reserve(n);
  for (int i = 0; i < n; i++)
    {
    // You can not delete the root
    if (this->Selected[i] == this->Root)
      {
      continue;
      }
    
    obj = this->Selected[i]->getDataObject();
    if (obj && (obj->getType() ==  pqCMBSceneObjectBase::Arc))
      {
      // Skip if the active node is being edited
      if ((!this->ArcWidgetManager) ||
          (this->ArcWidgetManager->getActiveNode() != this->Selected[i]))
        {
        arcs.push_back(this->Selected[i]);
        }
      }
    else
      {
      this->deleteNode(this->Selected[i], event);
      }
    }

  std::set<pqCMBPolygon*> polygons;
  typedef std::vector<pqCMBSceneNode*>::iterator arc_it;
  for(arc_it it= arcs.begin(); it != arcs.end(); ++it)
    {
    // See if there are polygons attached to the arc
    pqCMBArc* arc = dynamic_cast<pqCMBArc*>((*it)->getDataObject());
    polygons.insert( arc->polygonsUsingArc().begin(),
                     arc->polygonsUsingArc().end());
    }

  if(polygons.size() == 0)
    {
    //we have no polygons so just delete
    pqCMBSceneNode* arcNode;
    foreach(arcNode,arcs)
      {
      this->deleteNode(arcNode, event);
      }
    }
  else
    {
    //we have polygons so we need to prompt first
    QString text = "Some of the selected Arcs are connected to Polygons which will also need to be deleted.";
    QMessageBox msg;
    msg.setText(text);
    msg.setInformativeText("Delete both Arcs and Polygons?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    int ret = msg.exec();
    if ( ret == QMessageBox::Yes )
      {
      pqCMBPolygon* poly;
      foreach(poly,polygons)
        {
        this->deleteNode(this->ObjectMap[poly], event);
        }
      pqCMBSceneNode* arcNode;
      foreach(arcNode,arcs)
        {
        this->deleteNode(arcNode, event);
        }
      }
    }
  //Lets select the root after the deletion
  this->Selected.clear();
  this->Root->getWidget()->setSelected(true);
  this->nodesSelected();

  this->nodesSelected();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::insertTypeNode()
{
  if (this->Selected.size() != 1)
    {
    return;
    }

  pqCMBSceneNode *parent = this->Selected[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }
  this->createNode("New Node", parent, NULL, NULL);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::importObject()
{
  if (this->Selected.size() != 1)
    {
    return;
    }

  pqCMBSceneNode *parent = this->Selected[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }
  bool randomPlacement, translateBasedOnView, useTextureConstraint, useGlyphs;
  int count, glyphPlaybackOption;
  QMap<pqCMBSceneNode*, int> constraints;
  QString glyphPlaybackFilename;
  bool enableRandomPlacement = false, enableTextureConstraintPlacement = false;
  if (this->SnapTarget != NULL)
    {
    enableRandomPlacement = true;
    pqCMBTexturedObject *target =
      dynamic_cast<pqCMBTexturedObject*>(this->SnapTarget->getDataObject());
    if (target)
      {
      enableTextureConstraintPlacement = target->hasTexture();
      }
    }

  pqCMBSceneNode *node =
    qtCMBSceneObjectImporter::importNode(parent,
                                         enableRandomPlacement,
                                         enableTextureConstraintPlacement,
                                         &randomPlacement,
                                         &translateBasedOnView,
                                         &count, constraints,
                                         useGlyphs,
                                         useTextureConstraint,
                                         this->useGlyphPlayback,
                                         glyphPlaybackOption,
                                         glyphPlaybackFilename);
  if (!node)
    {
    return;
    }

  if (node->getDataObject())
    {
    this->setNewObjectPosition(node, randomPlacement, translateBasedOnView,
                               count - 1, &constraints, glyphPlaybackOption,
                               glyphPlaybackFilename, useTextureConstraint);
    }
  else // node created with children nodes containing data objects
    {
    std::vector<pqCMBSceneNode *>::const_iterator child;
    for (child = node->getChildren().begin(); child != node->getChildren().end(); child++)
      {
      if ((*child)->getDataObject())
        {
        this->setNewObjectPosition(*child, randomPlacement,
          translateBasedOnView, count - 1, &constraints,
          glyphPlaybackOption, glyphPlaybackFilename);
        }
      // for now, don't handle anything other than children, since that is
      // the most we expect from import (LIDAR data)
      }
    }

  node->getWidget()->setSelected(true);
  this->nodesSelected();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setNewObjectPosition(pqCMBSceneNode *node, bool randomPlacement,
                                          bool translateBasedOnView, int repeatCount,
                                          QMap<pqCMBSceneNode*, int> *constraints,
                                          int glyphPlaybackOption,
                                          QString glyphPlaybackFilename,
                                          bool useTextureConstraint)
{
  double pos[3] = {0, 0, 0};
  double scaleFactor[3];
  // See if we have a Glyph Object
  pqCMBGlyphObject *gobj =
    dynamic_cast<pqCMBGlyphObject*>(node->getDataObject());
  bool usingGlyphs = false;
  std::deque<double>* glyphPoints = new std::deque<double>();

  //If importing points, read them to the vector
  if(glyphPlaybackOption == 1)
    {
    if(glyphPlaybackFilename.isEmpty())
      {
      QMessageBox::critical(this->TreeWidget, "Glyph Playback File", "Please provide a file to import points from.");
      return;
      }
    ifstream glyphPlaybackFile(glyphPlaybackFilename.toAscii().data());
    if(!glyphPlaybackFile.good())
      {
      QMessageBox::critical(this->TreeWidget, "Glyph Playback File", "Error opening glyph playback file. Check the file and try again.");
      return;
      }
    double p;
    while(!glyphPlaybackFile.eof())
      {
      glyphPlaybackFile >> p;
      //Manage last new line character
      if(glyphPlaybackFile.eof())
        {
        break;
        }
      glyphPoints->push_back(p);
      }
    if((glyphPoints->size() % 3) != 0)
      {
      QMessageBox::critical(this->TreeWidget, "Glyph Playback File", QString(tr("Invalid point format. Read %1 values that does not give a valid set of 3D points")).arg(glyphPoints->size()));
      return;
      }
    repeatCount = glyphPoints->size()/3;
    }

  if (gobj)
    {
    usingGlyphs = true;
    // In the case of glyphs, the importObject method will set
    // a scale directly on the object - instead this needs to be
    // applied to the individual glyphs
    gobj->getScale(scaleFactor);
    double dummy[3] = {1.0, 1.0, 1.0};
    gobj->setScale(dummy);
    gobj->setScale(0, scaleFactor);
    }
  // Do we have a snap target?
  if (this->SnapTarget)
    {
    double p1[3];
    if (!randomPlacement)
      {
      if (translateBasedOnView)
        {
        pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, p1);
        this->SnapTarget->getDataObject()->getClosestPoint(p1, pos);
        if (usingGlyphs)
          {
          gobj->setPoint(0, pos);
          }
        else
          {
          node->getDataObject()->setPosition(pos);
          }
        }
      }
    else
      {
      if (usingGlyphs)
        {
        this->addPointsToGlyph(node, repeatCount, scaleFactor, glyphPoints, true, constraints,
                               useTextureConstraint, glyphPlaybackOption);
        }
      else
        {
        this->duplicateNode(node, glyphPoints, repeatCount,
                            true, true, constraints, false,
                            useTextureConstraint, glyphPlaybackOption);
        }
      }
    }
  else
    {
    if (translateBasedOnView)
      {
      pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, pos);
      }
    if (usingGlyphs)
      {
      gobj->setPoint(0, pos);
      }
    else
      {
      node->getDataObject()->setPosition(pos);
      }
    }
  if(glyphPlaybackOption == 0)
    {
    if(!glyphPlaybackFilename.isEmpty())
      {
      ofstream glyphPlaybackFile( glyphPlaybackFilename.toAscii().data() );
      std::deque<double>::iterator iter;
      for(iter = glyphPoints->begin(); iter != glyphPoints->end(); iter = iter+3)
        {
        glyphPlaybackFile << *iter << " " << *(iter+1) << " " << *(iter+2) << "\n";
        }
      glyphPlaybackFile.close();
      }
    }
  delete glyphPoints;
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::createVOI()
{
  if (this->Selected.size() != 1)
    {
    return;
    }

  pqCMBSceneNode *parent = this->Selected[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }
  double bounds[6];
  double pos[3];

  // Do we have a snap target?
  if (this->SnapTarget)
    {
    double p1[3];
    pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, p1);
    this->SnapTarget->getDataObject()->getClosestPoint(p1, pos);
    }
  else
    {
    pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, pos);
    }

  bounds[0] = bounds[2] = bounds[4] = -1.0;
  bounds[1] = bounds[3] = bounds[5] = 1.0;
  pqCMBSceneObjectBase *obj =
    new pqCMBVOI(pos, bounds,
                    this->CurrentServer, this->CurrentView);
  pqCMBSceneNode *node = this->createNode("New VOI", parent, obj, NULL);
  this->Selected[0]->getWidget()->setSelected(false);
  node->getWidget()->setSelected(true);
  this->nodesSelected();
}
//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::createBoreholeObject(pqPipelineSource* source,
  pqCMBSceneNode *parent)
{
  if (!source || !parent)
    {
    return NULL;
    }

  vtkNew<vtkPVSceneGenObjectInformation> info;
  source->getProxy()->GatherInformation(info.GetPointer());

  pqCMBSceneObjectBase *obj =
    new pqCMBBoreHole(source, this->CurrentView, this->CurrentServer);
  pqCMBSceneNode *node = this->createNode(info->GetObjectName(),
     parent, obj, NULL);
  //parent->getWidget()->setSelected(false);
  //node->getWidget()->setSelected(true);
  node->setIsLocked(true);
  return node;
}
//----------------------------------------------------------------------------
int pqCMBSceneTree::createBorFileObjects(
  const QString& filename, pqPipelineSource* source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  // force read
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();

  source->getProxy()->UpdateVTKObjects();
  source->getProxy()->UpdatePropertyInformation();

  int numberOfBoreholes=pqSMAdaptor::getElementProperty(
    source->getProxy()->GetProperty("NumberOfBoreholes")).toInt();
  int numberOfCrossSections=pqSMAdaptor::getElementProperty(
    source->getProxy()->GetProperty("NumberOfCrossSections")).toInt();
  if(numberOfBoreholes<=0)
    {
    QMessageBox::warning(this->getWidget(), "Borehole File Error",
      "There are no boreholes founds.");
    return 0;
    }

   //if the root node hasn't been create, create it
  if ( this->getRoot() == NULL )
    {
    this->createRoot("Geology");
    }
  pqCMBSceneNode *parent = (this->getSelected().size() != 1) ?
    this->getRoot() : this->getSelected()[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }
  parent->getWidget()->setExpanded(true);
  std::string geoNodeName =vtksys::SystemTools::GetFilenameName(
    filename.toStdString() ).c_str();
  pqCMBSceneNode* geoNode = this->createNode(
    geoNodeName.c_str(), parent, NULL, NULL);
  geoNode->getWidget()->setExpanded(true);
  pqCMBSceneNode* bhNode = this->createNode(
    "Bore Holes", geoNode, NULL, NULL);
  pqCMBSceneNode* csNode = this->createNode(
    "Cross Sections", geoNode, NULL, NULL);

  QList<QVariant> indices;
  //create the filter that extracts boreholes and cross-sections
  QProgressDialog progress("", "Abort Load", 0,
    numberOfBoreholes+numberOfCrossSections, this->getWidget());
  progress.setWindowModality(Qt::WindowModal);
  // vtkExtractBlock filter always assume block 0 is itself
  int bix;
  if(numberOfBoreholes>0)
    {
    // Loading boreholes
    pqPipelineSource* extractBH = builder->createFilter("filters",
      "ExtractBlock", source);
    progress.setWindowTitle("Loading Boreholes");
    indices << 1;
    pqSMAdaptor::setElementProperty(
      extractBH->getProxy()->GetProperty("MaintainStructure"), 1);
    pqSMAdaptor::setMultipleElementProperty(
      extractBH->getProxy()->GetProperty("BlockIndices"), indices);
    extractBH->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extractBH->getProxy() )->UpdatePipeline();
    for(bix=0; bix<numberOfBoreholes; bix++)
      {
      progress.setValue(bix);
      if ( progress.wasCanceled() )
        {
        break;
        }

      pqPipelineSource* extract = builder->createFilter("filters",
        "ExtractLeafBlock", extractBH);
      pqSMAdaptor::setElementProperty(
        extract->getProxy()->GetProperty("BlockIndex"), bix);
      extract->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();
      pqPipelineSource *pdSource = builder->createSource("sources",
        "HydroModelPolySource", this->CurrentServer);
      vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
        vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
      pdSource->updatePipeline();
      builder->destroy(extract);
      this->createBoreholeObject(pdSource, bhNode);
      }
    builder->destroy(extractBH);
    bhNode->getWidget()->setExpanded(true);
    }

  if(numberOfCrossSections>0)
    {
    // loading Cross Sections
    indices.clear();
    // The flat index
    indices << (numberOfBoreholes+2); // root + boreholeblock + numberborholes
    pqPipelineSource* extractCS = builder->createFilter("filters",
      "ExtractBlock", source);
    pqSMAdaptor::setElementProperty(
      extractCS->getProxy()->GetProperty("MaintainStructure"), 1);
    pqSMAdaptor::setMultipleElementProperty(
      extractCS->getProxy()->GetProperty("BlockIndices"), indices);
    extractCS->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extractCS->getProxy() )->UpdatePipeline();
    progress.setWindowTitle("Loading CrossSections");
    for(int cix=0; cix<numberOfCrossSections; cix++)
      {
      progress.setValue(bix+cix);
      if ( progress.wasCanceled() )
        {
        break;
        }

      pqPipelineSource* extract = builder->createFilter("filters",
        "ExtractLeafBlock", extractCS);
      pqSMAdaptor::setElementProperty(
        extract->getProxy()->GetProperty("BlockIndex"), cix);
      extract->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();
      pqPipelineSource *pdSource = builder->createSource("sources",
        "HydroModelPolySource", this->CurrentServer);
      vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
        vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
      pdSource->updatePipeline();
      builder->destroy(extract);
      this->createCrossSectionObject(pdSource, csNode);
      }
    builder->destroy(extractCS);
    csNode->getWidget()->setExpanded(true);
    }
  this->clearSelection();
  return (numberOfBoreholes+numberOfCrossSections);
}
//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::createCrossSectionObject(pqPipelineSource* source,
  pqCMBSceneNode* parent)
{
  if (!source || !parent)
    {
    return NULL;
    }

  vtkNew<vtkPVSceneGenObjectInformation> info;
  source->getProxy()->GatherInformation(info.GetPointer());

  pqCMBSceneObjectBase *obj =
    new pqCMBCrossSection(source, this->CurrentView, this->CurrentServer);
  pqCMBSceneNode *node = this->createNode(info->GetObjectName(),
    parent, obj, NULL);
  //parent->getWidget()->setSelected(false);
  //node->getWidget()->setSelected(true);
  node->setIsLocked(true);
  return node;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBSceneTree::getACrossSectionRepresentation()
{
  pqCMBSceneNode *node;
  pqCMBCrossSection *csObj;
  SceneObjectNodeIterator iter(this->getRoot());
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::GeoCrossSection);

  while((node = iter.next()))
    {
    csObj = dynamic_cast<pqCMBCrossSection*>(node->getDataObject());
    if(csObj)
      {
      return csObj->getRepresentation();
      }
    }
  return NULL;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBSceneTree::getABoreHoleRepresentation()
{
  pqCMBSceneNode *node;
  pqCMBBoreHole *bhObj;
  SceneObjectNodeIterator iter(this->getRoot());
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::GeoBoreHole);

  while((node = iter.next()))
    {
    bhObj = dynamic_cast<pqCMBBoreHole*>(node->getDataObject());
    if(bhObj)
      {
      return bhObj->getRepresentation();
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setBoreHolesRadius(double radius)
{
  pqCMBSceneNode *node;
  pqCMBBoreHole *bhObj;
  SceneObjectNodeIterator iter(this->getRoot());
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::GeoBoreHole);

  while((node = iter.next()))
    {
    bhObj = dynamic_cast<pqCMBBoreHole*>(node->getDataObject());
    if(bhObj)
      {
      bhObj->setTubeRadius(radius);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::createCone()
{
  if (this->Selected.size() != 1)
    {
    return;
    }

  pqCMBSceneNode *parent = this->Selected[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }
  double pos[3], dir[3];

  // The cone is pointed down the z-axis
  dir[0] = dir[1] = 0.0;
  dir[2] = -1.0;

  // Do we have a snap target?
  if (this->SnapTarget)
    {
    double p1[3];
    pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, p1);
    this->SnapTarget->getDataObject()->getClosestPoint(p1, pos);
    }
  else
    {
    pqCMBSceneObjectBase::getCameraFocalPoint(this->CurrentView, pos);
    }

  pqCMBSceneObjectBase *obj =
    new pqCMBConicalRegion(pos, 5.0, 5.0, 0.0, dir, 10,
                              this->CurrentServer, this->CurrentView);
  pqCMBSceneNode *node = this->createNode("New Conical Solid", parent, obj, NULL);
  this->Selected[0]->getWidget()->setSelected(false);
  node->getWidget()->setSelected(true);
  qtCMBConeNodeDialog::manageCone(node);
  this->nodesSelected();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::editCone()
{
  if ((this->Selected.size() == 1) &&
      (!(this->Selected[0]->isTypeNode())) &&
      (this->Selected[0]->getDataObject()->getType() ==
       pqCMBSceneObjectBase::GeneralCone))
    {
    pqCMBSceneNode *n = this->Selected[0];
    if (qtCMBConeNodeDialog::manageCone(n))
      {
      // Toggle the node's selection to make sure everyone knows its bounds
      // has changed
      this->clearSelection();
      n->select();
      this->TreeWidget->setCurrentItem(n->getWidget());
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::createLineObject()
{
  pqCMBSceneNode *node;
  double bounds[6];
  pqCMBLine::getDefaultBounds(this->CurrentView, bounds);

  if(!this->Selected.size())
    {
    node = this->createLineNode(bounds);
    }
  else
    {
    for(unsigned int i=0; i<this->Selected.size(); i++)
      {
      if(this->Selected[i]->getDataObject())
        {
        this->Selected[i]->getDataObject()->getBounds(bounds);
        }
      node = this->createLineNode(bounds);
      }
    }

  this->Selected[0]->getWidget()->setSelected(false);
  node->getWidget()->setSelected(true);
//  this->nodesSelected();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::createArcObject()
{
  if ( !this->ArcWidgetManager )
    {
    this->createArcWidgetManager();
    }

  //clear the selection so we don't have multiple items selected causing
  //the display tab to not work
  this->nodesSelected(true);

  //create the object so that we can get the widget into the main window interface
  pqCMBArc *obj = this->ArcWidgetManager->createpqCMBArc();

  //set the node up so we can change things
  pqCMBSceneNode *node = this->createNode("New Arc",
                                        this->getArcTypeNode(), obj, NULL);
  this->ArcWidgetManager->setActiveNode( node );

  this->ArcWidgetManager->create();

  //this is needed so that the display tab is up to date.
  if (this->Selected.size() > 0 )
    {
    this->Selected[0]->getWidget()->setSelected(false);
    }
  node->getWidget()->setSelected(true);

  this->nodesSelected();
}

//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::createLineNode(double bounds[6])
{
  double pos1[3], pos2[3];
  pos1[0]=bounds[0];pos1[1]=bounds[2];pos1[2]=bounds[4];
  pos2[0]=bounds[1];pos2[1]=bounds[3];pos2[2]=bounds[5];

  pqCMBLine *obj =
    new pqCMBLine(pos1, pos2,
                     this->CurrentServer, this->CurrentView);
  this->setLineWidgetCallbacks(obj);
  return this->createNode("New Line", this->getLineTypeNode(), obj, NULL);
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::createPolygonObject()
{
  //collect all the sources that are input for the polygon filter
  std::vector<pqCMBSceneNode*> inputNodes;
  for ( size_t i = 0; i < this->Selected.size(); ++i)
    {
    //add the selected item only if it is a polyline
    //Root object have a null data object, so we have to check for that
    //edge case.
    pqCMBSceneObjectBase *csObj = this->Selected[i]->getDataObject();
    if (csObj != NULL && csObj->getType() ==pqCMBSceneObjectBase::Arc)
      {
      //we have a small problem in that we can't create the polygon dataobject
      //without first having all it's inputs. But each input wants a reference
      //to the polygon that is creating it ( so we store the Arcs, so that
      //if the polygon ever returns a mask of which Arcs it doesn't need we
      //can update the relationships)
      inputNodes.push_back(this->Selected[i]);
      }
    }

  if ( inputNodes.size()==0)
    {
    //we can't create a polygon from the selection
    return;
    }

  //create the polygon filter client item
  //create a dialog for meshing constraints
  double minAngle=0,edgeLength=0;

  //spawn the dialog asking for the meshing controls
  QDialog mainDialog;
  Ui::qtCMBSceneArcPolygonMeshingDialog meshingDialog;
  meshingDialog.setupUi(&mainDialog);
  mainDialog.setModal(true);
  if (mainDialog.exec() == QDialog::Accepted)
    {
    if(meshingDialog.groupBox->isChecked())
      {
      minAngle = meshingDialog.MinAngle->text().toDouble();
      edgeLength = meshingDialog.EdgeLength->text().toDouble();
      }
    }
  else
    {
    //user cancelled dialog don't mesh
    return;
    }

  pqCMBPolygon* obj = new pqCMBPolygon(minAngle,edgeLength,inputNodes);
  if (obj->isValidPolygon())
    {
    //figure out the parent node that this polygon should be under
    pqCMBSceneNode *parentNode = (this->Selected[0]->Parent)
      ? this->Selected[0]->Parent : this->Selected[0];
    pqCMBSceneNode *node = this->createNode("New Polygon", parentNode, obj, NULL);

    //add the item to the QTreeWidget
    this->Selected[0]->getWidget()->setSelected(false);
    node->getWidget()->setSelected(true);
    }
  else
    {
    //faild to make a polygon
    QString text = "Unable to make a polygon, make sure the arcs form a loop";
    QMessageBox msg;
    msg.setText(text);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
    delete obj;
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::stitchTINs()
{
  emit requestTINStitch();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::stackTINs()
{
  qtCMBStackedTINDialog::processTIN(this->Selected[0]);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::generateArcsFromImage()
{
  if ((this->Selected.size() == 1) &&
      (!(this->Selected[0]->isTypeNode())) &&
      (this->Selected[0]->getDataObject()->getType() ==
       pqCMBSceneObjectBase::UniformGrid))
    {
    qtCMBGenerateContoursDialog generateContoursDlg(this->Selected[0]);
    generateContoursDlg.exec();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::generateMeshFromImage()
{
  if ((this->Selected.size() == 1) &&
      (!(this->Selected[0]->isTypeNode())) &&
      (this->Selected[0]->getDataObject()->getType() ==
       pqCMBSceneObjectBase::UniformGrid))
    {
    pqCMBSceneNode *gridNode = this->Selected[0];
    pqCMBUniformGrid *grid =
      dynamic_cast<pqCMBUniformGrid *>(gridNode->getDataObject());
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    pqPipelineSource* mesh = builder->createFilter("filters", "cmbStructedToMesh", grid->getImageSource());
    vtkSMPropertyHelper(mesh->getProxy(), "UseScalerForZ").Set(1);
    pqCMBFacetedObject * obj = new pqCMBFacetedObject(mesh,
                                                            this->CurrentServer,
                                                            this->CurrentView );
    this->createNode("Meshed Image", gridNode->getParent(), obj, NULL);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::duplicateSelected()
{
  if ((this->Selected.size() != 1) || this->Selected[0]->isTypeNode())
    {
    return;
    }

  std::vector<pqCMBSceneNode*> nodes = this->duplicateNode(this->Selected[0]);
  int i, n = static_cast<int>(nodes.size());
  this->TreeWidget->blockSignals(true);
  if (!n)
    {
    return;
    }
  this->Selected[0]->getWidget()->setSelected(false);
  for (i = 0; i < n; i++)
    {
    nodes[i]->getWidget()->setSelected(true);
    }
  this->TreeWidget->blockSignals(false);
  this->nodesSelected();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::duplicateSelectedRandomly()
{
  if ((this->Selected.size() != 1) || this->Selected[0]->isTypeNode())
    {
    return;
    }

  int count;
  QMap<pqCMBSceneNode*, int> constraints;
  pqCMBSceneObjectBase *obj = this->Selected[0]->getDataObject();
  bool enableGlyphing = (dynamic_cast<pqCMBGlyphObject*>(obj) != NULL) ||
    (dynamic_cast<pqCMBFacetedObject*>(obj) != NULL);
  bool useGlyphs, useTextureConstraint;
  int glyphPlaybackOption = -1;
  QString glyphPlaybackFilename;


  bool enableTexturePlacement = false;
  if (this->SnapTarget != NULL)
    {
    pqCMBTexturedObject *target =
      dynamic_cast<pqCMBTexturedObject*>(this->SnapTarget->getDataObject());
    if (target)
      {
      enableTexturePlacement = target->hasTexture();
      }
    }

  count = qtCMBSceneObjectDuplicateDialog::getCopyInfo(this->Selected[0],
                                                       enableGlyphing,
                                                       enableTexturePlacement,
                                                       this->useGlyphPlayback,
                                                       constraints, useGlyphs,
                                                       useTextureConstraint,
                                                       glyphPlaybackOption,
                                                       glyphPlaybackFilename);
  if (!count)
    {
    return;
    }

  std::deque<double>* glyphPoints = new std::deque<double>();

  //If importing points, read them to the vector
  if(glyphPlaybackOption == 1)
    {
    if(glyphPlaybackFilename.isEmpty())
      {
      QMessageBox::critical(this->TreeWidget, "Glyph Playback File", "Please provide a file to import points from.");
      return;
      }
    ifstream glyphPlaybackFile(glyphPlaybackFilename.toAscii().data());
    if(!glyphPlaybackFile.good())
      {
      QMessageBox::critical(this->TreeWidget, "Glyph Playback File", "Error opening glyph playback file. Check the file and try again.");
      }
    double p;
    while(!glyphPlaybackFile.eof())
      {
      glyphPlaybackFile >> p;
      //Manage last new line character
      if(glyphPlaybackFile.eof())
        {
        break;
        }
      glyphPoints->push_back(p);
      }
    count = glyphPoints->size()/3;
    }

  std::vector<pqCMBSceneNode*> nodes =
    this->duplicateNode(this->Selected[0], glyphPoints, count, true, false, &constraints,
    useGlyphs, useTextureConstraint, glyphPlaybackOption);
  if(glyphPlaybackOption == 0)
    {
    if(!glyphPlaybackFilename.isEmpty())
      {
      ofstream glyphPlaybackFile( glyphPlaybackFilename.toAscii().data() );
      std::deque<double>::iterator iter;
      for(iter = glyphPoints->begin(); iter != glyphPoints->end(); iter = iter+3)
        {
        glyphPlaybackFile << *iter << " " << *(iter+1) << " " << *(iter+2) << "\n";
        }
      glyphPlaybackFile.close();
      }
    }
  delete glyphPoints;

  int i, n = static_cast<int>(nodes.size());
  this->TreeWidget->blockSignals(true);
  if (!n)
    {
    return;
    }
  this->Selected[0]->getWidget()->setSelected(false);
  for (i = 0; i < n; i++)
    {
    nodes[i]->getWidget()->setSelected(true);
    }
  this->TreeWidget->blockSignals(false);
  this->nodesSelected();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::changeNumberOfPointsLoadedAction()
{
  pqCMBPoints *dataObj =
    dynamic_cast<pqCMBPoints*>(this->Selected[0]->getDataObject());
  vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(
    dataObj->getReaderSource()->getProxy() );

  int currentNumberOfPts = dataObj->getNumberOfPoints();
  int totalNumberOfPoints = dataObj->getPieceTotalNumberOfPoints();

  QDialog *mainDialog = new QDialog();
  Ui::qtLIDARNumberOfPointsDialog* numPtsDialog = new
    Ui::qtLIDARNumberOfPointsDialog;
  numPtsDialog->setupUi(mainDialog);

  char buffer[20]; // plenty of space!
  sprintf(buffer, "%d", currentNumberOfPts);
  numPtsDialog->currentNumberOfPoints->setText(buffer);
  sprintf(buffer, "%d", totalNumberOfPoints);
  numPtsDialog->totalNumberOfPoints->setText(buffer);
  numPtsDialog->targetNumberOfPoints->setMaximum(totalNumberOfPoints);
  numPtsDialog->targetNumberOfPoints->setValue(currentNumberOfPts);
  mainDialog->setModal(true);
  if (mainDialog->exec() == QDialog::Accepted)
    {
    if (dataObj->getPieceId() == -1) // supporting old scenegen files... loaded all as one piece
      {
      vtkSMPropertyHelper(sourceProxy, "MaxNumberOfPoints").Set(
        numPtsDialog->targetNumberOfPoints->value() );
      vtkSMPropertyHelper(sourceProxy, "LimitToMaxNumberOfPoints").Set(1);
      sourceProxy->UpdateVTKObjects();
      }
    else
      {
      // calculate new onRatio
      int onRatio;
      if (numPtsDialog->targetNumberOfPoints->value())
        {
        onRatio = ceil( static_cast<double>(totalNumberOfPoints) /
          numPtsDialog->targetNumberOfPoints->value() );
        }
      else
        {
        onRatio = totalNumberOfPoints; // not 0, but should be 1 per piece
        }

      if (onRatio != dataObj->getPieceOnRatio())
        {
        dataObj->setPieceOnRatio(onRatio);

        QList<QVariant> pieceOnRatioList;
        pieceOnRatioList << dataObj->getPieceId() << onRatio;

        pqSMAdaptor::setMultipleElementProperty(
          sourceProxy->GetProperty("RequestedPiecesForRead"), pieceOnRatioList);
        sourceProxy->UpdateVTKObjects();
        sourceProxy->UpdatePipeline();


        if (!strcmp("vtkLASReader", sourceProxy->GetVTKClassName()))
          {
          pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

          pqPipelineSource* extract = builder->createFilter("filters",
            "ExtractLeafBlock", dataObj->getReaderSource());

          // only 1 block when doing update to points loaded
          pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), 0);
          extract->getProxy()->UpdateVTKObjects();
          vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

          vtkSMDataSourceProxy::SafeDownCast(
            dataObj->getSource()->getProxy() )->CopyData(
            vtkSMSourceProxy::SafeDownCast(extract->getProxy()) );
          builder->destroy(extract);
          }
        else
          {
          vtkSMDataSourceProxy::SafeDownCast(
            dataObj->getSource()->getProxy() )->CopyData(sourceProxy);
          }
        }
      this->CurrentView->render();
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateSnapOptions()
{
  if (this->SetSnapTargetAction)
    {
    this->SetSnapTargetAction->
      setVisible(((this->Selected.size() == 1) &&
                  (!this->Selected[0]->isTypeNode()) &&
                  (!this->Selected[0]->isLineNode()) &&
                  (!this->Selected[0]->isArcNode()) &&
                  (this->Selected[0] != this->SnapTarget)));
    }

  if (this->UnsetSnapTargetAction)
    {
    this->UnsetSnapTargetAction->
      setVisible(((this->Selected.size() == 1) &&
                  (this->Selected[0] == this->SnapTarget)));
    }

  if (this->SetSnapObjectAction)
    {
    if (!this->SnapTarget)
      {
      this->SetSnapObjectAction->setEnabled(false);
      }
    else if (this->Selected.size() == 1)
      {
      this->SetSnapObjectAction->
        setEnabled(this->Selected[0] != this->SnapTarget);
      }
    else if (this->Selected.size() == 0)
      {
      this->SetSnapObjectAction->setEnabled(false);
      }
    else
      {
      this->SetSnapObjectAction->setEnabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setSelectSnapNodeAction(QAction *setAction, QAction *unsetAction)
{
  if (this->SetSnapTargetAction)
    {
    this->PropertiesMenu->removeAction(this->SetSnapTargetAction);
    }

  if (this->UnsetSnapTargetAction)
    {
    this->PropertiesMenu->removeAction(this->UnsetSnapTargetAction);
    }

  this->SetSnapTargetAction = setAction;
  if (this->SetSnapTargetAction)
    {
    this->PropertiesMenu->addAction(this->SetSnapTargetAction);
    QObject::connect(setAction, SIGNAL(triggered()), this, SLOT(setSnapTarget()));
    }

  this->UnsetSnapTargetAction = unsetAction;
  if (this->UnsetSnapTargetAction)
    {
    this->PropertiesMenu->addAction(this->UnsetSnapTargetAction);
    QObject::connect(unsetAction, SIGNAL(triggered()), this, SLOT(unsetSnapTarget()));
    }
  this->updateSnapOptions();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setTextureAction(QAction *changeAction)
{
  if (this->ChangeTextureAction)
    {
    this->PropertiesMenu->removeAction(this->ChangeTextureAction);
    }
  this->ChangeTextureAction = changeAction;
  if (this->ChangeTextureAction)
    {
    this->PropertiesMenu->addAction(this->ChangeTextureAction);
    QObject::connect(changeAction, SIGNAL(triggered()), this, SLOT(editTexture()));
    }
  this->updateTexturedObjectOptions();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setApplyBathymetryAction(QAction *changeAction)
{
  if (this->ApplyBathymetryAction)
    {
    this->PropertiesMenu->removeAction(this->ApplyBathymetryAction);
    }
  this->ApplyBathymetryAction = changeAction;
  if (this->ApplyBathymetryAction)
    {
    this->PropertiesMenu->addAction(this->ApplyBathymetryAction);
    QObject::connect(changeAction, SIGNAL(triggered()), this, SLOT(editBathymetry()));
    }
  this->updateBathymetryOptions();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setUndoRedoActions(QAction *undoAction, QAction *redoAction)
{
  if (this->UndoAction)
    {
    this->EditMenu->removeAction(this->UndoAction);
    }
  this->UndoAction = undoAction;
  if (undoAction)
    {
    this->EditMenu->addAction(this->UndoAction);
    QObject::connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    // If the current undo index is at the beginning of the list
    // there is nothing to be undone
    this->UndoAction->setEnabled(this->CurrentUndoIndex != 0);
    }

  if (this->RedoAction)
    {
    this->EditMenu->removeAction(this->RedoAction);
    }
  this->RedoAction = redoAction;
  if (redoAction)
    {
    this->EditMenu->addAction(this->RedoAction);
    QObject::connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    // If we are at the end of the list there is nothing to be redone
    this->RedoAction->setEnabled(this->CurrentUndoIndex < this->UndoRedoList.size());
    }

}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setSnapObjectAction(QAction *action)
{
  if (this->SetSnapObjectAction)
    {
    this->EditMenu->removeAction(this->SetSnapObjectAction);
    }
  this->SetSnapObjectAction = action;
  if (action)
    {
    this->EditMenu->addAction(this->SetSnapObjectAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(snapSelected()));
    }

  this->updateSnapOptions();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setInsertNodeAction(QAction *action)
{
  if (this->InsertAction)
    {
    this->CreateMenu->removeAction(this->InsertAction);
    }

  this->InsertAction = action;

  if (action)
    {
    this->CreateMenu->addAction(this->InsertAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(insertTypeNode()));
    this->InsertAction->setEnabled(this->Selected.size() == 1);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setImportObjectAction(QAction *action)
{
  if (this->ImportAction)
    {
    this->FileMenu->removeAction(this->ImportAction);
    }

  this->ImportAction = action;
  if (action)
    {
    this->FileMenu->addAction(this->ImportAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(importObject()));
    this->ImportAction->setEnabled(this->Selected.size() == 1);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setConvertToGlyphAction(QAction *action)
{
  if (this->ConvertToGlyphAction)
    {
    this->EditMenu->removeAction(this->ConvertToGlyphAction);
    }

 this->ConvertToGlyphAction = action;
  if (action)
    {
    this->EditMenu->addAction(this->ConvertToGlyphAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(convertNodesToGlyphs()));
    this->ConvertToGlyphAction->setEnabled(this->Selected.size() != 0);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setDeleteNodeAction(QAction *action)
{
  if (this->DeleteAction)
    {
    this->EditMenu->removeAction(this->DeleteAction);
    }

 this->DeleteAction = action;
  if (action)
    {
    this->EditMenu->addAction(this->DeleteAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    this->DeleteAction->setEnabled(this->Selected.size() != 0);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setDuplicateNodeAction(QAction *nonrandom, QAction *randomAction)
{

  if (this->DuplicateAction)
    {
    this->EditMenu->removeAction(this->DuplicateAction);
    }

  if (this->DuplicateRandomlyAction)
    {
    this->EditMenu->removeAction(this->DuplicateRandomlyAction);
    }

  this->DuplicateAction = nonrandom;
  if (this->DuplicateAction)
    {
    this->EditMenu->addAction(this->DuplicateAction);
    QObject::connect(nonrandom, SIGNAL(triggered()), this, SLOT(duplicateSelected()));
    this->DuplicateAction->setEnabled(this->Selected.size() == 1 && (!this->Selected[0]->isArcNode()));
    }

  this->DuplicateRandomlyAction = randomAction;

  if (this->DuplicateRandomlyAction)
    {
    this->EditMenu->addAction(this->DuplicateRandomlyAction);
    QObject::connect(randomAction, SIGNAL(triggered()), this, SLOT(duplicateSelectedRandomly()));
    this->DuplicateRandomlyAction->setEnabled(this->Selected.size() == 1
                                              && this->SnapTarget && (!this->Selected[0]->isArcNode()));
    }
}

////-----------------------------------------------------------------------------
//void pqCMBSceneTree::setUseGlyphPlaybackAction( QAction *useGlyphPlaybackAction )
//{
//  QObject::connect(useGlyphPlaybackAction, SIGNAL(stateChanged(int)), this, SLOT(updateUseGlyphPlayback(int)));
//}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateUseGlyphPlayback(bool checked)
{
  if(checked)
    {
    this->useGlyphPlayback = true;
    }
  else
    {
    this->useGlyphPlayback = false;
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCreateVOINodeAction(QAction *action)
{
  if (this->VOIAction)
    {
    this->CreateMenu->removeAction(this->VOIAction);
    }
  this->VOIAction = action;
  if (this->VOIAction)
    {
    this->CreateMenu->addAction(this->VOIAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(createVOI()));
    this->VOIAction->setEnabled(this->Selected.size() == 1);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setConicalNodeActions(QAction *create, QAction *edit)
{
  if (this->ConeCreateAction)
    {
    this->CreateMenu->removeAction(this->ConeCreateAction);
    }
  this->ConeCreateAction = create;
  if (this->ConeCreateAction)
    {
    this->CreateMenu->addAction(this->ConeCreateAction);
    QObject::connect(create, SIGNAL(triggered()), this, SLOT(createCone()));
    this->ConeCreateAction->setEnabled(this->Selected.size() == 1);
    }

  if (this->ConeEditAction)
    {
    this->EditMenu->removeAction(this->ConeEditAction);
    }

  this->ConeEditAction = edit;
  if (this->ConeEditAction)
    {
    this->EditMenu->addAction(this->ConeEditAction);
    QObject::connect(edit, SIGNAL(triggered()), this,
                     SLOT(editCone()));
    this->updateEditConeOption();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setGroundPlaneActions(QAction *create, QAction *edit)
{
  if (this->CreateGroundPlaneAction)
    {
    this->CreateMenu->removeAction(this->CreateGroundPlaneAction);
    }
  if (this->EditGroundPlaneAction)
    {
    this->EditMenu->removeAction(this->EditGroundPlaneAction);
    }

  this->CreateGroundPlaneAction = create;

  if (this->CreateGroundPlaneAction)
    {
    this->CreateMenu->addAction(this->CreateGroundPlaneAction);
    QObject::connect(create, SIGNAL(triggered()), this, SLOT(createGroundPlane()));
    }
  this->EditGroundPlaneAction = edit;
  if (this->EditGroundPlaneAction)
    {
    this->EditMenu->addAction(this->EditGroundPlaneAction);
    QObject::connect(edit, SIGNAL(triggered()), this, SLOT(editGroundPlane()));
    }
  this->updateGroundPlaneOptions();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCreateLineNodeAction(QAction *action)
{
  if (this->LineAction)
    {
    this->CreateMenu->removeAction(this->LineAction);
    }
  this->LineAction = action;
  if (this->LineAction)
    {
    this->CreateMenu->addAction(this->LineAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(createLineObject()));
    this->LineAction->setEnabled(this->Selected.size() == 1);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCreateArcNodeAction(QAction *action)
{
  if (this->ArcAction)
    {
    this->CreateMenu->removeAction(this->ArcAction);
    }
  this->ArcAction = action;
  if (this->ArcAction)
    {
    this->CreateMenu->addAction(this->ArcAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(createArcObject()));
    bool enable = !(( this->ArcWidgetManager && this->ArcWidgetManager->hasActiveNode() )
      || this->Selected.size() >= 1 );
    this->ArcAction->setEnabled(enable);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setArcActions(QAction *edit, QAction *snapArcs,
    QAction *mergeArcs,QAction *growSelection, QAction *autoConnArcs)
{
  if (this->EditArcAction)
    {
    this->EditMenu->removeAction(this->EditArcAction);
    }
  this->EditArcAction = edit;
  if (edit)
    {
    this->EditMenu->addAction(this->EditArcAction);
    QObject::connect(edit, SIGNAL(triggered()), this, SLOT(editArcObject()));
    this->EditArcAction->setVisible(false);
    }

  if (this->ArcSnappingAction)
    {
    this->ToolsMenu->removeAction(this->ArcSnappingAction);
    }
  this->ArcSnappingAction = snapArcs;
  if (snapArcs)
    {
    this->ToolsMenu->addAction(this->ArcSnappingAction);
    QObject::connect(snapArcs, SIGNAL(triggered()), this, SLOT(setArcSnapping()));
    this->ArcSnappingAction->setVisible(false);
    }

  if (this->MergeArcsAction)
    {
    this->ToolsMenu->removeAction(this->MergeArcsAction);
    }
  this->MergeArcsAction = mergeArcs;
  if (mergeArcs)
    {
    this->ToolsMenu->addAction(this->MergeArcsAction);
    QObject::connect(mergeArcs, SIGNAL(triggered()), this, SLOT(mergeArcs()));
    this->MergeArcsAction->setVisible(false);
    }

  if(this->GrowArcSelectionAction)
    {
    this->ToolsMenu->removeAction(this->GrowArcSelectionAction);
    }
  this->GrowArcSelectionAction = growSelection;
  if(growSelection)
    {
    this->ToolsMenu->addAction(this->GrowArcSelectionAction);
    QObject::connect(growSelection, SIGNAL(triggered()), this, SLOT(growArcSelection()));
    this->GrowArcSelectionAction->setVisible(false);
    }

  if(this->AutoConnectArcsAction)
    {
    this->ToolsMenu->removeAction(this->AutoConnectArcsAction);
    }
  this->AutoConnectArcsAction = autoConnArcs;
  if(growSelection)
    {
    this->ToolsMenu->addAction(this->AutoConnectArcsAction);
    QObject::connect(autoConnArcs, SIGNAL(triggered()), this, SLOT(autoConnectArcs()));
    this->AutoConnectArcsAction->setVisible(false);
    }
}


//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCreatePolygonAction(QAction *action)
{
  if (this->CreatePolygonAction)
    {
    this->CreateMenu->removeAction(this->CreatePolygonAction);
    }

  this->CreatePolygonAction = action;

  if (action)
    {
    this->CreateMenu->addAction(this->CreatePolygonAction);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(createPolygonObject()));
    this->CreatePolygonAction->setEnabled(this->Selected.size() == 1);
    }

}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setChangeNumberOfPointsLoadedAction(QAction *action)
{
  if (this->ChangeNumberOfPointsLoadedAction)
    {
    this->PropertiesMenu->removeAction(this->ChangeNumberOfPointsLoadedAction);
    }
  this->ChangeNumberOfPointsLoadedAction = action;
  if (this->ChangeNumberOfPointsLoadedAction)
    {
    this->PropertiesMenu->addAction(this->ChangeNumberOfPointsLoadedAction);
    QObject::connect(action, SIGNAL(triggered()), this,
                     SLOT(changeNumberOfPointsLoadedAction()));
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setChangeUserDefineObjectTypeAction(QAction *action)
{
  if (this->ChangeUserDefineObjectTypeAction)
    {
    this->PropertiesMenu->removeAction(this->ChangeUserDefineObjectTypeAction);
    }
  this->ChangeUserDefineObjectTypeAction = action;
  if (this->ChangeUserDefineObjectTypeAction)
    {
    this->PropertiesMenu->addAction(this->ChangeUserDefineObjectTypeAction);
    QObject::connect(action, SIGNAL(triggered()), this,
                     SLOT(changeUserDefineObjectTypeAction()));
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setTINStitchAction(QAction *action)
{
  if (this->TINStitchAction)
    {
    this->ToolsMenu->removeAction(this->TINStitchAction);
    }
  this->TINStitchAction = action;
  if (this->TINStitchAction)
    {
   this->ToolsMenu->addAction(this->TINStitchAction);
     QObject::connect(action, SIGNAL(triggered()), this, SLOT(stitchTINs()));
    this->updateTINStitchOptions();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setTINStackAction(QAction *action)
{
  if (this->TINStackAction)
    {
    this->ToolsMenu->removeAction(this->TINStackAction);
    }
  this->TINStackAction = action;
  if (this->TINStackAction)
    {
   this->ToolsMenu->addAction(this->TINStackAction);
     QObject::connect(action, SIGNAL(triggered()), this, SLOT(stackTINs()));
    this->updateTINStackOptions();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setGenerateArcsAction(QAction *action)
{
  if (this->GenerateArcsAction)
    {
    this->ToolsMenu->removeAction(this->GenerateArcsAction);
    }
  this->GenerateArcsAction = action;
  if (this->GenerateArcsAction)
    {
   this->ToolsMenu->addAction(this->GenerateArcsAction);
     QObject::connect(action, SIGNAL(triggered()), this, SLOT(generateArcsFromImage()));
    this->updateGenerateArcsOptions();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setGenerateImageMeshAction(QAction * action)
{
  if(this->GenerateImageMesh)
  {
    this->ToolsMenu->removeAction(this->GenerateImageMesh);
  }
  this->GenerateImageMesh = action;
  if (this->GenerateArcsAction)
    {
    this->ToolsMenu->addAction(this->GenerateArcsAction);
    QObject::connect(action, SIGNAL(triggered()),
                     this, SLOT(generateMeshFromImage()));
    this->updateGenerateArcsOptions();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setExportSolidsAction(QAction *action)
{
  if (this->ExportSolidsAction)
    {
    this->FileMenu->removeAction(this->ExportSolidsAction);
    }
  this->ExportSolidsAction = action;
  if (this->ExportSolidsAction)
    {
    this->FileMenu->addAction(this->ExportSolidsAction);
    QObject::connect(action, SIGNAL(triggered()), this,
                     SLOT(exportSelectedSolids()));
    this->ExportSolidsAction->setEnabled(this->Selected.size() != 0);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setExportPolygonsAction(QAction *action)
{
  if (this->ExportPolygonsAction)
    {
    this->FileMenu->removeAction(this->ExportPolygonsAction);
    }
  this->ExportPolygonsAction = action;
  if (this->ExportPolygonsAction)
    {
    this->FileMenu->addAction(this->ExportPolygonsAction);
    QObject::connect(action, SIGNAL(triggered()), this,
      SLOT(exportSelectedPolygons()));
    this->ExportPolygonsAction->setEnabled(this->Selected.size() != 0);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setDefineVOIAction(QAction *action)
{
  if (this->DefineVOIAction)
    {
    this->EditMenu->removeAction(this->DefineVOIAction);
    }

  this->DefineVOIAction = action;
  if (this->DefineVOIAction)
    {
    this->EditMenu->addAction(this->DefineVOIAction);
    QObject::connect(action, SIGNAL(triggered()), this,
                     SLOT(defineSelectedVOI()));
    this->updateDefineVOIOption();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setElevationAction(QAction *action)
{
  if (this->ElevationAction)
    {
    this->EditMenu->removeAction(this->ElevationAction);
    }

  this->ElevationAction = action;
  if (this->ElevationAction)
    {
    this->PropertiesMenu->addAction(this->ElevationAction);
    QObject::connect(action, SIGNAL(triggered()), this,
                     SLOT(updateElevation()));
    this->updateTexturedObjectOptions();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCurrentView(pqRenderView *view)
{
  this->CurrentView = view;
  emit newCurrentView(this->CurrentView);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setCurrentServer(pqServer *server)
{
  this->CurrentServer = server;
  emit newCurrentServer(this->CurrentServer);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setUnits(cmbSceneUnits::Enum utype)
{
  this->Units = utype;
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::showContextMenu(const QPoint &p)
{
  this->nodesSelected();
  this->ContextMenu->popup(this->TreeWidget->mapToGlobal(p));
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setSnapTarget(pqCMBSceneNode *node)
{
  if (this->SnapTarget == node)
    {
    // They are the same = nothing to be done
    return;
    }
  this->unsetSnapTarget();
  if (!node)
    {
    return;
    }
  this->SnapTarget = node;

  this->SnapTarget->getWidget()->setIcon(this->SnapTarget->getNameColumn(),
                                       *this->IconSnap);
  this->SnapTarget->getDataObject()->setLODMode(1);
  this->SnapTarget->getDataObject()->setIsSnapTarget(true);
  this->updateSnapOptions();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setSnapTarget()
{
  if ((this->Selected.size() != 1) || this->Selected[0]->isTypeNode() ||
      (this->Selected[0] == this->SnapTarget))
    {
    return;
    }
  this->setSnapTarget(this->Selected[0]);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::unsetSnapTarget()
{
  if (!this->SnapTarget)
    {
    return;
    }

  this->SnapTarget->getWidget()->setIcon(this->SnapTarget->getNameColumn(),
                                         *this->IconNULL);
  this->SnapTarget->getDataObject()->setLODMode(0);
  this->SnapTarget->getDataObject()->setIsSnapTarget(false);

  this->SnapTarget = NULL;
  this->updateSnapOptions();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::snapSelected()
{
  if (!this->SnapTarget)
    {
    return;
    }

  int i, n = static_cast<int>(this->Selected.size());
  for (i = 0; i < n; i++)
    {
    this->snapObject(this->Selected[i]);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::snapObject(pqCMBSceneNode *node)
{
  if ((!this->SnapTarget) || (this->SnapTarget == node))
    {
    return;
    }

  pqCMBSceneObjectBase *target = this->SnapTarget->getDataObject();
  pqCMBSceneObjectBase *obj;
  double p[3], np[3];

  if (!node->isTypeNode() &&
      node->getDataObject()->getType() != pqCMBSceneObjectBase::Line)
    {
    obj = node->getDataObject();
    // Get the position of the object
    obj->getPosition(p);
    // Get the closest point
    target->getClosestPoint(p, np);
    obj->setPosition(np);
    return;
    }

  SceneObjectNodeIterator iter(node);
  pqCMBSceneNode *objNode;
  while((objNode = iter.next()))
    {
    if (objNode == this->SnapTarget ||
        objNode->getDataObject()->getType() == pqCMBSceneObjectBase::Line)
      {
      continue;
      }
    obj = objNode->getDataObject();

    // Get the position of the object
    obj->getPosition(p);
    // Get the closest point
    target->getClosestPoint(p, np);
    obj->setPosition(np);
    }
}

//-----------------------------------------------------------------------------
bool pqCMBSceneTree::containsDataObjects() const
{
  SceneObjectNodeIterator iter(this->Root);
  pqCMBSceneNode *objNode;
  bool hasObjects = false;
  while((objNode = iter.next()))
    {
    hasObjects = true;
    break;
    }
  return hasObjects;
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setColorActions(QAction *setColor, QAction *unset)
{
  if (this->SetNodeColorAction)
    {
    this->PropertiesMenu->removeAction(this->SetNodeColorAction);
    }

  this->SetNodeColorAction = setColor;

  if (this->SetNodeColorAction)
    {
    this->PropertiesMenu->addAction(this->SetNodeColorAction);
    QObject::connect(setColor, SIGNAL(triggered()), this, SLOT(setNodeColor()));
    }

  if (this->UnsetNodeColorAction)
    {
    this->PropertiesMenu->removeAction(this->UnsetNodeColorAction);
    }

  this->UnsetNodeColorAction = unset;

  if (this->UnsetNodeColorAction)
    {
    this->PropertiesMenu->addAction(this->UnsetNodeColorAction);
    QObject::connect(unset, SIGNAL(triggered()), this, SLOT(unsetNodeColor()));
    }

  this->updateSelectedColorMode();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::unsetNodeColor()
{
  //the user wants to clear the explicit color
  size_t i,n = this->Selected.size();
  bool clear = false;
  for (i = 0; i < n; i++)
    {
    if (this->Selected[i]->hasExplicitColor())
      {
      this->Selected[i]->unsetExplicitColor();
      clear = true;
      }
    }
  if ( clear )
    {
    this->clearColorNodeIcon();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::setNodeColor()
{
  size_t i,n = this->Selected.size();
  bool state=false;
  for (i = 0; i < n; ++i)
    {
    if (this->Selected[i]->hasExplicitColor())
      {
      state = true;
      break;
      }
    }

  double color[4];
  this->Selected[0]->getColor(color);
  QColor qcolor;
  qcolor.setRgbF(color[0], color[1], color[2], color[3]);
  QColor result;
  result = qtCMBColorDialog::getColor(qcolor, this->TreeWidget);
  if (!result.isValid())
    {
    //the user wants to cancel setting a color
    return;
    }
  //update the explicit color
  result.getRgbF(&(color[0]), &(color[1]), &(color[2]), &(color[3]));
  for (i = 0; i < n; i++)
    {
    this->Selected[i]->setExplicitColor(color);
    }
  this->setColorNodeIcon(color);
  this->sceneObjectChanged();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::getVOIs(std::vector<pqCMBSceneNode *> *vois) const
{
  vois->clear();
  SceneObjectNodeIterator iter(this->Root);
  iter.setTypeFilter(pqCMBSceneObjectBase::VOI);
  pqCMBSceneNode *node;
  while((node = iter.next()))
    {
    vois->push_back(node);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::getArcs(std::vector<pqCMBSceneNode *> *arcs) const
{
  arcs->clear();
  SceneObjectNodeIterator iter(this->Root);
  iter.setTypeFilter(pqCMBSceneObjectBase::Arc);
  pqCMBSceneNode *node;
  while((node = iter.next()))
    {
    arcs->push_back(node);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::clearSelection()
{
  this->nodesSelected(true);
  this->clearSelectedGlyphPointsColor();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::clearSelectedGlyphPointsColor()
{
  if(this->isEmpty())
    {
    return;
    }
  SceneObjectNodeIterator nodeIter( this->Root );
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::Glyph );
  pqCMBSceneNode *n;
  while((n = nodeIter.next()))
    {
    dynamic_cast<pqCMBGlyphObject*>(n->getDataObject())->clearSelectedPointsColor();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::editTexture()
{
  this->CurrentTextureObj = NULL;

  if ((this->Selected.size() != 1) ||
      (this->Selected[0]->isTypeNode()))
    {
    return;
    }
  pqCMBSceneNode *selNode = this->Selected[0];
  pqCMBTexturedObject *tobj =
    dynamic_cast<pqCMBTexturedObject*>(selNode->getDataObject());
  if (!tobj)
    {
    return;
    }

  this->CurrentTextureObj = tobj;

  emit this->setCameraManipulationMode(1);
  //Disable scene tree and freeze the scene in 2D mode
  this->enableSceneTree(false);
  emit this->enableToolbars(false);
  emit this->resetViewDirection(0, 0, -1, 0, 1, 0);
  //We don't want the scene selected(Box widget cleared)
  this->clearSelection();

  pqPlanarTextureRegistrationDialog* textRegDlg = new pqPlanarTextureRegistrationDialog(
    this->CurrentServer, this->CurrentView,
    QString(tr("Texture Registration (") + tr(selNode->getName()) + tr(")")),
    this->getWidget());
  double bounds[6];
  tobj->getBounds(bounds);
  QObject::connect(textRegDlg, SIGNAL(removeCurrentTexture()),
    this, SLOT(unsetTextureMap()));
  QObject::connect(textRegDlg,
    SIGNAL(registerCurrentTexture(const QString&, int, double *)),
    this, SLOT(setTextureMap(const QString&, int, double*)));

  textRegDlg->initializeTexture(bounds,
    this->getTextureFileNames(), tobj->getTextureFileName(),
    tobj->getRegistrationPoints(),
    tobj->getNumberOfRegistrationPoints());

  textRegDlg->exec();

  this->sceneObjectChanged();
  this->CurrentTextureObj = NULL;
  delete textRegDlg;
  emit this->resetCameraManipulationMode();
  this->enableSceneTree(true);
  emit this->enableToolbars(true);
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateElevation()
{
  if ((this->Selected.size() != 1) ||
      (this->Selected[0]->isTypeNode()))
    {
    return;
    }

  pqCMBTexturedObject *tobj =
    dynamic_cast<pqCMBTexturedObject*>(this->Selected[0]->getDataObject());
  if (tobj)
    {
    tobj->toggleElevation();
    this->sceneObjectChanged();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::editBathymetry()
{
  if ((this->Selected.size() != 1) ||
    (this->Selected[0]->isTypeNode()))
    {
    return;
    }

  pqCMBTexturedObject *tobj =
    dynamic_cast<pqCMBTexturedObject*>(this->Selected[0]->getDataObject());
  if (tobj)
    {
    if(qtCMBBathymetryDialog::manageBathymetry(this->Selected[0]))
      {
      this->sceneObjectChanged();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::selectLineNode()
{
  pqCMBLineWidget* const lineWidget = qobject_cast<pqCMBLineWidget*>(
    QObject::sender());
  pqCMBSceneNode* lineNode = this->FindLineNode(lineWidget);
  if(lineNode && !lineNode->isSelected())
    {
    this->clearSelection();
    lineNode->select();
    }
}

pqCMBSceneNode* pqCMBSceneTree::getLineTypeNode(bool createIfDoesntExist/*=true*/)
{
  if(pqCMBSceneNode* lineTypeNode = this->findNode("Lines"))
    {
    return lineTypeNode;
    }
  if (createIfDoesntExist)
    {
    return this->createNode("Lines", this->Root, NULL, NULL);
    }
  return 0;
}

pqCMBSceneNode* pqCMBSceneTree::getArcTypeNode(bool createIfDoesntExist/*=true*/)
{
  pqCMBSceneNode* arcTypeNode = NULL;
  arcTypeNode = this->findNode("Arcs");
  if(arcTypeNode)
    {
    return arcTypeNode;
    }

  //for backwards compatibility we want to be able to handle scene files
  //that have saved the arcs under the contours node
  arcTypeNode = this->findNode("Contours");
  if(arcTypeNode)
    {
    return arcTypeNode;
    }

  if (createIfDoesntExist)
    {
    return this->createNode("Arcs", this->Root, NULL, NULL);
    }
  return 0;
}

//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::FindLineNode(pqCMBLineWidget* lineWidget)
{
  if(!lineWidget || !this->findNode("Lines"))
    {
    return NULL;
    }

  pqCMBSceneNodeIterator iter(this->getLineTypeNode());
  pqCMBSceneNode *n;
  while((n = iter.next()))
    {
    if (!n->getDataObject() ||
      n->getDataObject()->getType() != pqCMBSceneObjectBase::Line)
      {
      continue;
      }

    pqCMBLine* lineObj = static_cast<pqCMBLine*>(n->getDataObject());
    if(!lineObj)
      {
      continue;
      }
    if(lineObj->getLineWidget() == lineWidget)
      {
      return n;
      }
    }
  return NULL;
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setLineWidgetCallbacks(pqCMBLine* obj)
{
  if(obj)
    {
    QObject::connect(obj->getLineWidget(), SIGNAL(widgetStartInteraction()),
      this, SLOT(selectLineNode()));
    QObject::connect(obj->getLineWidget(), SIGNAL(widgetVisibilityChanged(bool)),
      this, SLOT(updateLineNodeVisibility(bool)));
    QObject::connect(obj->getLineWidget(), SIGNAL(widgetInteraction()),
      this, SLOT(collapseAllDataInfo()));
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateLineNodeVisibility(bool visible)
{
  pqCMBLineWidget* const lineWidget = qobject_cast<pqCMBLineWidget*>(
    QObject::sender());
  pqCMBSceneNode* lineNode = this->FindLineNode(lineWidget);
  if(lineNode)
    {
    if(visible != lineNode->isVisible())
      {
      lineNode->setVisibility(visible);
      }
    if(visible)
      {
      lineNode->select();
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::collapseAllDataInfo()
{
  this->getRoot()->collapseAllDataInfo();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::recomputeInfo(QTreeWidgetItem* item)
{
  if (this->getRoot())
    {
    this->getRoot()->recomputeInfo(item);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::exportSelectedSolids()
{
  emit requestSolidExport();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::exportSelectedPolygons()
{
  emit requestPolygonsExport();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::defineSelectedVOI()
{
  if ((this->Selected.size() == 1) &&
      (!(this->Selected[0]->isTypeNode())) &&
      (this->Selected[0]->getDataObject()->getType() ==
       pqCMBSceneObjectBase::VOI))
    {
    pqCMBSceneNode *n = this->Selected[0];
    if (qtCMBVOIDialog::manageVOI(n))
      {
      // Toggle the node's selection to make sure everyone knows its bounds
      // has changed
      this->clearSelection();
      n->select();
      this->TreeWidget->setCurrentItem(n->getWidget());
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::createGroundPlane()
{
  if (this->Selected.size() != 1)
    {
    return;
    }

  pqCMBSceneNode *parent = this->Selected[0];
  if (!parent->isTypeNode())
    {
    parent = parent->getParent();
    }

  double p1[3], p2[3];
  if (qtCMBGroundPlaneDialog::defineGroundPlane(p1, p2))
    {
    pqCMBPlane *obj =
      new pqCMBPlane(p1, p2,
                        this->CurrentServer, this->CurrentView);
    pqCMBSceneNode *node = this->createNode("New GroundPlane", parent, obj, NULL);
    this->Selected[0]->getWidget()->setSelected(false);
    node->getWidget()->setSelected(true);
    this->nodesSelected();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::editGroundPlane()
{
  if ((this->Selected.size() == 1) &&
      (!(this->Selected[0]->isTypeNode())) &&
      (this->Selected[0]->getDataObject()->getType() ==
       pqCMBSceneObjectBase::GroundPlane))
    {
    pqCMBSceneNode *n = this->Selected[0];
    if (qtCMBGroundPlaneDialog::manageGroundPlane(n))
      {
      // Toggle the node's selection to make sure everyone knows its bounds
      // has changed
      this->clearSelection();
      n->select();
      this->TreeWidget->setCurrentItem(n->getWidget());
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::editArcObject()
{
  if ( this->Selected.size() != 1 )
    {
    return;
    }

  pqCMBSceneNode *node = this->Selected[0];
  if ( !node->isArcNode() )
    {
    return;
    }
  if (!this->ArcWidgetManager)
    {
    this->createArcWidgetManager();
    }

  //reset the Arc object
  this->ArcWidgetManager->setActiveNode( node );
  this->ArcWidgetManager->edit();

  this->nodesSelected();
}


//-----------------------------------------------------------------------------
void pqCMBSceneTree::setArcSnapping()
{
  vtkNew<vtkCMBArcSnapClientOperator> snapOp;
  double currentSnapRadius = snapOp->GetCurrentRadius();

  //spawn the dialog asking for the Radius to use
  QDialog mainDialog;
  Ui::qtCMBSceneArcSnappingDialog snappingDialog;
  snappingDialog.setupUi(&mainDialog);
  snappingDialog.Radius->setValue(currentSnapRadius);
  snappingDialog.groupBox->setChecked( currentSnapRadius > 0 );
  mainDialog.setModal(true);
  if (mainDialog.exec() == QDialog::Accepted)
    {
    double newRadius = snappingDialog.Radius->value();
    if (!snappingDialog.groupBox->isChecked())
      {
      //if the checkbox is unchecked turn off snapping
      newRadius = 0.0;
      }
    if ( newRadius != currentSnapRadius )
      {
      snapOp->Operate(newRadius);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::mergeArcs()
{
  //Merge arcs should not be undoable intill we have a custom
  //undo command ( would be a split )
  bool recordingState = this->RecordEvents;
  this->RecordEvents = false;

  if (this->Selected.size() != 2)
    {
    return;
    }

  pqCMBSceneObjectBase *csObj = this->Selected[0]->getDataObject();
  pqCMBArc *arc = dynamic_cast<pqCMBArc*>(csObj);

  csObj = this->Selected[1]->getDataObject();
  pqCMBArc *arc2 = dynamic_cast<pqCMBArc*>(csObj);
  if (!arc || !arc2)
    {
    return;
    }

  if (arc->isUsedByPolygons() || arc2->isUsedByPolygons()
      )
    {
    //can't merge arcs that are being used by a polygon
    QString text = "Some of the selected Arcs are connected to Polygons which means they can't be merged.";
    QMessageBox msg;
    msg.setText(text);
    msg.setStandardButtons(QMessageBox::Ok);
    msg.setDefaultButton(QMessageBox::Ok);
    msg.exec();
    return;
    }

  vtkNew<vtkCMBArcMergeArcsClientOperator> mergeArcsOp;
  bool valid = mergeArcsOp->Operate(arc->getArcId(),arc2->getArcId());
  if (valid)
    {
    //delete the arc
    vtkIdType toDelete = mergeArcsOp->GetArcIdToDelete();
    pqCMBSceneNode *nodeToDel = (toDelete == arc->getArcId()) ?
      this->Selected[0] : this->Selected[1];
    pqCMBSceneNode *nodeToSave = (toDelete == arc->getArcId()) ?
      this->Selected[1] : this->Selected[0];

    nodeToSave->getWidget()->setSelected(false);
    nodeToDel->getWidget()->setSelected(true);
    this->nodesSelected();

    this->deleteSelected();

    //now actually delete the node after the selection is up to date
    this->RecordEvents = recordingState;
    }
  this->refreshArcsAndPolygons();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::growArcSelection()
{
  //pass in a collection of arc sets
  //create the operator that finds out what arc sets the first & last
  //nodes are connected too. Send back to the client the arc set id's
  //that should be selected.

  //on the client loop through all items in the tree, and find each arc
  //check if the id is in the list, if so add it to the selected

  //build up the list of arc ids for the operator
  std::list<vtkIdType> arcIds;
  for (size_t i=0; i<this->Selected.size(); ++i)
    {
    if (this->Selected[i]->isArcNode())
      {
      pqCMBArc *arc = dynamic_cast<pqCMBArc*>(this->Selected[i]->getDataObject());
      arcIds.push_back(arc->getArcId());
      }
    }

  vtkNew<vtkCMBArcGrowClientOperator> growOp;
  bool valid = growOp->Operate(arcIds);
  if (!valid)
    {
    return;
    }

  //we are going to build a map of all arc ids to scene nodes
  //than find which items are part of the selection
  std::vector<pqCMBSceneNode*> arcNodes;
  this->getArcs(&arcNodes);
  std::map<vtkIdType,pqCMBSceneNode*> arcIdsToNodes;

  std::map<vtkIdType,pqCMBSceneNode*>::iterator aIdNodeIt;
  std::vector<pqCMBSceneNode*>::const_iterator cnIt;

  for(cnIt=arcNodes.begin();cnIt!=arcNodes.end();++cnIt)
    {
    pqCMBArc *arc = dynamic_cast<pqCMBArc*>((*cnIt)->getDataObject());
    if (arc)
      {
      arcIdsToNodes.insert(
            std::pair<vtkIdType,pqCMBSceneNode*>(arc->getArcId(),*cnIt));
      }
    }

  vtkIdTypeArray *growIds = growOp->GetGrownArcIds();

  //build the selection
  QList<pqCMBSceneNode*> newselection, unselect;
  for (vtkIdType i= 0; i < growIds->GetNumberOfTuples(); ++i)
    {
    //now search for this id in all the scene nodes.
    aIdNodeIt = arcIdsToNodes.find(growIds->GetValue(i));
    if (aIdNodeIt != arcIdsToNodes.end() && aIdNodeIt->second
      && !aIdNodeIt->second->isSelected())
      {
      newselection.push_back(aIdNodeIt->second);
      }
    }
  if(newselection.size()>0)
    {
    this->TreeWidget->blockSignals(true);
    foreach (pqCMBSceneNode *node, newselection)
      {
      node->getWidget()->setSelected(true);
      node->setSelected(true);
      this->Selected.push_back(node);
      }
    this->TreeWidget->blockSignals(false);
    this->updateArcOptions();
    emit this->selectionUpdated(&unselect,&newselection);
    this->sceneObjectChanged();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::autoConnectArcs()
{
  //pass in two arcs
  //we are looking to find the two closest
  //end points that have only one line and connect them
  if (this->Selected.size() != 2)
    {
    return;
    }

  pqCMBSceneObjectBase *csObj = this->Selected[0]->getDataObject();
  pqCMBArc *arc = dynamic_cast<pqCMBArc*>(csObj);

  csObj = this->Selected[1]->getDataObject();
  pqCMBArc *arc2 = dynamic_cast<pqCMBArc*>(csObj);
  if (!arc || !arc2)
    {
    return;
    }

  vtkIdType connectedArcId = arc->autoConnect(arc2->getArcId());
  if (connectedArcId == -1)
    {
    return;
    }

  pqCMBArc* autoArc = new pqCMBArc();
  autoArc->createArc(connectedArcId);

  //we now need to set the correct plane information on the polyline
  autoArc->setPlaneProjectionNormal(arc->getPlaneProjectionNormal());
  autoArc->setPlaneProjectionPosition(arc->getPlaneProjectionPosition());

  this->createNode(this->Selected[0]->getName(),this->Selected[0]->getParent(),
                   autoArc,NULL);

  this->refreshArcsAndPolygons();
}

//-----------------------------------------------------------------------------
bool pqCMBSceneTree::getRandomConstraintPoint(double p[3],
                                              QMap<pqCMBSceneNode*, int> *constraints,
                                              int glyphPlaybackOption,
                                              std::deque<double> *glyphPoints)
{
  pqCMBSceneNode*initialNode =
    this->SnapTarget ? this->SnapTarget : this->getRoot();
  vtkBoundingBox rootbb;
  double rootBounds[6];
  initialNode->getBounds(&rootbb);
  rootbb.GetBounds(rootBounds);

  pqCMBSceneNode* node;
  double initialBounds[6];
  int invert;

  // First find one constraint which is Not invert and use that bounds;
  // otherwise, use root bounds
  for(int i=0; i<constraints->count(); i++)
    {
    node = constraints->keys().value(i);
    invert = constraints->value(node);
    if(!invert)
      {
      initialNode = node;
      break;
      }
    }
  vtkBoundingBox initbb;
  initialNode->getBounds(&initbb);
  initbb.GetBounds(initialBounds);
  // if the initial node is Contour, we need to update the bounds
  // with the scene or something else for one of the contour's bounds
  // because the contour is flat
  if(initialNode->getDataObject()->getType() == pqCMBSceneObjectBase::Arc)
    {
    pqCMBArc* obj = static_cast<pqCMBArc*>(
      initialNode->getDataObject());
    if(obj->getPlaneProjectionNormal()==2) // z-axis
      {
      initialBounds[4] = rootBounds[4];
      initialBounds[5] = rootBounds[5];
      }
    else if(obj->getPlaneProjectionNormal()==1) // y-axis
      {
      initialBounds[2] = rootBounds[2];
      initialBounds[3] = rootBounds[3];
      }
    else // x-axis
      {
      initialBounds[0] = rootBounds[0];
      initialBounds[1] = rootBounds[1];
      }
    }

  double pos[3];
  double mindis;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkImplicitSelectionLoop> loop = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
  bool findone = false;
  // Now try to find a point that satisfy all constraints within the bounds
  for(int tryIdx=0; tryIdx < MAX_RANDOM_PLACEMENT_TRY; tryIdx++)
    {
    if(!this->getGlyphPoint(pos, glyphPoints, glyphPlaybackOption, initialBounds))
      {
      break;
      }
    findone = true;
    for(int i=0; i<constraints->count(); i++)
      {
      node = constraints->keys().value(i);
      invert = constraints->value(node);
      if(node->getDataObject()->getType() == pqCMBSceneObjectBase::VOI)
        {
        vtkBoundingBox bb;
        node->getBounds(&bb);
        //Considering only 2D bounding box along XY plane
        if((!invert && !this->boundingBoxContainsPoint(&bb, pos)) ||
          (invert && this->boundingBoxContainsPoint(&bb, pos)))
          {
          // nope, try again
          findone = false;
          break;
          }
        }
      else if(node->getDataObject()->getType() == pqCMBSceneObjectBase::Arc)
        {
        pqCMBArc* contourObj = static_cast<pqCMBArc*>(
          node->getDataObject());
        vtkPVArcInfo* arcInfo = contourObj->getArcInfo();
        if(arcInfo && arcInfo->GetNumberOfPoints())
          {
          points->SetNumberOfPoints(arcInfo->GetNumberOfPoints());
          vtkDoubleArray* contourData = arcInfo->GetPointLocations();
          points->SetData(contourData);
          loop->SetLoop(points);
          // we need to modify the pos[3] for the contour, because the contour is flat
          double normal[3];
          double contourPos[3];
          contourData->GetTuple(0, contourPos);
          if(contourObj->getPlaneProjectionNormal()==2) // z-axis
            {
            contourPos[0] = pos[0]; contourPos[1] = pos[1];
            normal[0]=0.0;normal[1]=0.0;normal[2]=1.0;
            }
          else if(contourObj->getPlaneProjectionNormal()==0) // x-axis
            {
            contourPos[1] = pos[1]; contourPos[2] = pos[2];
            normal[0]=1.0;normal[1]=0.0;normal[2]=0.0;
            }
          else // y-axis
            {
            contourPos[0] = pos[0]; contourPos[2] = pos[2];
            normal[0]=0.0;normal[1]=1.0;normal[2]=0.0;
            }
          loop->SetNormal(normal);
          mindis = loop->EvaluateFunction(contourPos);
          if((invert && mindis<=0) || (!invert && mindis>0))
            {
            // nope, try again
            findone = false;
            break;
            }
          }
        }
      }//end for each constraint
      if(findone)
      {
      p[0] = pos[0];
      p[1] = pos[1];
      p[2] = pos[2];
      return true;
      }
    }// end for each try

  p[0] = pos[0];
  p[1] = pos[1];
  p[2] = pos[2];
  return false;

}

//-----------------------------------------------------------------------------
bool pqCMBSceneTree::boundingBoxContainsPoint(vtkBoundingBox *bb, double p[])
{
  //Considering only 2D bounding box i.e. along XY plane
  if(p[0] > bb->GetMaxPoint()[0] || p[0] < bb->GetMinPoint()[0])
    {
    return false;
    }
  if(p[1] > bb->GetMaxPoint()[1] || p[1] < bb->GetMinPoint()[1])
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBSceneTree::IsTemporaryPtsFileForMesherNeeded(QStringList &surfaceNames)
{
  if (surfaceNames.count() > 1)
    {
    return true;
    }

  SceneObjectNodeIterator nodeIter( this->getRoot() );
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::Points );
  pqCMBSceneNode *lidarNode;
  pqCMBPoints *pobj;
  while((lidarNode = nodeIter.next()))
    {
    if (surfaceNames.contains(lidarNode->getName()))
      {
      pobj = dynamic_cast<pqCMBPoints*>(lidarNode->getDataObject());
      if (pobj->getReaderSource())
        {
        // if not LIDARReader, need to create LIDAR Pts file
        if (strcmp("vtkLIDARReader",
          pobj->getReaderSource()->getProxy()->GetVTKClassName()))
          {
          // not LIDARReader
          return true;
          }
        else // must be LIDARReader... need to find out how many pieces in the file
          {
          vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(
            pobj->getReaderSource()->getProxy() );
          source->UpdatePropertyInformation();

          if (pqSMAdaptor::getElementProperty(
            source->GetProperty("KnownNumberOfPieces")).toInt() > 1)
            {
            return true;
            }
          }
        }
      else
        {
        // if no readerSource, check to see if coming from a pts file
        // (probably reading an osd.txt file with pts file within,
        // but could be vtk or something else)
        QFileInfo info(pobj->getFileName().c_str());
        QString extension(info.completeSuffix());
        if (extension == "pts" || extension == "bin.pts" || extension == "bin")
          {
          return false;  // just assume a single piece
          }
        return true;
        }
      }
    }

  return false;
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::addUserDefinedType(const char *typeName, bool cleanList)
{
  this->UserObjectTypes.append(typeName);
  if (cleanList)
    {
    this->cleanUpUserDefinedTypes();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::cleanUpUserDefinedTypes()
{
  this->UserObjectTypes.sort();
  this->UserObjectTypes.removeDuplicates();
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::changeUserDefineObjectTypeAction()
{
  if (this->Selected.size() == 1)
    {
    qtCMBUserTypeDialog::updateUserType(this->Selected[0]);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::createArcWidgetManager()
{
  this->ArcWidgetManager = new qtCMBArcWidgetManager(this->CurrentServer, this->CurrentView);

  //we connect to the free / busy so we can lock down the UI
  QObject::connect( this->ArcWidgetManager, SIGNAL(Ready()),this,SLOT(arcWidgetReady()));
  QObject::connect( this->ArcWidgetManager, SIGNAL(Busy()),this,SLOT(arcWidgetBusy()));
  QObject::connect( this->ArcWidgetManager,
    SIGNAL(ArcSplit(pqCMBSceneNode*,QList<vtkIdType>)),this,SLOT(updateArcsAfterSplit(pqCMBSceneNode*,QList<vtkIdType>)));
  QObject::connect( this->ArcWidgetManager,
    SIGNAL(ArcModified(pqCMBSceneNode*)),this,SLOT(updateArc(pqCMBSceneNode*)));

  //make sure the contour manager always has the same server and view the tree does
  QObject::connect( this, SIGNAL(newCurrentView(pqRenderView*)),
    this->ArcWidgetManager,SLOT(updateActiveView(pqRenderView*)));
  QObject::connect( this, SIGNAL(newCurrentServer(pqServer*)),
    this->ArcWidgetManager,SLOT(updateActiveServer(pqServer*)));
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::arcWidgetReady()
  {
  //we connect the manager on the free signal so that the edit menu will be always updated
  //when a user finished editing / creating a contour
  emit this->focusOnSceneTab();

  //unlock the tree
  this->enableSceneTree(true);

  this->nodesSelected();
  }
//-----------------------------------------------------------------------------
void pqCMBSceneTree::arcWidgetBusy()
  {
  //on this signal we need to move to the display tab
  emit this->focusOnDisplayTab();

  //and than lock the UI!
  this->enableSceneTree(false);
  }

//-----------------------------------------------------------------------------
void pqCMBSceneTree::enableSceneTree(const bool &lock)
{
  //lock the tree
  this->TreeWidget->setEnabled(lock);
  if (this->DeleteAction)
    {
    this->DeleteAction->setEnabled(lock);
    }
  emit this->enableMenuItems(lock);
}

//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::getSceneObjectNode(
  pqCMBSceneObjectBase* obj)
{
  if(!obj || !this->getRoot())
    {
    return NULL;
    }
  pqCMBSceneNodeIterator iter(this->getRoot());
  pqCMBSceneNode *n;
  while((n = iter.next()))
    {
    if (n->getDataObject() == obj)
      {
      return n;
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
pqCMBSceneNode* pqCMBSceneTree::getSceneNodeByName(
  const char* nodename)
{
  if(!nodename || !this->getRoot())
    {
    return NULL;
    }
  pqCMBSceneNodeIterator iter(this->getRoot());
  pqCMBSceneNode *n;
  while((n = iter.next()))
    {
    if (strcmp(n->getName(), nodename)==0)
      {
      return n;
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateArcsAfterSplit(pqCMBSceneNode* arcNode,QList<vtkIdType> newArcIds)
{
  // Creating the root should not be undoable
  bool recordingState = this->RecordEvents;
  this->RecordEvents = false;

  //this should be an operator
  pqCMBArc *csObj = dynamic_cast<pqCMBArc*>(arcNode->getDataObject());
  if (!csObj)
    {
    return;
    }
  //signal that this arc is dirty and all polygons that use it
  //must remesh
  csObj->arcIsModified();

  std::vector<pqCMBSceneNode*> newArcNodes;
  vtkIdType id;
  foreach(id,newArcIds)
    {
    //take all the objects on the server and now create arcs for each
    pqCMBArc* obj = new pqCMBArc();
    obj->createArc(id);
    obj->inheritPolygonRelationships(csObj);

    //we now need to set the correct plane information on the polyline
    obj->setPlaneProjectionNormal(csObj->getPlaneProjectionNormal());
    obj->setPlaneProjectionPosition(csObj->getPlaneProjectionPosition());

    pqCMBSceneNode *node =
      this->createNode(arcNode->getName(),arcNode->getParent(),obj,NULL);

    newArcNodes.push_back(node);
    }

  this->RecordEvents = recordingState;

  this->refreshArcsAndPolygons();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::updateArc(pqCMBSceneNode* arcNode)
{
  // Creating the root should not be undoable
  bool recordingState = this->RecordEvents;
  this->RecordEvents = false;

  //this should be an operator
  pqCMBArc *csObj = dynamic_cast<pqCMBArc*>(arcNode->getDataObject());
  if (csObj)
    {
    //signal that this arc is dirty and all polygons that use it
    //must remesh
    csObj->arcIsModified();
    }
  this->RecordEvents = recordingState;

  this->refreshArcsAndPolygons();
}

//-----------------------------------------------------------------------------
void pqCMBSceneTree::refreshArcsAndPolygons()
{
  if(!this->Root)
    {
    return;
    }
  SceneObjectNodeIterator iter(this->Root);
  iter.setTypeFilter(pqCMBSceneObjectBase::Arc);
  pqCMBSceneNode *node;
  while((node = iter.next()))
    {
    node->getDataObject()->updateRepresentation();
    }

  //just to be safe we update the polygons after the arcs have finished
  //updating
  iter.reset();
  iter.setTypeFilter(pqCMBSceneObjectBase::Polygon);
  while((node = iter.next()))
    {
    pqCMBPolygon* poly =
                dynamic_cast<pqCMBPolygon*>(node->getDataObject());

    poly->updateRepresentation();
    }

}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::convertNodesToGlyphs()
{
  // For each node in the selected list we will convert all faceted object children nodes
  // into a single glyph node.  If a leaf node is selected then all of the children nodes
  // of the leaf's parent that share the same filename will be converted
  // Copy the selection list in case the conversion would changed the nodes in the list

  // We don't know how many nodes are being replaced so we have to guess
  cmbSceneNodeReplaceEvent *event = new cmbSceneNodeReplaceEvent(10, 100);
  this->insertEvent(event);

  std::vector<pqCMBSceneNode *> nodes = this->Selected;
  int i, n = static_cast<int>(nodes.size());
  pqCMBFacetedObject *fobj;
  pqCMBSceneNode *child;
  for (i = 0; i < n; i++)
    {
    // If for some reason the node is already marked for deletion or
    // if the node is invisible then skip it
    if (nodes[i]->isMarkedForDeletion() || (!nodes[i]->isVisible()))
      {
      continue;
      }
    // Is this a leaf node?
    if (!nodes[i]->isTypeNode())
      {
      fobj = dynamic_cast<pqCMBFacetedObject*>(nodes[i]->getDataObject());
      // if the node does not contain a faceted object or its parent is
      // invisible (note that the node maybe invisible but its parent is not)
      if (!fobj)
        {
        continue;
        }
      this->convertChildrenToGlyphs(nodes[i]->getParent(), fobj->getFileName(),
                                    event);
      }
    else // This is a node with children
      {
      int j, m = static_cast<int>(nodes[i]->getChildren().size());
      for (j = 0; j < m; j++)
        {
        child = nodes[i]->getChildren()[j];
        // Skip the node if it is marked for deletion or is not a leaf node
        // since we are not doing this recursively
        if (child->isMarkedForDeletion() || child->isTypeNode())
          {
          continue;
          }
        fobj = dynamic_cast<pqCMBFacetedObject*>(child->getDataObject());
        if (fobj)
          {
          this->convertChildrenToGlyphs(nodes[i], fobj->getFileName(),
                                        event);
          }
        }
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::convertChildrenToGlyphs(pqCMBSceneNode *node,
                                             const std::string &filename,
                                             cmbSceneNodeReplaceEvent *event)
{
  // Convert all the children of the node that contain faceted object based on the
  // same filename to a single glyph node
  pqCMBGlyphObject *gobj = NULL;
  pqCMBSceneNode *child, *newNode = NULL;
  pqCMBFacetedObject *fobj;
  double color[4];
  double pos[3], ori[3], scale[3];
  pqCMBSceneObjectBase::enumSurfaceType stype;
  std::string userType;
  int gcount = 0;
  int i, n = static_cast<int>(node->getChildren().size());
  for (i = 0; i < n; i++)
    {
    child = node->getChildren()[i];
    // Skip all deleted nodes, invisible nodes, and nodes that don't have data objects
    // associated with them
    if (child->isMarkedForDeletion() || child->isTypeNode() || (!child->isVisible()))
      {
      continue;
      }
    fobj = dynamic_cast<pqCMBFacetedObject*>(child->getDataObject());
    //If the node does not contain a facet object or the object does
    // not use the same file then skip it
    if (!(fobj && (fobj->getFileName() == filename)))
      {
      continue;
      }

    // Is this the first node we've come across that is to be replaced?
    if (!gobj)
      {
      gobj = new pqCMBGlyphObject(fobj->getSource(), this->CurrentView,
                                     this->CurrentServer, filename.c_str());
      newNode = this->createNode(child->getName(), node, gobj, event);
      child->getColor(color);
      newNode->setExplicitColor(color);
      userType = fobj->getUserDefinedType();
      gobj->setUserDefinedType(userType.c_str());
      stype = fobj->getSurfaceType();
      gobj->setSurfaceType(stype);
      }
    fobj->getPosition(pos);
    fobj->getScale(scale);
    fobj->getOrientation(ori);
    gobj->insertNextPoint(pos);
    gobj->setOrientation(gcount, ori);
    gobj->setScale(gcount, scale);
    ++gcount;
    this->deleteNode(child, event);
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::unsetTextureMap()
{
  if(this->CurrentTextureObj)
    {
    this->CurrentTextureObj->unsetTextureMap();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::setTextureMap(const QString& filename, int numberOfRegistrationPoints,
                                       double *points)
{
  if(this->CurrentTextureObj)
    {
    if(this->CurrentTextureObj->hasTexture())
      {
      this->CurrentTextureObj->unsetTextureMap();
      this->CurrentView->forceRender();
      }
    this->CurrentTextureObj->setTextureMap(
      filename.toAscii().data(), numberOfRegistrationPoints, points);
    }
  this->addTextureFileName(filename.toAscii().data());
}
//-----------------------------------------------------------------------------
void pqCMBSceneTree::addTextureFileName(const char *filename)
{
  if (filename && !this->TextureFiles.contains(filename))
    {
    this->TextureFiles.append(filename);
    }
}
