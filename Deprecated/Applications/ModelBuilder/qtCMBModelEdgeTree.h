//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBModelEdgeTree - a CMB model edget tree object.
// .SECTION Description
//  This class contains a tree widget, which represents the hierarchy of
//  floating edges of the model, defined within a region.
//  It also handles all the user interactions on the tree widget and interactions
//  with the main application.
// .SECTION Caveats

#ifndef __qtCMBModelEdgeTree_h
#define __qtCMBModelEdgeTree_h

#include "qtCMBTree.h"
#include "cmbSystemConfig.h"

class pqCMBModel;
class QTreeWidgetItem;
class vtkModelEntity;

class  qtCMBModelEdgeTree :  public qtCMBTree
{
  Q_OBJECT

public:
  qtCMBModelEdgeTree(pqCMBModel* model);
  virtual ~qtCMBModelEdgeTree();

  // Description:
  // Create and Initialize the internal tree widget.
  virtual void createWidget(QWidget* parent);
  virtual void initializeTree();

  // Description:
  // Update all the tree nodes text that associated with this model entity.
  void updateRegionNodeText(
    vtkModelEntity* regionEntity);

  // Description:
  // Get number of selected edges.
  int getNumberOfSelectedEdges()
    { return this->NumberOfSelectedModelEdges; }

  // Description:
  // set line resolution on selected edges.
  void setLineResolutionOnSelected(int res);

  // Description:
  // get selected visible edges
  void getSelectedVisibleEdges(QList< vtkIdType > &selEdges);

  // Description:
  // Create a new ModelEdge node with the given ModelEdge id or entity
  // Return, the created new node
  // QTreeWidgetItem* createModelEdgeNode(vtkIdType ModelEdgeId);
  // QTreeWidgetItem* createModelEdgeNode(vtkModelEntity* modelEntity);

protected slots:
  virtual void onGroupClicked(QTreeWidgetItem*, int);
  virtual void onGroupSelectionChanged();
  virtual void onGroupChanged(QTreeWidgetItem*, int);

protected:

  // Description:
  // Get the column that has the ModelEdge name.
  virtual int getNameColumn();

  // Description:
  // Some internal convenient methods.
  virtual void customizeTreeWidget();
  void changeChildItemVisibilityIcon(
    QTreeWidgetItem* item, int visible);

  // Description:
  // Number of selected edges
  int NumberOfSelectedModelEdges;

};

#endif /* __qtCMBModelEdgeTree_h */
