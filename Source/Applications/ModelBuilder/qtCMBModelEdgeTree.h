/*=========================================================================

  Program:   CMB
  Module:    qtCMBModelEdgeTree.h

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
