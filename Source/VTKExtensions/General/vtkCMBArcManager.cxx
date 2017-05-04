//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"

#include "vtkDebugLeaks.h"
#include "vtkDebugLeaksManager.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

#include <algorithm>
#include <iterator>

//----------------------------------------------------------------------------

vtkCMBArcManager* vtkCMBArcManager::Instance = 0;
vtkCMBArcManagerCleanup vtkCMBArcManager::Cleanup;

//-----------------------------------------------------------------------------
vtkCMBArcManagerCleanup::vtkCMBArcManagerCleanup()
{
}
//-----------------------------------------------------------------------------
vtkCMBArcManagerCleanup::~vtkCMBArcManagerCleanup()
{
  vtkCMBArcManager::SetInstance(NULL);
}

//-----------------------------------------------------------------------------
vtkCMBArcManager::vtkCMBArcManager(std::set<vtkCMBArc*> subset)
  : PointLocator(NULL)
  , LocatorBounds()
  , Arcs(vtkCMBArcMap())
  , UndoArcs(vtkCMBArcSet())
  , PointIdsToEndNodes(vtkPointIdsToEndNodeMap())
  , EndNodesToArcs(vtkCmbEndNodesToArcMap())
  , UseSnapping(false)
  , SnapRadius(0)
  , SnapRadiusSquared(0)
  , LocatorNeedsRebuilding(true)
{
  this->BuildLocator(); //need a locator before we add any end nodes or we crash

  std::set<vtkCMBArc*>::iterator it;
  vtkCMBArc* arc;
  for (it = subset.begin(); it != subset.end(); ++it)
  {
    //add each arc to this subset manager so we have info
    arc = *it;
    this->RegisterArc(arc);
    this->AddEndNode(arc->GetEndNode(0), arc);
    this->AddEndNode(arc->GetEndNode(1), arc);
  }
  this->BuildLocator();
}

//-----------------------------------------------------------------------------
vtkCMBArcManager::vtkCMBArcManager()
  : PointLocator(NULL)
  , LocatorBounds()
  , Arcs(vtkCMBArcMap())
  , UndoArcs(vtkCMBArcSet())
  , PointIdsToEndNodes(vtkPointIdsToEndNodeMap())
  , EndNodesToArcs(vtkCmbEndNodesToArcMap())
  , UseSnapping(false)
  , SnapRadius(0)
  , SnapRadiusSquared(0)
  , LocatorNeedsRebuilding(true)
{
}

//-----------------------------------------------------------------------------
vtkCMBArcManager::~vtkCMBArcManager()
{
  if (this->PointLocator)
  {
    this->PointLocator->Delete();
  }

  //If the manager being deleted isn't the static manager
  //it means it was only working on a subset of arcs and shouldn't delete
  //anything as it can only query not add, remove or modify
  if (this != this->Instance)
  {
    return;
  }

  vtkCMBArcMap::iterator ait;
  while (this->Arcs.size() > 0)
  {
    ait = this->Arcs.begin();
    vtkCMBArc* arc = ait->second;
    arc->Delete();
  }

  vtkCMBArcSet::iterator asit;
  while (this->UndoArcs.size() > 0)
  {
    asit = this->UndoArcs.begin();
    vtkCMBArc* arc = (*asit);
    arc->Delete();
  }

  //make sure we delete the arcs before the end nodes
  //that way the arc destructor can properly query if we have a reference
  //to the end node
  vtkCmbEndNodesToArcMap::iterator it;
  while (this->EndNodesToArcs.size() > 0)
  {
    it = this->EndNodesToArcs.begin();
    vtkCMBArcEndNode* endNode = it->first;
    this->EndNodesToArcs.erase(it);
    delete endNode;
  }
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent
     << "vtkCMBArcManager Single instance = " << static_cast<void*>(vtkCMBArcManager::Instance)
     << endl;
}

//-----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkCMBArcManager* vtkCMBArcManager::New()
{
  vtkCMBArcManager* ret = vtkCMBArcManager::GetInstance();
  ret->Register(NULL);
  return ret;
}

//-----------------------------------------------------------------------------
// Return the single instance of the vtkCMBArcManager
vtkCMBArcManager* vtkCMBArcManager::GetInstance()
{
  if (!vtkCMBArcManager::Instance)
  {
    // Try the factory first
    vtkCMBArcManager::Instance =
      static_cast<vtkCMBArcManager*>(vtkObjectFactory::CreateInstance("vtkCMBArcManager"));
    // if the factory did not provide one, then create it here
    if (!vtkCMBArcManager::Instance)
    {
      // if the factory failed to create the object,
      // then destroy it now, as vtkDebugLeaks::ConstructClass was called
      // with "vtkCMBArcManager", and not the real name of the class
      vtkCMBArcManager::Instance = new vtkCMBArcManager;
    }
  }
  // return the instance
  return vtkCMBArcManager::Instance;
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::SetInstance(vtkCMBArcManager* instance)
{
  if (vtkCMBArcManager::Instance == instance)
  {
    return;
  }
  // preferably this will be NULL
  if (vtkCMBArcManager::Instance)
  {
    vtkCMBArcManager::Instance->Delete();
  }
  vtkCMBArcManager::Instance = instance;
  if (!instance)
  {
    return;
  }
  // user will call ->Delete() after setting instance
  instance->Register(NULL);
}

//-----------------------------------------------------------------------------
int vtkCMBArcManager::GetNumberOfArcs() const
{
  return static_cast<int>(this->Arcs.size());
}

//-----------------------------------------------------------------------------
int vtkCMBArcManager::GetNumberOfEndNodes() const
{
  return static_cast<int>(this->EndNodesToArcs.size());
}

//-----------------------------------------------------------------------------
vtkCMBArc* vtkCMBArcManager::GetArc(const vtkIdType& id)
{
  vtkCMBArcMap::const_iterator it = this->Arcs.find(id);
  if (it == this->Arcs.end())
  {
    return NULL;
  }
  return it->second;
}

//-----------------------------------------------------------------------------
vtkCMBArc* vtkCMBArcManager::GetArcReadyForDeletion(const vtkIdType& id)
{
  //this is slower since we don't index the arcs on id
  vtkCMBArcSet::const_iterator asit;
  for (asit = this->UndoArcs.begin(); asit != this->UndoArcs.end(); ++asit)
  {
    if ((*asit)->GetId() == id)
    {
      return (*asit);
    }
  }
  return NULL;
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::SetSnapRadius(double radius)
{
  if (radius >= 0)
  {
    this->SnapRadius = radius;
    this->SnapRadiusSquared = radius * radius;
  }
}

//-----------------------------------------------------------------------------
bool vtkCMBArcManager::IsManagedEndNode(vtkCMBArcEndNode* endNode) const
{
  return (this->EndNodesToArcs.find(endNode) != this->EndNodesToArcs.end());
}

//-----------------------------------------------------------------------------
int vtkCMBArcManager::GetNumberOfArcs(vtkCMBArcEndNode* endNode) const
{
  if (!this->IsManagedEndNode(endNode))
  {
    //error we are not managing this end node
    return -1;
  }
  return static_cast<int>(this->EndNodesToArcs.find(endNode)->second.size());
}

//-----------------------------------------------------------------------------
std::set<vtkCMBArc*> vtkCMBArcManager::GetConnectedArcs(vtkCMBArcEndNode* endNode)
{
  if (!this->IsManagedEndNode(endNode))
  {
    //error we are not managing this end node
    return std::set<vtkCMBArc*>();
  }
  vtkCmbEndNodesToArcMap::const_iterator it = this->EndNodesToArcs.find(endNode);
  return it->second;
}

//-----------------------------------------------------------------------------
std::set<vtkCMBArc*> vtkCMBArcManager::GetConnectedArcs(vtkCMBArc* arc)
{
  if (!arc)
  {
    return std::set<vtkCMBArc*>();
  }

  int numEndNodes = arc->GetNumberOfEndNodes();
  if (numEndNodes == 1)
  {
    vtkCMBArcSet set = this->GetConnectedArcs(arc->GetEndNode(0));
    set.erase(arc);
    return set;
  }
  else if (numEndNodes == 2)
  {
    //use a set to remove any duplicates arcs,
    //which happens when this is half of a loop
    vtkCMBArcSet combinedSet;
    vtkCMBArcSet set1 = this->EndNodesToArcs.find(arc->GetEndNode(0))->second;
    vtkCMBArcSet set2 = this->EndNodesToArcs.find(arc->GetEndNode(1))->second;
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
      std::inserter(combinedSet, combinedSet.begin()));
    combinedSet.erase(arc);
    return combinedSet;
  }
  else
  {
    return std::set<vtkCMBArc*>();
  }
}

//-----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArcManager::GetEndNodeAt(double position[3])
{
  this->BuildLocator(); //make sure we have a locator
  vtkIdType pointId = -1;
  if (this->UseSnapping && this->PointLocator->GetNumberOfPoints() > 0)
  {
    double dist;
    pointId = this->PointLocator->FindClosestPointWithinSquaredRadius(
      this->SnapRadiusSquared, position, dist);
  }
  else
  {
    pointId = this->PointLocator->IsInsertedPoint(position);
  }

  if (pointId == -1)
  {
    return NULL;
  }

  //get the end node for this id
  return this->EndNodeFromPointId(pointId);
}

//-----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArcManager::CreateEndNode(vtkCMBArc::Point const& point)
{
  double position[3] = { point[0], point[1], point[2] };
  vtkCMBArcEndNode* preExistingEndNode = this->GetEndNodeAt(position);
  if (preExistingEndNode)
  {
    //the end node already exists in the system, so return the current one
    //rather than create a new one
    return preExistingEndNode;
  }

  //we have to create a new end node.
  return new vtkCMBArcEndNode(position, point.GetId());
}

//-----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArcManager::MergeEndNodes(
  vtkCMBArcEndNode* endNode1, vtkCMBArcEndNode* endNode2)
{
  if (endNode1 == endNode2)
  {
    //can't merge yourself
    return NULL;
  }

  if (this->IsManagedEndNode(endNode1) && this->IsManagedEndNode(endNode2))
  {
    //if both the arcs are being tracked by this manager, we need
    //to merge the arc sets
    vtkCMBArcSet combinedSet;
    std::set<vtkCMBArc*> arcs1 = this->GetConnectedArcs(endNode1);
    std::set<vtkCMBArc*> arcs2 = this->GetConnectedArcs(endNode2);

    //so now we have to union the arcs that use both end nodes for the new
    //end node arc set
    std::set_union(arcs1.begin(), arcs1.end(), arcs2.begin(), arcs2.end(),
      std::inserter(combinedSet, combinedSet.begin()));

    //update the end node to arc relationship
    this->EndNodesToArcs.find(endNode1)->second = combinedSet;

    //now we have to mark all these arcs modified so the will rerender properly
    for (std::set<vtkCMBArc*>::iterator arcIt = arcs2.begin(); arcIt != arcs2.end(); arcIt++)
    {
      //go through and fix all the arcs to point to the correct end node
      (*arcIt)->ReplaceEndNode(endNode2, endNode1);
    }
    return endNode1;
  }

  //we can't merge this end nodes.
  return NULL;
}

//-----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArcManager::MoveEndNode(
  vtkCMBArcEndNode* endNode, vtkCMBArc::Point const& point)
{
  double position[3] = { point[0], point[1], point[2] };
  //does a point already exist at the passed in position?
  vtkCMBArcEndNode* preExistingEndNode = this->GetEndNodeAt(position);
  if (preExistingEndNode && preExistingEndNode == endNode)
  {
    //we found ourselves??
    return preExistingEndNode;
  }
  else if (preExistingEndNode)
  {
    //the end node already exists in the system!
    this->MergeEndNodes(preExistingEndNode, endNode);
    return preExistingEndNode;
  }
  else if (!endNode)
  {
    return this->CreateEndNode(point);
  }
  else
  {
    endNode->SetPosition(position);
    endNode->PointId = point.GetId();
    if (this->IsManagedEndNode(endNode))
    {
      //by moving this end node we just invalidated the point locator
      this->LocatorModified();

      //tell all arcs that use this end node that they are modified since the
      //end node has been moved. This will make them rerender
      //update the end node to arc relationship
      std::set<vtkCMBArc*> arcs = this->EndNodesToArcs.find(endNode)->second;

      //now we have to mark all these arcs modified so the will rerender properly
      for (std::set<vtkCMBArc*>::iterator arcIt = arcs.begin(); arcIt != arcs.end(); arcIt++)
      {
        (*arcIt)->Modified();
      }
    }
    return endNode;
  }
}

//-----------------------------------------------------------------------------
bool vtkCMBArcManager::RemoveEndNode(vtkCMBArcEndNode* en, vtkCMBArc* arc)
{
  if (!this->IsManagedEndNode(en))
  {
    return false;
  }
  if (arc == NULL)
  {
    return false;
  }

  vtkCmbEndNodesToArcMap::iterator it = this->EndNodesToArcs.find(en);
  it->second.erase(arc);
  if (it->second.size() == 0)
  {
    //if the end node has no arcs we need to delete it.
    this->EndNodesToArcs.erase(en);
    delete en;
    this->LocatorModified();
  }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkCMBArcManager::AddEndNode(vtkCMBArcEndNode* en, vtkCMBArc* arc)
{
  if (en == NULL || arc == NULL)
  {
    return false;
  }
  if (!this->IsManagedEndNode(en))
  {
    this->EndNodesToArcs.insert(std::pair<vtkCMBArcEndNode*, vtkCMBArcSet>(en, vtkCMBArcSet()));
    this->AddEndNodeToLocator(en);
  }
  this->EndNodesToArcs.find(en)->second.insert(arc);

  //Rebuild the locator every time a point is found
  //outside the bounding box of the locator
  //we rebuild the locator after the endnode is added
  //to EndNodeToArcs so the rebuilt is valid.
  this->BuildLocator();

  return true;
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::RegisterArc(vtkCMBArc* arc)
{
  if (arc)
  {
    this->Arcs.insert(std::pair<vtkIdType, vtkCMBArc*>(arc->GetId(), arc));
  }
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::UnRegisterArc(vtkCMBArc* arc)
{
  if (arc)
  {
    this->Arcs.erase(arc->GetId());

    //see if the arc is in the undo set
    if (this->UndoArcs.find(arc) != this->UndoArcs.end())
    {
      //this is needed because arcs that are in the undo set will call
      //unregister when they are fully removed from the system. But the
      //manager won't know it.
      this->UndoArcs.erase(arc);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::MarkedForDeletion(vtkCMBArc* arc)
{
  if (arc)
  {
    this->UnRegisterArc(arc);
    this->UndoArcs.insert(arc);
  }
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::UnMarkedForDeletion(vtkCMBArc* arc)
{
  if (arc)
  {
    this->UndoArcs.erase(arc);
    this->RegisterArc(arc);
  }
}

//-----------------------------------------------------------------------------
vtkCMBArcEndNode* vtkCMBArcManager::EndNodeFromPointId(const vtkIdType& pointId)
{
  if (pointId < 0 || pointId > static_cast<vtkIdType>(this->EndNodesToArcs.size()))
  {
    return NULL;
  }
  return this->PointIdsToEndNodes.find(pointId)->second;
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::AddEndNodeToLocator(vtkCMBArcEndNode* en)
{

  vtkIdType index = this->PointLocator->InsertNextPoint(en->GetPosition());
  this->PointIdsToEndNodes.insert(std::pair<vtkIdType, vtkCMBArcEndNode*>(index, en));

  //don't call build locator here, as we haven't created the bi-directional
  //map. Allow what ever is calling this method to call build locator, as
  //it will create the bi-directional map
  const double* pos = en->GetPosition();
  if (!this->LocatorBounds.ContainsPoint(pos[0], pos[1], pos[2]))
  {
    this->LocatorModified();
  }
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::BuildLocator()
{
  if (!this->LocatorNeedsRebuilding)
  {
    return;
  }

  //clear out all the old info
  this->PointIdsToEndNodes.clear();
  if (this->PointLocator)
  {
    this->PointLocator->Initialize();
  }
  else
  {
    this->PointLocator = vtkIncrementalOctreePointLocator::New();
  }

  double locatorBounds[6] = { -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 };
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  if (this->EndNodesToArcs.size() > 0)
  {
    points->SetNumberOfPoints(this->EndNodesToArcs.size());
    vtkIdType index = 0;
    vtkCmbEndNodesToArcMap::const_iterator it;
    for (it = this->EndNodesToArcs.begin(); it != this->EndNodesToArcs.end(); ++it, ++index)
    {
      //add each point and update the lookup from idtype to arc
      points->SetPoint(index, it->first->GetPosition());
      this->PointIdsToEndNodes.insert(std::pair<vtkIdType, vtkCMBArcEndNode*>(index, it->first));
    }
    points->GetBounds(locatorBounds);
  }

  //setup the new locator
  this->PointLocator->SetTolerance(0.0);
  this->PointLocator->SetBuildCubicOctree(1);
  this->PointLocator->SetMaxPointsPerLeaf(128);

  //we need to set the data set so that the point locator can properly
  //rebuild itself when needed for searching.
  vtkNew<vtkPolyData> data;
  data->SetPoints(points.GetPointer());
  this->PointLocator->SetDataSet(data.GetPointer());

  //we know have to to build the locator for searching and insertion
  //we need to call BuildLocator if the point set has values, since
  //it has the correct bounds. Otherwise we have to call InitPointInsertion
  //since the bounds that buildlocator has will be wrong.
  if (points->GetNumberOfPoints() > 0)
  {
    this->PointLocator->BuildLocator();
  }
  else
  {
    //use the custom default bounds that are not degenerate
    this->PointLocator->InitPointInsertion(points.GetPointer(), locatorBounds);
  }

  this->LocatorBounds.SetBounds(locatorBounds);
  this->LocatorNeedsRebuilding = false;
}

//-----------------------------------------------------------------------------
void vtkCMBArcManager::LocatorModified()
{
  this->LocatorNeedsRebuilding = true;
}
