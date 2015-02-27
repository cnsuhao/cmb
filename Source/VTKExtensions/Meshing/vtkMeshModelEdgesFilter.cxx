/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMeshModelEdgesFilter.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMeshModelEdgesFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkMeshModelEdgesFilter);

//----------------------------------------------------------------------------
vtkMeshModelEdgesFilter::vtkMeshModelEdgesFilter()
{
  this->TargetSegmentLengthCellArrayName = 0;
  this->UseLengthAlongEdge = true;
  // TemporaryPoints for use once we deal with residual
  //this->TemporaryPoints = vtkPoints::New();
  this->MeshPtIdsFromStart = vtkIdList::New();
  this->MeshPtIdsFromEnd = vtkIdList::New();
}

//----------------------------------------------------------------------------
vtkMeshModelEdgesFilter::~vtkMeshModelEdgesFilter()
{
  this->SetTargetSegmentLengthCellArrayName(0);
  //this->TemporaryPoints->Delete();
  this->MeshPtIdsFromStart->Delete();
  this->MeshPtIdsFromEnd->Delete();
}

//----------------------------------------------------------------------------
int vtkMeshModelEdgesFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // per chance there something other than lines (right now we only
  // expect and process lines, but don't want to crash if we have verts or polys)
  vtkCellData *outputCellData = output->GetCellData();
  outputCellData->Initialize();
  outputCellData->CopyAllOn();
  outputCellData->CopyAllocate(input->GetCellData(), input->GetNumberOfLines());


  vtkDoubleArray *targetSegmentLengthArray = 0;
  if (this->TargetSegmentLengthCellArrayName)
    {
    targetSegmentLengthArray = vtkDoubleArray::SafeDownCast(
      input->GetCellData()->GetArray(this->TargetSegmentLengthCellArrayName) );
    }

  if (!targetSegmentLengthArray)
    {
    vtkErrorMacro("Missing cell or point information needed to determine meshing resolution!");
    output->ShallowCopy( input );
    return VTK_OK;  // what should we return... if not VTK_OK, often not handled gracifully by consumer
    }

  // estimate # of points
  vtkIdType numEstimatedOutputPoints =
    this->EstimateNumberOfOutputPoints(input, targetSegmentLengthArray);

  // large enough number it should be good most of the time...
//  this->TemporaryPoints->Allocate(numEstimatedOutputPoints * 2);

  vtkPoints *outputPoints = vtkPoints::New();
  // for now, estimate number of output points based on number of input points
  outputPoints->Allocate( numEstimatedOutputPoints );
  output->SetPoints( outputPoints );
  outputPoints->FastDelete();

  // excessive, but (for now) want to guarantee we don't do multiple allocs
  this->MeshPtIdsFromStart->Allocate( numEstimatedOutputPoints );
  this->MeshPtIdsFromEnd->Allocate( numEstimatedOutputPoints );

  // only output lines for now...
  vtkIdType cellOffset = input->GetNumberOfVerts(), outputIndex = 0;
  vtkCellArray *modelEdges = input->GetLines();
  vtkCellArray *outputModelEdges = vtkCellArray::New();
  output->SetLines( outputModelEdges );
  outputModelEdges->FastDelete();
  modelEdges->InitTraversal();
  vtkIdType npts, *pts;
  vtkIdList *outputPtIds = vtkIdList::New();
  while( modelEdges->GetNextCell(npts, pts) )
    {
    double targetSegmentLength =
      targetSegmentLengthArray->GetValue( outputIndex + cellOffset );
    this->MeshPolyLine(npts, pts, input->GetPoints(), targetSegmentLength,
      targetSegmentLength, outputPoints, outputPtIds);
    outputModelEdges->InsertNextCell( outputPtIds );

    outputCellData->CopyData( input->GetCellData(), outputIndex + cellOffset,
      outputIndex);
    outputIndex++;
    }
  outputPtIds->Delete();

  // reclaim any wasted memory
  outputPoints->Squeeze();
  return VTK_OK;
}

//-----------------------------------------------------------------------------
vtkIdType vtkMeshModelEdgesFilter::EstimateNumberOfOutputPoints(vtkPolyData *input,
                                                                vtkDoubleArray *targetSegmentLengthArray)
{
  vtkIdType outputIndex = input->GetNumberOfVerts();
  vtkCellArray *modelEdges = input->GetLines();
  modelEdges->InitTraversal();
  vtkIdType npts, *pts;
  vtkIdType numberOfPointsEstimate = 0;
  while( modelEdges->GetNextCell(npts, pts) )
    {
    double targetSegmentLength =
      targetSegmentLengthArray->GetValue( outputIndex++ );

    double edgeLength = this->ComputeEdgeLength(input->GetPoints(), npts, pts);

    // for now, estimate assuming length along arc;  +2 because:
    // +1 because numPts 1 greater than number of segments, and
    // +1 qccounting for partial segmetn in calculation (round up)
    numberOfPointsEstimate += 2 + edgeLength / targetSegmentLength;
    }

  return numberOfPointsEstimate;
}

//-----------------------------------------------------------------------------
void vtkMeshModelEdgesFilter::MeshPolyLine(vtkIdType npts, vtkIdType *pts,
                                           vtkPoints *inputPoints,
                                           double startTargetSegmentLength,
                                           double endTargetSegmentLength,
                                           vtkPoints *outputPoints,
                                           vtkIdList *outputPtIds)
{
  // s1 + (s2 - s1) l / L;
  outputPtIds->Reset();
  double edgeLength = this->ComputeEdgeLength(inputPoints, npts, pts);

  bool canMesh = true;
  if (edgeLength == 0)
    {
    vtkErrorMacro("Invalid edge of lenght 0; Edge not modified");
    canMesh = false;
    }
  if (startTargetSegmentLength <= 0 || endTargetSegmentLength <= 0)
    {
    vtkErrorMacro("Start and end target segment lengths must be > 0.  Edge no modified");
    canMesh = false;
    }

  if (!canMesh)
    {
    outputPtIds->Allocate( npts );
    for (vtkIdType i = 0; i < npts; i++)
      {
      outputPtIds->InsertNextId(
        outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[i]) ) );
      }
    return;
    }

  if (this->UseLengthAlongEdge) // simple case
    {
    vtkIdType numberOfSegments = 0.5 + edgeLength / startTargetSegmentLength;
    if (numberOfSegments <= 1)
      {
      outputPtIds->Allocate( 2 );
      outputPtIds->InsertNextId(
          outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[0]) ) );
      outputPtIds->InsertNextId(
          outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[npts-1]) ) );
      return;
      }

    double segmentLength = edgeLength / numberOfSegments;
    outputPtIds->Allocate( numberOfSegments + 1 );

    // the 1st points
    outputPtIds->InsertNextId(
      outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[0]) ) );

    vtkIdType nextPtId = 1;
    double currentPt[3];
    inputPoints->GetPoint(pts[0], currentPt);
    for (vtkIdType i = 1; i < numberOfSegments; i++)
      {
      double lengthSum = 0;
      for (; nextPtId < npts; nextPtId++)
        {
        vtkIdType tempPtId;
        double nextPt[3];
        inputPoints->GetPoint( pts[nextPtId], nextPt );
        double dist = sqrt( vtkMath::Distance2BetweenPoints(currentPt, nextPt) );
        if (lengthSum + dist > segmentLength)
          {
          double t = (segmentLength - lengthSum) / dist;
          tempPtId = this->ComputeRequiredPointAlongLine(outputPoints,
            currentPt, nextPt, t);
          outputPtIds->InsertNextId( tempPtId );
          outputPoints->GetPoint(tempPtId, currentPt);
          break;
          }
        else
          {
          lengthSum += dist;
          memcpy(currentPt, nextPt, sizeof(double) * 3);
          if (lengthSum >= segmentLength) // really testing "==" here
            {
            tempPtId = outputPoints->InsertNextPoint( nextPt );
            outputPtIds->InsertNextId( tempPtId );
            nextPtId++;
            break;
            }
          }
        }
      }
    // now add the last point
    outputPtIds->InsertNextId(
      outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[npts - 1]) ) );
    return;
    }

  // we compute "next" segment length as
  //    s1 + (s2 - s1) * l / L
  // where
  // s1 is startTargetSegmentLength
  // s2 is endTargetSegmentLength
  // l is current summed length along edge
  // L total edge length

  // Instead of starting on one end and doing computation until we get to the
  // other, "simultaneosuly" do the computation of next mesh segment on both
  // ends.  (for now) We assume a straight line for purpose of computing the
  // next segment length for each end of the edge, using l as midpoint of
  // initial computation using the above equation.
  // If s1 = 1, s2 = 5, and L = 20, we "initially" get
  // 1 + (5 - 1) * 0 / 20 = 1 on the "start" side of the edge,
  // and
  // 1 + (5 - 1) * 20 / 20 = 5 on the "end" side of the edge.
  //
  // On the "start" side we instead use 0 + 1 / 2 = 0.5 for l and
  // on the "end" side we use 20 - 5 / 2 = 17.5 for l.  So instead we have
  // 1 + (5 - 1) * 0.5 / 20 = 1.1 on the "start" side of the edge,
  // and
  // 1 + (5 - 1) * 17.5 / 20 = 4.5 on the "end" side of the edge.
  //this->TemporaryPoints->Reset();
  this->MeshPtIdsFromStart->Reset();
  this->MeshPtIdsFromEnd->Reset();

  double lengthAlongLineFromStart = 0;
  double lengthAlongLineFromEnd = edgeLength;
  double currentPtFromStart[3], currentPtFromEnd[3];

  inputPoints->GetPoint(pts[0], currentPtFromStart);
  //vtkIdType currentStartPtId = this->TemporaryPoints->InsertNextPoint( currentPtFromStart );
  vtkIdType currentStartPtId = outputPoints->InsertNextPoint( currentPtFromStart );
  this->MeshPtIdsFromStart->InsertNextId( currentStartPtId );

  inputPoints->GetPoint(pts[npts-1], currentPtFromEnd);
  //vtkIdType currentEndPtId = this->TemporaryPoints->InsertNextPoint( currentPtFromEnd );
  vtkIdType currentEndPtId = outputPoints->InsertNextPoint( currentPtFromEnd );
  this->MeshPtIdsFromEnd->InsertNextId( currentEndPtId );

  vtkIdType nextFromStart = 1, nextFromEnd = npts - 2;
  while(1)
    {
    // determine target segment length for each end of edge
    double startLength = this->ComputeSegmentLength(startTargetSegmentLength,
      endTargetSegmentLength, lengthAlongLineFromStart, edgeLength, 0.5);
    double endLength = this->ComputeSegmentLength(startTargetSegmentLength,
      endTargetSegmentLength, lengthAlongLineFromEnd, edgeLength, -0.5);

    if (endLength > startLength)
      {
      //currentEndPtId = this->FindRequiredPointOnEdge(currentPtFromEnd, endLength,
      //  inputPoints, this->TemporaryPoints, pts, nextFromEnd, nextFromStart - 1, -1, currentStartPtId,
      currentEndPtId = this->FindRequiredPointOnEdge(currentPtFromEnd, endLength,
        inputPoints, outputPoints, pts, nextFromEnd, nextFromStart - 1, -1, currentStartPtId,
        nextFromEnd);
      this->MeshPtIdsFromEnd->InsertNextId( currentEndPtId );
      if (currentEndPtId == currentStartPtId)
        {
        break;
        }
      //currentStartPtId = this->FindRequiredPointOnEdge(currentPtFromStart, startLength,
      //  inputPoints, this->TemporaryPoints, pts, nextFromStart, nextFromEnd + 1, 1, currentEndPtId,
      currentStartPtId = this->FindRequiredPointOnEdge(currentPtFromStart, startLength,
        inputPoints, outputPoints, pts, nextFromStart, nextFromEnd + 1, 1, currentEndPtId,
        nextFromStart);
      this->MeshPtIdsFromStart->InsertNextId( currentStartPtId );
      }
    else
      {
      //currentStartPtId = this->FindRequiredPointOnEdge(currentPtFromStart, startLength,
      //  inputPoints, this->TemporaryPoints, pts, nextFromStart, nextFromEnd + 1, 1, currentEndPtId,
      //  nextFromStart);
      currentStartPtId = this->FindRequiredPointOnEdge(currentPtFromStart, startLength,
        inputPoints, outputPoints, pts, nextFromStart, nextFromEnd + 1, 1, currentEndPtId,
        nextFromStart);
      this->MeshPtIdsFromStart->InsertNextId( currentStartPtId );
      if (currentEndPtId == currentStartPtId)
        {
        break;
        }
      //currentEndPtId = this->FindRequiredPointOnEdge(currentPtFromEnd, endLength,
      //  inputPoints, this->TemporaryPoints, pts, nextFromEnd, nextFromStart - 1, -1, currentStartPtId,
      //  nextFromEnd);
      currentEndPtId = this->FindRequiredPointOnEdge(currentPtFromEnd, endLength,
        inputPoints, outputPoints, pts, nextFromEnd, nextFromStart - 1, -1, currentStartPtId,
        nextFromEnd);
      this->MeshPtIdsFromEnd->InsertNextId( currentEndPtId );
      }

    //this->TemporaryPoints->GetPoint(currentStartPtId, currentPtFromStart);
    //this->TemporaryPoints->GetPoint(currentEndPtId, currentPtFromEnd);
    outputPoints->GetPoint(currentStartPtId, currentPtFromStart);
    outputPoints->GetPoint(currentEndPtId, currentPtFromEnd);


    if (currentEndPtId == currentStartPtId)
      {
      break;
      }
    }

  // will need to resolve middle segments
  // maxDesiredError
  // maxNumberOfSegmentsToModify segments to involve
  // have limit on # attempts if can
  // add from temporary points to output points based on start and end lists
  // to form the fully meshed edge.

  outputPtIds->Allocate( this->MeshPtIdsFromStart->GetNumberOfIds() +
    this->MeshPtIdsFromEnd->GetNumberOfIds() - 1 );
  // 1st, handle the "start" list
  for (vtkIdType i = 0; i < this->MeshPtIdsFromStart->GetNumberOfIds(); i++)
    {
    //outputPtIds->InsertNextId( outputPoints->InsertNextPoint(
    //  this->TemporaryPoints->GetPoint( this->MeshPtIdsFromStart->GetId(i) )) );
    outputPtIds->InsertNextId( this->MeshPtIdsFromStart->GetId(i) );
    }
  // then the end list (and the two point share one id, so skip here
  for (vtkIdType i = this->MeshPtIdsFromEnd->GetNumberOfIds() - 2; i >= 0; i--)
    {
    //outputPtIds->InsertNextId( outputPoints->InsertNextPoint(
    //  this->TemporaryPoints->GetPoint( this->MeshPtIdsFromEnd->GetId(i) )) );
    outputPtIds->InsertNextId( this->MeshPtIdsFromEnd->GetId(i) );
    }
}

//-----------------------------------------------------------------------------
double vtkMeshModelEdgesFilter::ComputeSegmentLength(double s1, double s2,
                                                     double l, double L,
                                                     double factor)
{
  double initialLength = s1 + (s2 - s1) * l / L;
  return s1 + (s2 - s1) * (l + factor * initialLength) / L;
}

//-----------------------------------------------------------------------------
vtkIdType vtkMeshModelEdgesFilter::FindRequiredPointOnEdge(double currentPt[3],
                                                           double segmentLength,
                                                           vtkPoints *inputPoints,
                                                           vtkPoints *outputPoints,
                                                           vtkIdType *pts,
                                                           vtkIdType firstIndex,
                                                           vtkIdType afterLastIndex,
                                                           vtkIdType searchDirection,
                                                           vtkIdType finalPtId,
                                                           vtkIdType &nextPtId)
{
  vtkIdType ptIndex;
  double lengthSquared = segmentLength * segmentLength, dist2;
  for (ptIndex = firstIndex; ptIndex != afterLastIndex; ptIndex += searchDirection)
    {
    if ( (dist2 = vtkMath::Distance2BetweenPoints(currentPt,
      inputPoints->GetPoint(pts[ptIndex]))) >= lengthSquared )
      {
      break;
      }
    }

  nextPtId = ptIndex;

  // what if firstIndex == afterLastIndex????
  if (ptIndex == afterLastIndex)
    {
    double finalPt[3];
    outputPoints->GetPoint( finalPtId, finalPt );
    dist2 = vtkMath::Distance2BetweenPoints(currentPt, finalPt);
    // if within 5% (for now hard-coded, later a variable), call it done
    if ( dist2 <= lengthSquared * (1.05 * 1.05))
      {
      return finalPtId;
      }

    // between finalPtId and ptIndex - searchDirection
    double startPt[3];
    inputPoints->GetPoint( pts[ptIndex - searchDirection], startPt );
    double distToStartPoint = sqrt(
      vtkMath::Distance2BetweenPoints(currentPt, startPt) );
    double distToFinalPoint = sqrt( dist2 );

    return this->ComputeRequiredPointAlongLine(outputPoints, startPt, finalPt,
      (segmentLength - distToStartPoint) / (distToFinalPoint - distToStartPoint));
    }
  else if (dist2 == lengthSquared)
    {
    nextPtId += searchDirection;
    return outputPoints->InsertNextPoint( inputPoints->GetPoint(pts[ptIndex]) );
    }
  else if (ptIndex == firstIndex)
    {
    // find point between currentPoint and firstIndex point
    double *nextPt = inputPoints->GetPoint(pts[ptIndex]);
    return this->ComputeRequiredPointAlongLine(outputPoints,
      currentPt, nextPt,
      segmentLength / sqrt( vtkMath::Distance2BetweenPoints(currentPt, nextPt) ));
    }

  // between ptIndex and ptIndex - searchDirection
  double startPt[3], endPt[3];
  inputPoints->GetPoint( pts[ptIndex - searchDirection], startPt );
  inputPoints->GetPoint( pts[ptIndex], endPt );
  double distToStartPoint = sqrt(
    vtkMath::Distance2BetweenPoints(currentPt, startPt) );
  double distToEndPoint = sqrt(
    vtkMath::Distance2BetweenPoints(currentPt, endPt) );

  return this->ComputeRequiredPointAlongLine(outputPoints, startPt, endPt,
    (segmentLength - distToStartPoint) / (distToEndPoint - distToStartPoint));
}

//-----------------------------------------------------------------------------
vtkIdType vtkMeshModelEdgesFilter::ComputeRequiredPointAlongLine(vtkPoints *points,
                                                                 double *pt0,
                                                                 double *pt1,
                                                                 double t)
{
  double newPt[3];
  newPt[0] = pt0[0] + (pt1[0] - pt0[0]) * t;
  newPt[1] = pt0[1] + (pt1[1] - pt0[1]) * t;
  newPt[2] = pt0[2] + (pt1[2] - pt0[2]) * t;

  return points->InsertNextPoint( newPt );
}

//-----------------------------------------------------------------------------
double vtkMeshModelEdgesFilter::ComputeEdgeLength(vtkPoints *inputPoints,
                                                  vtkIdType npts, vtkIdType *pts)
{
  double pt[2][3], edgeLength = 0;
  inputPoints->GetPoint(pts[0], pt[0]);
  for (vtkIdType i = 1; i < npts; i++)
    {
    inputPoints->GetPoint(pts[i], pt[i%2]);
    edgeLength += sqrt( vtkMath::Distance2BetweenPoints(pt[0], pt[1]) );
    }

  return edgeLength;
}

//----------------------------------------------------------------------------
void vtkMeshModelEdgesFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Target Segment Length CellArray Name: " <<
    (this->TargetSegmentLengthCellArrayName ? this->TargetSegmentLengthCellArrayName : "Not Set") << "\n";
  os << indent << "Use Length Along Edge: " <<
    (this->UseLengthAlongEdge ? "On" : "Off") << endl;
}
