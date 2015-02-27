/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBArcManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBArcManager
// .SECTION Description
// This class is made to manager both the arcs on the server and
// the arc connectivity relationships

#ifndef __vtkCMBArcManager_h
#define __vtkCMBArcManager_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "cmbSystemConfig.h"
#include <map>
#include <set>

#include "vtkObject.h"
#include "vtkBoundingBox.h" //needed to determine when to rebuild locator

class vtkCMBArc;
class vtkCMBArcEndNode;
class vtkIncrementalOctreePointLocator;
class vtkPoints;

//BTX
class VTKCMBGENERAL_EXPORT vtkCMBArcManagerCleanup
{
public:
  vtkCMBArcManagerCleanup();
  ~vtkCMBArcManagerCleanup();
};
//ETX

class VTKCMBGENERAL_EXPORT vtkCMBArcManager : public vtkObject
{
friend class vtkCMBArc;
public:
// Methods from vtkObject
  vtkTypeMacro(vtkCMBArcManager,vtkObject);
  // Description:
  // Print ObjectFactor to stream.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //Description
  //Constructs a special temporary arc manager
  //for a subset of arcs. You will only be able to query about
  //the arcs you created with, as you will not be able to add new arcs
  //or change any of the relationships
  vtkCMBArcManager(std::set<vtkCMBArc*> arcs);
  virtual ~vtkCMBArcManager();

  // Description:
  // This is a singleton pattern New.  There will only be ONE
  // reference to a vtkCMBArcManager object per process.  Clients that
  // call this must call Delete on the object so that the reference
  // counting will work.   The single instance will be unreferenced when
  // the program exits.
  static vtkCMBArcManager* New();
  // Description:
  // Return the singleton instance with no reference counting.
  static vtkCMBArcManager* GetInstance();
  // Description:
  // Supply a user defined output window. Call ->Delete() on the supplied
  // instance after setting it.
  static void SetInstance(vtkCMBArcManager *instance);

  // use this as a way of memory management when the
  // program exits the SmartPointer will be deleted which
  // will delete the Instance singleton
  static vtkCMBArcManagerCleanup Cleanup;

  //Description:
  //Get the Snap Radius
  vtkGetMacro(SnapRadius,double);

  //Description:
  //Set the Snap Radius
  void SetSnapRadius(double radius);

  //Description:
  //Enable the use of snapping when creating and moving end nodes
  vtkSetMacro(UseSnapping,bool);
  vtkGetMacro(UseSnapping,bool);

  //Description:
  //Get the number of arcs
  int GetNumberOfArcs() const;

  //Description:
  //Get the number of end nodes
  int GetNumberOfEndNodes() const;

  //Description:
  //Get the active arc with the passed in id.
  //if the arc is marked to be deleted this will fail
  vtkCMBArc* GetArc(const vtkIdType& id);

  //Description:
  //Get the arc with the passed in Id if it exists
  //in the set of arcs ready to be deleted.
  vtkCMBArc* GetArcReadyForDeletion(const vtkIdType& id);

  //Description:
  //Returns the if the arc end node is being managed by the
  //arc manager.
  bool IsManagedEndNode(vtkCMBArcEndNode *endNode) const;

  //Description:
  //Returns the number of arcs that use the passed in endnode.
  //Returns -1 if endNode isn't managed by this arc manager
  int GetNumberOfArcs(vtkCMBArcEndNode *endNode) const;

  //Description:
  //Get the set of connected arcs for the passed in endNode
  std::set<vtkCMBArc*> GetConnectedArcs(vtkCMBArcEndNode *endNode);

  //Description:
  //Get the set of connected arcs for the passed in arc. Does not
  //include the arc that is passed in.
  std::set<vtkCMBArc*> GetConnectedArcs(vtkCMBArc* arc);

  //Description:
  //If an end node exists at the position it will return it
  //otherwise it will return NULL;
  vtkCMBArcEndNode* GetEndNodeAt(double position[3]);

protected:
  vtkCMBArcManager();

  //Description:
  //creates an endnode at the positon in space.
  //if an endnode already exists there it returns that one
  vtkCMBArcEndNode* CreateEndNode(double position[3]);

  //Description:
  //Merge the the two end node together. This method will update the point locator with
  //The new point locations, it will also make sure that all the arcs and end nodes are
  //updated with the new merged end node.
  //Returns the merged endnode. NULL if the nodes aren't managed by the arc manager
  //Note: This method will invalidate any EndNode pointers you currently have,
  //so after calling this method you will need to reget the end nodes for all arcs.
  vtkCMBArcEndNode* MergeEndNodes(vtkCMBArcEndNode* endNode1, vtkCMBArcEndNode *endNode2);


  //Description:
  //moves the passed in end node to the position also passed in.
  //Since this can cause endNodes to be merged you have to use the
  //end node that is returned.
  vtkCMBArcEndNode* MoveEndNode(vtkCMBArcEndNode *endNode, double position[3]);

  //Description:
  //Removes the passed in arc end node with the associated arc
  //Note: If this removes all arcs from using an end node
  //it will delete the end node.
  //Returns: True if the endNode can be removed for the arc.
  bool RemoveEndNode(vtkCMBArcEndNode *en, vtkCMBArc *arc);

  //Description:
  //Connects the passed in arc end node with the associated arc
  bool AddEndNode(vtkCMBArcEndNode *en, vtkCMBArc *arc);

  //Description:
  //Register the passed in arc as an arc that this manager will control
  //You need to Register any arcs you want to use with operators
  //or rendered
  void RegisterArc(vtkCMBArc* arc);

  //Description:
  //Un register an arc, generally done before deletion so that
  //you don't mess up any arc to arc connectivity maps
  void UnRegisterArc(vtkCMBArc* arc);

  //Description:
  //Mark the arc as going to be deleted. this done
  //when an arc is on the undo stack. We move the arc to the undo
  //arc set so we can clean up memory properly.
  void MarkedForDeletion(vtkCMBArc* arc);

  //Description:
  //Move the arc from the undo arc set to the arc set.
  void UnMarkedForDeletion(vtkCMBArc* arc);

  //Description:
  //For a given id from the vtkPoints return the related end node.
  vtkCMBArcEndNode* EndNodeFromPointId(const vtkIdType &pointId);

  //Description:
  //For a given id from the vtkPoints return the related end node.
  void AddEndNodeToLocator(vtkCMBArcEndNode *en);

  //Description:
  //Build up the point set and the point locator for the current end nodes
  void BuildLocator();

  //Description:
  //Destroy the current point locator
  void LocatorModified();

  vtkIncrementalOctreePointLocator *PointLocator;
  vtkBoundingBox LocatorBounds;

  typedef std::set<vtkCMBArc*> vtkCMBArcSet;
  typedef std::map<vtkIdType,vtkCMBArc*> vtkCMBArcMap;
  typedef std::map<vtkIdType,vtkCMBArcEndNode*> vtkPointIdsToEndNodeMap;
  typedef std::map<vtkCMBArcEndNode*,vtkCMBArcSet> vtkCmbEndNodesToArcMap;


  //valid arcs
  vtkCMBArcMap Arcs;

  //arcs on the undo stack
  vtkCMBArcSet UndoArcs;

  //lookup table: for point locator points to end nodes
  vtkPointIdsToEndNodeMap PointIdsToEndNodes;

  //storage for the end node to arc connectivity
  vtkCmbEndNodesToArcMap EndNodesToArcs;

  bool UseSnapping;
  double SnapRadius;
  double SnapRadiusSquared;

  bool LocatorNeedsRebuilding;

private:
  static vtkCMBArcManager* Instance;
  vtkCMBArcManager(const vtkCMBArcManager&);  // Not implemented.
  void operator=(const vtkCMBArcManager&);  // Not implemented.
};

#endif
