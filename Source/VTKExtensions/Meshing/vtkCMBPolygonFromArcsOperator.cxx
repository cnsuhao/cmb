/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBPolygonFromArcsOperator.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBPolygonFromArcsOperator.h"

#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkMergePoints.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkSmartPointer.h"


#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcProvider.h"

#include "cmbFaceMeshHelper.h"
#include "cmbFaceMesherInterface.h"

#include <algorithm>
#include <stack>
#include <set>
#include <vector>

namespace
{
class TarjanVertex
  {
public:
  TarjanVertex(vtkCMBArc* arc);
  virtual ~TarjanVertex();

  int Index;
  int LoopUses;
  double Angle; //Used for sorting connected vertices
  int CurrentEndNode; //Used for figuring out which end node to search by on the arc
  vtkCMBArc* Arc;
  };
typedef std::vector<TarjanVertex*> VertexVector;

class InternalTarjan
  {
public:
  InternalTarjan(std::set<vtkCMBArc*> input);
  ~InternalTarjan();
  void find();

  //all the loops we have
  std::vector<VertexVector> Loops;

protected:

  typedef std::map<vtkIdType,TarjanVertex*> IdToVertexMap;
  typedef std::stack<TarjanVertex*> VertexStack;

  //returns true when it finds a loop
  bool findconnections(TarjanVertex* v);
  void getVertices(TarjanVertex *v,VertexVector &otherVerts) const;
  void addLoop(VertexVector &loop);

  vtkIdType Index;
  VertexStack Stack;
  IdToVertexMap Vertices;

  vtkCMBArcManager* SubSetManager;
  };

//-----------------------------------------------------------------------------
bool vertices_comp(const TarjanVertex* first, const TarjanVertex* second)
  {
    return first->Angle < second->Angle;
  }


//-----------------------------------------------------------------------------
TarjanVertex::TarjanVertex(vtkCMBArc *arc)
  :Index(-1),LoopUses(0),Angle(0),CurrentEndNode(-1)
{
this->Arc = arc;
}

//-----------------------------------------------------------------------------
TarjanVertex::~TarjanVertex()
{
  this->Arc = NULL;
}

//-----------------------------------------------------------------------------
InternalTarjan::InternalTarjan(std::set<vtkCMBArc *> input)
  :Index(0),
  Stack(),
  Vertices()
  {
  this->SubSetManager = new vtkCMBArcManager(input);

  //make a vector of vertices from the arcs in the subset manager
  std::set<vtkCMBArc*>::iterator it;
  for (it=input.begin();it!=input.end();++it)
    {
    TarjanVertex *tv = new TarjanVertex((*it));
    this->Vertices.insert( std::pair<vtkIdType,TarjanVertex*>((*it)->GetId(),tv));
    }
  }

//-----------------------------------------------------------------------------
InternalTarjan::~InternalTarjan()
 {
  IdToVertexMap::iterator it;
  while(this->Vertices.size() > 0)
    {
    it = this->Vertices.begin();
    delete it->second;
    this->Vertices.erase(it);
    }

  if(this->SubSetManager)
    {
    this->SubSetManager->SetReferenceCount(0);
    delete this->SubSetManager;
    }
 }

//-----------------------------------------------------------------------------
void InternalTarjan::find()
{
  this->Index = 0;
  this->Stack = VertexStack();
  IdToVertexMap::iterator it;
  for (it=this->Vertices.begin();it!=this->Vertices.end();++it)
    {
    //only can start from arcs that are loops or arcs that are connected on both ends
    vtkCMBArc *arc = it->second->Arc;
    bool isAlreadyLoop = arc->IsClosedArc();
    bool connectedOnBothEnds = arc->GetNumberOfConnectedArcs(0) > 0 &&
        arc->GetNumberOfConnectedArcs(1) > 0;
    bool validStart = (it->second->LoopUses == 0) &&
        (isAlreadyLoop || connectedOnBothEnds);
    if (validStart)
      {
      it->second->CurrentEndNode = 1;
      bool foundLoop = this->findconnections(it->second);
      if (foundLoop || isAlreadyLoop)
        {
        //we have a strongly connected component
        VertexVector component;
        TarjanVertex* nextVert;
        do{
          nextVert = this->Stack.top();
          nextVert->Index = -1;
          ++nextVert->LoopUses;
          nextVert->Angle = 0;

          this->Stack.pop();
          component.push_back(nextVert);
          }while(this->Stack.size() > 0 );

        this->addLoop(component);
        }
      }
    }
}

//-----------------------------------------------------------------------------
bool InternalTarjan::findconnections(TarjanVertex* v)
{

  //set index to the smallest used index ( which is the depth )
  v->Index = this->Index;
  ++this->Index;

  //find all the arcs that are connected to this vertex
  //from those arcs find all the connected end nodes, convert
  //those end nodes to vertices and see which ones we haven't visited
  VertexVector otherVerts;
  this->getVertices(v,otherVerts);

  //we have to push this ontop of the stack after getVertices or else
  //getVertices won't work properly
  this->Stack.push(v);

  VertexVector::iterator overtIt;
  for(overtIt=otherVerts.begin();overtIt!=otherVerts.end();++overtIt)
    {
    TarjanVertex *nextVert = (*overtIt);
    if (nextVert->Index == -1)
      {
      //we haven't touch this Index yet, walk it
      bool loopFound = this->findconnections(nextVert);
      if ( loopFound )
        {
        return true;
        }
      }
    else
      {
      //found loop
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void InternalTarjan::getVertices(TarjanVertex *v,
                                 VertexVector &otherVerts) const
{
  //We are using a modified Tarjan strongly connected algorithm
  //rather than finding all the vertices. We want to find the vertices
  //that haven't been used in a loop already.
  //the key here is that we are going to sort by angle of the edge,
  //rather than randomly, this way we always walk the correct inner loop first

  vtkCMBArcEndNode *currentEndNode = v->Arc->GetEndNode(v->CurrentEndNode);
  std::set<vtkCMBArc*> edges =
      this->SubSetManager->GetConnectedArcs(currentEndNode);
  std::set<vtkCMBArc*>::iterator edgeIt;

  //remove the current arc from the found arcs
  edges.erase(v->Arc);

  //now find all arcs that are connected to the current end node
  for(edgeIt=edges.begin();edgeIt!=edges.end();++edgeIt)
    {
    //find the current
    vtkCMBArc* arc = (*edgeIt);
    int nextEndNodeIndex = 0;
    if(arc->GetEndNode(0) == currentEndNode)
      {
      nextEndNodeIndex = 1;
      }

    TarjanVertex* w = this->Vertices.find(arc->GetId())->second;
    w->CurrentEndNode = nextEndNodeIndex;
    //make sure we get the right order on the dot product
    //stay consitent with our right hand rule

    double v1[3],v2[3];
    v->Arc->GetEndNodeDirection(currentEndNode,v1);
    arc->GetEndNodeDirection(currentEndNode,v2);

    //factor in loop uses
    //todo: handle the use case of the mesh not being z aligned
    if (v->LoopUses % 2 == 1)
      {
      v1[0] *= -1;v1[1] *= -1;v1[2] *= -1;
      //invert the atan2 call to be left handed since we have a loop use already
      w->Angle = atan2(v2[0]*v1[1]-v1[0]*v2[1],v2[0]*v1[0]+v2[1]*v1[1]);
      }
    else
      {
      w->Angle = atan2(v1[0]*v2[1]-v2[0]*v1[1],v1[0]*v2[0]+v1[1]*v2[1]);
      }

    if (w->Angle < 0)
      {
      //correct the angle if it is negative
      w->Angle = 2.0 * vtkMath::Pi() + w->Angle;
      }
    otherVerts.push_back(w);

    }
  //we have < set to compare on angle so this will sort by
  //largest to smallest angle
  std::sort(otherVerts.begin(),otherVerts.end(),vertices_comp);
}

//-----------------------------------------------------------------------------
void InternalTarjan::addLoop(VertexVector &loop)
{
    this->Loops.push_back(loop);
}


//-----------------------------------------------------------------------------
//this is used for sorting loops to determine the outer loop
struct polygonInfo
  {
  vtkBoundingBox box;
  vtkIdTypeArray* Ids;
  };

//-----------------------------------------------------------------------------
//return true if box i contains box j
bool boundingBoxCompare(const polygonInfo& i, const polygonInfo& j)
  {
  return i.box.Contains(j.box) != 0;
  }

}

//-----------------------------------------------------------------------------
class vtkCMBPolygonFromArcsOperator::InternalLoops
{
public:
  InternalLoops()
    {
    this->OuterLoop = NULL;
    }
  ~InternalLoops()
    {
    if (this->OuterLoop)
      {
      this->OuterLoop->Delete();
      }

    for(size_t i=0;i<this->InnerLoops.size();++i)
      {
      this->InnerLoops[i]->Delete();
      this->InnerLoops[i] = NULL;
      }
    }

  vtkIdTypeArray* OuterLoop;
  std::vector<vtkIdTypeArray*> InnerLoops;
};


vtkStandardNewMacro(vtkCMBPolygonFromArcsOperator);
//-----------------------------------------------------------------------------
vtkCMBPolygonFromArcsOperator::vtkCMBPolygonFromArcsOperator() :
  ArcsToUse(ArcSet()),
  ArcIdsToUse(ArcIdSet())
{
  this->Loops = new vtkCMBPolygonFromArcsOperator::InternalLoops();
}

//-----------------------------------------------------------------------------
vtkCMBPolygonFromArcsOperator::~vtkCMBPolygonFromArcsOperator()
{
  if (this->Loops)
    {
    delete this->Loops;
    }
}

//----------------------------------------------------------------------------
void vtkCMBPolygonFromArcsOperator::AddArcId(vtkIdType arcId)
{
  this->ArcIdsToUse.insert(arcId);
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCMBPolygonFromArcsOperator::GetOuterLoop()
{
  if (this->Loops)
    {
    return this->Loops->OuterLoop;
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkIdType vtkCMBPolygonFromArcsOperator::GetNumberOfInnerLoops()
{
  if (this->Loops)
    {
    return this->Loops->InnerLoops.size();
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkIdTypeArray* vtkCMBPolygonFromArcsOperator::GetInnerLoop(
  const vtkIdType& index)
{
  if (this->Loops)
    {
    size_t size = this->Loops->InnerLoops.size();
    if (index < 0 || static_cast<size_t>(index) >= size)
      {
      return NULL;
      }
    return this->Loops->InnerLoops[index];
    }
  return NULL;
}

//----------------------------------------------------------------------------
bool vtkCMBPolygonFromArcsOperator::Operate()
{

  //delete our internal storage
  if(!this->Loops)
    {
    this->Loops = new vtkCMBPolygonFromArcsOperator::InternalLoops();
    }

  if(this->Loops->OuterLoop)
    {
    this->Loops->OuterLoop->Delete();
    this->Loops->OuterLoop = NULL;
    }

  if(this->Loops->InnerLoops.size() > 0)
    {
    for(size_t i=0;i<this->Loops->InnerLoops.size();++i)
      {
      this->Loops->InnerLoops[i]->Delete();
      this->Loops->InnerLoops[i] = NULL;
      }
    this->Loops->InnerLoops.clear();
    }



  this->BuildArcsToUse();
  if (this->ArcIdsToUse.size() == 0)
    {
    return false;
    }
  this->BuildLoops();



  //if we have an outer loop we return true
  return this->Loops->OuterLoop != NULL;
}

//----------------------------------------------------------------------------
void vtkCMBPolygonFromArcsOperator::BuildArcsToUse()
{
  vtkCMBArcManager* arcManager = vtkCMBArcManager::GetInstance();
  this->ArcsToUse.clear();
  //build up the Arc list
  ArcIdSet::iterator idIt;
  for(idIt=this->ArcIdsToUse.begin();idIt!=this->ArcIdsToUse.end();idIt++)
    {
    vtkCMBArc *arc = arcManager->GetArc((*idIt));
    this->ArcsToUse.insert(arc);
    }
}

//----------------------------------------------------------------------------
void vtkCMBPolygonFromArcsOperator::BuildLoops()
{
  //we are using Tarjan algorithm to find strongly connected components
  //each component will become a loop
  InternalTarjan connections(this->ArcsToUse);
  connections.find();

  //find the outer loop and than we are going to classify everything else as an
  //inner loop
  //we will use a point locator to find the outer loop

  //Note: using the bounds isn't the safest bet on making sure the arcs
  //are inside but currently I have a deadline, so I am going with it

  //basic pod struct
  size_t size = connections.Loops.size();
  if (size == 0)
    {
    //we failed for some rare reason
    return;
    }

  std::vector<polygonInfo> polyBounds;
  polyBounds.reserve(size);

  std::vector<VertexVector>::const_iterator vvit;
  VertexVector::const_iterator it;

  for(vvit=connections.Loops.begin();vvit!=connections.Loops.end();++vvit)
    {
    polygonInfo info;
    info.Ids = vtkIdTypeArray::New();
    info.Ids->SetNumberOfValues((*vvit).size());

    vtkIdType index=0;
    for(it=(*vvit).begin();it!=(*vvit).end();++it,++index)
      {
      vtkCMBArcProvider *tempRep = vtkCMBArcProvider::New();
      tempRep->SetArcId( (*it)->Arc->GetId() );
      tempRep->Update();

      //push back the info for this loop so we can find the largest
      info.box.AddBounds(tempRep->GetOutput()->GetBounds());
      info.Ids->SetValue(index,(*it)->Arc->GetId());

      tempRep->Delete();
      }
    polyBounds.push_back(info);
    }

  //now determine the largest bounding box
  //we are going to sort the bounding box using a custom function.
  //this means the largest box is at the start.
  std::sort(polyBounds.begin(),polyBounds.end(),boundingBoxCompare);

  //store the outer loop now
  std::vector<polygonInfo>::iterator pIt = polyBounds.begin();
  this->Loops->OuterLoop = (*pIt).Ids;


  //store the inner loops
  this->Loops->InnerLoops.reserve(polyBounds.size());
  ++pIt;
  while(pIt!=polyBounds.end())
    {
    this->Loops->InnerLoops.push_back((*pIt).Ids);
    ++pIt;
    }

}


//----------------------------------------------------------------------------
void vtkCMBPolygonFromArcsOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
