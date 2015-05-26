//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBModelTree - a CMB model tree object.
// .SECTION Description
//  This class contains a tree widget, which represents the hierarchy of
//  topology (material<=>region<=>model face) of the model, and contains all
//  the operations that can be invoked on the model. It also handles all the
//  user interactions on the tree widget and communications
//  with the main application.
// .SECTION Caveats

#ifndef __qtCMBModelTree_h
#define __qtCMBModelTree_h

#include "qtCMBTree.h"
#include <QMap>
#include "cmbSystemConfig.h"

class pqCMBModel;
class vtkModelMaterial;
class QTreeWidgetItem;
class QAction;
class vtkCollection;

enum ModelTreeColumns
{
  MTree_MATERIAL_COL=0,
  MTree_TEXTURE_COL=2,
  MTree_MESH_LENGTH_COL=3,
  MTree_MESH_MIN_ANGLE_COL=4
};

class  qtCMBModelTree :  public qtCMBTree
{
  typedef qtCMBTree Superclass;
  Q_OBJECT

public:
  qtCMBModelTree(pqCMBModel* model);
  virtual ~qtCMBModelTree();

  // Description:
  // Create and Initialize the internal tree widget.
  virtual void createWidget(QWidget* parent);
  virtual void initializeTree();

  // Description:
  // Select model faces according to the selection mode, Material
  // or Region or Model face.
  void selectModelEntitiesByMode(QList<vtkIdType>& faces,
    int selectionMode, int clearSelFirst);

  // Description:
  // Select/Get the tree items according to the model entity type,
  // Material or Region or Model face.
  void selectItemsByType(QList<vtkIdType>& entityIds,
    int entityType, int clearSelFirst);
  QList<QTreeWidgetItem*> getTreeItemsWithType(int entityType);

  // Description,
  // Methods to merge and split model faces
  void splitModelFaces(double angle);
  void mergeSelectedFaces();

  // Description,
  // Methods to create Model Edges
  void createModelEdges();

  // Description:
  // Update the region faces nodes. After split operation, the new faces
  // need to be added to the tree.
  void updateWithNewEntityNodes(QMap< vtkIdType, QList<vtkIdType> >& splitMap);

  // Description:
  // Remove the region faces nodes. After merge operation, the merged faces
  // need to be removed from the tree.
  void removeMergedEntityNodes(vtkIdType toFaceId, QList<vtkIdType>& faces);

  // Description:
  // Methods to create and remove material nodes.
  QTreeWidgetItem* createMaterialNode(vtkModelMaterial* cmbMaterial);
  void removeSelectedMaterialNodes();

  // Description:
  // These methods only works for 2D model currently,
  // and they ignore the visibility of the model or mesh entity.
  void getSelectedMeshFaces(vtkCollection* selEntities);
  void getSelectedMeshEdges(vtkCollection* selEntities);
  void getSelectedMeshEntities(
    const QMap<vtkIdType, pqCMBModelEntity*>& entityMap,
    vtkCollection* selEntities);

  // Description:
  // Some convenient methods
  int getNumberOfSelectedItems()
  {return this->NumberOfSelectedItems;}
  int getNumberOfSelectedFaces()
  {return this->NumberOfSelectedFaces;}
  int getSelectedFacesMergable()
  {return this->SelectedFacesMergable;}
  int getNumberOfSelectedEmptyMaterialItems()
  {return this->NumberOfSelectedEmptyMaterialItems;}
  void setSharedEntitiesVisibility(bool visible);
  void setFacesVisibility(bool visible, bool excludeShared=false);
  void setEdgesVisibility(bool visible, bool excludeShared=false);
  void showMeshInfo(bool flag);
  void updateFaceMeshInfo(vtkCollection* selEdges);
  void updateEdgeMeshInfo(vtkCollection* selFaces);
  void showTextureColumn(bool show);
  void updateTextureInfo();

signals:
  void createBCS();
  void splitSelected();
  void mergeSelected();
  void meshItemChanged(QTreeWidgetItem* item, int col);

public slots:
  void clear(bool blockSignal = false);

protected slots:
  // Description:
  // Tree interaction related slots
  virtual void onGroupClicked(QTreeWidgetItem*, int);
  virtual void onGroupSelectionChanged();
  virtual void onItemsDroppedOnItem(QTreeWidgetItem*, QDropEvent*);
  virtual void onGroupChanged(QTreeWidgetItem*, int);
  virtual void onMeshInfoChanged(QTreeWidgetItem*, int);
  virtual void updateInterfaceNodeText(
    QTreeWidgetItem* changedFaceItem, vtkIdType entId);

  // Description:
  // The context menu related slots
  void onShowAllEdges();
  void onShowCommonEdges(bool reverse=false);
  void onShowNonCommonEdges();
  void onHideEdges();

protected:

  // Description:
  // Some internal convenient methods
  virtual void customizeTreeWidget();
  virtual int getNameColumn();
  void changeChildItemVisibilityIcon(
    QTreeWidgetItem* item, int visible,
    QList<vtkIdType>& changedFaces, bool force=false, bool recursive=true);
  void dropItemsToNode(QTreeWidgetItem* matNode,
    QList<QTreeWidgetItem*> newChildren);
  void getCommonFaceEdgeIds(QList<QTreeWidgetItem*>& selFaces,
    QList<vtkIdType>& commonEdges);
  void updateEdgeActions(int numSelFaces);
  void showAllEdges(QTreeWidgetItem* faceNode);
  bool isAFreeFaceNode(QTreeWidgetItem* item);

  // Description:
  // ivars
  int NumberOfSelectedItems;
  int NumberOfSelectedFaces;
  int NumberOfSelectedEmptyMaterialItems;
  int SelectedFacesMergable;

  QAction* Action_ShowEdges;
  QAction* Action_HideEdges;
  QAction* Action_ShowAllEdges;
  QAction* Action_ShowCommonEdges;
  QAction* Action_ShowNonCommonEdges;

  QTreeWidgetItem* freeFacesNode;
  QIcon* IconCheck;
  QIcon* IconUncheck;
};

#endif /* __qtCMBModelTree_h */
