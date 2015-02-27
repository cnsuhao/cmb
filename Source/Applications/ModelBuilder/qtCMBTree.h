/*=========================================================================

  Program:   CMB
  Module:    qtCMBTree.h

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
// .NAME qtCMBTree - a CMB tree object base class.
// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBTree_h
#define __qtCMBTree_h

#
#
#include "pqCMBModel.h"
#include "qtCMBTreeWidget.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelFace;
class vtkModelEntity;
class QTreeWidgetItem;
class QDropEvent;
class QTreeWidget;
class QIcon;
class QColor;
class QAction;

#define TREE_VISIBLE_COL    1
#define FACE_TEXT_BASE     "Face "

class qtCMBTree :  public QObject
{
  Q_OBJECT

public:
  qtCMBTree(pqCMBModel* model);
  virtual ~qtCMBTree();

  // Description:
  // Get the model object
  pqCMBModel* getModel();

  // Description:
  // Create and Initialize the internal tree widget.
  // The derived classes need to implement these methods.
  virtual void createWidget(QWidget* parent)=0;
  virtual void initializeTree(){};

  // Description:
  // Set the tree widget for the source of the drag-n-drop operation
  void setDragFromTree(qtCMBTree* sourceTree)
  {this->DragFromTree = sourceTree;}

  // Description:
  // Convenient methods related to selection on tree widgets
  virtual void clearSelection(bool blockSignal = false);
  QList<QTreeWidgetItem*> getSelectedItems() const;
  QList<QTreeWidgetItem*> getSelectedItems(int entityType);
  virtual QList<vtkIdType> getSelectedModelEntityIds(int entityType);
  void selectItem(QTreeWidgetItem* item);
  void setSelectionMode(QAbstractItemView::SelectionMode mode)
  {this->TreeWidget->setSelectionMode(mode);}
  QAbstractItemView::SelectionMode selectionMode() const
  {return this->TreeWidget->selectionMode();}
  vtkModelEntity* getItemObject(QTreeWidgetItem* treeItem);
  virtual void setActionsEnabled(bool enabled);

  // Description:
  // Method to enable/disable sorting in tree widgets
  void setSortingEnabled(bool enable);

  // Description:
  // Set the solid color on selected entity objects
  void setSolidColorOnSelections(const QColor&);

  qtCMBTreeWidget* widget(){return this->TreeWidget;}

public slots:
  virtual void onModelLoaded()
    {this->initializeTree();}
  virtual void clear(bool blockSignal = false);

signals:
  void dragStarted(qtCMBTree*);
  void selectionChanged(qtCMBTree*);
  void createNew();
  void deleteSelected();
  void toggleVisibility();

protected slots:

  // Description:
  // Tree widget interactions related slots
  virtual void onDragStarted(QTreeWidget*);
  virtual void onGroupClicked(QTreeWidgetItem*, int){}
  virtual void onGroupSelectionChanged(){}
  virtual void onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*){}
  virtual void onGroupChanged(QTreeWidgetItem*, int){}

  // Description:
  // Tree widget context menu related slots
  virtual void onCreateNew();
  virtual void onDelete();
  virtual void onToggleVisibility();

protected:
  // Description:
  // Methods to create a tree node on the tree widget
  QTreeWidgetItem* createTreeNode(
    QTreeWidgetItem* parentNode, int colId,
    vtkModelEntity* modelEntity, Qt::ItemFlags commFlags,
    const QString& text,
    bool setVisibleIcon=true, int type=0, bool setWidget=true);
  QTreeWidgetItem* addNewTreeNodeOnRoot(int newIdColumn,
    vtkModelEntity* modelEntity);
  QTreeWidgetItem* createEntityNode(
    QTreeWidgetItem* parentNode, int colId,
    vtkModelEntity* modelEntity, Qt::ItemFlags commFlags,
    bool setVisibleIcon=true, bool setWidget=true);
  QTreeWidgetItem* createRootEntityNode(
    vtkModelEntity* modelEntity, int col);

  // Description:
  // Some internal convenient methods.
  virtual void customizeTreeWidget();
  virtual int getNameColumn() = 0;
  bool IsEntityVisibleInTree(vtkIdType faceId, QTreeWidget* treeWidget);
  void getVisibleChildEntityIds(
    QTreeWidgetItem* item, QList<vtkIdType> &visEntities);
  void addUniqueChildren(QTreeWidgetItem*, QList<QTreeWidgetItem*> &list);
  bool canTurnOffEntityVisibility(
    QTreeWidgetItem* changedFaceItem, vtkIdType faceId);
  void clearChildren(QTreeWidgetItem* parentItem);

  // Description:
  // Some internal ivars.
  qtCMBTreeWidget* TreeWidget;
  pqCMBModel* CMBModel;
  QIcon* IconVisible;
  QIcon* IconInvisible;
  QAction* Action_ToggleVisibility;
  QAction* Action_CreateNew;
  QAction* Action_Delete;

  qtCMBTree* DragFromTree;

};

#endif /* __qtCMBTree_h */
