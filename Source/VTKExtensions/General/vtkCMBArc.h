//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArc - vtkDataObject that represent a single arc
// .SECTION Description
// An arc is represented by a line with 1 or 2 end nodes
// and a collection of internal points.
// Each arc has a unique Id

#ifndef __vtkCMBArc_h
#define __vtkCMBArc_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkDataObject.h"
#include "vtkVector.h"
#include "cmbSystemConfig.h"
#include <list>

class vtkCMBArcEndNode;
class vtkCMBArcManager;
class vtkDoubleArray;
class vtkPolyData;

class VTKCMBGENERAL_EXPORT vtkCMBArc : public vtkDataObject
{
friend class vtkCMBArcManager;
public:
  vtkTypeMacro(vtkCMBArc,vtkDataObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBArc *New();

  //comparison operator needed for storage
  bool operator<(const vtkCMBArc &p) const;

  //Description:
  //Get the Id of this arc
  vtkIdType GetId() const;

  //Description:
  //Get if the arc is a closed loop.
  //This is detected by seeing if the first and second node are equal
  //and non NULL
  bool IsClosedArc() const;

  //Description:
  //Gets the number of unique end nodes.
  //Note: In case the first and second end node
  //point to the same end node this will return 1
  int GetNumberOfEndNodes() const;

  //Description:
  //Get the ith end node
  //Note: index must be 0 or 1
  vtkCMBArcEndNode* GetEndNode(const int& index) const;

  //Description:
  //Set the ith end node. If the ith end node
  //already exists it will replace it.
  //Returns true if the end node has been set
  bool SetEndNode(const int& index, double position[3]);

  //Description:
  //Move the ith end node position
  bool MoveEndNode(const int& index, double position[3]);

  //Description:
  //Removes the End Nodes and all the internal end nodes
  void Initialize();

  //Description:
  //Clear all the internal points from the given arc
  bool ClearPoints();

  //Description:
  // Replace the points between startIndex, and endIndex with the list
  // of new points. The startIndex and endIndex are relative the all
  // the arc points (internal points and end nodes).
  // NOTE: this method will not update end nodes of the arc if they
  // are changed by replacing original points. Whoever calling this
  // should update the arc end nodes properly, otherwise, the resuling
  // arc may be invalid.
  bool ReplacePoints(vtkIdType startIndex, vtkIdType endIndex,
    std::list<vtkVector3d>& newPoints, bool includeEnds);

  //Description:
  //Insert an internal point position to the front
  //NOTE: Do not add end node position to the internal
  //point list
  bool InsertPointAtFront(double point[3]);

  //Description:
  //Push back an internal point position
  //NOTE: Do not add end node position to the internal
  //point list
  bool InsertNextPoint(double point[3]);

  //Description:
  //Push back an internal point position
  //NOTE: Do not add end node position to the internal
  //point list
  bool InsertNextPoint(const double& x, const double& y, const double& z);

  //Description:
  //Initialize Traversal of the internal points.
  //This must be called before you call GetNextPoint
  //Setting forwardDirection to false will allow iteration from end to start
  bool InitTraversal(const bool& forwardDirection=true);

  //Description:
  //Initialize Traversal of the internal points.
  //This must be called before you call GetNextPoint
  //startIndex is where the traversal will start;
  //numPoints is how many points will be traversed from startIndex;
  //Setting InvertTraversalRange to false means it will only iterate over those
  // part(s) that are outside of the range (startIndex to startIndex+numPoints)
  //Setting forwardDirection to false will allow backward iteration
  // Note: the indices are only referring to the internal points of the arc,
  // NOT including the end nodes.
  bool InitTraversal(const vtkIdType& startIndex, const vtkIdType& numPoints,
    const bool& forwardDirection=true, const bool& invertTraversalRange=false);

  //Description
  //If end of internal points is encountered, false is returned.
  //You must call InitTraversal() before call this method
  bool GetNextPoint(double point[3]);

  //Description
  //return the position of a point given its id (relative to the internal
  // arc points, NOT including the end nodes).
  // Note: This is not an efficient way to access internal arc points.
  // Use InitTraversal and GetNextPoint if trying to get all internal arc points.
  bool GetArcInternalPoint(vtkIdType pid, double point[3]);

  //Description
  //Returns the number of internal points for this arc
  vtkIdType GetNumberOfInternalPoints() const;

  //Description
  //Returns the number of points for the arc, including internal points
  // and number of end nodes.
  vtkIdType GetNumberOfArcPoints() const;

  //Description:
  //Get the direction vector for an end node
  //the direction array will be filled with the directional vector
  //Note: Directional vector is normalized
  //Returns false if the arc has only one point when you add the
  //end nodes and internal points together or if the index you
  //request is invalid
  bool GetEndNodeDirection(const int& index, double direction[3]) const;

  //Description:
  //Get the direction vector for an end node
  //the direction array will be filled with the directional vector
  //Note: Directional vector is normalized
  //Returns false if the arc has only one point when you add the
  //end nodes and internal points together or if the end node ppassed in
  //isn't used by this arc
  bool GetEndNodeDirection(vtkCMBArcEndNode* endNode, double direction[3]) const;

  //Description:
  // Returns the number of arcs connected to this arc
  // This only counts unique arcs. So if both end nodes
  // are connected to the same arc this will count that arc only once
  int GetNumberOfConnectedArcs( );

  //Description:
  // Returns the number of arcs connected to the given end node.
  // This will not count the arc this method is called on
  // Will return -1 if index is out of range
  int GetNumberOfConnectedArcs(const int& index) const;

  //Description:
  // Returns the number of arcs connected to the given end node.
  // This will not count the arc this method is called on
  // Will return -1 if the end node is NULL
  // Will return -2 if the end node is not a end node of this arc
  int GetNumberOfConnectedArcs(vtkCMBArcEndNode* endNode) const;

  //Description:
  //Since the node is going to be removed ( added to the undo stack )
  //We need to remove the ends from the global end node collection
  void MarkedForDeletion();

  //Description:
  //Since the node is not being removed ( removed from the undo stack )
  //We need to add the ends from the global end node collection
  void UnMarkedForDeletion();

  //Description:
  // check if the specified range is the whole arc
  static bool IsWholeArcRange(vtkIdType startId, vtkIdType endId,
    vtkIdType numArcPoints, bool closedArc);

protected:
  vtkCMBArc();
  virtual ~vtkCMBArc();

  //Description:
  //Replaces the oldEndNode with the newEndNode, this method is called by the arc
  //manager to complete the merge end nodes operation
  bool ReplaceEndNode(vtkCMBArcEndNode* oldEndNode, vtkCMBArcEndNode* newEndNode);

  vtkCMBArcEndNode **EndNodes;
  vtkCMBArcManager *ArcManager;

  struct internalPoint
    {
    public:
      double x;
      double y;
      double z;
    };


  //list of internal points that are between the end nodes.
  typedef std::list<internalPoint> InternalPointList;
  InternalPointList Points;
  InternalPointList::const_iterator PointIterator;
#ifdef VTK_CONST_REVERSE_ITERATOR_COMPARISON
  typedef
  InternalPointList::const_reverse_iterator InternalConstReversePointIterator;
#else
  typedef InternalPointList::reverse_iterator InternalConstReversePointIterator;
#endif
  InternalConstReversePointIterator ReversePointIterator;
  bool PointIterationForward;

  //Description:
  //Insert next point to the list
  bool InternalInsertNextPoint(
    InternalPointList& Points, const double point[3]);

  bool InvertTraversalRange;
  vtkIdType TraversalStartIndex;
  vtkIdType NumberOfSpecifiedTraversalPoints;
  vtkIdType NumberOfPointsToTraverse;
  vtkIdType CurrentNumberOfTraversedPoints;

  //list of end nodes we had. We use this when we are marked for
  //deleted since the end nodes will become invalidated
  InternalPointList UndoEndNodes;

private:
  bool InvalidEndNodeIndex(const int& index) const;
  const vtkIdType Id;
  static vtkIdType NextId;
};

//----------------------------------------------------------------------------
inline bool vtkCMBArc::InvalidEndNodeIndex(const int& index) const
{
  return ( index < 0 || index > 1 );
}

#endif
