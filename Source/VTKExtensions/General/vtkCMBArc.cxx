//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArc - Data storage that represent a single arc
// .SECTION Description
// An arc is represented by a line with 1 or 2 end nodes
// and a collection of internal points.
// Each arc has a unique Id

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"

#include <algorithm>

//----------------------------------------------------------------------------
vtkIdType vtkCMBArc::NextId = 0;
vtkStandardNewMacro(vtkCMBArc);

//----------------------------------------------------------------------------
vtkCMBArc::vtkCMBArc()
  :Points(),PointIterationForward(true),UndoEndNodes(),Id(NextId++)
{
  this->PointIterator = this->Points.end();
  this->ReversePointIterator = this->Points.rend();

  this->EndNodes = new vtkCMBArcEndNode*[2];
  this->EndNodes[0] = NULL;
  this->EndNodes[1] = NULL;

  //Create the arc manager and than register this arc
  this->ArcManager = vtkCMBArcManager::GetInstance();
  this->ArcManager->RegisterArc(this);

  // variables for the new traversal logic
  this->TraversalStartIndex = 0;
  this->NumberOfSpecifiedTraversalPoints = 0;
  this->NumberOfPointsToTraverse = 0;
  this->CurrentNumberOfTraversedPoints = 0;
  this->InvertTraversalRange = false;
}

//----------------------------------------------------------------------------
vtkCMBArc::~vtkCMBArc()
{
  this->ArcManager->UnRegisterArc(this);
  this->Initialize();

  this->ArcManager = NULL;
  delete[] this->EndNodes;
}

//----------------------------------------------------------------------------
void vtkCMBArc::PrintSelf(ostream& os, vtkIndent indent)
{
  double position[3];
  this->Superclass::PrintSelf(os,indent);
  cout << "Id: " << this->Id << endl;
  cout << "IsACloseArc: "  << this->IsClosedArc() << endl;
  cout << "End Nodes Info: " << endl;
  for (int i=0; i < this->GetNumberOfEndNodes(); ++i)
    {
    cout << "End Node: " << i+1 << endl;
    cout << indent.GetNextIndent() << "ID: " << this->GetEndNode(i)->GetPointId()<< endl;
    this->GetEndNode(i)->GetPosition(position);
    cout << indent.GetNextIndent() << " position " << position[0] << ","
            << position[1] << "," << position[2] << endl;
    cout << endl;
    }

  cout << "Number of Internal Points " << this->GetNumberOfInternalPoints() << endl;
  int idx = 0;
  this->InitTraversal();
  vtkCMBArc::Point point;
  while(this->GetNextPoint(point))
    {
    cout << indent.GetNextIndent() << " internal point("<<idx++<<"): " << point[0] << ","
            << point[1] << "," << point[2] << " ID: " << point.GetId() << endl;
    }

}

//----------------------------------------------------------------------------
bool vtkCMBArc::operator<(const vtkCMBArc &p) const
{
   return (this->Id < p.Id);
}

//----------------------------------------------------------------------------
vtkIdType vtkCMBArc::GetId() const
{
  return this->Id;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::IsClosedArc() const
{
  return (this->EndNodes[1] && this->EndNodes[1] == this->EndNodes[0]);
}

//----------------------------------------------------------------------------
int vtkCMBArc::GetNumberOfEndNodes() const
{
  int count = (this->EndNodes[0]) ? 1 : 0;
  count += (this->EndNodes[1] && this->EndNodes[1] != this->EndNodes[0]) ? 1 : 0;
  return count;
}

//----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArc::GetEndNode(const int& index) const
{
  if (this->InvalidEndNodeIndex(index))
    {
    return NULL;
    }
  return this->EndNodes[index];
}

//----------------------------------------------------------------------------
bool vtkCMBArc::SetEndNode(const int& index, vtkCMBArc::Point const& point)
{
  if (this->InvalidEndNodeIndex(index))
    {
    return false;
    }

  //SetEndNode is used to create end nodes and to update end nodes.
  vtkCMBArcEndNode *en = this->ArcManager->CreateEndNode(point);
  if (!en)
    {
    //should never happen
    return false;
    }

  vtkCMBArcEndNode *oldEn = this->EndNodes[index];
  if (oldEn && oldEn == en)
    {
    //we have nothing to update, make sure the end node is registed
    //this might not happen if we are coming from UnMarkedForDelete
    this->ArcManager->AddEndNode(en,this);
    return true;
    }
  //next we remove this connection only if this arc isn't closed
  //and the oldEn exists. The reason for this is that the manager
  //doesn't the number of times an arc is connected to an end node so
  //we do
  if (oldEn && !this->IsClosedArc())
    {
    this->ArcManager->RemoveEndNode(oldEn,this);
    }

  //update the end nodes
  this->EndNodes[index] = en;
  this->ArcManager->AddEndNode(en,this);

  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::MoveEndNode(const int& index, vtkCMBArc::Point const& point)
{
  if (this->InvalidEndNodeIndex(index))
    {
    return false;
    }

  //verify we have an end node at the current index, if not call SetEndNode.
  vtkCMBArcEndNode *oldEn = this->EndNodes[index];
  if(!oldEn)
    {
    return this->SetEndNode(index, point);
    }

  vtkCMBArcEndNode *movedEn = this->ArcManager->MoveEndNode(oldEn, point);
  if (!movedEn)
    {
    return false;
    }
  this->EndNodes[index] = movedEn;

  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArc::Initialize()
{
  //We only want to delete end nodes that aren't used by any other arc
  int size = this->GetNumberOfEndNodes();
  bool managed = false;
  for ( int i=0; i < size; ++i)
    {
    managed = this->ArcManager->IsManagedEndNode(this->EndNodes[i]);
    if ( managed )
      {
      this->ArcManager->RemoveEndNode(this->EndNodes[i],this);
      }
    else
      {
      delete this->EndNodes[i];
      }
    this->EndNodes[i] = NULL;
    }
  this->ClearPoints();
  this->UndoEndNodes.clear();
}

//----------------------------------------------------------------------------
bool vtkCMBArc::ClearPoints()
{
  this->Points.clear();
  this->PointIterator = this->Points.end();
  this->ReversePointIterator = this->Points.rend();
  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::InsertPointAtFront(unsigned int ptId, double point[3])
{
  //create a constant point and push that back
  vtkCMBArc::Point p(point, ptId);
  return this->InsertPointAtFront(p);
}

bool vtkCMBArc::InsertPointAtFront(vtkCMBArc::Point& point)
{
  //make sure we push to the front an unique point, so don't allow consecutive
  //duplicate points
  if ( this->Points.size() != 0 )
    {
    vtkCMBArc::Point& backPoint = this->Points.front();
    if ( backPoint == point )
      {
      return false;
      }
    }
  //create a constant point and push that back
  this->Points.push_front(point);
  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::InsertNextPoint(unsigned int ptId, double point[3])
{
  vtkCMBArc::Point pt(point, ptId);
  return this->InsertNextPoint(pt);
}

//----------------------------------------------------------------------------

bool vtkCMBArc::InsertNextPoint(vtkCMBArc::Point& point)
{
  bool res = this->InternalInsertNextPoint(this->Points,point);
  if(res)
  {
    this->Modified();
  }
  return res;
}

bool vtkCMBArc::InternalInsertNextPoint( InternalPointList& inPoints,
                                         vtkCMBArc::Point& point )
{
  //make sure we push back the next unique point, so don't allow consecutive
  //duplicate points
  if ( inPoints.size() != 0 )
    {
    vtkCMBArc::Point& backPoint = inPoints.back();
    if ( backPoint == point )
      {
      return false;
      }
    }

  //create a constant point and push that back
  inPoints.push_back(point);
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::InsertNextPoint(unsigned int ptId, const double& x,
                                const double& y, const double& z)
{
  vtkCMBArc::Point point(x,y,z,ptId);
  return this->InsertNextPoint(point);
}

//----------------------------------------------------------------------------
bool vtkCMBArc::InitTraversal(const bool& forward)
{
  return this->InitTraversal(
    forward ? 0 : this->GetNumberOfInternalPoints()-1,
    this->GetNumberOfInternalPoints(), forward, false);
}

//----------------------------------------------------------------------------
bool vtkCMBArc::InitTraversal(const vtkIdType& startIndex,
                              const vtkIdType& numPoints,
                              const bool& forwardDirection,
                              const bool& invertTraversalRange)
  {
  if(numPoints <= 0 || startIndex < 0 ||
     numPoints > this->GetNumberOfInternalPoints() ||
    // if the whole range is specified and invertTraversalRange is set,
    // it will not get any points.
    (numPoints == this->GetNumberOfInternalPoints() && invertTraversalRange))
    {
    return false;
    }
  this->NumberOfPointsToTraverse = numPoints;
  this->NumberOfSpecifiedTraversalPoints = numPoints;
  this->TraversalStartIndex = startIndex;
  this->ReversePointIterator = this->Points.rbegin();
  this->PointIterator = this->Points.begin();
  // if not invert traversal range, the iterator needs to be moved to
  // proper starting point.
  if(!invertTraversalRange)
    {
    std::advance(this->PointIterator, forwardDirection ? startIndex :
      startIndex - numPoints + 1);
    std::advance(this->ReversePointIterator, forwardDirection ?
      this->GetNumberOfInternalPoints() - startIndex - numPoints:
      this->GetNumberOfInternalPoints() - startIndex - 1);
    }
  else
    {
    // when inverting the traversal range, the end points of the range will be
    // included with the iteration.
    this->NumberOfPointsToTraverse = this->GetNumberOfInternalPoints() - numPoints + 2;
    // if any of the end points of the specified range includes the start or end
    // point of the arc, the NumberOfPointsToTraverse needs to minus 1.
    if(startIndex == 0 || startIndex == this->GetNumberOfInternalPoints()-1 ||
      (startIndex + numPoints) == this->GetNumberOfInternalPoints())
      {
      this->NumberOfPointsToTraverse--;
      }
    }
  this->PointIterationForward = forwardDirection;
  this->InvertTraversalRange = invertTraversalRange;
  this->CurrentNumberOfTraversedPoints = 0;

  return true;
  }

//----------------------------------------------------------------------------
bool vtkCMBArc::GetNextPoint( vtkCMBArc::Point& point )
{
  int numArcPoints = this->GetNumberOfInternalPoints();
  if (this->PointIterationForward)
    {
    //make sure the iterator is valid
    if ( this->PointIterator == this->Points.end() ||
      this->CurrentNumberOfTraversedPoints >= this->NumberOfPointsToTraverse)
      {
      return false;
      }
    // if inversing range, we skip the range specified,
    // but include the two end points
    if(this->InvertTraversalRange &&
      this->CurrentNumberOfTraversedPoints > this->TraversalStartIndex &&
      this->NumberOfSpecifiedTraversalPoints > 2)
      {
      std::advance(this->PointIterator,
        this->NumberOfSpecifiedTraversalPoints - 2);
      }
    if (this->PointIterator == this->Points.end())
      {
      return false;
      }
    point = (*this->PointIterator);

    this->PointIterator++;
    this->CurrentNumberOfTraversedPoints++;
    return true;
    }
  else
    {
    //reversed iterator
    if ( this->ReversePointIterator == this->Points.rend() ||
      this->CurrentNumberOfTraversedPoints >= this->NumberOfPointsToTraverse)
      {
      return false;
      }
    // if inversing range, we skip the range specified,
    // but include the two end points
    if(this->InvertTraversalRange &&
      this->CurrentNumberOfTraversedPoints >
       (numArcPoints - this->TraversalStartIndex) &&
      this->NumberOfSpecifiedTraversalPoints > 2)
      {
      std::advance(this->ReversePointIterator,
        this->NumberOfSpecifiedTraversalPoints - 2);
      }
    if (this->ReversePointIterator == this->Points.rend())
      {
      return false;
      }

    point = (*this->ReversePointIterator);

    this->ReversePointIterator++;
    this->CurrentNumberOfTraversedPoints++;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
vtkIdType vtkCMBArc::GetNumberOfInternalPoints() const
{
  return this->Points.size();
}

//----------------------------------------------------------------------------
bool vtkCMBArc::GetArcInternalPoint(vtkIdType pid, vtkCMBArc::Point& point)
{
  if(pid >=0 && pid < static_cast<vtkIdType>(this->Points.size()))
    {
    InternalPointList::const_iterator startIndexIt = this->Points.begin();
    std::advance(startIndexIt, pid);
    point = *startIndexIt;

    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
vtkIdType vtkCMBArc::GetNumberOfArcPoints() const
{
  return this->GetNumberOfInternalPoints()+this->GetNumberOfEndNodes();
}

//----------------------------------------------------------------------------
bool vtkCMBArc::GetEndNodeDirection(const int& index, double direction[3]) const
{
  if (this->InvalidEndNodeIndex(index))
    {
    return false;
    }
  if (this->IsClosedArc() && this->GetNumberOfInternalPoints())
    {
    //can't find a directional vector if we only have one point
    return false;
    }

  double pos[2][3];
  if (this->GetNumberOfInternalPoints() > 0)
    {
    //the point we want to use is an internal node
    const vtkCMBArc::Point iteratorPoint = (index == 0) ?
          this->Points.front():this->Points.back();
    pos[0][0] = iteratorPoint.x;
    pos[0][1] = iteratorPoint.y;
    pos[0][2] = iteratorPoint.z;
    }
  else
    {
    //the point is the other end node
    int otherIndex = (index == 0) ? 1 : 0;
    this->EndNodes[otherIndex]->GetPosition(pos[0]);
    }

  this->EndNodes[index]->GetPosition(pos[1]);
  vtkMath::Subtract(pos[1],pos[0],direction);
  vtkMath::Normalize(direction);
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::GetEndNodeDirection(vtkCMBArcEndNode* endNode, double direction[3]) const
{
  if (!endNode)
    {
    return false;
    }
  else if (endNode == this->EndNodes[0])
    {
    return this->GetEndNodeDirection(0,direction);
    }
  else if (endNode == this->EndNodes[1])
    {
    return this->GetEndNodeDirection(1,direction);
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
int vtkCMBArc::GetNumberOfConnectedArcs( )
{
  return static_cast<int>(this->ArcManager->GetConnectedArcs(this).size());
}

//----------------------------------------------------------------------------
int vtkCMBArc::GetNumberOfConnectedArcs(const int& index) const
{
  if (this->InvalidEndNodeIndex(index))
    {
    return -1;
    }
  int size = this->ArcManager->GetNumberOfArcs(this->EndNodes[index]);
  size = ( size > 0 ) ? ( size - 1 ) : 0;

  return size;
}

//----------------------------------------------------------------------------
int vtkCMBArc::GetNumberOfConnectedArcs(vtkCMBArcEndNode *endNode ) const
{
  //make sure this is a valid end node of the arc
  if (!endNode)
    {
    return -1;
    }

  if (endNode != this->EndNodes[0] && endNode != this->EndNodes[1] )
    {
    //this end node isn't part of the set
    return -2;
    }

  int size = this->ArcManager->GetNumberOfArcs(endNode);
  size = ( size > 0 ) ? ( size - 1 ) : 0;

  return size;
}

//----------------------------------------------------------------------------
void vtkCMBArc::MarkedForDeletion()
{
  this->ArcManager->MarkedForDeletion(this);
  this->UndoEndNodes.clear();

  for ( int i=0; i < 2; ++i)
    {
    if ( this->EndNodes[i] )
      {
      //store the end nodes position
      const double *pos = this->EndNodes[i]->GetPosition();

      vtkCMBArc::Point ip(pos[0],pos[1],pos[2], this->EndNodes[i]->GetPointId());
      this->UndoEndNodes.push_back(ip);

      //remove this end node reference
      this->ArcManager->RemoveEndNode(this->EndNodes[i],this);
      this->EndNodes[i] = NULL;
      }
    }
}

//----------------------------------------------------------------------------
void vtkCMBArc::UnMarkedForDeletion()
{
  this->ArcManager->UnMarkedForDeletion(this);
  int index = 0;
  InternalPointList::iterator it;

  for ( it = this->UndoEndNodes.begin(); it != this->UndoEndNodes.end();
                                                                it++, index++)
    {
    this->SetEndNode(index, *it);
    }
  this->UndoEndNodes.clear();
}

//----------------------------------------------------------------------------
bool vtkCMBArc::ReplaceEndNode(vtkCMBArcEndNode* oldEndNode, vtkCMBArcEndNode* newEndNode)
{
  if ( oldEndNode == NULL || newEndNode == NULL)
    {
    return false;
    }

  //these are two if statements so we update both end nodes when they
  //both point to the same end node.
  if ( this->EndNodes[0] == oldEndNode )
    {
    this->EndNodes[0] = newEndNode;
    this->ArcManager->RemoveEndNode(oldEndNode,this);
    }
  if ( this->EndNodes[1] == oldEndNode )
    {
    this->EndNodes[1] = newEndNode;
    this->ArcManager->RemoveEndNode(oldEndNode,this);
    }

  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::ReplacePoints(vtkIdType startIndex, vtkIdType endIndex,
                   std::list<vtkCMBArc::Point>& newPoints, bool includeEnds)
{
  if(endIndex < 0 || startIndex < 0 || startIndex == endIndex)
    {
    return false;
    }
  vtkIdType numPoints = endIndex>startIndex ? (endIndex-startIndex+1) :
    (startIndex-endIndex+1);
  // if the specified range is out of the bounds of all arc points
  if(numPoints > this->GetNumberOfArcPoints())
    {
    return false;
    }

  // NOTE: we are not doing anything to the end nodes, and whoever calling this
  // should update the arc end nodes properly, otherwise, the resulting
  // arc may be invalid.

  InternalPointList subPoints;
  bool forwardDir = startIndex < endIndex;
  while(newPoints.size() != 0)
    {
    this->InternalInsertNextPoint(subPoints,
      forwardDir ? newPoints.front() : newPoints.back());
    if(forwardDir)
      {
      newPoints.pop_front();
      }
    else
      {
      newPoints.pop_back();
      }
    }
  // we don't allow removing of all points including ends. If it is not
  // including ends, that means making a straight line between ends
  if(subPoints.size()==0 && includeEnds)
    {
    return false;
    }

  // erase the points with the range from the original Points list.
  InternalPointList::iterator startIndexIt = this->Points.begin();
  InternalPointList::iterator endIndexIt;
  // this is where we insert the new subPoints.
  InternalPointList::iterator spliceIt;
  // if this is a closed arc, and startIndex is greater than endIndex,
  // we are replacing the the other half of the loop.
  if(this->IsClosedArc() && startIndex > endIndex)
    {
    // list::erase(first, last)
    // remove the range includes all the elements between first and last,
    // including the element pointed by first but not the one pointed by last.
    int startOffset = includeEnds ? startIndex - 1 : startIndex;
    if(startOffset > 0)
      {
      std::advance(startIndexIt, startOffset);
      this->Points.erase(startIndexIt, this->Points.end());
      }

    // if the endIndex is not the end node, we also need to remove the points
    // from Points.begin() to endIndex
    if(endIndex != 0)
      {
      int endOffset = includeEnds ? endIndex : endIndex - 1;
      if(endOffset > 0 && endOffset > startOffset)
        {
        endIndexIt = this->Points.begin();
        endOffset = std::min(endOffset, static_cast<int>(this->GetNumberOfInternalPoints()));

        std::advance(endIndexIt, endOffset);
        this->Points.erase(this->Points.begin(), endIndexIt);
        }
      }
    spliceIt = startIndexIt == this->Points.end() ? startIndexIt : startIndexIt++;
    }
  else
    {
    int startOffset = includeEnds ? startIndex - 1 : startIndex;
    int endOffset = includeEnds ? endIndex : endIndex - 1;
    if(startIndex > endIndex)
      {
      startOffset = includeEnds ? endIndex - 1 : endIndex;
      endOffset = includeEnds ? startIndex : startIndex - 1;
      }
    //startOffset = std::min(startOffset, this->GetNumberOfInternalPoints());
    if(startOffset > 0)
      {
      std::advance(startIndexIt, startOffset);
      }
    endIndexIt = this->Points.begin();
    if(endOffset > 0 && endOffset > startOffset)
      {
      endOffset = std::min(endOffset, static_cast<int>(this->GetNumberOfInternalPoints()));
      std::advance(endIndexIt, endOffset);
      this->Points.erase(startIndexIt, endIndexIt);
      }
    spliceIt = endIndexIt == this->Points.end() ? endIndexIt : endIndexIt++;
    }

  // Now insert the subPoints in the correct order.
  if(subPoints.size() > 0)
    {
    this->Points.splice(spliceIt, subPoints);
    }
  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArc::IsWholeArcRange(vtkIdType startId, vtkIdType endId,
  vtkIdType numArcPoints, bool closedArc)
{
  // In the case of a looped arc, if startId==endId, return true;
  // In the case of opened arc, the number has to match whole arc points
  vtkIdType numRequestedArcPoints = numArcPoints;
  if(endId >= 0 && startId >= 0)
    {
    numRequestedArcPoints = (endId >= startId) ?
      (endId - startId + 1) : (startId - endId + 1);
    }

  return ((numRequestedArcPoints == numArcPoints &&
   !closedArc) || (closedArc && startId == endId));
}
