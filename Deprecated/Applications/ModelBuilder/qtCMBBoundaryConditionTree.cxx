//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBBoundaryConditionTree.h"

// cmb headers
#include "pqCMBModelFace.h"
#include "pqCMBTreeItem.h"
#include "pqCMBModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelUserName.h"
#include "vtkModelItemIterator.h"

// Qt headers
#include <QStringList>
#include <QHeaderView>
#include <QPalette>
#include <QLayout>
#include <QVBoxLayout>

enum BCSNodeType
{
  UndefinedNodesGroup=10001,
};

#define BCTree_BC_COL       0
//#define BCTree_BC_COL     2

#define BC_TEXT_BASE       "BC "
//-----------------------------------------------------------------------------
qtCMBBoundaryConditionTree::qtCMBBoundaryConditionTree(pqCMBModel* cmbModel)
  : qtCMBTree(cmbModel)
{
  this->undefinedGroupNode = NULL;
  this->undefined3DEdgesNode = NULL;
  this->NumberOfSelectedRemovableItems = 0;
  this->NumberOfSelectedBCS = 0;
}

//-----------------------------------------------------------------------------
qtCMBBoundaryConditionTree::~qtCMBBoundaryConditionTree()
{
}

//-----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::clear(bool blockSignal)
{
  if(this->undefinedGroupNode)
    {
    this->undefinedGroupNode->takeChildren();
    delete this->undefinedGroupNode;
    this->undefinedGroupNode = NULL;
    }
  if(this->undefined3DEdgesNode)
    {
    this->undefined3DEdgesNode->takeChildren();
    delete this->undefined3DEdgesNode;
    this->undefined3DEdgesNode = NULL;
    }

  this->Superclass::clear(blockSignal);
}
//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::createWidget(QWidget* parent)
{
  QLayout* boxlayout = parent->layout();
  if(!boxlayout)
    {
    boxlayout = new QVBoxLayout(parent);
    }
  boxlayout->setObjectName("bcsTreeLayout");
  boxlayout->setMargin(0);
  this->TreeWidget = new qtCMBTreeWidget(parent);
  this->TreeWidget->setObjectName("bcsTreeWidget");
  boxlayout->addWidget(this->TreeWidget);

  this->customizeTreeWidget();
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::customizeTreeWidget()
{
  qtCMBTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->setColumnCount(2);

  treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
  this->qtCMBTree::customizeTreeWidget();

  treeWidget->setAcceptDrops(true);
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::initializeTree()
{
  this->clear(true);
  QTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  treeWidget->model()->blockSignals(true);
  treeWidget->setHeaderLabels(
              QStringList() << tr("Entity Set") << tr("Visibility"));
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  Qt::ItemFlags bcFlags(commFlags | Qt::ItemIsDropEnabled |
    Qt::ItemIsEditable);

  vtkDiscreteModel *cmbModel = this->CMBModel->getModel();
  vtkModelItemIterator* iter=cmbModel->NewIterator(vtkDiscreteModelEntityGroupType);

  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkDiscreteModelEntityGroup* bcsEntity =
      vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem());
    if(bcsEntity)
      {
      // Materials node
      QString bcsTextBase = QString::number(bcsEntity->GetUniquePersistentId());
      const char* bcsName = vtkModelUserName::GetUserName(bcsEntity);
      if(bcsName)
        {
        bcsTextBase = bcsName;
        }

      QTreeWidgetItem* bcsNode = this->createTreeNode(
        treeWidget->invisibleRootItem(), BCTree_BC_COL, bcsEntity,
        bcFlags, bcsTextBase);
      vtkModelItemIterator* iterFace=bcsEntity->NewIterator(
        dim2D ? vtkModelEdgeType : vtkModelFaceType);
      for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
        {
        vtkModelEntity* faceEntity =
          vtkModelEntity::SafeDownCast(iterFace->GetCurrentItem());
        if(faceEntity)
          {
          this->createEntityNode(bcsNode, BCTree_BC_COL,
            faceEntity, commFlags, false, false);
          }
        }
      iterFace->Delete();
      }
    }
  iter->Delete();

  // Create the undefined group
  if(!this->undefinedGroupNode)
    {
    this->undefinedGroupNode = this->createTreeNode(
      treeWidget->invisibleRootItem(), BCTree_BC_COL, NULL,
      commFlags, QString("Undefined"),
      true, UndefinedNodesGroup);
    }
  if(this->CMBModel->getModelDimension() == 3 &&
     this->CMBModel->has2DEdges())
    {
    this->undefinedGroupNode->setText(BCTree_BC_COL, "Undefined Faces");
    if(!this->undefined3DEdgesNode)
      {
      this->undefined3DEdgesNode = this->createTreeNode(
        treeWidget->invisibleRootItem(), BCTree_BC_COL, NULL,
        commFlags, QString("Undefined Edges"),
        true, UndefinedNodesGroup);
      }
    }
  this->updateUndifinedBCSGroup();

  treeWidget->sortByColumn(BCTree_BC_COL, Qt::AscendingOrder);
  treeWidget->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  treeWidget->expandAll();
  treeWidget->model()->blockSignals(false);
  treeWidget->blockSignals(false);
}

//----------------------------------------------------------------------------
int qtCMBBoundaryConditionTree::getNameColumn()
{
  return BCTree_BC_COL;
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::updateUndifinedBCSGroup()
{
  QMap<vtkIdType, pqCMBModelEntity*> faceMap =
    this->CMBModel->GetCurrentModelEntityMap();

  QList<vtkIdType> faceIds=faceMap.uniqueKeys();
  QList<vtkIdType> undefinedBCFaces;

  vtkModelEntity* modelEntity = NULL;

  for(int i=0; i<faceIds.count(); i++)
    {
    modelEntity = faceMap[faceIds.value(i)]->getModelEntity();
    if(!modelEntity->GetNumberOfAssociations(vtkDiscreteModelEntityGroupType))
      {
      undefinedBCFaces.append(faceIds.value(i));
      }
    }
  this->updateUndefinedNode(faceMap, undefinedBCFaces, this->undefinedGroupNode);
  if(this->CMBModel->getModelDimension() == 3 &&
     this->CMBModel->has2DEdges())
    {
    this->undefinedGroupNode->setText(BCTree_BC_COL, "Undefined Faces");
    QMap<vtkIdType, pqCMBModelEntity*> edgeMap =
      this->CMBModel->Get2DEdgeID2EdgeMap();
    QList<vtkIdType> edgeIds=edgeMap.uniqueKeys();
    QList<vtkIdType> undefinedBCEdges;
    for(int i=0; i<edgeIds.count(); i++)
      {
      modelEntity = edgeMap[edgeIds.value(i)]->getModelEntity();
      if(!modelEntity->GetNumberOfAssociations(vtkDiscreteModelEntityGroupType))
        {
        undefinedBCEdges.append(edgeIds.value(i));
        }
      }
    this->updateUndefinedNode(edgeMap, undefinedBCEdges, this->undefined3DEdgesNode);
    this->undefined3DEdgesNode->setText(BCTree_BC_COL, "Undefined Edges");
    }
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::updateUndefinedNode(
  QMap<vtkIdType, pqCMBModelEntity*> & entityMap,
  QList<vtkIdType> & undefinedEntities,
  QTreeWidgetItem* undefNode)
{
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);

  QList<QTreeWidgetItem*> remList;
  QList<vtkIdType> existingEntities;
  if(!undefNode)
    {
    Qt::ItemFlags commFlags(
      Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    undefNode = this->createTreeNode(
      treeWidget->invisibleRootItem(), BCTree_BC_COL, NULL,
      commFlags, "Undefined", true, UndefinedNodesGroup);
    }
  vtkIdType entityId;
  QTreeWidgetItem* item = undefNode;
  for(int c=0; c<item->childCount(); c++)
    {
    //faceId = item->child(c)->text(BCTree_BC_COL).toInt();
    entityId = this->getItemObject(item->child(c))->GetUniquePersistentId();
    if(undefinedEntities.contains(entityId))
      {
      existingEntities.append(entityId);
      }
    else
      {
      remList.append(item->child(c));
      }
    }

  // remove faces that are not there anymore
  for(int face=0; face<remList.count(); face++)
    {
    //item->removeChild();
    delete remList.value(face);
    }

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

  for(int face=0; face<undefinedEntities.count(); face++)
    {
    entityId = undefinedEntities.value(face);
    if(!existingEntities.contains(entityId))
      {
      vtkModelEntity* entity =
        entityMap[entityId]->getModelEntity();
      if(entity)
        {
        this->createEntityNode(item, BCTree_BC_COL,
          entity, commFlags, false, false);
        }
      }
    }
  treeWidget->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::onGroupClicked(QTreeWidgetItem* item, int col)
{
  // Change visibility
  int nodeType = item->type();
  int visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
  if(col == TREE_VISIBLE_COL)
    {
    // Change visible icon
    int itemVisible = !visible;

    //we also need to change the parent icon accordingly.
    //If the child is visible, the parent has to be visible first
    if(itemVisible)
      {
      QTreeWidgetItem* parent = NULL;
      if(nodeType == UndefinedNodesGroup ||
        this->getItemObject(item)->GetType() == vtkDiscreteModelEntityGroupType)
        {
        parent = item->parent();
        }

      if(parent)
        {
        parent->setIcon(TREE_VISIBLE_COL, *this->IconVisible);
        parent->setData(TREE_VISIBLE_COL, Qt::UserRole, itemVisible);
        }
      }

    QList<vtkIdType> changedFaces;
    this->changeChildItemVisibilityIcon(item, itemVisible, changedFaces);
    if(itemVisible)
      {
      this->onGroupSelectionChanged();
      }
    else
      {
      this->TreeWidget->clearSelection();
      }
    }
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::onGroupSelectionChanged()
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* item = NULL;
  int nodeType, visible;
  vtkIdType faceId;

  QList<QTreeWidgetItem*> selBCs = treeWidget->selectedItems();
  QList<vtkIdType> selFaceIds;
  int numRemovable=0;
  for(int i=0; i<selBCs.count();i++)
    {
    item = selBCs.value(i);
    nodeType = (item->type()==UndefinedNodesGroup) ? UndefinedNodesGroup :
      this->getItemObject(item)->GetType();
    if(nodeType == UndefinedNodesGroup ||
      nodeType == vtkDiscreteModelEntityGroupType)
      {
      numRemovable = (nodeType == UndefinedNodesGroup) ?
        numRemovable : numRemovable+1;
      visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
      if(!visible)
        {
        continue;
        }
      for(int r=0; r<item->childCount(); r++)
        {
        //faceId = item->child(r)->text(BCTree_BC_COL).toInt();
        faceId = this->getItemObject(item->child(r))->GetUniquePersistentId();
        if(!selFaceIds.contains(faceId))
          {
          selFaceIds.append(faceId);
          }
        }
      }
    else if(nodeType == vtkModelFaceType || nodeType == vtkModelEdgeType)
      {
      if(item->parent()->type()!= UndefinedNodesGroup)
        {
        numRemovable++;
        }
      //faceId = item->text(BCTree_BC_COL).toInt();
      faceId = this->getItemObject(item)->GetUniquePersistentId();
      if(!selFaceIds.contains(faceId))
        {
        selFaceIds.append(faceId);
        }
      }
    }

  if(selFaceIds.count()>0)
    {
    this->CMBModel->clearAllEntityHighlights(false);
    this->CMBModel->highlightModelEntities(selFaceIds);
    }
  else
    {
    this->CMBModel->clearAllEntityHighlights();
    }

  this->NumberOfSelectedBCS = selBCs.count();
  this->NumberOfSelectedRemovableItems = numRemovable;

  emit this->selectionChanged(this);

}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::onItemsDroppedOnItem(
  QTreeWidgetItem* parentItem, QDropEvent*)
{
  if(!parentItem)
    {
    return;
    }
  qtCMBTree* sourceWidget = this->DragFromTree;
  if(!sourceWidget)
    {
    return;
    }
  QTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);

  // BCS node
  QTreeWidgetItem *bcNode = NULL;
  if(parentItem == treeWidget->invisibleRootItem())
    {
    vtkDiscreteModelEntityGroup* bcsGroup = this->CMBModel->createBCS();
    bcNode = this->createRootEntityNode(bcsGroup, BCTree_BC_COL);
    }
  else if(parentItem->type() != UndefinedNodesGroup)
    {
    if(this->getItemObject(parentItem))
      {
      int nodeType = this->getItemObject(parentItem)->GetType();
      if(nodeType == vtkDiscreteModelEntityGroupType)
        {
        bcNode = parentItem;
        }
      else if(nodeType == vtkModelFaceType || nodeType == vtkModelEdgeType)
        {
        bcNode = parentItem->parent();
        }
      }
    }

  if(bcNode)
    {
    bcNode->setExpanded(true);
    this->copyBCSItemsToNode(bcNode,
      sourceWidget->getSelectedItems());
    this->updateUndifinedBCSGroup();
    }
  treeWidget->blockSignals(false);

  if(bcNode)
    {
    treeWidget->setCurrentItem(bcNode);
    bcNode->setSelected(true);
    this->CMBModel->onLookupTableModified();
    }

  this->DragFromTree = NULL;
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::onGroupChanged(QTreeWidgetItem* item, int col)
{
  if(col != BCTree_BC_COL || item == this->undefinedGroupNode)
    {
    return;
    }
  int nodeType = this->getItemObject(item)->GetType();
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  if(nodeType == vtkDiscreteModelEntityGroupType)
    {
    vtkIdType bcId = this->getItemObject(item)->GetUniquePersistentId();
    this->CMBModel->modifyUserSpecifiedName(
      vtkDiscreteModelEntityGroupType, bcId, item->text(BCTree_BC_COL).toStdString().c_str());
    }
//  this->updateUndifinedBCSGroup();

  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::changeChildItemVisibilityIcon(
  QTreeWidgetItem* item, int visible, QList<vtkIdType>& changedFaces)
{
  QIcon *visIcon = visible ? this->IconVisible :
    this->IconInvisible;
  vtkModelEntity* entity = this->getItemObject(item);
  if(item->type() == UndefinedNodesGroup ||
    (entity && entity->GetType() == vtkDiscreteModelEntityGroupType))
    {
    item->setIcon(TREE_VISIBLE_COL, *visIcon);
    }
  item->setData(TREE_VISIBLE_COL, Qt::UserRole, visible);

  for(int i=0; i<item->childCount(); i++)
    {
    this->changeChildItemVisibilityIcon(item->child(i), visible, changedFaces);
    }

  vtkIdType faceId;
  if(entity && (entity->GetType()==vtkModelFaceType ||
    entity->GetType()==vtkModelEdgeType))
    {
    //faceId = item->text(BCTree_BC_COL).toInt();
    faceId = entity->GetUniquePersistentId();
    if(!visible)
      {
      if(item->parent()->type() == UndefinedNodesGroup)
        {
        this->CMBModel->changeModelEntityVisibility(faceId, visible);
        if(!changedFaces.contains(faceId))
          {
          changedFaces.append(faceId);
          }
        }
      else
        {
        vtkIdType bcId = this->getItemObject(item->parent())
          ->GetUniquePersistentId();
        if(!this->IsBCFaceVisibleInOtherBCS(faceId, bcId))
          {
          this->CMBModel->changeModelEntityVisibility(faceId, visible);
          if(!changedFaces.contains(faceId))
            {
            changedFaces.append(faceId);
            }
          }
        }
      }
    else
      {
//      if(this->IsEntityVisibleInTree(faceId, this->TreeWidget))
//        {
        this->CMBModel->changeModelEntityVisibility(faceId, visible);
//        }
      }
    }
}

//-----------------------------------------------------------------------------
bool qtCMBBoundaryConditionTree::IsBCFaceVisibleInOtherBCS(
  vtkIdType faceId, vtkIdType BCId)
{
  // Change visibility
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();

  int visible;
  vtkIdType bFaceId, bBCId;
  for(int r=0; r<rootNode->childCount(); r++)
    {
    QTreeWidgetItem* item = rootNode->child(r);
    if(item->type() != UndefinedNodesGroup)
      {
      bBCId = this->getItemObject(item)->GetUniquePersistentId();
      if(bBCId == BCId)
        {
        continue;
        }

      visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
      if(visible && item->childCount()>0)
        {
        for(int i=0; i<item->childCount();i++)
          {
          //bFaceId = item->child(i)->text(BCTree_BC_COL).toInt();
          bFaceId = this->getItemObject(item->child(i))->GetUniquePersistentId();
          if(bFaceId == faceId)
            {
            return true;
            }
          }
        }
      }
    }

  return false;
}
//-----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::copyBCSItemsToNode(
  QTreeWidgetItem* copytoNode, QList<QTreeWidgetItem*> selItems)
{
  vtkDiscreteModelEntityGroup* groupentity =
    vtkDiscreteModelEntityGroup::SafeDownCast(this->getItemObject(copytoNode));
  if(copytoNode->type() == UndefinedNodesGroup || !groupentity)
    {
    return;
    }

  QTreeWidgetItem* copyItem;
  QList<QTreeWidgetItem*> newChildren;
  for(int n=0; n<selItems.count(); n++)
    {
    copyItem = selItems.value(n);

    if(copyItem->childCount()==0)
      {
      if(copyItem->type() != UndefinedNodesGroup &&
        (this->getItemObject(copyItem)->GetType() == vtkModelFaceType ||
        this->getItemObject(copyItem)->GetType() == vtkModelEdgeType) &&
        !newChildren.contains(copyItem))
        {
        newChildren.append(copyItem);
        }
      }
    else
      {
      // for 3d model edges
      if(copyItem->type() != UndefinedNodesGroup &&
        this->getItemObject(copyItem) &&
        this->getItemObject(copyItem)->GetType() == vtkModelFaceType &&
        !newChildren.contains(copyItem))
        {
        newChildren.append(copyItem);
        }
      this->addUniqueChildren(copyItem, newChildren);
      }
    }

  QList<vtkIdType> curFaces;
  for(int i=0; i<copytoNode->childCount(); i++)
    {
    curFaces.append(this->getItemObject(copytoNode->child(i))->
      GetUniquePersistentId());
    }

  vtkIdType bcId = groupentity->GetUniquePersistentId();
  int bcType = groupentity->GetEntityType();
  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

  // Copying the nodes
  // These tree node could be dragged from the material tree.
  QTreeWidgetItem* faceNode = NULL;
  QList<vtkIdType> newfaces;
  vtkIdType faceId;
  vtkModelEntity* entity = NULL;
  for(int f=0; f<newChildren.count(); f++)
    {
    faceNode = newChildren.value(f);
    entity = this->getItemObject(faceNode);
    faceId = entity->GetUniquePersistentId();
    if(!curFaces.contains(faceId) && entity->GetType() == bcType)
      {
      faceNode = this->createEntityNode(copytoNode, BCTree_BC_COL,
            this->getItemObject(faceNode),
            commFlags, false, false);
      curFaces.append(faceId);
      newfaces.append(faceId);
      }
    }

  this->CMBModel->addEntitiesToBCGroups(bcId, newfaces);
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::updateFaceNodeText(
  vtkModelEntity* faceEntity)
{
  QString fName(vtkModelUserName::GetUserName(faceEntity));
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);

  QTreeWidgetItem* rootItem=treeWidget->invisibleRootItem();
  for(int c=0; c<rootItem->childCount(); c++)
    {
    QTreeWidgetItem* pItem = rootItem->child(c);
    for(int i=0; i<pItem->childCount(); i++)
      {
      if(faceEntity == this->getItemObject(pItem->child(i)))
        {
        pItem->child(i)->setText(BCTree_BC_COL, fName);
        }
      }
    }
}

//----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBBoundaryConditionTree::createBCNodeWithEntities(
  vtkIdType bcId, QList<vtkIdType>& bcsFaces)
{
  QTreeWidgetItem* bcNode = this->createBCNode(bcId);
  if(bcNode)
    {
    this->TreeWidget->blockSignals(true);
    bcNode->setExpanded(true);
    Qt::ItemFlags commFlags(
      Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    vtkModelEntity* faceEntity = NULL;
    for(int j=0; j<bcsFaces.count(); j++)
      {
      faceEntity = this->CMBModel->GetCurrentModelEntityMap()[bcsFaces.value(j)]->
        getModelEntity();
      this->createEntityNode(bcNode, BCTree_BC_COL,
        faceEntity, commFlags, false, false);
      }
    this->updateUndifinedBCSGroup();
    this->TreeWidget->blockSignals(false);
    }
  return bcNode;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBBoundaryConditionTree::createBCNode(vtkIdType bcId)
{
  if(bcId >=0)
    {
    vtkModelEntity* modelEntity =
      this->CMBModel->getModel()->GetModelEntity(
      vtkDiscreteModelEntityGroupType, bcId);
    if(modelEntity)
      {
      QTreeWidgetItem* newBCNode =
        this->addNewTreeNodeOnRoot(BCTree_BC_COL, modelEntity);
      if(newBCNode)
        {
        this->TreeWidget->sortByColumn(
          BCTree_BC_COL, Qt::AscendingOrder);
        // this->UpdateInfoTable();
        return newBCNode;
        }
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::addNewEntitiesFromSplitToBCGroups(
  QMap< vtkIdType, QList<vtkIdType> >& changedFaces)
{
  if(changedFaces.count()==0)
    {
    return;
    }
  QList<vtkIdType> splitFaces = changedFaces.keys();
  vtkIdType entityId = splitFaces.value(0);
  vtkModelEntity* modEntity = this->CMBModel->getModel()->GetModelEntity(entityId);
  if(!modEntity || (modEntity->GetType() != vtkModelFaceType &&
    modEntity->GetType() != vtkModelEdgeType))
    {
    return;
    }
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    modEntity->GetType() == vtkModelFaceType ?
    this->CMBModel->GetFaceIDToFaceMap() :
  this->CMBModel->Get2DEdgeID2EdgeMap();

  QTreeWidget* treeWidget =this->TreeWidget;
  treeWidget->blockSignals(true);

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  vtkIdType faceId;
  QTreeWidgetItem* rootItem = treeWidget->invisibleRootItem();
  QTreeWidgetItem* bcNode = NULL;
  QList<vtkIdType> newfaces;
  vtkModelEntity* faceEntity = NULL;

  for(int r=0; r<rootItem->childCount(); r++)
    {
    bcNode=rootItem->child(r);
    for(int f=0; f<bcNode->childCount(); f++)
      {
      faceEntity = this->getItemObject(bcNode->child(f));
      if(faceEntity)
        {
        faceId = faceEntity->GetUniquePersistentId();
        // if this face is one of the split faces/edges,
        // we put the new faces/edges as its siblings
        if(splitFaces.contains(faceId))
          {
          newfaces = changedFaces[faceId];
          for(QList<vtkIdType>::iterator it=newfaces.begin(); it != newfaces.end(); it++)
          //for(int face=0; face<newfaces.count(); face++)
            {
            faceId = *it;
            if(!splitFaces.contains(faceId))
              {
              faceEntity = entityMap[faceId]->getModelEntity();
              if(faceEntity)
                {
                this->createEntityNode(bcNode, BCTree_BC_COL,
                  faceEntity, commFlags, false, false);
                }
              }
            }
          }
        }
      }
    }

//  this->updateUndifinedBCSGroup();
  treeWidget->sortByColumn(BCTree_BC_COL, Qt::AscendingOrder);
  treeWidget->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::removeEntitiesFromBCGroups(QList<vtkIdType>& faces)
{
  QTreeWidget* treeWidget = this->TreeWidget;
  QTreeWidgetItem* item;
  treeWidget->blockSignals(true);

  QList<QTreeWidgetItem*> remItems;
  QTreeWidgetItem* rootItem = treeWidget->invisibleRootItem();
  vtkIdType faceId;
  for(int i=0; i<rootItem->childCount();i++)
    {
    item = rootItem->child(i);
    for(int j=0; j<item->childCount();  j++)
      {
      pqCMBTreeItem* cmbItem = static_cast<pqCMBTreeItem*>(item->child(j));
      if(cmbItem && cmbItem->getModelObject())
        {
        pqCMBModelEntity* cmbEntity =
          static_cast<pqCMBModelEntity*>(cmbItem->getModelObject());
        if(cmbEntity)
          {
          faceId = cmbEntity->getUniqueEntityId();
          if(faces.contains(faceId))
            {
            remItems.append(item->child(j));
            }
          }
        }
      }
    }

  for(int r=0; r<remItems.count();r++)
    {
    delete remItems.value(r);
    }

  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBBoundaryConditionTree::removeSelectedBCNodes()
{
  QTreeWidget* treeWidget = this->TreeWidget;
  QTreeWidgetItem* item = NULL;

  treeWidget->blockSignals(true);
  QList<QTreeWidgetItem*> selItems = treeWidget->selectedItems();

  QList<QTreeWidgetItem*> remBCs;
  QList<QTreeWidgetItem*> remFaces;

  QMap<vtkIdType, QList<vtkIdType> > remBCFaces;

  //remove selected faces first
  int numRemoved = 0;
  vtkIdType bcId, faceId;
  for(int i=0; i<selItems.count();i++)
    {
    item = selItems.value(i);
    if(item != this->undefinedGroupNode &&
      item->parent() != this->undefinedGroupNode)
      {
      vtkModelEntity* modEntity = this->getItemObject(item);
      if(modEntity->GetType() == vtkModelFaceType ||
          modEntity->GetType() == vtkModelEdgeType)
        {
        faceId = modEntity->GetUniquePersistentId();
        bcId = this->getItemObject(item->parent())->GetUniquePersistentId();
        if(remBCFaces.contains(bcId))
          {
          remBCFaces[bcId].append(faceId);
          }
        else
          {
          QList<vtkIdType> faces;
          faces.append(faceId);
          remBCFaces.insert(bcId, faces);
          }
        delete item;
        item = NULL;
        numRemoved++;
        }
      }
    }

  for(int bc=0; bc<remBCFaces.uniqueKeys().count(); bc++)
    {
    vtkIdType remBC = remBCFaces.uniqueKeys().value(bc);
    this->CMBModel->removeModelFacesFromBCS(
      remBC, remBCFaces[remBC]);
    }

  if(numRemoved < selItems.count())
    {
    for(int j=0; j<selItems.count();j++)
      {
      vtkModelEntity* entity = (item != this->undefinedGroupNode) ?
        this->getItemObject(selItems.value(j)) : NULL;
      if( entity && entity->GetType() == vtkDiscreteModelEntityGroupType)
        {
        bcId = entity->GetUniquePersistentId();
        this->CMBModel->removeBCS(bcId);
        delete item;
        item = NULL;
        }
      }
    }

  this->updateUndifinedBCSGroup();

  treeWidget->blockSignals(false);

  this->CMBModel->clearAllEntityHighlights();
  this->CMBModel->onLookupTableModified();
}
