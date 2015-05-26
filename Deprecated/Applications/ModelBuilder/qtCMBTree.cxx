//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBTree.h"

// cmb headers
#include "vtkDiscreteModel.h"
#include "pqCMBModelFace.h"
#include "pqCMBFloatingEdge.h"
#include "pqCMBModelEdge.h"
#include "pqCMBTreeItem.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelUserName.h"
#include "vtkModel.h"

#include "pqDataRepresentation.h"
#include "pqSMAdaptor.h"

// Qt headers
#include <QAction>
#include <QStringList>
#include <QHeaderView>
#include <QIcon>
#include <QPixmap>
#include <QPalette>
#include <QVariant>

//-----------------------------------------------------------------------------
qtCMBTree::qtCMBTree(pqCMBModel* cmbModel)
{
  this->CMBModel = cmbModel;
  QPixmap pix(":/cmb/pqEyeball16.png");
  QPixmap pixd(":/cmb/pqEyeballd16.png");
  this->IconVisible = new QIcon(pix);
  this->IconInvisible = new QIcon(pixd);

  this->Action_ToggleVisibility = NULL;
  this->Action_CreateNew = NULL;
  this->Action_Delete = NULL;

  this->DragFromTree = NULL;

}

//-----------------------------------------------------------------------------
qtCMBTree::~qtCMBTree()
{
  delete this->IconInvisible;
  delete this->IconVisible;
  if(this->Action_ToggleVisibility)
    {
    delete this->Action_ToggleVisibility;
    }
  if(this->Action_CreateNew)
    {
    delete this->Action_CreateNew;
    }
  if(this->Action_Delete)
    {
    delete this->Action_Delete;
    }

  if(this->TreeWidget)
    {
    delete this->TreeWidget;
    }
}

//-----------------------------------------------------------------------------
pqCMBModel* qtCMBTree::getModel()
{
  return this->CMBModel;
}

//----------------------------------------------------------------------------
void qtCMBTree::customizeTreeWidget()
{
  qtCMBTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->setAlternatingRowColors(true);
  treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  for(int col=0; col<treeWidget->columnCount(); col++)
    {
    treeWidget->header()->setResizeMode(col, QHeaderView::ResizeToContents);
    }

  //QObject::connect(treeWidget, SIGNAL(itemEntered(QTreeWidgetItem*, int)),
  //  this, SLOT(onStartEditing(QTreeWidgetItem*, int)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(dragStarted(QTreeWidget*)),
    this, SLOT(onDragStarted(QTreeWidget*)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemClicked (QTreeWidgetItem*, int)),
    this, SLOT(onGroupClicked(QTreeWidgetItem*, int)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(onGroupSelectionChanged()), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*)),
    this, SLOT(onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
    this, SLOT(onGroupChanged(QTreeWidgetItem*, int)));

  QPalette p = treeWidget->palette();
//p.setColor(QPalette::Highlight, Qt::red);
// or even different colors for different color groups (states)
//p.setColor(QPalette::BrightText, QPalette::Highlight, Qt::white);
  p.setColor(QPalette::All, QPalette::Highlight, Qt::blue);
  treeWidget->setPalette(p);

  // treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
  treeWidget->setDropIndicatorShown(true);
  treeWidget->setDragEnabled(true);
}

//----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBTree::createTreeNode(
  QTreeWidgetItem* parentNode,
  int colId, vtkModelEntity* modelEntity,
  Qt::ItemFlags commFlags, const QString& text,
  bool setVisibleIcon, int type, bool setWidget)
{
  //QTreeWidgetItem* mNode = new QTreeWidgetItem(parentNode, type);
  pqCMBTreeItem* mNode = new pqCMBTreeItem(parentNode, type);

  if(setVisibleIcon)
    {
    mNode->setIcon(TREE_VISIBLE_COL, *this->IconVisible);
    }
  mNode->setData(TREE_VISIBLE_COL, Qt::UserRole, 1);

  if(modelEntity)
    {
    pqCMBModelEntity* cmbEntity = NULL;
    if(modelEntity->GetType() == vtkModelFaceType)
      {
      cmbEntity = this->getModel()->
        GetFaceIDToFaceMap()[modelEntity->GetUniquePersistentId()];
      }
    else if(modelEntity->GetType() == vtkModelEdgeType)
      {
      if(this->getModel()->getModel()->GetModelDimension() == 3)
        {
        // floating edge
        if(modelEntity->GetNumberOfAssociations(vtkModelRegionType))
          {
          cmbEntity = this->getModel()->
            Get3DFloatingEdgeId2EdgeMap()[modelEntity->GetUniquePersistentId()];
          }
        else // edges in 3D model
          {
          cmbEntity = this->getModel()->
            Get2DEdgeID2EdgeMap()[modelEntity->GetUniquePersistentId()];
          }
        }
      else if(this->getModel()->getModel()->GetModelDimension() == 2)
        {
        cmbEntity = this->getModel()->
          Get2DEdgeID2EdgeMap()[modelEntity->GetUniquePersistentId()];
        }
      }

    if(!cmbEntity)
      {
      cmbEntity = new pqCMBModelEntity();
      cmbEntity->setModelEntity(modelEntity);
      }
    mNode->setModelObject(cmbEntity, setWidget);
    if(cmbEntity->getModelWidgets().count()>1 && setWidget)
      {
      for(int w=0; w<cmbEntity->getModelWidgets().count(); w++)
        {
        cmbEntity->getModelWidgets().value(w)->setBackgroundColor(colId, Qt::yellow);
        }
      }
    }

// mNode->setData(colId, Qt::UserRole, colData);
  mNode->setText(colId, text);
  mNode->setFlags(commFlags);
  mNode->setChildIndicatorPolicy(
    QTreeWidgetItem::DontShowIndicatorWhenChildless);
  return mNode;
}

//----------------------------------------------------------------------------
vtkModelEntity* qtCMBTree::getItemObject(QTreeWidgetItem* treeItem)
{
  pqCMBTreeItem* cmbItem = static_cast<pqCMBTreeItem*>(treeItem);
  if(cmbItem && cmbItem->getModelObject())
    {
    return cmbItem->getModelObject()->getModelEntity();
    }
  return NULL;
}

//----------------------------------------------------------------------------
void qtCMBTree::onDragStarted(QTreeWidget*)
{
  emit this->dragStarted(this);
}

//----------------------------------------------------------------------------
void qtCMBTree::clearSelection(bool blockSignal)
{
  this->TreeWidget->blockSignals(blockSignal);

  this->TreeWidget->clearSelection();
  if(blockSignal)
    {
    this->TreeWidget->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
bool qtCMBTree::IsEntityVisibleInTree(
  vtkIdType faceId, QTreeWidget* treeWidget)
{
  QList<vtkIdType> visibleFaces;
  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();
  this->getVisibleChildEntityIds(rootNode, visibleFaces);
  return visibleFaces.contains(faceId) ? true : false;
}

//-----------------------------------------------------------------------------
void qtCMBTree::getVisibleChildEntityIds(
  QTreeWidgetItem* item, QList<vtkIdType> &visEntities)
{
  int visible;
  for(int r=0; r<item->childCount(); r++)
    {
    visible = item->child(r)->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
    if(visible)
      {
      if(item->child(r)->childCount()>0)
        {
        this->getVisibleChildEntityIds(item->child(r), visEntities);
        }
      else
        {
        vtkIdType faceid;
        vtkModelEntity* modelEntity = this->getItemObject(item->child(r));
        if(modelEntity && (modelEntity->GetType() == vtkModelFaceType ||
            modelEntity->GetType() == vtkModelEdgeType))
          {
          //faceid = item->child(r)->text(MTree_FACE_COL).toInt();
          faceid = modelEntity->GetUniquePersistentId();

          if(!visEntities.contains(faceid))
            {
            visEntities.append(faceid);
            }
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBTree::addUniqueChildren(
  QTreeWidgetItem* copyItem, QList<QTreeWidgetItem*> &newChildren)
{
  for(int k=0; k<copyItem->childCount(); k++)
    {
    if(copyItem->child(k)->childCount()>0)
      {
      this->addUniqueChildren(copyItem->child(k), newChildren);
      }
    else
      {
      if(!newChildren.contains(copyItem->child(k)))
        {
        newChildren.append(copyItem->child(k));
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBTree::clearChildren(QTreeWidgetItem* parentItem)
{
  if(parentItem->childCount()==0)
    {
    return;
    }
  this->TreeWidget->blockSignals(true);
  for(int k=0; k<parentItem->childCount(); k++)
    {
    QTreeWidgetItem* item = parentItem->child(k);
    // remove reference of this item from the cmb entity object
    pqCMBTreeItem* cmbItem = static_cast<pqCMBTreeItem*>(item);
    if(cmbItem && cmbItem->getModelObject())
      {
      cmbItem->getModelObject()->removeModelWidget(cmbItem);
      }
    // turn off the model entity if it is visible
    int visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
    if(visible)
      {
      vtkModelEntity* modEntity = this->getItemObject(item);
      if(modEntity && this->canTurnOffEntityVisibility(
        item, modEntity->GetUniquePersistentId()))
        {
        this->CMBModel->changeModelEntityVisibility(
          modEntity->GetUniquePersistentId(), false, false);
        }
      }
    }
  parentItem->takeChildren();
  this->TreeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
bool qtCMBTree::canTurnOffEntityVisibility(
  QTreeWidgetItem* currentItem, vtkIdType entityid)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetCurrentModelEntityMap();
  QMap<vtkIdType, pqCMBModelEntity*> edgeMap =
    this->CMBModel->Get2DEdgeID2EdgeMap();

  pqCMBModelEntity* cmbEntity=NULL;
  if(entityMap.contains(entityid))
    {
    cmbEntity = entityMap[entityid];
    }
  else if(this->CMBModel->has2DEdges() && edgeMap.contains(entityid))
    {
    cmbEntity = edgeMap[entityid];
    }
  if(cmbEntity)
    {
    QList<pqCMBTreeItem*> widgets = cmbEntity->getModelWidgets();
    QTreeWidgetItem* item = NULL;
    int visible =0;
    for(int i=0; i<widgets.count(); i++)
      {
      item = widgets.value(i);
      if( item != currentItem)
        {
        visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
        if(visible)
          {
          return 0;
          }
        }
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBTree::createEntityNode(
    QTreeWidgetItem* parentNode, int colId,
    vtkModelEntity* modelEntity, Qt::ItemFlags itemFlags,
    bool setVisibleIcon, bool setWidget)
{
  if(modelEntity)
    {
    vtkIdType faceId = modelEntity->GetUniquePersistentId();
    QString faceText("entity-id ");
    faceText.append(QString::number(faceId));

    const char* fName = vtkModelUserName::GetUserName(modelEntity);
    if(fName && strcmp(fName, ""))
      {
      faceText = fName;
      }

    QTreeWidgetItem* faceNode = this->createTreeNode(
      parentNode, colId, modelEntity, itemFlags, faceText,
      setVisibleIcon, 0, setWidget);
    return faceNode;
    }
  return NULL;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBTree::addNewTreeNodeOnRoot(int newIdColumn,
  vtkModelEntity* modelEntity)
{
  QString itemText = QString::number(modelEntity->GetUniquePersistentId());
  const char* mName = vtkModelUserName::GetUserName(modelEntity);
  if(mName)
    {
    itemText = mName;
    }

  QTreeWidgetItem* parentItem = this->TreeWidget->invisibleRootItem();

  Qt::ItemFlags itemFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
    Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
  return this->createTreeNode(
    parentItem, newIdColumn, modelEntity, itemFlags, itemText);
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBTree::createRootEntityNode(
  vtkModelEntity* modelEntity, int col)
{
  if(modelEntity)
    {
    QTreeWidgetItem* newNode = this->addNewTreeNodeOnRoot(
      col, modelEntity);
    if(newNode)
      {
      this->TreeWidget->sortByColumn(col, Qt::AscendingOrder);
      //this->UpdateInfoTable();
      return newNode;
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void qtCMBTree::clear(bool blockSignal)
{
  this->TreeWidget->blockSignals(blockSignal);

  this->TreeWidget->clear();
  if(blockSignal)
    {
    this->TreeWidget->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
QList<QTreeWidgetItem*> qtCMBTree::getSelectedItems() const
{
  //QList<QTreeWidgetItem*> selList;
  //return selList;
  return this->TreeWidget->selectedItems();
}

//-----------------------------------------------------------------------------
QList<QTreeWidgetItem*> qtCMBTree::getSelectedItems(int entityType)
{
  QList<QTreeWidgetItem*> selList;
  QTreeWidgetItem* item = NULL;
  vtkModelEntity* modEntity = NULL;

  for(int id=0; id<this->getSelectedItems().count(); id++)
    {
    item = this->getSelectedItems().value(id);
    modEntity = this->getItemObject(item);
    if(modEntity && modEntity->GetType() == entityType)
      {
      selList.append(item);
      }
    }

  return selList;
}

//-----------------------------------------------------------------------------
QList<vtkIdType> qtCMBTree::getSelectedModelEntityIds(
  int entityType)
{
  QTreeWidgetItem* item = NULL;
  vtkModelEntity* modEntity = NULL;
  QList<vtkIdType> selEntIds;
  vtkIdType entId;

  for(int id=0; id<this->getSelectedItems().count();id++)
    {
    item = this->getSelectedItems().value(id);
    modEntity = this->getItemObject(item);
    if(!modEntity)
      {
      continue;
      }
    entId = modEntity->GetUniquePersistentId();
    if(modEntity->GetType() == entityType && !selEntIds.contains(entId))
      {
      selEntIds.append(entId);
      }
    }
  return selEntIds;
}

//-----------------------------------------------------------------------------
void qtCMBTree::selectItem(QTreeWidgetItem* item)
{
  this->TreeWidget->setCurrentItem(item);
  item->setSelected(true);
}

//-----------------------------------------------------------------------------
void qtCMBTree::setSortingEnabled(bool enable)
{
  this->TreeWidget->setSortingEnabled(enable);
  if(enable)
    {
    this->TreeWidget->sortByColumn(
      this->getNameColumn(), Qt::AscendingOrder);
    }
}

//-----------------------------------------------------------------------------
void qtCMBTree::setSolidColorOnSelections(const QColor& setColor)
{
  QTreeWidget* treeWidget = this->TreeWidget;
  int nodeType;
  vtkIdType id;
  QList<vtkIdType> doneFaces;
  QTreeWidgetItem* item;
  vtkModelEntity* modEntity = NULL;
  for(int i=0; i<treeWidget->selectedItems().count() ; i++)
    {
    item = treeWidget->selectedItems().value(i);
    modEntity = this->getItemObject(item);
    if(!modEntity)
      {
      continue;
      }
    nodeType = modEntity->GetType();
    id = modEntity->GetUniquePersistentId();
    if(!doneFaces.contains(id))
      {
      this->CMBModel->modifyUserSpecifiedColor(nodeType, id, setColor, false);
      doneFaces.append(id);
      }
    }

  if(doneFaces.count()>0)
    {
    this->CMBModel->onLookupTableModified();
    }
}

//-----------------------------------------------------------------------------
void qtCMBTree::onCreateNew()
{
  emit this->createNew();
}

//-----------------------------------------------------------------------------
void qtCMBTree::onDelete()
{
  emit this->deleteSelected();
}

//-----------------------------------------------------------------------------
void qtCMBTree::onToggleVisibility()
{
  emit this->toggleVisibility();
}

//-----------------------------------------------------------------------------
void qtCMBTree::setActionsEnabled(bool /*enabled*/)
{
}
