//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcPolygonProvider.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcProvider.h"

#include "smtk/extension/vtk/meshing/cmbFaceMeshHelper.h"
#include "smtk/extension/vtk/meshing/cmbFaceMesherInterface.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

#include <vector>

class vtkCMBArcPolygonProvider::InternalStorage
{
public:
  std::vector<vtkIdType> OuterLoop;
  std::vector< std::vector<vtkIdType> > InnerLoops;
};

vtkStandardNewMacro(vtkCMBArcPolygonProvider);
//----------------------------------------------------------------------------
vtkCMBArcPolygonProvider::vtkCMBArcPolygonProvider()
{
  this->ArcManager = vtkCMBArcManager::GetInstance();
  this->Loops = new vtkCMBArcPolygonProvider::InternalStorage();
  this->MinAngle = 0;
  this->EdgeLength = 0;

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkCMBArcPolygonProvider::~vtkCMBArcPolygonProvider()
{
  this->ArcManager = NULL;
  delete this->Loops;
}

//----------------------------------------------------------------------------
void vtkCMBArcPolygonProvider::SetOuterLoopArcIds(vtkIdTypeArray *ids)
{
  this->Loops->OuterLoop.clear();
  vtkIdType size = ids->GetNumberOfTuples();
  this->Loops->OuterLoop.reserve(size);
  for (vtkIdType i=0; i < size; ++i)
    {
    this->Loops->OuterLoop.push_back(ids->GetValue(i));
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBArcPolygonProvider::AddInnerLoopArcIds(vtkIdTypeArray *ids)
{
  vtkIdType size = ids->GetNumberOfTuples();
  std::vector<vtkIdType> loop;
  loop.reserve(size);
  for (vtkIdType i=0; i < size; ++i)
    {
    loop.push_back(ids->GetValue(i));
    }
  this->Loops->InnerLoops.push_back(loop);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBArcPolygonProvider::ClearInnerLoops( )
{
  this->Loops->InnerLoops.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkPolyData* vtkCMBArcPolygonProvider::CreatePolyDataRepresentation()
{
  

  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();


  CmbFaceMesherClasses::ModelFaceRep polygon;
  CmbFaceMesherClasses::ModelLoopRep outerLoop(0,false);

  double fixedZLevel=0;
  double pos[3];
  std::vector<vtkIdType>::const_iterator it;
  for(it=this->Loops->OuterLoop.begin();
      it!=this->Loops->OuterLoop.end();++it)
    {
    vtkCMBArc *arc = manager->GetArc((*it));
    //using the arc id as the edge id is horribly imporant!
    //we use it to recover which arcs made the loop
    CmbFaceMesherClasses::ModelEdgeRep edge(arc->GetId());
    arc->GetEndNode(0)->GetPosition(pos);
    edge.addModelVert(0,pos);
    fixedZLevel = pos[2]; //just grab a z level each time rather worry about branching

    if (!arc->IsClosedArc())
      {
      arc->GetEndNode(1)->GetPosition(pos);
      edge.addModelVert(1,pos);
      }

    //now generate the arcs points as a poly data
    vtkSmartPointer<vtkCMBArcProvider> filter =
      vtkSmartPointer<vtkCMBArcProvider>::New();
    filter->SetArcId(arc->GetId());
    filter->Update();
    edge.setMeshPoints(filter->GetOutput());

    outerLoop.addEdge(edge);
    }
  polygon.addLoop(outerLoop);

  int size = static_cast<int>(this->Loops->InnerLoops.size());
  for (int i=0; i < size; ++i)
    {
    CmbFaceMesherClasses::ModelLoopRep innerLoop(i+1,true);
    for (it=this->Loops->InnerLoops[i].begin();
         it!=this->Loops->InnerLoops[i].end();++it)
      {
      vtkCMBArc *arc = manager->GetArc((*it));
      //using the arc id as the edge id is horribly imporant!
      //we use it to recover which arcs made the loop
      CmbFaceMesherClasses::ModelEdgeRep edge(arc->GetId());
      arc->GetEndNode(0)->GetPosition(pos);
      edge.addModelVert(0,pos);

      if (!arc->IsClosedArc())
        {
        arc->GetEndNode(1)->GetPosition(pos);
        edge.addModelVert(1,pos);
        }

      //now generate the arcs points as a poly data
      vtkSmartPointer<vtkCMBArcProvider> filter =
        vtkSmartPointer<vtkCMBArcProvider>::New();
      filter->SetArcId(arc->GetId());
      filter->Update();
      edge.setMeshPoints(filter->GetOutput());

      innerLoop.addEdge(edge);
      }
    polygon.addLoop(innerLoop);
    }

  int numPoints = polygon.numberOfVertices();
  int numSegs = polygon.numberOfEdges();
  int numHoles = polygon.numberOfHoles();
  cmbFaceMesherInterface interface(numPoints,numSegs,numHoles);

  vtkPolyData* polygonResult = vtkPolyData::New();
  interface.setOutputMesh(polygonResult);

  polygon.fillTriangleInterface(&interface);

  interface.setUseMinAngle(this->MinAngle > 0);
  interface.setUseMaxArea(this->EdgeLength > 0);
  if (this->MinAngle > 0)
    {
    interface.setMinAngle(this->MinAngle);
    }
  if (this->EdgeLength > 0)
    {
    interface.setMaxArea( 0.5 * this->EdgeLength * this->EdgeLength);
    }

  interface.buildFaceMesh(0,fixedZLevel);

  return polygonResult;
}

//----------------------------------------------------------------------------
int vtkCMBArcPolygonProvider::RequestData(vtkInformation *,
      vtkInformationVector **, vtkInformationVector *outputVector)
{

  if (!this->Loops || this->Loops->OuterLoop.size() == 0)
    {
    vtkErrorMacro("Unable to create an arc representation as we don't have an outer loop");
    return 1;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData* arcRep = this->CreatePolyDataRepresentation();
  output->ShallowCopy(arcRep);
  arcRep->FastDelete();

  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBArcPolygonProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
