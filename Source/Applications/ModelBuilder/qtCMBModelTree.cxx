/*=========================================================================

  Program:   CMB
  Module:    qtCMBModelTree.cxx

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
#include "qtCMBModelTree.h"

// cmb headers
#include "pqCMBModelFace.h"
#include "vtkCollection.h"
#include "vtkCMBModelEdgeMesh.h"
#include "vtkCMBModelFaceMesh.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkModelUserName.h"
#include "vtkIdTypeArray.h"
#include "vtkMergeOperatorClient.h"
#include "vtkModelEdge.h"
#include "vtkModelFaceUse.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"

#include "pqCMBTreeItem.h"
// Qt headers
#include <QAction>
#include <QStringList>
#include <QHeaderView>
#include <QMessageBox>
#include <QPalette>
#include <QVBoxLayout>

#define MATERIAL_TEXT_BASE "Domain Set "
#define SHELL_TEXT_BASE    "Region "
//-----------------------------------------------------------------------------
qtCMBModelTree::qtCMBModelTree(pqCMBModel* cmbModel)
: qtCMBTree(cmbModel)
{
  this->NumberOfSelectedItems = 0;
  this->NumberOfSelectedFaces = 0;
  this->NumberOfSelectedEmptyMaterialItems = 0;
  this->SelectedFacesMergable = false;
  this->Action_ShowEdges = NULL;
  this->Action_HideEdges = NULL;
  this->Action_ShowAllEdges = NULL;
  this->Action_ShowCommonEdges = NULL;
  this->Action_ShowNonCommonEdges = NULL;
  this->freeFacesNode = NULL;

  QPixmap pix(":/cmb/TextureOn24.png");
  QPixmap pixd(":/cmb/TextureOff24.png");
  this->IconCheck = new QIcon(pix);
  this->IconUncheck = new QIcon(pixd);
}

//-----------------------------------------------------------------------------
qtCMBModelTree::~qtCMBModelTree()
{
  delete this->IconCheck;
  delete this->IconUncheck;

  if(this->Action_ShowEdges)
    {
    delete this->Action_ShowEdges;
    }
  if(this->Action_HideEdges)
    {
    delete this->Action_HideEdges;
    }
  if(this->Action_ShowAllEdges)
    {
    delete this->Action_ShowAllEdges;
    }
  if(this->Action_ShowCommonEdges)
    {
    delete this->Action_ShowCommonEdges;
    }
  if(this->Action_ShowNonCommonEdges)
    {
    delete this->Action_ShowNonCommonEdges;
    }
}
//-----------------------------------------------------------------------------
void qtCMBModelTree::clear(bool blockSignal)
{
  if(this->freeFacesNode)
    {
    this->freeFacesNode->takeChildren();
    delete this->freeFacesNode;
    this->freeFacesNode = NULL;
    }

  this->Superclass::clear(blockSignal);
}

//----------------------------------------------------------------------------
void qtCMBModelTree::createWidget(QWidget* parent)
{
  QLayout* vboxlayout = parent->layout()?parent->layout():new QVBoxLayout(parent);
  vboxlayout->setObjectName("materialTreeLayout");
  vboxlayout->setMargin(0);
  this->TreeWidget = new qtCMBTreeWidget(parent);
  this->TreeWidget->setObjectName("materialTreeWidget");
  vboxlayout->addWidget(this->TreeWidget);

  this->customizeTreeWidget();
}

//----------------------------------------------------------------------------
void qtCMBModelTree::customizeTreeWidget()
{
  qtCMBTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->setColumnCount(5);
  // treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

  this->qtCMBTree::customizeTreeWidget();
  // treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
  treeWidget->setAcceptDrops(true);
  this->showMeshInfo(false);
  treeWidget->setColumnHidden(MTree_TEXTURE_COL, 1);

  this->Action_ShowAllEdges = new QAction(treeWidget);
  this->Action_ShowAllEdges->setObjectName(QString::fromUtf8("action_showAllEdgesBCS"));
  this->Action_ShowAllEdges->setText(QString::fromUtf8("Show All Edges"));
  QObject::connect(this->Action_ShowAllEdges, SIGNAL(triggered()),
    this, SLOT(onShowAllEdges()));

  this->Action_ShowCommonEdges = new QAction(treeWidget);
  this->Action_ShowCommonEdges->setObjectName(QString::fromUtf8("action_showCommonEdgesBCS"));
  this->Action_ShowCommonEdges->setText(QString::fromUtf8("Show Common Edges"));
  QObject::connect(this->Action_ShowCommonEdges, SIGNAL(triggered()),
    this, SLOT(onShowCommonEdges()));

  this->Action_ShowNonCommonEdges = new QAction(treeWidget);
  this->Action_ShowNonCommonEdges->setObjectName(QString::fromUtf8("action_showNonCommonEdgesBCS"));
  this->Action_ShowNonCommonEdges->setText(QString::fromUtf8("Show Non-Common Edges"));
  QObject::connect(this->Action_ShowNonCommonEdges, SIGNAL(triggered()),
    this, SLOT(onShowNonCommonEdges()));

//  this->Action_ShowEdges = new QAction(treeWidget);
//  this->Action_ShowEdges->setObjectName(QString::fromUtf8("action_showEdges"));
//  this->Action_ShowEdges->setText(QString::fromUtf8("Show Edges"));
//  QObject::connect(this->Action_ShowEdges, SIGNAL(triggered()),
//    this, SLOT(onShowEdges()));

  this->Action_HideEdges = new QAction(treeWidget);
  this->Action_HideEdges->setObjectName(QString::fromUtf8("action_hideEdges"));
  this->Action_HideEdges->setText(QString::fromUtf8("Hide Edges"));
  QObject::connect(this->Action_HideEdges, SIGNAL(triggered()),
    this, SLOT(onHideEdges()));

  treeWidget->addAction(this->Action_ShowAllEdges);
  treeWidget->addAction(this->Action_ShowCommonEdges);
  treeWidget->addAction(this->Action_ShowNonCommonEdges);
//  treeWidget->addAction(this->Action_ShowEdges);
  treeWidget->addAction(this->Action_HideEdges);
}

//----------------------------------------------------------------------------
void qtCMBModelTree::initializeTree()
{
  this->clear(true);
  QTreeWidget* treeWidget =this->TreeWidget;
  treeWidget->blockSignals(true);
  treeWidget->model()->blockSignals(true);

  bool dim2D = this->getModel()->getModel()->GetModelDimension()==2 ? true : false;
  treeWidget->setHeaderLabels(
              QStringList() << tr("Model Entity") << tr("Visibility")
      << tr("Texture") << tr("Edge Length") << tr("Min Angle") );

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable
    | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
  Qt::ItemFlags sheFlags(commFlags);

  vtkDiscreteModel *cmbModel = this->CMBModel->getModel();
  vtkModelItemIterator* iter=cmbModel->NewIterator(vtkModelMaterialType);

  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelMaterial* matertialEntity =
      vtkModelMaterial::SafeDownCast(iter->GetCurrentItem());
    if(matertialEntity)
      {
      QTreeWidgetItem* materialNode = this->createMaterialNode(matertialEntity);
      vtkModelItemIterator* iterRegion=matertialEntity->NewIterator(
        dim2D ? vtkModelFaceType : vtkModelRegionType);
      for(iterRegion->Begin();!iterRegion->IsAtEnd();iterRegion->Next())
        {
        vtkModelEntity* regionEntity =
          vtkModelEntity::SafeDownCast(iterRegion->GetCurrentItem());
        if(regionEntity)
          {
          QString shellTextBase = QString::number(regionEntity->GetUniquePersistentId());
          const char* rName = vtkModelUserName::GetUserName(regionEntity);
          if(rName)
            {
            shellTextBase = rName;
            }
          QTreeWidgetItem* regionNode = this->createTreeNode(
            materialNode, this->getNameColumn(),  regionEntity, sheFlags,
            shellTextBase);
          vtkModelItemIterator* iterFace=
            dim2D ? vtkDiscreteModelFace::SafeDownCast(regionEntity)->NewAdjacentModelEdgeIterator() :
            vtkDiscreteModelRegion::SafeDownCast(regionEntity)->NewAdjacentModelFaceIterator();

          for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
            {
            if(dim2D)
              {
              vtkDiscreteModelEdge* edgeEntity =
                vtkDiscreteModelEdge::SafeDownCast(iterFace->GetCurrentItem());
              if(edgeEntity)
                {
                QTreeWidgetItem* edgeNode = this->createEntityNode(
                  regionNode, this->getNameColumn(), edgeEntity, commFlags);
                }
              }
            else
              {
              vtkDiscreteModelFace* faceEntity =
                vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
              if(faceEntity)
                {
                QTreeWidgetItem* faceNode = this->createEntityNode(regionNode, this->getNameColumn(), faceEntity, commFlags);
                }
              }
            }
          iterFace->Delete();
          }
        }
      iterRegion->Delete();
      }
    }
  iter->Delete();

  // Need to check if there are any faces that are not associated with
  // any region. This is possible with hybrid models.
  if(this->CMBModel->getModelDimension() == 3)
    {
    QList<vtkModelFace*> freeFaces;
    vtkModelItemIterator* fiter=cmbModel->NewIterator(vtkModelFaceType);
    for(fiter->Begin();!fiter->IsAtEnd();fiter->Next())
      {
      vtkModelFace* faceEntity =
        vtkModelFace::SafeDownCast(fiter->GetCurrentItem());
      if(faceEntity && faceEntity->GetNumberOfModelRegions()==0)
        {
        freeFaces.append(faceEntity);
        }
      }
    fiter->Delete();
    if(freeFaces.count() > 0)
      {
      Qt::ItemFlags freeFlags(
        Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//      vtkModelMaterial* matEntity = this->CMBModel->createMaterial();
//      this->CMBModel->modifyUserSpecifiedName(vtkModelMaterialType,
//        matEntity->GetUniquePersistentId(), "Free Domain");
//      this->freeFacesNode = this->createRootEntityNode(matEntity, MTree_MATERIAL_COL);

      this->freeFacesNode = this->createTreeNode(
        treeWidget->invisibleRootItem(), this->getNameColumn(), NULL,
        freeFlags, QString("Free Faces"), true);
//      QTreeWidgetItem* regionNode = this->createTreeNode(
//        this->freeFacesNode, this->getNameColumn(), NULL,
//        freeFlags, QString("Free Region"), true);
      foreach(vtkModelFace* faceEntity, freeFaces)
        {
        QTreeWidgetItem* faceNode = this->createEntityNode(
          this->freeFacesNode, this->getNameColumn(), faceEntity, commFlags);
        }
      }
    }

  treeWidget->sortByColumn(MTree_MATERIAL_COL, Qt::AscendingOrder);
  treeWidget->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  if(this->CMBModel->getModelDimension() == 3 && this->CMBModel->has2DEdges())
    {
    treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    this->updateEdgeActions(0);
    }
  else
    {
    treeWidget->setContextMenuPolicy(Qt::NoContextMenu);
    }

  treeWidget->expandAll();
  treeWidget->model()->blockSignals(false);
  treeWidget->blockSignals(false);
}

//----------------------------------------------------------------------------
int qtCMBModelTree::getNameColumn()
{
  return MTree_MATERIAL_COL;
}

//----------------------------------------------------------------------------
void qtCMBModelTree::showTextureColumn(bool show)
{
  if(show)
    {
    this->updateTextureInfo();
    }
  this->TreeWidget->setColumnHidden(MTree_TEXTURE_COL, !show);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::updateTextureInfo()
{
  QTreeWidget* treeWidget =this->TreeWidget;
  treeWidget->blockSignals(true);

  vtkModelGeometricEntity* faceEntity;
  QList<QTreeWidgetItem*> treeItems =
    this->getTreeItemsWithType(vtkModelFaceType);
  int numItems = treeItems.count();
  pqCMBTreeItem* item;
  for(int i=0; i<numItems; i++)
    {
    item = static_cast<pqCMBTreeItem*>(treeItems.value(i));
    faceEntity = vtkModelGeometricEntity::SafeDownCast(
      item->getModelObject()->getModelEntity());
    if(faceEntity)
      {
      faceEntity->SetShowTexture(1); // server side default to ON
      item->setIcon(MTree_TEXTURE_COL, *this->IconCheck);
      }
    }
  treeWidget->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtCMBModelTree::onGroupClicked(QTreeWidgetItem* item, int col)
{
  // Change visibility
  vtkModelEntity* selObj = this->getItemObject(item);
  int nodeType;
  if(selObj)
    {
    nodeType = selObj->GetType();
    }
  else if(item == this->freeFacesNode)
    {
    nodeType = vtkModelMaterialType;
    }
  else if(this->freeFacesNode && this->freeFacesNode->childCount() > 0 &&
    item == this->freeFacesNode->child(0))
    {
    nodeType = vtkModelRegionType;
    }

  int visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
  if(col == TREE_VISIBLE_COL)
    {
    bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

    // Change visible icon
    int itemVisible = !visible;
    //we also need to change the parent icon accordingly.
    //If the child is visible, the parent has to be visible first
    if(itemVisible)
      {
      QTreeWidgetItem* parent = NULL;
      if(nodeType == vtkModelRegionType ||
        (nodeType == vtkModelFaceType && dim2D))
        {
        parent = item->parent();
        }
      else if(nodeType ==vtkModelFaceType && !dim2D)
        {
        parent = item->parent();
        parent->setIcon(TREE_VISIBLE_COL, *this->IconVisible);
        parent->setData(TREE_VISIBLE_COL, Qt::UserRole, itemVisible);
        parent = parent->parent();
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
  else if(!this->TreeWidget->isColumnHidden(MTree_TEXTURE_COL) &&
    col == MTree_TEXTURE_COL && nodeType == vtkModelFaceType)
    {
    vtkModelGeometricEntity* faceEntity =
      vtkModelGeometricEntity::SafeDownCast(selObj);
    if(faceEntity)
      {
      item->setIcon(MTree_TEXTURE_COL, faceEntity->GetShowTexture() ?
        *this->IconUncheck : *this->IconCheck);
      this->CMBModel->setEntityShowTexture(selObj->GetUniquePersistentId(),
        faceEntity->GetShowTexture() ? 0 : 1);
      }
    }
}

//----------------------------------------------------------------------------
void qtCMBModelTree::updateEdgeActions(int numSelFaces)
{
  bool enabled = false;
  this->Action_ShowAllEdges->setEnabled(enabled);
  this->Action_ShowNonCommonEdges->setEnabled(enabled);
  this->Action_ShowCommonEdges->setEnabled(enabled);
  this->Action_HideEdges->setEnabled(enabled);
  if(numSelFaces == 1)
    {
    enabled = true;
    this->Action_ShowAllEdges->setEnabled(enabled);
    this->Action_HideEdges->setEnabled(enabled);
    }
  else if(numSelFaces > 1)
    {
    enabled = true;
    this->Action_ShowAllEdges->setEnabled(enabled);
    this->Action_ShowNonCommonEdges->setEnabled(enabled);
    this->Action_ShowCommonEdges->setEnabled(enabled);
    this->Action_HideEdges->setEnabled(enabled);
    }
}

//----------------------------------------------------------------------------
void qtCMBModelTree::onGroupSelectionChanged()
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* item = NULL;
  int nodeType, visible;
  vtkIdType shellId;
  int numSelEmptyMat=0, numAllSelFaces=0;

  QList<vtkIdType> selVisibleModelEntities;
  QList<QTreeWidgetItem*> selItems = treeWidget->selectedItems();
  QList<vtkIdType> selFaceIds;
  QList<vtkIdType> selShellIds;
  QList<vtkIdType> selFaceShellIds;

  for(int i=0; i<selItems.count();i++)
    {
    item = selItems.value(i);
    vtkModelEntity* selObj = this->getItemObject(item);
    if(item == this->freeFacesNode)
      {
      nodeType = vtkModelMaterialType;
      }
//    else if(this->freeFacesNode && this->freeFacesNode->childCount() > 0 &&
//      item == this->freeFacesNode->child(0))
//      {
//      nodeType = vtkModelRegionType;
//      }
    else
      {
      nodeType = selObj->GetType();
      }
    if(nodeType == vtkModelMaterialType &&
      item->childCount()==0)
      {
      numSelEmptyMat++;
      }
    if(nodeType == vtkModelFaceType)
      {
      numAllSelFaces++;
      }

    visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
    if(!visible)
      {
      continue;
      }
    if(nodeType == vtkModelMaterialType)
      {
      for(int r=0; r<item->childCount(); r++)
        {
        visible = item->child(r)->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
        if(!visible)
          {
          continue;
          }
        //shellId = item->child(r)->data(this->getNameColumn(), Qt::UserRole).toInt();
        vtkModelEntity* modelEntity = this->getItemObject(item->child(r));
        if(modelEntity && modelEntity->GetType() == vtkModelRegionType)
          {
          shellId = modelEntity->GetUniquePersistentId();
          selShellIds.append(shellId);
          }
        else if(modelEntity && modelEntity->GetType() == vtkModelFaceType)
          {
          vtkIdType faceid = modelEntity->GetUniquePersistentId();
          if(!selVisibleModelEntities.contains(faceid))
            {
            selVisibleModelEntities.append(faceid);
            }
          }
        if(item->child(r)->childCount()>0)
          {
          this->getVisibleChildEntityIds(item->child(r), selVisibleModelEntities);
          }
        }
      }
    else if(nodeType == vtkModelRegionType && this->CMBModel->getModelDimension() == 3)
      {
      if(selObj)
        {
        shellId = selObj->GetUniquePersistentId();
        if(!selShellIds.contains(shellId))
          {
          selShellIds.append(shellId);
          }
        if(item->childCount()>0)
          {
          this->getVisibleChildEntityIds(item, selVisibleModelEntities);
          }
        }
      else if(item->childCount()>0) // this is where free faces are in hybrid model
        {
        this->getVisibleChildEntityIds(item, selVisibleModelEntities);
        }
      }
    else if(nodeType == vtkModelFaceType )
      {
      // int faceid = item->text(this->getNameColumn()).toInt();
      vtkIdType faceid = selObj->GetUniquePersistentId();
      if(!selVisibleModelEntities.contains(faceid))
        {
        selVisibleModelEntities.append(faceid);
        }

      if(this->CMBModel->getModelDimension() == 3)
        {
        if(!selFaceIds.contains(faceid))
          {
          selFaceIds.append(faceid);
          }
        }
      else if(this->CMBModel->getModelDimension() == 2)
        {
        if(item->childCount()>0)
          {
          //this->getVisibleChildEntityIds(item, selVisibleModelEntities);
          }
        }
      }
    else if(nodeType == vtkModelEdgeType )
      {
      vtkIdType edgeId = this->getItemObject(item)->GetUniquePersistentId();
      if(!selVisibleModelEntities.contains(edgeId))
        {
        selVisibleModelEntities.append(edgeId);
        }
      }
    }

  if(selVisibleModelEntities.count()>0)
    {
    //QList<int> faces;
    //for(int index=0; index < selVisibleModelEntities.count(); index++)
    //  {
    //  faces.append(selVisibleModelEntities.value(index)->text(this->getNameColumn()).toInt());
    //  }
    this->CMBModel->clearAllEntityHighlights(false);
    this->CMBModel->highlightModelEntities(selVisibleModelEntities);
    }
  else
    {
    this->CMBModel->clearAllEntityHighlights();
    }

  this->NumberOfSelectedItems = selItems.count();
  this->NumberOfSelectedEmptyMaterialItems = numSelEmptyMat;

  if(this->CMBModel->getModelDimension() == 3)
    {
    this->NumberOfSelectedFaces = selFaceIds.count();
    this->updateEdgeActions(numAllSelFaces);
    if(this->NumberOfSelectedFaces <= 1)
      {
      this->SelectedFacesMergable = false;
      }
    else
      {
      this->SelectedFacesMergable = true;
      vtkIdType toFaceId = selFaceIds.value(0);
      for(int id=1; id<selFaceIds.count();id++)
        {
        vtkIdType faceId = selFaceIds.value(id);
        if(faceId < toFaceId)
          {
          toFaceId = faceId;
          }
        }
      int findex = selFaceIds.indexOf(toFaceId);
      if(findex >=0)
        {
        selFaceIds.removeAt(findex);
        }

      for(int i=0;i<selFaceIds.count() && this->SelectedFacesMergable;i++)
        {
        vtkIdType sourceId = selFaceIds.value(i);
        vtkSmartPointer<vtkMergeOperatorClient> MergeOperator =
          vtkSmartPointer<vtkMergeOperatorClient>::New();
        MergeOperator->SetTargetId(toFaceId);
        MergeOperator->SetSourceId(sourceId);
        if(MergeOperator->AbleToOperate(this->CMBModel->getModel()) == 0)
          {
          this->SelectedFacesMergable = false;
          }
        }
      }
    }

  emit this->selectionChanged(this);
}

//----------------------------------------------------------------------------
void qtCMBModelTree::onItemsDroppedOnItem(
  QTreeWidgetItem* parentItem, QDropEvent*)
{
  if(this->DragFromTree != this)
    {
    return;
    }
  if(!parentItem)
    {
    return;
    }

  QTreeWidget *treeWidget = this->TreeWidget;
  if(treeWidget->selectedItems().count()<=0)
    {
    return;
    }
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  treeWidget->blockSignals(true);
  // Materials node
  QTreeWidgetItem *materialNode = NULL;
  if(parentItem == treeWidget->invisibleRootItem())
    {
    vtkModelMaterial* matEntity = this->CMBModel->createMaterial();
    materialNode = this->createRootEntityNode(matEntity, MTree_MATERIAL_COL);
    }
  else if(!this->isAFreeFaceNode(parentItem))
    {
    int nodeType = this->getItemObject(parentItem)->GetType();
    if(nodeType == vtkModelMaterialType)
      {
      materialNode = parentItem;
      }
   else if(nodeType == vtkModelRegionType || (dim2D && nodeType == vtkModelFaceType))
      {
      materialNode = parentItem->parent();
      }
    }

  if(materialNode)
    {
    this->dropItemsToNode(materialNode,
      treeWidget->selectedItems());
    }
  treeWidget->blockSignals(false);

  if(materialNode)
    {
    treeWidget->setCurrentItem(materialNode);
    materialNode->setSelected(true);
    this->CMBModel->onLookupTableModified();
    }

  this->DragFromTree = NULL;
}

//----------------------------------------------------------------------------
bool qtCMBModelTree::isAFreeFaceNode(QTreeWidgetItem* item)
{
  if(!this->freeFacesNode)
    {
    return false;
    }
  if(item == this->freeFacesNode)
    {
    return true;
    }
  for(int i=0; i<this->freeFacesNode->childCount(); i++)
    {
    if(this->freeFacesNode->child(i) == item)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void qtCMBModelTree::onGroupChanged(QTreeWidgetItem* item, int col)
{
  if(col == TREE_VISIBLE_COL || this->TreeWidget->signalsBlocked())
    {
    return;
    }
  if(col == MTree_MESH_MIN_ANGLE_COL || col == MTree_MESH_LENGTH_COL)
    {
    return this->onMeshInfoChanged(item, col);
    }
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  int nodeType = this->getItemObject(item)->GetType();
  bool validChange = (col==this->getNameColumn()) ? true : false;
  if(!validChange)
    {
    item->setText(col, "");
    treeWidget->blockSignals(false);
    return;
    }
  if(item->text(col).isEmpty())
    {
    item->setText(col, vtkModelUserName::GetUserName(this->getItemObject(item)));
    treeWidget->blockSignals(false);
    return;
    }
  vtkIdType entityId = this->getItemObject(item)->GetUniquePersistentId();
  this->CMBModel->modifyUserSpecifiedName(nodeType,
    entityId, item->text(col).toStdString().c_str());

  if(col == this->getNameColumn())
    {
    this->updateInterfaceNodeText(item, entityId);
    }

  treeWidget->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtCMBModelTree::updateInterfaceNodeText(
  QTreeWidgetItem* changedItem, vtkIdType entId)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetCurrentModelEntityMap();

  if(entityMap.contains(entId) && changedItem)
    {
    pqCMBModelEntity* cmbFace = entityMap[entId];
    QList<pqCMBTreeItem*> widgets = cmbFace->getModelWidgets();
    this->TreeWidget->blockSignals(true);
    for(int i=0; i<widgets.count(); i++)
      {
      if(widgets.value(i) != changedItem)
        {
        widgets.value(i)->setText(
          this->getNameColumn(), changedItem->text(this->getNameColumn()));
        }
      }
    this->TreeWidget->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::changeChildItemVisibilityIcon(QTreeWidgetItem* item,
  int visible, QList<vtkIdType>& changedFaces, bool force, bool recursive)
{
  QIcon *visIcon = visible ? this->IconVisible :
    this->IconInvisible;
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  item->setIcon(TREE_VISIBLE_COL, *visIcon);
  item->setData(TREE_VISIBLE_COL, Qt::UserRole, visible);

  if(recursive)
    {
    for(int i=0; i<item->childCount(); i++)
      {
      this->changeChildItemVisibilityIcon(item->child(i), visible,
        changedFaces, force);
      }
    }

  if(vtkModelEntity* selObj = this->getItemObject(item))
    {
    vtkIdType faceId = selObj->GetUniquePersistentId();
    int selType = selObj->GetType();
    if( (selType == vtkModelFaceType && !dim2D) ||
        (selType == vtkModelEdgeType))
      {
      //faceId = item->text(this->getNameColumn()).toInt();
      if(!visible)
        {
        if(force || this->canTurnOffEntityVisibility(item, faceId))
          {
          this->CMBModel->changeModelEntityVisibility(faceId, visible);
          if(!changedFaces.contains(faceId))
            {
            changedFaces.append(faceId);
            }
          }
        }
      else
        {
  //      if(this->IsEntityVisibleInTree(faceId, this->TreeWidget))
  //        {
          this->CMBModel->changeModelEntityVisibility(faceId, visible);
          if(!changedFaces.contains(faceId))
            {
            changedFaces.append(faceId);
            }
  //        }
        }
      }
    else if(selType == vtkModelFaceType && dim2D)
      {
      QMap<vtkIdType, pqCMBModelEntity*> entityMap =
        this->getModel()->GetFaceIDToFaceMap();
      if(entityMap.contains(faceId))
        {
        this->CMBModel->changeModelEntityVisibility(entityMap[faceId], visible);
  //      entityMap[faceId]->setVisibility(visible);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::dropItemsToNode(
  QTreeWidgetItem* movetoNode, QList<QTreeWidgetItem*> selItems)
{
  if(!movetoNode)
    {
    return;
    }
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  QList<vtkIdType> shellIds;
  QTreeWidgetItem* movItem;
  QList<QTreeWidgetItem*> newChildren;
  int nodeType;
  for(int n=0; n<selItems.count(); n++)
    {
    movItem = selItems.value(n);
    if(!this->getItemObject(movItem))
      {
      continue;
      }
    nodeType = this->getItemObject(movItem)->GetType();
    if(nodeType == vtkModelMaterialType)
      {
      QList<QTreeWidgetItem*> remChildren;
      for(int k=0; k<movItem->childCount(); k++)
        {
        if(!newChildren.contains(movItem->child(k)))
          {
          shellIds.append(
            this->getItemObject(movItem->child(k))->GetUniquePersistentId());
          remChildren.append(movItem->child(k));
          }
        }
      newChildren.append(remChildren);
      for(int r=0; r<remChildren.count(); r++)
        {
        movItem->removeChild(remChildren[r]);
        }
      }
    else if(nodeType == vtkModelRegionType || (dim2D && nodeType == vtkModelFaceType))
      {
      if(!newChildren.contains(movItem))
        {
        shellIds.append(this->getItemObject(movItem)->GetUniquePersistentId());
        newChildren.append(movItem);
        movItem->parent()->removeChild(movItem);
        }
      }
    }

  if(newChildren.count()>0)
    {
    vtkModelMaterial* matEntity = vtkModelMaterial::SafeDownCast(
      this->getItemObject(movetoNode));
    this->CMBModel->changeShellMaterials(matEntity, shellIds);
    movetoNode->insertChildren(movetoNode->childCount(),newChildren);
    this->CMBModel->onModelModified();
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::selectItemsByType(QList<vtkIdType>& entityIds,
  int entityType, int clearSelFirst)
{
  QList<QTreeWidgetItem*> treeItems =
    this->getTreeItemsWithType(entityType);
  int numItems = treeItems.count();
  if(numItems <=0)
    {
    return;
    }
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  int col = -1;
  if(entityType == vtkModelEdgeType)
    {
    col = this->getNameColumn();
    }
  else if(entityType == vtkModelFaceType)
    {
    col = dim2D ? this->getNameColumn() : this->getNameColumn();
    }
  else if(entityType == vtkModelRegionType)
    {
    col = this->getNameColumn();
    }
  else if(entityType == vtkModelMaterialType)
    {
    col = MTree_MATERIAL_COL;
    }
  if(col < 0)
    {
    return;
    }
  qtCMBTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  if(clearSelFirst)
    {
    treeWidget->clearSelection();
    }

  vtkIdType Id;
  QTreeWidgetItem* item;
  QItemSelection itemSels;
  for(int i=0; i<numItems; i++)
    {
    item = treeItems.value(i);
    Id = this->getItemObject(item)->GetUniquePersistentId();
    if(entityIds.contains(Id))
      {
      QModelIndex idx = treeWidget->getItemIndex(item);
      QModelIndex endIdx = idx.sibling(idx.row(),item->columnCount()-1);
      QItemSelection itemSel(idx, endIdx);
      itemSels.merge(itemSel, QItemSelectionModel::Select);
      }
    }
  treeWidget->selectionModel()->select(itemSels,
    QItemSelectionModel::Select);
  treeWidget->blockSignals(false);

  this->onGroupSelectionChanged();
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::selectModelEntitiesByMode(
  QList<vtkIdType>& entities, int selectionMode, int clearSelFirst)
{
  QList<vtkIdType> selIds;
  int i, selType;
  vtkIdType entityId;
  vtkModelEntity* selEntity = NULL;
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetCurrentModelEntityMap();

  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;
  selType = vtkModelFaceType;

  for(i=0; i<entities.count(); i++)
    {
    entityId = entities.value(i);
    if(!entityMap.contains(entityId))
      { // This is the case where model faces are selected in 2D,
        // which the entity is not in the edge mapping
      continue;
      }
    selEntity = entityMap[entityId]->getModelEntity();
    if(dim2D) // for 2d selected edges
      {
      vtkModelItemIterator* iterFace = NULL;
      vtkModelEdge *edgeEntity = vtkModelEdge::SafeDownCast(selEntity);
      vtkModelFace* Face = NULL;
      vtkModelItemIterator* matIte=NULL;
      switch(selectionMode)
        {
        case 0: // model edge / arc
          selType = vtkModelEdgeType;
          entityId = entities.value(i);
          break;
        case 1: // polygons around a model face (group of arcs)
          selType = vtkModelFaceType;
          iterFace = edgeEntity->NewAdjacentModelFaceIterator();
          for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
            {
            entityId = vtkModelFace::SafeDownCast(
              iterFace->GetCurrentItem())->GetUniquePersistentId();
            if(!selIds.contains(entityId))
              {
              selIds.append(entityId);
              }
            }
          iterFace->Delete();
          break;
        case 2: // materials
          selType = vtkModelMaterialType;
          iterFace = edgeEntity->NewAdjacentModelFaceIterator();
          for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
            {
            Face = vtkModelFace::SafeDownCast(
              iterFace->GetCurrentItem());
            matIte = Face->NewIterator(vtkModelMaterialType);
            matIte->Begin();
            entityId = vtkModelMaterial::SafeDownCast(matIte->GetCurrentItem())->GetUniquePersistentId();
            if(!selIds.contains(entityId))
              {
              selIds.append(entityId);
              }
            }
          matIte->Delete();
          iterFace->Delete();
          break;
        default:
          entityId = -1;
          break;
        }
      }
    else
      {
      vtkDiscreteModelFace *faceEntity = vtkDiscreteModelFace::SafeDownCast(selEntity);
      switch(selectionMode)
        {
        case 0: // model face
          selType = vtkModelFaceType;
          entityId = entities.value(i);
          break;
        case 1: // regions
          selType = vtkModelRegionType;
          entityId = faceEntity->GetModelRegion(0)->GetUniquePersistentId();
          break;
        case 2: // materials
          selType = vtkModelMaterialType;
          entityId = vtkDiscreteModelRegion::SafeDownCast(faceEntity->GetModelRegion(0))
            ->GetMaterial()->GetUniquePersistentId();
          break;
        default:
          entityId = -1;
          break;
        }
      }

    if(entityId >=0 && !selIds.contains(entityId))
      {
      selIds.append(entityId);
      }
    }

  // Now this is a special case for 2D, where ONLY faces are selected
  if(selIds.count()==0)
    {
    QMap<vtkIdType, pqCMBModelEntity*> faceMap =
      this->CMBModel->GetFaceIDToFaceMap();
    for(i=0; i<entities.count(); i++)
      {
      entityId = entities.value(i);
      if(dim2D && !entityMap.contains(entityId))
        {
        vtkModelItemIterator* iterEdge=NULL;
        vtkModelItemIterator* matIte=NULL;
        vtkModelFace *faceEntity = vtkModelFace::SafeDownCast(
          faceMap[entityId]->getModelEntity());
        switch(selectionMode)
          {
          case 0: // model edge / arc
            selType = vtkModelEdgeType;
            iterEdge =
              faceEntity->NewAdjacentModelEdgeIterator();
            for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
              {
              entityId = vtkModelEdge::SafeDownCast(
                iterEdge->GetCurrentItem())->GetUniquePersistentId();
              if(!selIds.contains(entityId))
                {
                selIds.append(entityId);
                }
              }
            iterEdge->Delete();
            break;
          case 1: // polygons around a model face (group of arcs)
            selType = vtkModelFaceType;
            if(!selIds.contains(entityId))
              {
              selIds.append(entityId);
              }
            break;
          case 2: // materials
            selType = vtkModelMaterialType;
            matIte = faceEntity->NewIterator(vtkModelMaterialType);
            matIte->Begin();
            entityId = vtkModelMaterial::SafeDownCast(matIte->GetCurrentItem())->GetUniquePersistentId();
            if(!selIds.contains(entityId))
              {
              selIds.append(entityId);
              }
            break;
          default:
            entityId = -1;
            break;
          }
        }
      }
    }

  if(selIds.count())
    {
    this->selectItemsByType(selIds, selType, clearSelFirst);
    }
}

//----------------------------------------------------------------------------
QList<QTreeWidgetItem*> qtCMBModelTree::getTreeItemsWithType(
  int entityType)
{
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;

  QList<QTreeWidgetItem*> reply;
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();
  if(entityType == vtkModelMaterialType)
    {
    for(int i=0; i<rootNode->childCount(); i++)
      {
      reply.append(rootNode->child(i));
      }
    }
  else if(entityType == vtkModelRegionType)
    {
    for(int i=0; i<rootNode->childCount(); i++)
      {
      QTreeWidgetItem* matNode = rootNode->child(i);
      for(int j=0; j<matNode->childCount(); j++)
        {
        reply.append(matNode->child(j));
        }
      }
    }
  else if(entityType == vtkModelFaceType)
    {
    for(int i=0; i<rootNode->childCount(); i++)
      {
      QTreeWidgetItem* matNode = rootNode->child(i);
      for(int j=0; j<matNode->childCount(); j++)
        {
        QTreeWidgetItem* shellNode = matNode->child(j);
        if(dim2D) // shell node is the face node for 2D
          {
          reply.append(shellNode);
          }
        else
          {
          for(int k=0; k<shellNode->childCount();k++)
            {
            reply.append(shellNode->child(k));
            }
          }
        }
      }
    }
  else if(entityType == vtkModelEdgeType)
    {
    for(int i=0; i<rootNode->childCount(); i++)
      {
      QTreeWidgetItem* matNode = rootNode->child(i);
      for(int j=0; j<matNode->childCount(); j++)
        {
        QTreeWidgetItem* shellNode = matNode->child(j);
        // shell node is the face node for 2D
        for(int k=0; k<shellNode->childCount();k++)
          {
          reply.append(shellNode->child(k));
          }
        }
      }
    }

  return reply;
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::removeMergedEntityNodes(
  vtkIdType toFaceId, QList<vtkIdType>& mergedFaces)
{
  vtkModelEntity* modEntity = this->CMBModel->getModel()->GetModelEntity(toFaceId);
  if(!modEntity || (modEntity->GetType() != vtkModelFaceType &&
    modEntity->GetType() != vtkModelEdgeType))
    {
    return;
    }
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    modEntity->GetType() == vtkModelFaceType ?
    this->CMBModel->GetFaceIDToFaceMap() :
    this->CMBModel->Get2DEdgeID2EdgeMap();
  vtkIdType faceId;
  for(int i=0; i<mergedFaces.count(); i++)
    {
    faceId = mergedFaces.value(i);
    if(entityMap.contains(faceId))
      {
      for(int w=0; w<entityMap[faceId]->getModelWidgets().count(); w++)
        {
        if(entityMap[faceId]->getModelWidgets().value(w))
          {
          delete entityMap[faceId]->getModelWidgets().value(w);
          }
        }
      entityMap[faceId]->clearWidgets();
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::updateWithNewEntityNodes(
  QMap< vtkIdType, QList<vtkIdType> >& splitMap)
{
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  QList<vtkIdType> splitFaces = splitMap.keys();
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

  bool hybridEdges = (this->CMBModel->getModelDimension() == 3 && this->CMBModel->has2DEdges());
  // Add faces that are not in the tree yet (new faces)
  pqCMBModelEntity* faceEntity = NULL;
  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable |
    Qt::ItemIsDragEnabled | Qt::ItemIsEditable);

  QTreeWidgetItem* parentNode = NULL;
  foreach(vtkIdType faceId, splitFaces)
    {
    if(entityMap.contains(faceId))
      {
      faceEntity = entityMap[faceId];
      if(faceEntity && faceEntity->getModelWidgets().count()>0)
        {
        QList<pqCMBTreeItem*> widgets = faceEntity->getModelWidgets();
        for(int w=0; w<widgets.count(); w++)
          {
          pqCMBTreeItem* treeitem = widgets.value(w);
          if(!treeitem)
            {
            continue;
            }
          bool updateEdges = hybridEdges && treeitem->childCount()>0;
          if(updateEdges)
            {
            this->showAllEdges(treeitem);
            }
          parentNode = treeitem->parent();
          QList<vtkIdType> faces = splitMap[faceId];
          for(int id=0; id<faces.count(); id++)
            {
            vtkIdType newfaceId = faces.value(id);
            pqCMBModelEntity* newfaceEntity = entityMap[newfaceId];
            if(newfaceEntity && newfaceEntity->getModelWidgets().count() == 0)
              {
              QTreeWidgetItem* entNode = this->createEntityNode(parentNode, this->getNameColumn(),
                newfaceEntity->getModelEntity(), commFlags);
              if(updateEdges)
                {
                this->showAllEdges(entNode);
                }
              }
            }
          }
        }
      }
    }

  treeWidget->blockSignals(false);

}
/*
//----------------------------------------------------------------------------
void qtCMBModelTree::updateAllGeometryEntityNodes()
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QList<QTreeWidgetItem*> shellNodes =
    this->getTreeItemsWithType(vtkModelRegionType);
  int numItems = shellNodes.count();
  if(numItems <=0)
    {
    return;
    }

  treeWidget->blockSignals(true);

  QTreeWidgetItem* item;
  QList<vtkIdType> faceIds;
  QList<QTreeWidgetItem*> remList;
  QList<vtkIdType> existingFaces;
  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable |
    Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
  vtkIdType faceId;
  vtkDiscreteModelFace* faceEntity = NULL;
  vtkDiscreteModelRegion* regionEntity = NULL;
  for(int index=0; index<numItems; index++)
    {
    faceIds.clear();
    item = shellNodes.value(index);
    regionEntity = vtkDiscreteModelRegion::SafeDownCast(this->getItemObject(item));
    if(regionEntity)
      {
      vtkModelItemIterator* iterFace=regionEntity->NewAdjacentModelFaceIterator();
      for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
        {
        faceEntity = vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
        if(faceEntity)
          {
          faceId = faceEntity->GetUniquePersistentId();
          faceIds.append(faceId);
          }
        }
      iterFace->Delete();
      }

    remList.clear();
    existingFaces.clear();
    for(int c=0; c<item->childCount(); c++)
      {
      //faceId = item->child(c)->text(this->getNameColumn()).toInt();
      faceId = this->getItemObject(item->child(c))->GetUniquePersistentId();
      if(faceIds.contains(faceId))
        {
        existingFaces.append(faceId);
        continue;
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

    // Add faces that are not in the tree yet (new faces)
    for(int n = 0; n<faceIds.count(); n++)
      {
      faceId = faceIds.value(n);
      if(!existingFaces.contains(faceId))
        {
        faceEntity = this->CMBModel->GetFaceIDToFaceMap()[faceId]->getModelFaceEntity();
        this->createEntityNode(item, this->getNameColumn(), faceEntity, commFlags);
        }
      }
    }
  treeWidget->blockSignals(false);
}
*/

//-----------------------------------------------------------------------------
QTreeWidgetItem* qtCMBModelTree::createMaterialNode(
  vtkModelMaterial* cmbMaterial)
{
  QTreeWidgetItem* materialNode = NULL;
  if(cmbMaterial)
    {
    materialNode = this->createRootEntityNode(
      cmbMaterial, MTree_MATERIAL_COL);
    }

  return materialNode;
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::removeSelectedMaterialNodes()
{
  QTreeWidget* treeWidget = this->TreeWidget;
  QTreeWidgetItem* item;
  int errMsg = 0;
  treeWidget->blockSignals(true);
  QList<QTreeWidgetItem*> selItems = treeWidget->selectedItems();
  vtkModelMaterial* matEntity = NULL;
  for(int i=0; i<selItems.count();i++)
    {
    item = selItems.value(i);
    matEntity = vtkModelMaterial::SafeDownCast(this->getItemObject(item));
    if(matEntity->GetType() == vtkModelMaterialType)
      {
      if(item->childCount()<=0)
        {
        this->CMBModel->removeMaterial(matEntity);
        delete item;
        }
      else
        {
        errMsg = 1;
        }
      }
    else
      {
      errMsg = 1;
      }
    }

  if(errMsg)
    {
    QMessageBox::warning(NULL, "Removing Domain Set Warning!",
      "Only Domain Set node without children can be removed!");
    }
  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::mergeSelectedFaces()
{
  if (!this->CMBModel->GetFaceIDToFaceMap().count())
    {
    return;
    }

  QList<vtkIdType> selFaceIds =
    this->getSelectedModelEntityIds(vtkModelFaceType);

  this->CMBModel->mergeModelFaces(selFaceIds);

  // this->updateAllGeometryEntityNodes();

  //if(targetItem)
  //  {
  //  treeWidget->setCurrentItem(targetItem);
  //  targetItem->setSelected(true);
  //  }

  this->onGroupSelectionChanged();
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::splitModelFaces(double angle)
{
  QList<vtkIdType> selFaceIds =
    this->getSelectedModelEntityIds(vtkModelFaceType);

  if(selFaceIds.count()==0)
    {
    QMessageBox::warning(NULL, "No Model Faces Selected",
      "You must explicitly select model faces to split them!");
    return;
    }

  this->CMBModel->splitModelFaces(selFaceIds, angle);
//  this->updateAllGeometryEntityNodes();
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::createModelEdges()
{
  this->CMBModel->createModelEdges();
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::setFacesVisibility(bool visible, bool excludeShared)
{
  this->TreeWidget->blockSignals(true);
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetFaceIDToFaceMap();

  pqCMBModelEntity* cmbEntity;
  QMap<vtkIdType, pqCMBModelEntity*>::iterator mapIter;
  for (mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    cmbEntity = mapIter.value();
    QList<pqCMBTreeItem*> widgets = cmbEntity->getModelWidgets();
    if(widgets.count()>1 && excludeShared)
      {
      continue;
      }

    for(int w=0; w<widgets.count(); w++)
      {
      QList<vtkIdType> changedFaces;
      this->changeChildItemVisibilityIcon(widgets.value(w),
        visible, changedFaces, true, false);
      }
    }
  this->TreeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::setEdgesVisibility(bool visible, bool excludeShared)
{
  bool dim2D = (this->CMBModel->getModelDimension()==2) ? true : false;
  if(!dim2D)
    {
    return;
    }
  this->TreeWidget->blockSignals(true);
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetCurrentModelEntityMap();

  pqCMBModelEntity* cmbEntity;
  QMap<vtkIdType, pqCMBModelEntity*>::iterator mapIter;

  for (mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    cmbEntity = mapIter.value();
    QList<pqCMBTreeItem*> widgets = cmbEntity->getModelWidgets();
    if(widgets.count()>1 && excludeShared)
      {
      continue;
      }
    for(int w=0; w<widgets.count(); w++)
      {
      QList<vtkIdType> changedFaces;
      this->changeChildItemVisibilityIcon(widgets.value(w),
        visible, changedFaces, true);
      }
    }
  this->TreeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::setSharedEntitiesVisibility(bool visible)
{
  this->TreeWidget->blockSignals(true);
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->CMBModel->GetCurrentModelEntityMap();

  pqCMBModelEntity* cmbEntity;
  QMap<vtkIdType, pqCMBModelEntity*>::iterator mapIter;

  for (mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    cmbEntity = mapIter.value();
    QList<pqCMBTreeItem*> widgets = cmbEntity->getModelWidgets();
    if(widgets.count()>1)
      {
      for(int w=0; w<widgets.count(); w++)
        {
        QList<vtkIdType> changedFaces;
        this->changeChildItemVisibilityIcon(widgets.value(w),
          visible, changedFaces, true);
        }
      }
    }
  this->TreeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::onShowAllEdges()
{
  QList<QTreeWidgetItem*> selFaces = this->getSelectedItems(vtkModelFaceType);

  for(int w=0; w<selFaces.count(); w++)
    {
    QTreeWidgetItem* faceNode = selFaces.value(w);
    this->showAllEdges(faceNode);
    }

  this->CMBModel->clearAllEntityHighlights();
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::showAllEdges(QTreeWidgetItem* faceNode)
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable
    | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
  // remove all its children first if exists.
  this->clearChildren(faceNode);
  vtkDiscreteModelFace* faceEntity =
    vtkDiscreteModelFace::SafeDownCast(this->getItemObject(faceNode));
  if(faceEntity)
    {
    vtkModelItemIterator* iterEdge=faceEntity->NewAdjacentModelEdgeIterator();
    for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
      {
      vtkDiscreteModelEdge* edgeEntity =
        vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if(edgeEntity)
        {
        QTreeWidgetItem* edgeNode = this->createEntityNode(
          faceNode, this->getNameColumn(), edgeEntity, commFlags);
        if(!edgeEntity->GetVisibility())
          {
          // turn on the edge visibility
          this->CMBModel->changeModelEntityVisibility(
            edgeEntity->GetUniquePersistentId(), true, false);
          }
        }
      }
    iterEdge->Delete();
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::onShowCommonEdges(bool reverse)
{
  QList<QTreeWidgetItem*> selFaces = this->getSelectedItems(vtkModelFaceType);
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable
    | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);

  QList<vtkIdType> commonEdges;
  this->getCommonFaceEdgeIds(selFaces, commonEdges);

  for(int w=0; w<selFaces.count(); w++)
    {
    QTreeWidgetItem* faceNode = selFaces.value(w);
    // remove all its children first if exists.
    this->clearChildren(faceNode);

    vtkDiscreteModelFace* faceEntity =
      vtkDiscreteModelFace::SafeDownCast(this->getItemObject(faceNode));
    if(faceEntity)
      {
      vtkModelItemIterator* iterEdge=faceEntity->NewAdjacentModelEdgeIterator();
      for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
        {
        vtkDiscreteModelEdge* edgeEntity =
          vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
        if(edgeEntity)
          {
          bool isCommon = commonEdges.contains(edgeEntity->GetUniquePersistentId());
          if((!reverse && isCommon) || (reverse && !isCommon))
            {
            QTreeWidgetItem* edgeNode = this->createEntityNode(
              faceNode, this->getNameColumn(), edgeEntity, commFlags);
            if(!edgeEntity->GetVisibility())
              {
              // turn on the edge visibility
              this->CMBModel->changeModelEntityVisibility(
                edgeEntity->GetUniquePersistentId(), true, false);
              }
            }
          }
        }
      iterEdge->Delete();
      }
    }
  this->CMBModel->clearAllEntityHighlights();
}
//-----------------------------------------------------------------------------
void qtCMBModelTree::onShowNonCommonEdges()
{
  this->onShowCommonEdges(true);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::onHideEdges()
{
  QList<QTreeWidgetItem*> selFaces = this->getSelectedItems(vtkModelFaceType);
  for(int w=0; w<selFaces.count(); w++)
    {
    QTreeWidgetItem* faceNode = selFaces.value(w);
    // remove all its children first if exists.
    this->clearChildren(faceNode);
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::showMeshInfo(bool show)
{
  this->TreeWidget->setColumnHidden(MTree_MESH_LENGTH_COL, !show);
  this->TreeWidget->setColumnHidden(MTree_MESH_MIN_ANGLE_COL, !show);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::updateEdgeMeshInfo(vtkCollection* updateEdges)
{
  QTreeWidget* treeWidget =this->TreeWidget;
  treeWidget->blockSignals(true);
  QList<QTreeWidgetItem*> treeItems =
    this->getTreeItemsWithType(vtkModelEdgeType);
  int numItems = treeItems.count();
  vtkCMBModelEntityMesh* meshEntity;
  vtkCMBModelEdgeMesh* meshEdge;
  pqCMBTreeItem* item;
  for(int i=0; i<numItems; i++)
    {
    item = static_cast<pqCMBTreeItem*>(treeItems.value(i));
    meshEntity = item->getModelObject()->getMeshEntity();
    if(updateEdges->IsItemPresent(meshEntity))
      {
      meshEdge = vtkCMBModelEdgeMesh::SafeDownCast(meshEntity);
      item->setText(MTree_MESH_LENGTH_COL,
        QString::number(meshEdge->GetLength()));
      }
    }

  treeWidget->expandAll();
  treeWidget->blockSignals(false);
}
//-----------------------------------------------------------------------------
void qtCMBModelTree::updateFaceMeshInfo(vtkCollection* updateFaces)
{
  QTreeWidget* treeWidget =this->TreeWidget;
  treeWidget->blockSignals(true);

  vtkCMBModelEntityMesh* meshEntity;
  QList<QTreeWidgetItem*> treeItems =
    this->getTreeItemsWithType(vtkModelFaceType);
  int numItems = treeItems.count();
  pqCMBTreeItem* item;
  vtkCMBModelFaceMesh* meshFace;
  for(int i=0; i<numItems; i++)
    {
    item = static_cast<pqCMBTreeItem*>(treeItems.value(i));
    meshEntity = item->getModelObject()->getMeshEntity();
    if(updateFaces->IsItemPresent(meshEntity))
      {
      meshFace = vtkCMBModelFaceMesh::SafeDownCast(meshEntity);
      item->setText(MTree_MESH_LENGTH_COL,
        QString::number(meshFace->GetLength()));
      item->setText(MTree_MESH_MIN_ANGLE_COL,
        QString::number(meshFace->GetMinimumAngle()));
      }
    }

  treeWidget->expandAll();
  treeWidget->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtCMBModelTree::onMeshInfoChanged(QTreeWidgetItem* item, int col)
{
  if(col != MTree_MESH_MIN_ANGLE_COL && col != MTree_MESH_LENGTH_COL)
    {
    return;
    }
  int nodeType = this->getItemObject(item)->GetType();
  if(nodeType == vtkModelFaceType ||
    (nodeType == vtkModelEdgeType && col == MTree_MESH_LENGTH_COL))
    {
    emit this->meshItemChanged(item, col);
    }
}
//----------------------------------------------------------------------------
void qtCMBModelTree::getSelectedMeshEntities(
  const QMap<vtkIdType, pqCMBModelEntity*>& entityMap,
  vtkCollection* meshEntities)
{
  if(this->CMBModel->getModelDimension() == 3)
    {
    return;
    }
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* item = NULL;
  int nodeType;

  QList<QTreeWidgetItem*> selItems = treeWidget->selectedItems();
  for(int i=0; i<selItems.count();i++)
    {
    item = selItems.value(i);
    nodeType = this->getItemObject(item)->GetType();
    if(nodeType == vtkModelMaterialType)
      {
      for(int r=0; r<item->childCount(); r++)
        {
        vtkModelEntity* faceEntity = this->getItemObject(item->child(r));
        vtkIdType faceid = faceEntity->GetUniquePersistentId();
        if(entityMap.contains(faceid)) //Asking for Model face
          {
          vtkCMBModelEntityMesh* meshEntity = entityMap[faceid]->getMeshEntity();
          if(meshEntity && !meshEntities->IsItemPresent(meshEntity))
            {
            meshEntities->AddItem(meshEntity);
            }
          }
        else // Asking for the model edges
          {
          for(int c=0; c<item->child(r)->childCount(); c++)
            {
            vtkModelEntity* engeEntity = this->getItemObject(
              item->child(r)->child(c));
            vtkIdType edgeid = engeEntity->GetUniquePersistentId();
            if(entityMap.contains(edgeid))
              {
              vtkCMBModelEntityMesh* meshEntity = entityMap[edgeid]->getMeshEntity();
              if(meshEntity && !meshEntities->IsItemPresent(meshEntity))
                {
                meshEntities->AddItem(meshEntity);
                }
              }
            }
          }
        }
      }
    else if(nodeType == vtkModelFaceType )
      {
      vtkIdType faceid = this->getItemObject(item)->GetUniquePersistentId();
      if(entityMap.contains(faceid))
        {
        vtkCMBModelEntityMesh* meshEntity = entityMap[faceid]->getMeshEntity();
        if(meshEntity && !meshEntities->IsItemPresent(meshEntity))
          {
          meshEntities->AddItem(meshEntity);
          }
        }
      }
    else if(nodeType == vtkModelEdgeType )
      {
      vtkIdType edgeid = this->getItemObject(item)->GetUniquePersistentId();
      if(entityMap.contains(edgeid))
        {
        vtkCMBModelEntityMesh* meshEntity = entityMap[edgeid]->getMeshEntity();
        if(meshEntity && !meshEntities->IsItemPresent(meshEntity))
          {
          meshEntities->AddItem(meshEntity);
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::getSelectedMeshFaces(vtkCollection* meshEntities)
{
  this->getSelectedMeshEntities(
    this->CMBModel->GetFaceIDToFaceMap(), meshEntities);
}
//-----------------------------------------------------------------------------
void qtCMBModelTree::getSelectedMeshEdges(vtkCollection* meshEntities)
{
  this->getSelectedMeshEntities(
    this->CMBModel->Get2DEdgeID2EdgeMap(), meshEntities);
}

//-----------------------------------------------------------------------------
void qtCMBModelTree::getCommonFaceEdgeIds(
  QList<QTreeWidgetItem*>& selFaces, QList<vtkIdType>& commonEdges)
{
  QList<vtkIdType> allEdges;
  vtkIdType edgeId;
  for(int w=0; w<selFaces.count(); w++)
    {
    QTreeWidgetItem* faceNode = selFaces.value(w);
    vtkDiscreteModelFace* faceEntity =
      vtkDiscreteModelFace::SafeDownCast(this->getItemObject(faceNode));
    if(faceEntity)
      {
      vtkModelItemIterator* iterEdge=faceEntity->NewAdjacentModelEdgeIterator();
      for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
        {
        vtkDiscreteModelEdge* edgeEntity =
          vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
        if(edgeEntity)
          {
          edgeId = edgeEntity->GetUniquePersistentId();
          if(allEdges.contains(edgeId))
            {
            if(!commonEdges.contains(edgeId))
              {
              commonEdges.append(edgeId);
              }
            }
          else
            {
            allEdges.append(edgeId);
            }
          }
        }
      iterEdge->Delete();
      }
    }
}
