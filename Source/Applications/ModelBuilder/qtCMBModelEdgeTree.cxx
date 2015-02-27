/*=========================================================================

  Program:   CMB
  Module:    qtCMBModelEdgeTree.cxx

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
#include "qtCMBModelEdgeTree.h"

// cmb headers
#include "vtkDiscreteModelRegion.h"
#include "pqCMBTreeItem.h"
#include "vtkDiscreteModelEdge.h"
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

enum ModelEdgeNodeType
{
  RegionNodeType=10001,
  EdgeNodeType
};

#define ModelEdgeTree_Region_COL       0
//#define ModelEdgeTree_Region_COL         2

//-----------------------------------------------------------------------------
qtCMBModelEdgeTree::qtCMBModelEdgeTree(pqCMBModel* cmbModel)
  : qtCMBTree(cmbModel)
{
  this->NumberOfSelectedModelEdges = 0;
}

//-----------------------------------------------------------------------------
qtCMBModelEdgeTree::~qtCMBModelEdgeTree()
{
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::createWidget(QWidget* parent)
{
  QLayout* boxlayout = parent->layout();
  if(!boxlayout)
    {
    boxlayout = new QVBoxLayout(parent);
    }
  boxlayout->setObjectName("ModelEdgeTreeLayout");
  boxlayout->setMargin(0);
  this->TreeWidget = new qtCMBTreeWidget(parent);
  this->TreeWidget->setObjectName("ModelEdgeTreeWidget");
  boxlayout->addWidget(this->TreeWidget);

  this->customizeTreeWidget();
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::customizeTreeWidget()
{
  qtCMBTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->setColumnCount(2);

  this->qtCMBTree::customizeTreeWidget();
  treeWidget->setAcceptDrops(false);
  treeWidget->setDragEnabled(false);
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::initializeTree()
{
  QTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  treeWidget->clear();
  treeWidget->setHeaderLabels(
              QStringList() << tr("Entity Set") << tr("Visibility"));

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  Qt::ItemFlags ModelEdgeFlags(commFlags | Qt::ItemIsEditable);

  vtkDiscreteModel *cmbModel = this->CMBModel->getModel();
  vtkModelItemIterator* iter=cmbModel->NewIterator(vtkModelRegionType);

  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkDiscreteModelRegion* regionEntity =
      vtkDiscreteModelRegion::SafeDownCast(iter->GetCurrentItem());
    if(regionEntity && regionEntity->GetNumberOfAssociations(vtkModelEdgeType))
      {
      QString shellTextBase = QString::number(regionEntity->GetUniquePersistentId());
      const char* rName = vtkModelUserName::GetUserName(regionEntity);
      if(rName)
        {
        shellTextBase = rName;
        }
      QTreeWidgetItem* regionNode = this->createTreeNode(
        treeWidget->invisibleRootItem(), ModelEdgeTree_Region_COL,  regionEntity, commFlags,
        shellTextBase, true, RegionNodeType);
      vtkModelItemIterator* iterEdge=regionEntity->NewIterator(vtkModelEdgeType);
      for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
        {
        vtkDiscreteModelEdge* edgeEntity =
          vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
        if(edgeEntity && edgeEntity->GetNumberOfAssociations(vtkModelRegionType))
          {
          QString edgeTextBase = QString::number(edgeEntity->GetUniquePersistentId());
          const char* eName = vtkModelUserName::GetUserName(edgeEntity);
          if(eName)
            {
            edgeTextBase = eName;
            }
          this->createTreeNode(regionNode, ModelEdgeTree_Region_COL, edgeEntity,
            ModelEdgeFlags, edgeTextBase, true, EdgeNodeType);
          }
        }
      iterEdge->Delete();
      }
    }
  iter->Delete();

  treeWidget->sortByColumn(ModelEdgeTree_Region_COL, Qt::AscendingOrder);
  treeWidget->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  treeWidget->expandAll();

  treeWidget->blockSignals(false);
}

//----------------------------------------------------------------------------
int qtCMBModelEdgeTree::getNameColumn()
{
  return ModelEdgeTree_Region_COL;
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::onGroupClicked(QTreeWidgetItem* item, int col)
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
      if(nodeType == EdgeNodeType )
        {
        parent = item->parent();
        }

      if(parent)
        {
        parent->setIcon(TREE_VISIBLE_COL, *this->IconVisible);
        parent->setData(TREE_VISIBLE_COL, Qt::UserRole, itemVisible);
        }
      }

    this->changeChildItemVisibilityIcon(item, itemVisible);
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
void qtCMBModelEdgeTree::onGroupSelectionChanged()
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QList<QTreeWidgetItem*> selModelEdge = treeWidget->selectedItems();
  QList<vtkIdType> selEdgeIds;
  this->getSelectedVisibleEdges(selEdgeIds);

  this->CMBModel->clearAllModelEdgesHighlights();
  if(selEdgeIds.count()>0)
    {
    this->CMBModel->highlightModelEdges(selEdgeIds);
    }

  this->NumberOfSelectedModelEdges = selModelEdge.count();

  emit this->selectionChanged(this);
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::onGroupChanged(QTreeWidgetItem* item, int col)
{
  if(col != ModelEdgeTree_Region_COL)
    {
    return;
    }
  int nodeType = this->getItemObject(item)->GetType();
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);
  if(nodeType == vtkModelEdgeType)
    {
    if(this->getItemObject(item)->GetNumberOfAssociations(vtkModelRegionType))
      {
      vtkIdType ModelEdgeId = this->getItemObject(item)->GetUniquePersistentId();
      this->CMBModel->modifyUserSpecifiedName(
        vtkModelEdgeType, ModelEdgeId,
        item->text(ModelEdgeTree_Region_COL).toStdString().c_str());
      }
    }

  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBModelEdgeTree::changeChildItemVisibilityIcon(
  QTreeWidgetItem* item, int visible)
{
  QIcon *visIcon = visible ? this->IconVisible :
    this->IconInvisible;
  vtkModelEntity* entity = this->getItemObject(item);
  item->setIcon(TREE_VISIBLE_COL, *visIcon);
  item->setData(TREE_VISIBLE_COL, Qt::UserRole, visible);

  for(int i=0; i<item->childCount(); i++)
    {
    this->changeChildItemVisibilityIcon(item->child(i), visible);
    }

  vtkIdType edgeId;
  if(entity && entity->GetType()==vtkModelEdgeType &&
     entity->GetNumberOfAssociations(vtkModelRegionType))
    {
    edgeId = entity->GetUniquePersistentId();
    this->CMBModel->changeModelEdgeVisibility(edgeId, visible);
    }
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::updateRegionNodeText(
  vtkModelEntity* regionEntity)
{
  QString rName(vtkModelUserName::GetUserName(regionEntity));
  QTreeWidget *treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);

  QTreeWidgetItem* rootItem=treeWidget->invisibleRootItem();
  for(int c=0; c<rootItem->childCount(); c++)
    {
    if(regionEntity == this->getItemObject(rootItem->child(c)))
      {
      rootItem->child(c)->setText(ModelEdgeTree_Region_COL, rName);
      break;
      }
    }
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::setLineResolutionOnSelected(int res)
{
  QList<vtkIdType> selEdgeIds;
  this->getSelectedVisibleEdges(selEdgeIds);

  if(selEdgeIds.count()>0)
    {
    this->CMBModel->setModelEdgesResolution(selEdgeIds, res);
    }
}

//----------------------------------------------------------------------------
void qtCMBModelEdgeTree::getSelectedVisibleEdges(
  QList< vtkIdType > &selEdgeIds)
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* item = NULL;
  int visible;
  vtkIdType edgeId;

  QList<QTreeWidgetItem*> selModelEdge = treeWidget->selectedItems();
  for(int i=0; i<selModelEdge.count();i++)
    {
    item = selModelEdge.value(i);
    visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
    if(!visible)
      {
      continue;
      }
    if(item->type() == RegionNodeType)
      {
      for(int r=0; r<item->childCount(); r++)
        {
        edgeId = this->getItemObject(item->child(r))->GetUniquePersistentId();
        if(!selEdgeIds.contains(edgeId))
          {
          selEdgeIds.append(edgeId);
          }
        }
      break;
      }
    else
      {
      edgeId = this->getItemObject(item)->GetUniquePersistentId();
      if(!selEdgeIds.contains(edgeId))
        {
        selEdgeIds.append(edgeId);
        }
      }
    }
}
