/*=========================================================================

  Program:   CMB
  Module:    qtCMBBoundaryConditionTree.h

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
// .NAME qtCMBBoundaryConditionTree - a CMB BCS tree object.
// .SECTION Description
//  This class contains a tree widget, which represents the hierarchy of
//  boundary conditions of the model, defined on the group of model faces.
//  It also handles all the user interactions on the tree widget and interactions
//  with the main application.
// .SECTION Caveats

#ifndef __qtCMBBoundaryConditionTree_h
#define __qtCMBBoundaryConditionTree_h

#include "qtCMBTree.h"
#include "cmbSystemConfig.h"

class pqCMBModel;
class QTreeWidgetItem;
class vtkModelEntity;

class  qtCMBBoundaryConditionTree :  public qtCMBTree
{
  typedef qtCMBTree Superclass;
  Q_OBJECT

public:
  qtCMBBoundaryConditionTree(pqCMBModel* model);
  virtual ~qtCMBBoundaryConditionTree();

  // Description:
  // Create and Initialize the internal tree widget.
  virtual void createWidget(QWidget* parent);
  virtual void initializeTree();

  // Description:
  // This method will create and add tree nodes for all selected model faces
  // in the DragFromTree.
  void addDraggedFacesToBCNode(QTreeWidgetItem* bcNode)
  { this->onItemsDroppedOnItem(bcNode, NULL); }

  // Description:
  // Update all the tree nodes text that associated with this model entity.
  void updateFaceNodeText(
    vtkModelEntity* faceEntity);

  // Description:
  // Create a new BC node with the given bc id
  // Return, the created new node
  QTreeWidgetItem* createBCNode(vtkIdType bcId);

  // Description:
  // Create a new BC node given a bc id, and the model faces.
  // Return, the created new node
  QTreeWidgetItem* createBCNodeWithEntities(
    vtkIdType bcId, QList<vtkIdType>& faces);

  // Description:
  // Add the new faces, create from a split operation, to corresponding
  // BCS groups.
  void addNewEntitiesFromSplitToBCGroups(
    QMap< vtkIdType, QList<vtkIdType> >&);

  // Description:
  // Remove the input model faces from corresponding BCS groups, such as
  // after a merge, some model faces need to be removed
  void removeEntitiesFromBCGroups(QList<vtkIdType>& faces);

  // Description:
  // Remove seleted BCS nodes.
  void removeSelectedBCNodes();

  // Description:
  // Some convenient methods.
  int GetNumberOfSelectedBCS()
  {return this->NumberOfSelectedBCS;}
  int GetNumberOfSelectedRemovableItems()
  {return this->NumberOfSelectedRemovableItems;}

public slots:
    virtual void clear(bool blockSignal = false);

protected slots:
  virtual void onGroupClicked(QTreeWidgetItem*, int);
  virtual void onGroupSelectionChanged();
  virtual void onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*);
  virtual void onGroupChanged(QTreeWidgetItem*, int);

protected:

  // Description:
  // Get the column that has the BC name.
  virtual int getNameColumn();

  // Description:
  // Some internal convenient methods.
  virtual void customizeTreeWidget();
  virtual void updateUndifinedBCSGroup();
  void changeChildItemVisibilityIcon(
    QTreeWidgetItem* item, int visible, QList<vtkIdType>& changedFaces);
  bool IsBCFaceVisibleInOtherBCS(vtkIdType faceId, vtkIdType BCId);
  void copyBCSItemsToNode(QTreeWidgetItem* matNode,
    QList<QTreeWidgetItem*> newChildren);
  void updateUndefinedNode(
    QMap<vtkIdType, pqCMBModelEntity*> & entityMap,
    QList<vtkIdType> &undefinedBCFaces,
    QTreeWidgetItem* undefNode);

  // Description:
  // Internal ivars
  QTreeWidgetItem* undefinedGroupNode;
  QTreeWidgetItem* undefined3DEdgesNode;
  int NumberOfSelectedBCS;
  int NumberOfSelectedRemovableItems;
};

#endif /* __qtCMBBoundaryConditionTree_h */
