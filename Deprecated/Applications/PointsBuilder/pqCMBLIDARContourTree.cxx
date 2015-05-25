//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLIDARContourTree.h"

#include "pqContourWidget.h"
#include "pqCMBContourTreeItem.h"
#include "qtCMBTreeWidget.h"

// Qt headers
#include <QAction>
#include <QStringList>
#include <QHeaderView>
#include <QIcon>
#include <QMessageBox>
#include <QPixmap>
#include <QPalette>
#include <QVariant>
#include <QVBoxLayout>

#include "pqSMAdaptor.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"

enum GroupType
{
  ContourGroupType=10001,
};

//-----------------------------------------------------------------------------
pqCMBLIDARContourTree::pqCMBLIDARContourTree(QWidget* parent)
{
  this->contourUnfinishedColor = QColor(255, 221, 218); // light red / rose
  this->contourFinishedColor = QColor(255, 255, 255);

  this->createWidget(parent);
}

//-----------------------------------------------------------------------------
pqCMBLIDARContourTree::~pqCMBLIDARContourTree()
{
  if(this->TreeWidget)
    {
    delete this->TreeWidget;
    }
}

//-----------------------------------------------------------------------------
unsigned int pqCMBLIDARContourTree::getNumberOfGroups()
{
  return this->TreeWidget->invisibleRootItem()->childCount();
}

//----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::getGroup(unsigned int idx)
{
  if (idx < this->getNumberOfGroups())
    {
    return this->TreeWidget->invisibleRootItem()->child(idx);
    }
  return 0;
}

//----------------------------------------------------------------------------
void pqCMBLIDARContourTree::createWidget(QWidget* parent)
{
  QLayout* boxlayout = parent->layout();
  if(!boxlayout)
    {
    boxlayout = new QVBoxLayout(parent);
    }
  boxlayout->setObjectName("contourTreeLayout");
  boxlayout->setMargin(0);
  this->TreeWidget = new qtCMBTreeWidget(parent);
  this->TreeWidget->setObjectName("contourTreeWidget");
  boxlayout->addWidget(this->TreeWidget);

  this->customizeTreeWidget();
}

//----------------------------------------------------------------------------
void pqCMBLIDARContourTree::customizeTreeWidget()
{
  qtCMBTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->setAlternatingRowColors(true);
  treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

  treeWidget->setColumnCount(3);
  treeWidget->setHeaderLabels(
    QStringList() << tr("Apply") << tr("Name") << tr("Clip Inside") );

  treeWidget->setAcceptDrops(true);

  for(int col=0; col<treeWidget->columnCount(); col++)
    {
    treeWidget->header()->setResizeMode(col, QHeaderView::ResizeToContents);
    }

  //QObject::connect(treeWidget, SIGNAL(itemEntered(QTreeWidgetItem*, int)),
  //  this, SLOT(onStartEditing(QTreeWidgetItem*, int)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(dragStarted(QTreeWidget*)),
    this, SLOT(onDragStarted(QTreeWidget*)), Qt::QueuedConnection);
  //QObject::connect(treeWidget, SIGNAL(itemClicked (QTreeWidgetItem*, int)),
  //  this, SLOT(onGroupClicked(QTreeWidgetItem*, int)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(onSelectionChanged()), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*)),
    this, SLOT(onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
    this, SLOT(onItemChanged(QTreeWidgetItem*, int)), Qt::QueuedConnection);

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
QTreeWidgetItem* pqCMBLIDARContourTree::createContourNode(
  QTreeWidgetItem* parentNode, pqContourWidget* contourObj,
  Qt::ItemFlags commFlags, const QString& text, int id, int type,
  bool setApplyContour, bool invert)
{
  //QTreeWidgetItem* mNode = new QTreeWidgetItem(parentNode, type);
  pqCMBContourTreeItem* mNode = new pqCMBContourTreeItem(parentNode, id, type);

  if(contourObj)
    {
    mNode->setContourObject(contourObj);
    }

// mNode->setData(colId, Qt::UserRole, colData);
  mNode->setText(NameCol, text);
  mNode->setCheckState(UseFilterCol, setApplyContour ? Qt::Checked : Qt::Unchecked);
  if(type == ContourGroupType)
    {
    mNode->setCheckState(InvertCol, invert ? Qt::Checked : Qt::Unchecked);
    }
  mNode->setFlags(commFlags);
  mNode->setChildIndicatorPolicy(
    QTreeWidgetItem::DontShowIndicatorWhenChildless);
  mNode->setBackgroundColor(NameCol, this->contourUnfinishedColor);
  this->selectItem(mNode);
  return mNode;
}

//----------------------------------------------------------------------------
pqContourWidget* pqCMBLIDARContourTree::getItemObject(QTreeWidgetItem* treeItem)
{
  pqCMBContourTreeItem* cmbItem = static_cast<pqCMBContourTreeItem*>(treeItem);
  return cmbItem ?  cmbItem->getContourObject() : NULL;
}

//----------------------------------------------------------------------------
void pqCMBLIDARContourTree::onDragStarted(QTreeWidget*)
{
  emit this->dragStarted(this);
}

//----------------------------------------------------------------------------
void pqCMBLIDARContourTree::clearSelection(bool blockSignal)
{
  this->TreeWidget->blockSignals(blockSignal);

  this->TreeWidget->clearSelection();
  if(blockSignal)
    {
    this->TreeWidget->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::createContourNode(
    QTreeWidgetItem* parentNode,
    pqContourWidget* contourObj, Qt::ItemFlags itemFlags, int type,
    bool setApplyContour, bool invert)
{
  if(contourObj)
    {
    static int contourId=0;
    contourId++;
    QString contourText("Contour ");
    contourText.append(QString::number(contourId));
    QTreeWidgetItem* contourNode = this->createContourNode(
      parentNode, contourObj, itemFlags, contourText, contourId, type,
      setApplyContour, invert);
    return contourNode;
    }
  return NULL;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::addNewTreeNodeOnRoot()
{
  static int contourGId = 0;
  contourGId++;
  QString itemText("Group ");
  itemText.append(QString::number(contourGId));

  QTreeWidgetItem* parentItem = this->TreeWidget->invisibleRootItem();

  Qt::ItemFlags itemFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
    Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
  return this->createContourNode(
    parentItem, NULL, itemFlags, itemText, contourGId, ContourGroupType);
}

//-----------------------------------------------------------------------------
pqCMBContourTreeItem* pqCMBLIDARContourTree::createContourGroupNode()
{
  QTreeWidgetItem* newNode = this->addNewTreeNodeOnRoot();
  if(newNode)
    {
    //this->TreeWidget->sortByColumn(col, Qt::AscendingOrder);
    //this->UpdateInfoTable();
    return static_cast<pqCMBContourTreeItem*>(newNode);
    }
  return NULL;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::addNewContourNode(
  pqContourWidget* contourObj)
{
  pqCMBContourTreeItem* selItem = this->getSelectedItems().size() ?
    static_cast<pqCMBContourTreeItem*>(this->getSelectedItems().value(0)) : NULL;
  if(!selItem || !contourObj)
    {
    return NULL;
    }
  QTreeWidgetItem* parentItem = selItem->isGroupType()? selItem : selItem->parent();
  Qt::ItemFlags itemFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
    Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

  return this->createContourNode(parentItem, contourObj, itemFlags);

/*
  //Add the new filter to the table
  QTreeWidget* treeWidget = this->Internal->LIDARPanel->getGUIPanel()->tablePolygon;
  int row_count = table->rowCount();
  table->setRowCount(row_count+1);
  table->blockSignals(true);
  //not enabled until contour is done.
  //QColor contourUnfinishedColor(255, 221, 218); // light red / rose
  Qt::ItemFlags commFlags(Qt::ItemIsSelectable);
  QTableWidgetItem* objItem = new QTableWidgetItem();
  QVariant vdata;
  vdata.setValue(static_cast<void*>(contourWidget));
  objItem->setData(Qt::UserRole, vdata);
  table->setItem(row_count, UseFilterCol, objItem);
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(Qt::Unchecked);
  objItem->setBackgroundColor(this->Internal->contourUnfinishedColor);

  QTableWidgetItem *toAdd = new QTableWidgetItem();
  //Name the filter New Filter [index]
  QString filterText = "New Contour ";
  static int contourNum = 0;
  filterText += QString::number(contourNum++);
  toAdd->setText(filterText);
  table->setItem(row_count,NameCol,toAdd);

  QTableWidgetItem* invertItem = new QTableWidgetItem();
  table->setItem(row_count, InvertCol, invertItem);
  invertItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  invertItem->setCheckState(Qt::Checked);

  QTableWidgetItem* roiItem = new QTableWidgetItem();
  table->setItem(row_count, ROICol, roiItem);
  roiItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  roiItem->setCheckState(Qt::Unchecked);
  table->selectRow(row_count);
  this->onPolygonTableSelectionChanged();
*/

}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::clear(bool blockSignal)
{
  this->TreeWidget->blockSignals(blockSignal);

  this->TreeWidget->clear();
  if(blockSignal)
    {
    this->TreeWidget->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
QList<QTreeWidgetItem*> pqCMBLIDARContourTree::getSelectedItems() const
{
  //QList<QTreeWidgetItem*> selList;
  //return selList;
  return this->TreeWidget->selectedItems();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::selectItem(QTreeWidgetItem* item)
{
  this->TreeWidget->setCurrentItem(item);
  item->setSelected(true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::onSelectionChanged()
{
  QTreeWidget *treeWidget = this->TreeWidget;
  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();
  pqContourWidget* contourObj = NULL;
  for(int r=0; r<rootNode->childCount(); r++)
    {
    QTreeWidgetItem* item = rootNode->child(r);
    bool pSelected = item->isSelected();
    for(int i=0; i<item->childCount();i++)
      {
      contourObj = this->getItemObject(item->child(i));
      bool active = (pSelected || item->child(i)->isSelected()) ? true : false;
      if(active)
        {
        contourObj->select();
        }
      else
        {
        contourObj->deselect();
        }
      pqSMAdaptor::setElementProperty(
        contourObj->getWidgetProxy()->GetProperty("ProcessEvents"),
        active);
      contourObj->getWidgetProxy()->UpdateVTKObjects();
      }
    }

  QTreeWidgetItem* selItem =
    treeWidget->selectedItems().size() ? treeWidget->selectedItems().value(0) : NULL;
  emit selectionChanged(selItem);
}

//----------------------------------------------------------------------------
void pqCMBLIDARContourTree::onItemsDroppedOnItem(
  QTreeWidgetItem* parentItem, QDropEvent*)
{
  if(!parentItem)
    {
    return;
    }

  QTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->blockSignals(true);

  // BCS node
  QTreeWidgetItem *parentNode = NULL;
  if(parentItem == treeWidget->invisibleRootItem())
    {
    parentNode = this->createContourGroupNode();
    }
  else
    {
    int nodeType = parentItem->type();
    parentNode = (nodeType == ContourGroupType) ? parentItem : parentItem->parent();
    }

  if(parentItem == parentNode)
    {
    return;
    }

  if(parentNode)
    {
    parentNode->setExpanded(true);
    this->moveContourItemsToNode(parentNode,
      this->getSelectedItems());
    }
  treeWidget->blockSignals(false);

  if(parentNode)
    {
    this->selectItem(parentNode);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::onItemChanged(QTreeWidgetItem* item, int col)
{
  if(col != UseFilterCol && col != InvertCol)
    {
    return;
    }
  this->TreeWidget->blockSignals(true);
  QList<QTreeWidgetItem*> changedItems;
  if(item->type() == ContourGroupType)
    {
    int val = (item->checkState(col) == Qt::Checked) ? 1 : 0;
    if (col != pqCMBLIDARContourTree::InvertCol)
      {
      for(int r=0; r<item->childCount(); r++)
        {
        if(item->child(r)->checkState(col) != item->checkState(col))
          {
          item->child(r)->setCheckState(col, item->checkState(col));
          changedItems.append(item->child(r));
          }
        }
      }
    else
      {
      changedItems.append(item);
      }
    }
  else
    {
    changedItems.append(item);

 /////Need logic to handle group node state
    if(col == UseFilterCol && item->checkState(col)==Qt::Checked &&
      item->parent()->checkState(col) == Qt::Unchecked)
      {
      item->parent()->setCheckState(col, Qt::Checked);
      }
    }
  this->TreeWidget->blockSignals(false);

  if(changedItems.count() > 0)
    {
    emit this->itemChanged(changedItems, col,
      item->checkState(col)==Qt::Checked ? 1 : 0);
    }
/*

  int idx = item->row();
  int val = (item->checkState(col) == Qt::Checked) ? 1 : 0;

  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  for(int i = 0; i < visiblePieces.size(); ++i)
    {
    if(item->column() == UseFilterCol)
      {
      visiblePieces[i]->updatePolygonUseFilter(idx, val);
      }
    else if(item->column() == InvertCol)
      {
      visiblePieces[i]->updatePolygonInvert(idx, val);
      }
    else if(item->column() == ROICol)
      {
      visiblePieces[i]->updatePolygonROI(idx, val);
      QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->tablePolygon;
      QTableWidgetItem* invertItem = table->item(item->row(), InvertCol);

      invertItem->setFlags(val ? Qt::ItemIsSelectable :
        Qt::ItemIsSelectable| Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
      //if(!val)
      //  {
      //  invertItem->setCheckState(Qt::Unchecked);
      //  }
      }
    }
  this->activeRenderView()->render();
*/
}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::moveContourItemsToNode(
  QTreeWidgetItem* copytoNode, QList<QTreeWidgetItem*> selItems)
{
  QTreeWidgetItem* copyItem;
  int fromGroup = -1;
  QList<QTreeWidgetItem*> newChildren;
  for(int n=0; n<selItems.count(); n++)
    {
    copyItem = selItems.value(n);

    if(copyItem->childCount()==0)
      {
      if(copyItem->type() != ContourGroupType &&
        !newChildren.contains(copyItem))
        {
        newChildren.append(copyItem);
        fromGroup = static_cast<pqCMBContourTreeItem*>(copyItem->parent())->itemId();
        if(copyItem->parent() != copytoNode)
          {
          copyItem->parent()->removeChild(copyItem);
          }
        }
      }
    else
      {
     this->addUniqueChildren(copyItem, newChildren);
     if(copyItem != copytoNode)
       {
       fromGroup = static_cast<pqCMBContourTreeItem*>(copyItem)->itemId();
       copyItem->parent()->removeChild(copyItem);
       }
      }
    }

  if(newChildren.count()>0)
    {
    copytoNode->insertChildren(copytoNode->childCount(),newChildren);
    emit this->onItemsDropped(copytoNode, fromGroup, newChildren);
    }
}


//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::addUniqueChildren(
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
void pqCMBLIDARContourTree::clearAllUseContours()
{
  // Reset all filters to be un-used
  QTreeWidget* treeWidget = this->TreeWidget;
  treeWidget->clearSelection();
  treeWidget->blockSignals(true);

  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();
  for(int r=0; r<rootNode->childCount(); r++)
    {
    QTreeWidgetItem* item = rootNode->child(r);
    item->setCheckState(UseFilterCol, Qt::Unchecked);
    for(int i=0; i<item->childCount();i++)
      {
      item->child(i)->setCheckState(UseFilterCol, Qt::Unchecked);
      }
    }
  treeWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::FindContourItem(
  pqContourWidget* contourWidget)
{
  if(!contourWidget)
    {
    return NULL;
    }

  QTreeWidget* treeWidget = this->TreeWidget;
  QTreeWidgetItem* rootNode = treeWidget->invisibleRootItem();
  for(int r=0; r<rootNode->childCount(); r++)
    {
    QTreeWidgetItem* item = rootNode->child(r);
    for(int i=0; i<item->childCount();i++)
      {
      if(this->getItemObject(item->child(i)) == contourWidget)
        {
        return item->child(i);
        }
      }
    }
  return NULL;
  }

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::contourFinished(pqContourWidget* contourWidget)
{
  QTreeWidgetItem* contourItem = this->FindContourItem(contourWidget);
  if(contourItem)
    {
    contourItem->setDisabled(0);
    contourItem->setBackgroundColor(NameCol, this->contourFinishedColor);
    contourItem->parent()->setDisabled(0);
    contourItem->parent()->setBackgroundColor(NameCol, this->contourFinishedColor);
    if(!contourItem->isSelected())
      {
      this->selectItem(contourItem);
      // this->selectionChanged(contourItem);
      }
    }
  return contourItem;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARContourTree::isContourApplied(
  QTreeWidgetItem* contourItem)
{
  if(contourItem)
    {
    return (contourItem->checkState(UseFilterCol) == Qt::Checked);
    }
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARContourTree::deleteSelected()
  {
  QTreeWidget *treeWidget = this->TreeWidget;
  QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
  if (!selectedItems.count())
    {
    QMessageBox::warning(NULL,
      tr("Cannot Remove Polygons"),
      tr("You must select a polygon first."),
      QMessageBox::Ok);
    return;
    }
  treeWidget->blockSignals(true);
  QList<pqContourWidget*> removeItems;
  pqContourWidget* contourObj;

  // if we allow Multiple selection, the following logic need to be modified
  for(int r=0; r<selectedItems.size(); r++)
    {
    QTreeWidgetItem* item = selectedItems.value(r);
    if(item->type() == ContourGroupType)
      {
      for(int i=0; i<item->childCount();i++)
        {
        contourObj = this->getItemObject(item->child(i));
        if(!removeItems.contains(contourObj))
          {
          removeItems.append(contourObj);
          }
        }
      treeWidget->invisibleRootItem()->removeChild(item);
      }
    else
      {
      contourObj = this->getItemObject(item);
      if(!removeItems.contains(contourObj))
        {
        removeItems.append(contourObj);
        }
      item->parent()->removeChild(item);
      }
    }
  treeWidget->blockSignals(false);
  emit this->itemRemoved(removeItems);
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBLIDARContourTree::onContourChanged(
  pqContourWidget* contourWidget)
{
  QTreeWidgetItem* contourItem = this->FindContourItem(contourWidget);
  if(contourItem)
    {
    //QColor contourFinishedColor(255, 255, 255);
    contourItem->setBackgroundColor(NameCol, this->contourUnfinishedColor);
    }
  return contourItem;
}
