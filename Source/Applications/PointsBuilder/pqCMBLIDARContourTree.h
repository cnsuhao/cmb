//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLIDARContourTree - a LIDAR contour tree object class.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLIDARContourTree_h
#define __pqCMBLIDARContourTree_h

#include <QObject>
#include <QAbstractItemView>

#include "qtCMBTreeWidget.h"
#include "cmbSystemConfig.h"

class QTreeWidgetItem;
class QDropEvent;
class QTreeWidget;
class QIcon;
class QColor;
class QAction;
class pqContourWidget;
class pqCMBContourTreeItem;

class pqCMBLIDARContourTree :  public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARContourTree(QWidget* parent);
  virtual ~pqCMBLIDARContourTree();

  // enum for different column types
  enum enumColumns
    {
    UseFilterCol  = 0,
    NameCol       = 1,
    InvertCol     = 2
    };

  // Description:
  // Convenient methods related to selection on tree widgets
  virtual void clearSelection(bool blockSignal = false);
  QList<QTreeWidgetItem*> getSelectedItems() const;
  void selectItem(QTreeWidgetItem* item);
  void setSelectionMode(QAbstractItemView::SelectionMode mode)
  {this->TreeWidget->setSelectionMode(mode);}
  QAbstractItemView::SelectionMode selectionMode() const
  {return this->TreeWidget->selectionMode();}
  pqContourWidget* getItemObject(QTreeWidgetItem* treeItem);

  // Description:
  // Method to enable/disable sorting in tree widgets
  void setSortingEnabled(bool enable);
  //pqContourWidget* getContourWidgetFromRow(int row);
  QTreeWidgetItem* FindContourItem(pqContourWidget*);
  bool isContourApplied(QTreeWidgetItem* contourItem);

  // Description:
  // Get the number of Groups (top level children) added to the tree
  unsigned int getNumberOfGroups();

  // Description:
  // Get group node item by index
  QTreeWidgetItem* getGroup(unsigned int idx);

  QColor contourUnfinishedColor; // light red / rose
  QColor contourFinishedColor;
  // Description:
  // Some internal ivars.
  qtCMBTreeWidget* TreeWidget;

public slots:
  virtual void clear(bool blockSignal = false);
  virtual void clearAllUseContours();
  virtual QTreeWidgetItem* contourFinished(pqContourWidget*);
  QTreeWidgetItem* addNewContourNode(pqContourWidget*);
  void deleteSelected();
  pqCMBContourTreeItem* createContourGroupNode();
  QTreeWidgetItem* onContourChanged(pqContourWidget* contourWidget);

signals:
  void dragStarted(pqCMBLIDARContourTree*);
  void selectionChanged(QTreeWidgetItem*);
  void itemChanged(QList<QTreeWidgetItem*>, int, int);
  void onItemsDropped(QTreeWidgetItem*toNode, int fromGroup,
    QList<QTreeWidgetItem*> newChildren);
  void itemRemoved(QList<pqContourWidget*>);

protected slots:

  // Description:
  // Tree widget interactions related slots
  virtual void onDragStarted(QTreeWidget*);
  //virtual void onGroupClicked(QTreeWidgetItem*, int){}
  virtual void onSelectionChanged();
  virtual void onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*);
  virtual void onItemChanged(QTreeWidgetItem*, int);

protected:

  // Description:
  // Create and Initialize the internal tree widget.
  virtual void createWidget(QWidget* parent);

  // Description:
  // Methods to create a tree node on the tree widget
  QTreeWidgetItem* createContourNode(
    QTreeWidgetItem* parentNode,
    pqContourWidget* contourObj, Qt::ItemFlags commFlags,
    const QString& text, int itemid, int type=0,
    bool setApplyContour=false, bool invert=true);
  QTreeWidgetItem* addNewTreeNodeOnRoot();
  QTreeWidgetItem* createContourNode(
    QTreeWidgetItem* parentNode,
    pqContourWidget* contourObj, Qt::ItemFlags commFlags, int type=0,
    bool setApplyContour=false, bool invert=true);
  void moveContourItemsToNode(
    QTreeWidgetItem* copytoNode, QList<QTreeWidgetItem*> selItems);
  void addUniqueChildren(
    QTreeWidgetItem* copyItem, QList<QTreeWidgetItem*> &newChildren);
  // Description:
  // Some internal convenient methods.
  virtual void customizeTreeWidget();

};

#endif /* __pqCMBLIDARContourTree_h */
