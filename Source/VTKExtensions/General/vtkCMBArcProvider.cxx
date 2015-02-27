/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBPolyDataProvider.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBArcProvider.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

vtkStandardNewMacro(vtkCMBArcProvider);
//----------------------------------------------------------------------------
vtkCMBArcProvider::vtkCMBArcProvider()
{
  this->ArcId = -1;
  this->Arc = NULL;
  this->ArcManager = vtkCMBArcManager::GetInstance();

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkCMBArcProvider::~vtkCMBArcProvider()
{
  this->ArcId = -1;
  this->StartPointId = -1;
  this->EndPointId = -1;
  this->Arc = NULL;
  this->ArcManager = NULL;
}

//----------------------------------------------------------------------------
void vtkCMBArcProvider::SetArcId(vtkIdType arcId)
{
  if (this->ArcId != arcId)
    {
    //we need to keep a reference to the arc so that
    //we can properly report the correct MTime if any of the end nodes change
    this->ArcId = arcId;
    this->StartPointId = -1;
    this->EndPointId = -1;
    this->Arc = this->ArcManager->GetArc(this->ArcId);
    this->Modified();
    }
}
//----------------------------------------------------------------------------
void vtkCMBArcProvider::SetStartPointId(vtkIdType ptId)
{
  if (this->StartPointId != ptId)
    {
    this->StartPointId = ptId;
    this->Modified();
    }
}
//----------------------------------------------------------------------------
void vtkCMBArcProvider::SetEndPointId(vtkIdType ptId)
{
  if (this->EndPointId != ptId)
    {
    this->EndPointId = ptId;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
unsigned long vtkCMBArcProvider::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  if (this->Arc)
    {
    unsigned long time = this->Arc->GetMTime();
    mTime = ( time > mTime ? time : mTime );

    }
  return mTime;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkCMBArcProvider::CreatePolyDataRepresentation()
{
  vtkPolyData* representation = vtkPolyData::New();
  int numEndNodes = this->Arc->GetNumberOfEndNodes();
  if ( numEndNodes == 0 )
    {
    //we have no end nodes so we are empty
    return representation;
    }

  //see if both end nodes are the same
  bool closedArc = this->Arc->IsClosedArc();

  //construct a single polyline from the data
  vtkIdType pointSize = this->Arc->GetNumberOfArcPoints();

  vtkIdType cellSize = pointSize;
  cellSize += static_cast<int>(closedArc);

  vtkPoints *points = vtkPoints::New();

  vtkCellArray *vertCell = vtkCellArray::New();

  vtkIdTypeArray *cellArray = vtkIdTypeArray::New();
  vtkCellArray *cells = vtkCellArray::New();

  points->SetNumberOfPoints(pointSize);
  cellArray->SetNumberOfValues(cellSize+1); //make room for the cell size value
  vertCell->InsertNextCell(numEndNodes);

  //setup the first end node as the first point, and the first point id in the cell array
  vtkIdType index = 0;
  cellArray->SetValue(index,cellSize); //setup the size of the cell
  points->SetPoint(index,this->Arc->GetEndNode(0)->GetPosition());
  vertCell->InsertCellPoint(index);
  cellArray->SetValue(index+1,index);
  ++index;

  //add all the points in the internal point list
  this->Arc->InitTraversal();
  double point[3];
  while(this->Arc->GetNextPoint(point))
    {
    points->SetPoint(index,point[0],point[1],point[2]);
    cellArray->SetValue(index+1,index);
    ++index;
    }

  //finish off with the last end node
  if (closedArc)
    {
    cellArray->SetValue(index+1,0);
    }
  else if ( numEndNodes == 2 )
    {
    points->SetPoint(index,this->Arc->GetEndNode(1)->GetPosition());
    vertCell->InsertCellPoint(index);
    cellArray->SetValue(index+1,index);
    }

  cells->SetCells(1,cellArray);
  cellArray->Delete();

  representation->SetPoints(points);
  points->FastDelete();

  representation->SetVerts(vertCell);
  vertCell->FastDelete();

  representation->SetLines(cells);
  cells->FastDelete();

  representation->Squeeze();
  return representation;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkCMBArcProvider::CreateSubArcPolyDataRepresentation()
{
  vtkIdType numPoints =
    (this->EndPointId >= this->StartPointId) ?
    (this->EndPointId - this->StartPointId + 1) :
    (this->StartPointId - this->EndPointId + 1);

  vtkPolyData* representation = vtkPolyData::New();
  int numEndNodes = this->Arc->GetNumberOfEndNodes();
  if ( numEndNodes == 0 || numPoints <= 1 )
    {
    // we have no end nodes, or requested points are not enough for a sub-arc,
    // so we are empty
    return representation;
    }

  // construct a single polyline from the data,
  // and two verts for the two end points of the sub-arc

  //see if both end nodes are the same
  bool closedArc = this->Arc->IsClosedArc();
  bool forwardDir = this->EndPointId > this->StartPointId;
  bool invertClosedArc = false;
  // If the arc is closed loop, and the end-point is the end node, we will select
  // the sub-arc (startIndex, ..., n-1, n, 0), NOT (startIdex, startIndex-1,..., 1, 0).
  // this way all points of the loop arc can be selectable.
  if(closedArc && this->EndPointId < this->StartPointId)
    {
    forwardDir = true;
    invertClosedArc = true;
    numPoints = this->Arc->GetNumberOfArcPoints() - numPoints + 2;
    }

  // Since we are dealing only with sub-arcs here (previously should have
  // eliminated the full arc case), we need to figure out whether one of
  // the end nodes of the arc is picked, if yes, we have to adjust the numPoints
  // and startPointId to traverse the internal points of the arc.
  vtkIdType pointSize = numPoints;

  vtkPoints *points = vtkPoints::New();
  vtkCellArray *vertCell = vtkCellArray::New();
  vtkIdTypeArray *cellArray = vtkIdTypeArray::New();
  vtkCellArray *cells = vtkCellArray::New();

  //the sub-arc can only be an open line
  vtkIdType cellSize = pointSize;
  points->SetNumberOfPoints(pointSize);
  cellArray->SetNumberOfValues(cellSize+1);
  cellArray->SetValue(0,cellSize); //setup the size of the cell

  //setup the first end node as the first point, and the first point id in the cell array
  vtkIdType index = 0;
  int startIndex = this->StartPointId;
  // if start point is an end node
  if(this->StartPointId == 0 ||
    (this->StartPointId == this->Arc->GetNumberOfArcPoints() - 1 && !closedArc))
    {
    int nodeIndex = this->StartPointId == 0 ? 0 : 1;
    points->SetPoint(index,this->Arc->GetEndNode(nodeIndex)->GetPosition());
    vertCell->InsertNextCell(1,&index);
    cellArray->SetValue(index+1,index);
     // then set the start point for arc traversal
    startIndex = this->StartPointId == 0 ? 0 :
      this->Arc->GetNumberOfInternalPoints() - 1;
    --numPoints;
    ++index;
    }
  else
    {
    --startIndex; // for arc internal points
    }
  // if end point is an end node, or requesting invertRange on a closed arc,
  // requested points from internal arc points will be 1 less.
  bool endPtisEndNode = (this->EndPointId == 0 ||
    (this->EndPointId == this->Arc->GetNumberOfArcPoints() - 1 && !closedArc));
  if(endPtisEndNode)
    {
    --numPoints;
    }
  else if(invertClosedArc)
    {
    numPoints = this->Arc->GetNumberOfInternalPoints() - this->StartPointId + 1;
    }

  //add all the points in the internal point list
  this->Arc->InitTraversal(startIndex, numPoints, forwardDir);
  double point[3];
  // if this is the very first point, make it a vertex
  if(index == 0)
    {
    if(this->Arc->GetNextPoint(point))
      {
      points->SetPoint(index,point[0],point[1],point[2]);
      vertCell->InsertNextCell(1, &index);
      cellArray->SetValue(index+1,index);
      ++index;
      }
    }

  // This is just traversing the internal points of the arc
  while(this->Arc->GetNextPoint(point))
    {
    points->SetPoint(index,point[0],point[1],point[2]);
    cellArray->SetValue(index+1,index);
    ++index;
    }
  // if end point is an end node
  if(endPtisEndNode)
    {
    int nodeIndex = this->EndPointId == 0 ? 0 : 1;
    points->SetPoint(pointSize - 1,this->Arc->GetEndNode(nodeIndex)->GetPosition());
    cellArray->SetValue(pointSize,pointSize - 1);
    }
  // if we are doing an invert on a closed arc, and the end point is not an end node
  // which means we reached the end node in the middle of the traversal, we need to get that
  // node, then continue.
  else if(invertClosedArc && index < pointSize)
    {
    points->SetPoint(index,this->Arc->GetEndNode(0)->GetPosition());
    cellArray->SetValue(index+1,index);
    index++;
    //add all the points in the internal point list
    this->Arc->InitTraversal(0, pointSize - index, forwardDir);
    // traversing from start
    while(this->Arc->GetNextPoint(point))
      {
      points->SetPoint(index,point[0],point[1],point[2]);
      cellArray->SetValue(index+1,index);
      ++index;
      }
    }
  // make the last point a vertex.
  vtkIdType vtxId = pointSize - 1;
  vertCell->InsertNextCell(1, &vtxId);

  cells->SetCells(1,cellArray);
  cellArray->Delete();

  representation->SetPoints(points);
  points->FastDelete();

  representation->SetVerts(vertCell);
  vertCell->FastDelete();

  representation->SetLines(cells);
  cells->FastDelete();

  representation->Squeeze();
  return representation;
}

//----------------------------------------------------------------------------
int vtkCMBArcProvider::RequestData(vtkInformation *,
      vtkInformationVector **, vtkInformationVector *outputVector)
{

  if (!this->Arc)
    {
    vtkErrorMacro("Unable to create an arc representation as given invalid arc id");
    return 1;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //see if both end nodes are the same
  vtkPolyData* arcRep;
  // by default we always use the whole arc, unless the sub arc is specified
  // if whole arc is requested, use original method
  // otherwise, create polydata for sub-arc.
  if(vtkCMBArc::IsWholeArcRange(this->StartPointId, this->EndPointId,
    this->Arc->GetNumberOfArcPoints(), this->Arc->IsClosedArc()))
    {
    arcRep = this->CreatePolyDataRepresentation();
    }
  else
    {
    arcRep = this->CreateSubArcPolyDataRepresentation();
    }
  output->Initialize();
  output->ShallowCopy(arcRep);
  arcRep->FastDelete();

  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBArcProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ArcId: " << this->ArcId << endl;
  os << indent << "StartPointId: " << this->StartPointId << endl;
  os << indent << "EndPointId: " << this->EndPointId << endl;
}
