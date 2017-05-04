//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkTINStitcher.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkFeatureEdges.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolygon.h"
#include "vtkSmartPointer.h"
#include "vtkStripper.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGrid.h"

#include <map>

// for Triangle
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#define VOID void
#endif

#ifndef TRIANGLE_REAL
#ifdef SINGLE
#define TRIANGLE_REAL float
#else /* not SINGLE */
#define TRIANGLE_REAL double
#endif /* not SINGLE */
#endif
extern "C" {
#include "share_declare.h"
#include "triangle.h"
void Init_triangluateio(struct triangulateio*);
void Free_triangluateio(struct triangulateio*);
}
// END for Triangle

vtkStandardNewMacro(vtkTINStitcher);

//-----------------------------------------------------------------------------
vtkTINStitcher::vtkTINStitcher()
{
  this->TINType = -1;
  this->LoopLines = vtkCellArray::New();
  this->LoopCorners[0] = vtkIdTypeArray::New();
  this->LoopCorners[1] = vtkIdTypeArray::New();

  // internal tolerance parameters
  this->MaxDistance = 0;
  this->MaxDistance2 = 0;

  this->UseQuads = true;
  this->MinimumAngle = 25;
  this->AllowInteriorPointInsertion = false;
  this->Tolerance = 1e-6;
  this->SetNumberOfInputPorts(2);

  this->AppendedPolyData = vtkPolyData::New();
  this->PreppedStitchingInput = vtkPolyData::New();
  this->UserSpecifiedTINType = 0;
}

//-----------------------------------------------------------------------------
vtkTINStitcher::~vtkTINStitcher()
{
  this->LoopLines->Delete();
  this->LoopCorners[0]->Delete();
  this->LoopCorners[1]->Delete();
  this->AppendedPolyData->Delete();
  this->PreppedStitchingInput->Delete();
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::Set2ndInputData(vtkUnstructuredGrid* input)
{
  this->SetInputData(1, input);
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::Set2ndInputData(vtkPolyData* input)
{
  this->SetInputData(1, input);
}

//----------------------------------------------------------------------------
void vtkTINStitcher::Set2ndInputConnection(vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::GetTINType()
{
  return this->TINType;
}

//-----------------------------------------------------------------------------
bool vtkTINStitcher::AreInputsOK()
{
  if (this->GetTotalNumberOfInputConnections() != 2)
  {
    vtkErrorMacro("Two inputs are required!");
    return false;
  }

  // make sure vtkPolyData or vtkUnstructuredGrid inputs
  for (int i = 0; i < 2; i++)
  {
    if (!vtkPolyData::SafeDownCast(this->GetInputDataObject(i, 0)) &&
      !vtkUnstructuredGrid::SafeDownCast(this->GetInputDataObject(i, 0)))
    {
      vtkErrorMacro("Inputs must be vtkPolyData or vtkUnstructuredGrid!");
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/)
{
  // get the info objects

  if (!this->AreInputsOK() || this->PrepInputsForStitching() == VTK_ERROR)
  {
    return 0;
  }

  this->TINType = -1;
  if (this->UserSpecifiedTINType == 1)
  {
    if (this->LoopNPts[0] != this->LoopNPts[1])
    {
      vtkErrorMacro("Unable to stitch as Type 1 since loops don't have same # of pts!");
      return 0;
    }
    if (this->SetupToStitchAsType1() == VTK_ERROR)
    {
      return 0;
    }
  }
  else
  {
    double tolerance = this->Tolerance;
    double maxDistance = this->MaxDistance;
    double maxDistance2 = this->MaxDistance2;

    while (1)
    {
      if (this->SetupToStitchUsingAutoDetect(maxDistance, maxDistance2) == VTK_ERROR)
      {
        return 0;
      }
      if (this->TINType == 1 || this->TINType == 2)
      {
        break;
      }
      tolerance *= 10;
      if (tolerance > .05)
      {
        vtkErrorMacro("Unable to stitch as type I or type II.");
        return 0;
      }
      vtkWarningMacro("Increasing tolerance factor to " << tolerance);
      maxDistance *= 10;
      maxDistance2 *= 100;
    }
  }

  this->MapLoopLinesToAppendedData();

  int tinType = this->UserSpecifiedTINType;
  if (tinType != 1)
  {
    tinType = this->TINType;
  }

  vtkNew<vtkPolyData> tempData;
  tempData->DeepCopy(this->AppendedPolyData);
  tempData->GetCellData()->Initialize();

  if (tinType == 1 && this->UseQuads)
  {
    this->CreateQuadStitching(tempData.GetPointer());
  }
  else if (tinType == 1 && !this->AllowInteriorPointInsertion)
  {
    this->CreateTriStitching(tempData.GetPointer());
  }
  else // use Triangle to stitch
  {
    vtkNew<vtkIdTypeArray> firstSideExtraPoints;
    vtkNew<vtkIdTypeArray> sideExtraPoints[2];
    for (int i = 0; i < this->LoopCorners[0]->GetNumberOfTuples() - 1; i++)
    {
      if (i < this->LoopCorners[0]->GetNumberOfTuples() - 2)
      {
        this->ProcessSegmentWithTriangle(tempData.GetPointer(), i,
          sideExtraPoints[i % 2].GetPointer(), sideExtraPoints[!(i % 2)].GetPointer());
      }
      else
      {
        this->ProcessSegmentWithTriangle(tempData.GetPointer(), i,
          sideExtraPoints[i % 2].GetPointer(), firstSideExtraPoints.GetPointer());
      }
      if (i == 0)
      {
        // save the side0ExtraPoints for processing of the last segment
        firstSideExtraPoints->DeepCopy(sideExtraPoints[i % 2].GetPointer());
      }
      sideExtraPoints[i % 2]->Reset();
    }
  }

  // if any points were added, we need to get rid of the point data
  if (tempData->GetNumberOfPoints() != this->AppendedPolyData->GetNumberOfPoints())
  {
    tempData->GetPointData()->Initialize();
  }

  // haven't been careful to make sure normals are oriented outward; do so now
  vtkNew<vtkPolyDataNormals> normals;
  normals->SetInputData(tempData.GetPointer());
  normals->ComputePointNormalsOff();
  normals->ComputeCellNormalsOn();
  normals->SplittingOff();
  normals->Update();
  this->GetOutputDataObject(0)->ShallowCopy(normals->GetOutput());
  vtkPolyData* output = vtkPolyData::SafeDownCast(this->GetOutputDataObject(0));
  output->SetLines(0); // per chance lines were passed in, we don't want them anymore

  return 1;
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::PrepInputsForStitching()
{
  // if one or both inputs are not vtkPolyData, convert to vtkPolyData,
  // then append the inputs
  vtkNew<vtkAppendPolyData> append;
  for (int i = 0; i < 2; i++)
  {
    if (vtkUnstructuredGrid::SafeDownCast(this->GetInputDataObject(i, 0)))
    {
      vtkNew<vtkDataSetSurfaceFilter> surface;
      surface->SetInputData(this->GetInputDataObject(i, 0));
      surface->Update();
      append->AddInputData(surface->GetOutput());
    }
    else
    {
      append->AddInputData(vtkPolyData::SafeDownCast(this->GetInputDataObject(i, 0)));
    }
  }
  append->Update();

  this->AppendedPolyData->ShallowCopy(append->GetOutput());

  // add a PointData array so that we can track what points from the original
  // dataset are being used
  vtkNew<vtkPolyData> appendCopy;
  appendCopy->ShallowCopy(this->AppendedPolyData);
  vtkNew<vtkIdTypeArray> pointIds;
  pointIds->SetName("PointIds");
  pointIds->SetNumberOfComponents(1);
  pointIds->SetNumberOfTuples(this->AppendedPolyData->GetNumberOfPoints());
  for (vtkIdType i = 0; i < this->AppendedPolyData->GetNumberOfPoints(); i++)
  {
    pointIds->SetValue(i, i);
  }
  appendCopy->GetPointData()->AddArray(pointIds.GetPointer());

  vtkNew<vtkCleanPolyData> clean;
  vtkSmartPointer<vtkStripper> stripper;
  vtkPolyData* input;
  if (appendCopy->GetNumberOfCells() != 2 || appendCopy->GetLines()->GetNumberOfCells() != 2)
  {
    vtkNew<vtkFeatureEdges> extractBoundary;
    extractBoundary->BoundaryEdgesOn();
    extractBoundary->FeatureEdgesOff();
    extractBoundary->ManifoldEdgesOff();
    extractBoundary->NonManifoldEdgesOff();
    extractBoundary->SetInputData(appendCopy.GetPointer());

    clean->SetInputConnection(extractBoundary->GetOutputPort());

    stripper = vtkSmartPointer<vtkStripper>::New();
    stripper->SetInputConnection(clean->GetOutputPort());
    stripper->SetMaximumLength(1000000);
    stripper->Update();
    input = stripper->GetOutput();

    if (input->GetNumberOfCells() != 2 || input->GetLines()->GetNumberOfCells() != 2)
    {
      vtkErrorMacro("Unable to create 2 and only 2 polylines from the inputs");
      return VTK_ERROR;
    }
  }
  else
  {
    // makes sure we try to close the loop, per chance 1st and last points are
    // co-located but different points in the dataset
    clean->SetInputData(appendCopy.GetPointer());
    clean->Update();
    input = clean->GetOutput();
  }

  input->GetLines()->InitTraversal();
  input->GetLines()->GetNextCell(this->LoopNPts[0], this->LoopPts[0]);
  input->GetLines()->GetNextCell(this->LoopNPts[1], this->LoopPts[1]);

  // also make sure the two lines are closed loops
  if (this->LoopPts[0][0] != this->LoopPts[0][this->LoopNPts[0] - 1] ||
    this->LoopPts[1][0] != this->LoopPts[1][this->LoopNPts[1] - 1])
  {
    vtkErrorMacro("Unable to create 2 closed loops for TIN stitching!");
    return VTK_ERROR;
  }

  // see if we need to reverse the second LOOP / polyline
  double normal1[3], normal2[3];
  vtkPolygon::ComputeNormal(input->GetPoints(), this->LoopNPts[0] - 1, this->LoopPts[0], normal1);
  vtkPolygon::ComputeNormal(input->GetPoints(), this->LoopNPts[1] - 1, this->LoopPts[1], normal2);

  if (vtkMath::Dot(normal1, normal2) < 0)
  {
    // reverse the 2nd loop (1st and last pt not swapped, as they are equal
    vtkIdType tempId;
    for (int i = 1; i<this->LoopNPts[1]>> 1; i++)
    {
      tempId = this->LoopPts[1][i];
      this->LoopPts[1][i] = this->LoopPts[1][this->LoopNPts[1] - 1 - i];
      this->LoopPts[1][this->LoopNPts[1] - 1 - i] = tempId;
    }
  }

  // setup tolerance factors (MaxDistance and MaxDistance2)
  double bounds[6];
  input->GetBounds(bounds);
  double xLength = bounds[1] - bounds[0];
  double yLength = bounds[3] - bounds[2];
  double minLength = xLength < yLength ? xLength : yLength;
  this->MaxDistance = minLength * this->Tolerance;
  this->MaxDistance2 = this->MaxDistance * this->MaxDistance;

  // the "input" we're going to use to further setup for stitching
  this->PreppedStitchingInput->ShallowCopy(input);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::SetupToStitchAsType1()
{
  double pt0[3], pt1[3];
  vtkIdType loopLinkIndex[2] = { 0, 0 };

  // "define" the best fit between the two loops as the one which minimizes
  // the maximum distance
  double smallestMaxDistance2 = VTK_FLOAT_MAX;
  for (vtkIdType i = 0; i < this->LoopNPts[0] - 1; i++)
  {
    double currentMaxDistance2 = 0, currentMinDistance2 = VTK_FLOAT_MAX;
    vtkIdType currentLoopLinkIndex[2] = { 0, 0 };
    bool betterOffsetFound = true;
    for (vtkIdType j = 0; j < this->LoopNPts[1] - 1; j++)
    {
      this->PreppedStitchingInput->GetPoint(
        this->LoopPts[0][(i + j > this->LoopNPts[0] - 1) ? (i + j - (this->LoopNPts[0] - 1))
                                                         : (i + j)],
        pt0);
      pt0[2] = 0;
      this->PreppedStitchingInput->GetPoint(this->LoopPts[1][j], pt1);
      pt1[2] = 0;
      double dist2 = vtkMath::Distance2BetweenPoints(pt0, pt1);
      if (dist2 > smallestMaxDistance2)
      {
        betterOffsetFound = false;
        break;
      }
      else if (dist2 > currentMaxDistance2)
      {
        currentMaxDistance2 = dist2;
      }
      if (dist2 < currentMinDistance2)
      {
        currentMinDistance2 = dist2;
        currentLoopLinkIndex[0] =
          (i + j > this->LoopNPts[0] - 1) ? (i + j - (this->LoopNPts[0] - 1)) : (i + j);
        currentLoopLinkIndex[1] = j;
      }
    }
    if (betterOffsetFound)
    {
      smallestMaxDistance2 = currentMaxDistance2;
      loopLinkIndex[1] = currentLoopLinkIndex[1];
      loopLinkIndex[0] = currentLoopLinkIndex[0];
    }
  }

  // setup "LoopLines" to hold our two loops
  this->LoopLines->Allocate(this->LoopNPts[0] + this->LoopNPts[1] + 2);
  this->LoopLines->Reset();

  // reorder the loops so that they "match".  How we reorder depends on
  // whether we are going to allow point insertion or not
  if (this->AllowInteriorPointInsertion)
  {
    // determine if dataset appropriate for interior point insertion
    this->FindPolyLineCorners(this->PreppedStitchingInput, this->LoopNPts[0], this->LoopPts[0],
      this->LoopCorners[0], this->MaxDistance2);
    vtkIdType loop0ReorderIndex = this->LoopCorners[0]->GetValue(0);
    this->ReorderPolyLine(
      this->LoopLines, this->LoopCorners[0], this->LoopNPts[0], this->LoopPts[0], 0);

    // the 2nd loop needs to be reordered by (bestClosestPtIndex - basePtIndex)
    // relative to 1st loop
    this->LoopCorners[1]->Reset(); // better be empty, because we didn't set it up
    vtkIdType reorderIndex = loop0ReorderIndex + (loopLinkIndex[1] - loopLinkIndex[0]);
    if (reorderIndex < 0)
    {
      reorderIndex += this->LoopNPts[1] - 1; // -1 because 1st/last point are same
    }
    else if (reorderIndex > this->LoopNPts[1] - 1)
    {
      reorderIndex -= this->LoopNPts[1] - 1; // -1 because 1st/last point are same
    }
    this->ReorderPolyLine(this->LoopLines, 0, this->LoopNPts[1], this->LoopPts[1], reorderIndex);

    // has to be a loop, so the last point is also a corner (actually same corner
    // as the 1st); makes processing later easier
    this->LoopCorners[0]->InsertNextValue(this->LoopNPts[0] - 1);
  }
  else
  {
    this->ReorderPolyLine(
      this->LoopLines, 0, this->LoopNPts[0], this->LoopPts[0], loopLinkIndex[0]);
    this->ReorderPolyLine(
      this->LoopLines, 0, this->LoopNPts[1], this->LoopPts[1], loopLinkIndex[1]);
  }

  // update this->LoopNPts and this->LoopPts given reordering
  this->LoopLines->InitTraversal();
  this->LoopLines->GetNextCell(this->LoopNPts[0], this->LoopPts[0]);
  this->LoopLines->GetNextCell(this->LoopNPts[1], this->LoopPts[1]);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::SetupToStitchUsingAutoDetect(double /*maxDistance*/, double maxDistance2)
{
  // figure out the endPts of the line segments
  this->FindPolyLineCorners(this->PreppedStitchingInput, this->LoopNPts[0], this->LoopPts[0],
    this->LoopCorners[0], maxDistance2);
  this->FindPolyLineCorners(this->PreppedStitchingInput, this->LoopNPts[1], this->LoopPts[1],
    this->LoopCorners[1], maxDistance2);

  // if corners don't match-up (or have same #, in which case they certainly
  // don't match-up) then must be Type III
  if (this->LoopCorners[0]->GetNumberOfTuples() != this->LoopCorners[1]->GetNumberOfTuples())
  {
    // only "sort of" ok... we may need to process further depending on how
    // we're going to handle type 3
    this->TINType = 3;
    return VTK_OK;
  }

  // start by finding if there is a match for the 1st corner point in the 1st loop
  // by a corner point in the 2nd loop
  double loopBasePt[3], testPt[3];
  this->PreppedStitchingInput->GetPoint(
    this->LoopPts[0][this->LoopCorners[0]->GetValue(0)], loopBasePt);
  loopBasePt[2] = 0;
  vtkIdType loopPivot = -1;
  for (vtkIdType i = 0; i < this->LoopCorners[1]->GetNumberOfTuples(); i++)
  {
    this->PreppedStitchingInput->GetPoint(
      this->LoopPts[1][this->LoopCorners[1]->GetValue(i)], testPt);
    testPt[2] = 0;
    if (vtkMath::Distance2BetweenPoints(loopBasePt, testPt) < maxDistance2)
    {
      loopPivot = i;
      break;
    }
  }
  if (loopPivot == -1)
  {
    // only "sort of" ok... we may need to process further depending on how
    // we're going to handle type 3
    this->TINType = 3;
    return VTK_OK;
  }

  // setup "LoopLines" to hold our two loops
  this->LoopLines->Allocate(this->LoopNPts[0] + this->LoopNPts[1] + 2);
  this->LoopLines->Reset();
  this->ReorderPolyLine(
    this->LoopLines, this->LoopCorners[0], this->LoopNPts[0], this->LoopPts[0], 0);

  // update this->LoopPts[0] to output from ReorderPolyline
  this->LoopLines->InitTraversal();
  this->LoopLines->GetNextCell(this->LoopNPts[0], this->LoopPts[0]);

  // find the next corner match, better be either the one before "loopPivot",
  // or the one after.
  double loopNextPt[3];
  this->PreppedStitchingInput->GetPoint(
    this->LoopPts[0][this->LoopCorners[0]->GetValue(1)], loopNextPt);
  loopNextPt[2] = 0;
  // looking for the "next" loop point; 1st and last point are the same, so
  // if loopPivot is the last point, the next point is the 2nd point (not the
  // 1st)
  // is the corner after, so don't need to reverse
  this->ReorderPolyLine(
    this->LoopLines, this->LoopCorners[1], this->LoopNPts[1], this->LoopPts[1], loopPivot);
  this->LoopLines->GetNextCell(this->LoopNPts[1], this->LoopPts[1]);

  // now that we have them lined up, both starting at the same location
  // check to make sure every corner matches.
  double ptLine0[3], ptLine1[3];
  bool cornerIndicesMatch = true;
  for (vtkIdType i = 0; i < this->LoopCorners[0]->GetNumberOfTuples(); i++)
  {
    if (this->LoopCorners[0]->GetValue(i) != this->LoopCorners[1]->GetValue(i))
    {
      cornerIndicesMatch = false;
    }

    this->PreppedStitchingInput->GetPoint(
      this->LoopPts[0][this->LoopCorners[0]->GetValue(i)], ptLine0);
    this->PreppedStitchingInput->GetPoint(
      this->LoopPts[1][this->LoopCorners[1]->GetValue(i)], ptLine1);
    ptLine0[2] = ptLine1[2] = 0;
    if (vtkMath::Distance2BetweenPoints(ptLine0, ptLine1) > maxDistance2)
    {
      // only "sort of" ok... we may need to process further depending on how
      // we're going to handle type 3
      this->TINType = 3;
      return VTK_OK;
    }
  }

  // has to be a loop, so the last point is also a corner (actually same corner
  // as the 1st); makes processing later easier
  this->LoopCorners[0]->InsertNextValue(this->LoopNPts[0] - 1);
  this->LoopCorners[1]->InsertNextValue(this->LoopNPts[1] - 1);

  // Type 2 unless corner indices match AND every other point also matches up (same x,y)
  if (cornerIndicesMatch)
  {
    this->TINType = 1;
    // check EVERY point to verify same x, y
    for (vtkIdType i = 0; i < this->LoopNPts[0]; i++)
    {
      this->PreppedStitchingInput->GetPoint(this->LoopPts[0][i], ptLine0);
      this->PreppedStitchingInput->GetPoint(this->LoopPts[1][i], ptLine1);
      ptLine0[2] = ptLine1[2] = 0;
      if (vtkMath::Distance2BetweenPoints(ptLine0, ptLine1) > maxDistance2)
      {
        this->TINType = 2;
        break;
      }
    }
  }
  else
  {
    this->TINType = 2;
  }
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::FindPolyLineCorners(
  vtkPolyData* input, vtkIdType npts, vtkIdType* pts, vtkIdTypeArray* corners, double maxDistance2)
{
  double startPt[3], testPt[3], endPt[3];
  input->GetPoint(pts[npts - 2], startPt);
  startPt[2] = 0;
  corners->Reset();
  corners->Allocate(npts);
  vtkIdType startIndex = -1;
  for (vtkIdType i = 1; i < npts; i++)
  {
    input->GetPoint(pts[i], endPt);
    endPt[2] = 0;
    // search back through points in this line (defined by startPt and endPt);
    // see if any point between start and end deviates by more than tolerance
    for (vtkIdType testIndex = i - 1; testIndex > startIndex; testIndex--)
    {
      input->GetPoint(pts[testIndex], testPt);
      testPt[2] = 0;
      if (vtkLine::DistanceToLine(testPt, startPt, endPt) > maxDistance2)
      {
        corners->InsertNextValue(i - 1);
        startIndex = i - 1; // for next iteration
        if (testIndex != i - 1)
        {
          input->GetPoint(pts[i - 1], startPt);
          startPt[2] = 0;
        }
        else
        {
          startPt[0] = testPt[0];
          startPt[1] = testPt[1];
        }
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
int vtkTINStitcher::ReorderPolyLine(vtkCellArray* newLines, vtkIdTypeArray* corners, vtkIdType npts,
  vtkIdType* pts, vtkIdType startCorner)
{
  vtkIdType startOffset = startCorner;
  if (corners)
  {
    // if we pass in "corners", the offset is the point index of the startCorner
    startOffset = corners->GetValue(startCorner);
  }

  newLines->InsertNextCell(npts);
  for (vtkIdType i = startOffset; i < npts; i++)
  {
    newLines->InsertCellPoint(pts[i]);
  }
  for (vtkIdType i = 1; i <= startOffset; i++)
  {
    newLines->InsertCellPoint(pts[i]);
  }

  // now adjust the "corners"
  if (corners && startOffset != 0)
  {
    if (startCorner != 0)
    {
      vtkSmartPointer<vtkIdTypeArray> newCorners = vtkSmartPointer<vtkIdTypeArray>::New();
      newCorners->SetNumberOfComponents(1);
      newCorners->SetNumberOfTuples(corners->GetNumberOfTuples());
      vtkIdType index = 0;
      for (vtkIdType i = startCorner; i < corners->GetNumberOfTuples(); i++, index++)
      {
        newCorners->SetValue(index, corners->GetValue(i) - startOffset);
      }
      for (vtkIdType i = 0; i < startCorner; i++, index++)
      {
        newCorners->SetValue(index, corners->GetValue(i) - startOffset + (npts - 1));
      }
      // copy of the reordered corners
      corners->DeepCopy(newCorners);
    }
    else
    {
      for (vtkIdType i = 0; i < corners->GetNumberOfTuples(); i++)
      {
        corners->SetValue(i, corners->GetValue(i) - startOffset);
      }
    }
  }
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::MapLoopLinesToAppendedData()
{
  // IMPORTANT - "map" the LoopLines to pull from the full set of points
  vtkIdTypeArray* filteredPointIds =
    vtkIdTypeArray::SafeDownCast(this->PreppedStitchingInput->GetPointData()->GetArray("PointIds"));
  vtkIdType* arrayPtr = this->LoopLines->GetPointer();
  for (vtkIdType i = 1; i < this->LoopLines->GetNumberOfConnectivityEntries(); i++)
  {
    arrayPtr[i] = filteredPointIds->GetValue(arrayPtr[i]);
    // need to skip 1st entry (number of points) of the 2nd cell
    if (i == this->LoopNPts[0])
    {
      i++;
    }
  }
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::CreateQuadStitching(vtkPolyData* outputPD)
{
  for (int i = 0; i < this->LoopNPts[0] - 1; i++)
  {
    vtkIdType quadPts[4] = { this->LoopPts[0][i], this->LoopPts[0][i + 1], this->LoopPts[1][i + 1],
      this->LoopPts[1][i] };
    outputPD->GetPolys()->InsertNextCell(4, quadPts);
  }
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::CreateTriStitching(vtkPolyData* outputPD)
{
  // Creates triangle pointing in opposite directions, so will need to fix the
  // normals (index ordering);  Not doing the analysis to figure out what is
  // "out", so could have them point in same direction, but sometimes they'd all
  // be right, or all be wrong, as opposed to half are always pointing the wrong
  // direction.
  for (int i = 0; i < this->LoopNPts[0] - 1; i++)
  {
    vtkIdType triPts[3] = { this->LoopPts[0][i], this->LoopPts[0][i + 1], this->LoopPts[1][i] };
    outputPD->GetPolys()->InsertNextCell(3, triPts);
    triPts[0] = this->LoopPts[1][i + 1];
    outputPD->GetPolys()->InsertNextCell(3, triPts);
  }
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::ProcessSegmentWithTriangle(vtkPolyData* outputPD, vtkIdType startCornerIndex,
  vtkIdTypeArray* sidePoints0, vtkIdTypeArray* sidePoints1)
{
  // Structures for Triangle v1.6
  struct triangulateio input, output;

  // initialize structures
  debug_initialize();
  Init_triangluateio(&input);
  Init_triangluateio(&output);

  vtkSmartPointer<vtkTransform> transformToXZPlane = vtkSmartPointer<vtkTransform>::New();

  std::vector<vtkIdType> triangleToPD;
  int numberOfPointsInLoop0Segment, numberOfPointsInLoop1Segment;
  bool loop0HasGreaterZ;

  // setup the input points
  this->SetupPointsForTriangle(input, transformToXZPlane, outputPD, startCornerIndex, sidePoints0,
    sidePoints1, numberOfPointsInLoop0Segment, numberOfPointsInLoop1Segment, loop0HasGreaterZ,
    triangleToPD);

  // setup the input segments (boundaries/markers)
  this->SetupSegmentsForTriangle(
    input, numberOfPointsInLoop0Segment, numberOfPointsInLoop1Segment, sidePoints0, sidePoints1);

  // Options Fed to Triangle
  char options[64];
  if (this->AllowInteriorPointInsertion)
  {
    sprintf(options, "pzq%f", this->MinimumAngle);
  }
  else
  {
    sprintf(options, "pzYY");
  }

  // Call Triangle v1.6
  triangulate(options, &input, &output, static_cast<struct triangulateio*>(NULL));

  // invert the transform... want to map points back to original space
  transformToXZPlane->Inverse();

  // insert any new points into our pointset
  for (int i = input.numberofpoints; i < output.numberofpoints; i++)
  {
    double pt[3] = { output.pointlist[i * 2], 0, output.pointlist[i * 2 + 1] };
    transformToXZPlane->TransformPoint(pt, pt);
    triangleToPD.push_back(outputPD->GetPoints()->InsertNextPoint(pt));
  }

  // not add the triangles that were created
  vtkCellArray* outputPolys = outputPD->GetPolys();
  for (int i = 0; i < output.numberoftriangles; i++)
  {
    vtkIdType pts[3] = { triangleToPD[output.trianglelist[i * 3]],
      triangleToPD[output.trianglelist[i * 3 + 1]], triangleToPD[output.trianglelist[i * 3 + 2]] };
    outputPolys->InsertNextCell(3, pts);
  }

  // check for points added to the sides, which need to be passed to later iterations
  double startPt[3] = { 0, 0, 0 }, endPt[3] = { 0, 0, 0 };
  if (sidePoints0->GetNumberOfTuples() == 0)
  {
    // only time we hit this case is first segment, which won't have only "side 0" pts
    startPt[0] = input.pointlist[0];
    startPt[1] = input.pointlist[1];
    endPt[0] = input.pointlist[(input.numberofpoints - 1) * 2];
    endPt[1] = input.pointlist[(input.numberofpoints - 1) * 2 + 1];
    // if 0 wasn't fixed (fix via -1 entry), so see if any added on the boundary;
    // if not, add -1 for the next time this boundary is used
    this->SetupSidePoints(output.pointlist, input.numberofpoints, output.numberofpoints, startPt,
      endPt, triangleToPD, sidePoints0, loop0HasGreaterZ);
  }
  if (sidePoints1->GetNumberOfTuples() == 0)
  {
    startPt[0] = input.pointlist[(numberOfPointsInLoop0Segment - 1) * 2];
    startPt[1] = input.pointlist[(numberOfPointsInLoop0Segment - 1) * 2 + 1];
    endPt[0] = input.pointlist[numberOfPointsInLoop0Segment * 2];
    endPt[1] = input.pointlist[numberOfPointsInLoop0Segment * 2 + 1];
    // if 0 wasn't fixed (fix via -1 entry), so see if any added on the boundary;
    // if not, add -1 for the next time this boundary is used
    this->SetupSidePoints(output.pointlist, input.numberofpoints, output.numberofpoints, startPt,
      endPt, triangleToPD, sidePoints1, loop0HasGreaterZ);
  }

  //Release all the memory used by triangle
  bool pointListShared = (input.pointlist == output.pointlist);
  bool segmentListShared = (input.segmentlist == output.segmentlist);
  bool holeListShared = (input.holelist == output.holelist);

  Free_triangluateio(&input);
  if (pointListShared)
  {
    //The free on input released the memory
    output.pointlist = NULL;
  }
  if (segmentListShared)
  {
    //The free on input released the memory
    output.segmentlist = NULL;
  }
  if (holeListShared)
  {
    //The free on input released the memory
    output.holelist = NULL;
  }
  Free_triangluateio(&output);
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::SetupPointsForTriangle(triangulateio& input, vtkTransform* ptTransform,
  vtkPolyData* outputPD, vtkIdType startCornerIndex, vtkIdTypeArray* sidePoints0,
  vtkIdTypeArray* sidePoints1, int& numberOfPointsInLoop0Segment, int& numberOfPointsInLoop1Segment,
  bool& loop0HasGreaterZ, std::vector<vtkIdType>& triangleToPD)
{
  vtkIdType nextCornerIndex = startCornerIndex + 1;

  vtkIdTypeArray*(loopCorners[2]) = { this->LoopCorners[0], this->LoopCorners[1] };
  // if we don't use auto-detect (user specified = type I), we only have the
  // corners for the 1st loop; the 2nd loop uses the same corner indices
  if (loopCorners[1]->GetNumberOfTuples() == 0)
  {
    loopCorners[1] = loopCorners[0];
  }

  // Put points into "input" data structure;
  // 1st need to "map" points to XZ plane... determine rotation about
  // Z to get there, by dot product between normal of our segment and [0 1 0]
  double pt0[3], pt1[3], pt2[3];
  outputPD->GetPoint(this->LoopPts[0][loopCorners[0]->GetValue(startCornerIndex)], pt0);
  outputPD->GetPoint(this->LoopPts[0][loopCorners[0]->GetValue(nextCornerIndex)], pt1);
  outputPD->GetPoint(this->LoopPts[1][loopCorners[1]->GetValue(startCornerIndex)], pt2);

  // which mesh has greater z?
  loop0HasGreaterZ = true;
  if (pt2[2] > pt0[2])
  {
    loop0HasGreaterZ = false;
  }

  // get segment normal, which is used to determine rotation required to put on XZ plane
  double v1[3], v2[3], segmentNormal[3];
  v1[0] = pt1[0] - pt0[0];
  v1[1] = pt1[1] - pt0[1];
  v1[2] = pt1[2] - pt0[2];
  v2[0] = pt2[0] - pt0[0];
  v2[1] = pt2[1] - pt0[1];
  v2[2] = pt2[2] - pt0[2];
  vtkMath::Cross(v1, v2, segmentNormal);

  vtkMath::Normalize(segmentNormal);

  double xzNormal[3] = { 0, 1, 0 };
  double angle = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(segmentNormal, xzNormal)));

  // figure out the axis to rotate about
  double rotationAxis[3];
  vtkMath::Cross(segmentNormal, xzNormal, rotationAxis);

  ptTransform->PreMultiply();
  ptTransform->RotateWXYZ(angle, rotationAxis);
  ptTransform->Translate(-pt0[0], -pt0[1], -pt0[2]);

  numberOfPointsInLoop0Segment =
    loopCorners[0]->GetValue(nextCornerIndex) - loopCorners[0]->GetValue(startCornerIndex) + 1;
  numberOfPointsInLoop1Segment =
    loopCorners[1]->GetValue(nextCornerIndex) - loopCorners[1]->GetValue(startCornerIndex) + 1;

  // how many points in all?
  input.numberofpoints = numberOfPointsInLoop0Segment + numberOfPointsInLoop1Segment;

  if (sidePoints0->GetNumberOfTuples() > 0 && sidePoints0->GetValue(0) >= 0)
  {
    input.numberofpoints += sidePoints0->GetNumberOfTuples();
  }
  if (sidePoints1->GetNumberOfTuples() > 0 && sidePoints1->GetValue(0) >= 0)
  {
    input.numberofpoints += sidePoints1->GetNumberOfTuples();
  }

  // allocate memory for the input points
  input.pointlist =
    static_cast<TRIANGLE_REAL*>(tl_alloc(sizeof(TRIANGLE_REAL), input.numberofpoints * 2, 0));

  // transform all the input points and add them to the inputpointlist
  vtkIdType pointListInsertIndex = 0;
  // add points from loop 0
  for (int i = loopCorners[0]->GetValue(startCornerIndex);
       i <= loopCorners[0]->GetValue(nextCornerIndex); i++)
  {
    outputPD->GetPoint(this->LoopPts[0][i], pt0);
    triangleToPD.push_back(this->LoopPts[0][i]);
    ptTransform->TransformPoint(pt0, pt0);
    input.pointlist[pointListInsertIndex++] = pt0[0];
    input.pointlist[pointListInsertIndex++] = pt0[2];
  }
  // add any "side1" points
  for (int i = 0; i < sidePoints1->GetNumberOfTuples(); i++)
  {
    if (sidePoints1->GetValue(i) >= 0)
    {
      outputPD->GetPoint(sidePoints1->GetValue(i), pt0);
      triangleToPD.push_back(sidePoints1->GetValue(i));
      ptTransform->TransformPoint(pt0, pt0);
      input.pointlist[pointListInsertIndex++] = pt0[0];
      input.pointlist[pointListInsertIndex++] = pt0[2];
    }
  }

  // add points from loop 1
  for (int i = loopCorners[1]->GetValue(nextCornerIndex);
       i >= loopCorners[1]->GetValue(startCornerIndex); i--)
  {
    outputPD->GetPoint(this->LoopPts[1][i], pt0);
    triangleToPD.push_back(this->LoopPts[1][i]);
    ptTransform->TransformPoint(pt0, pt0);
    if (i == loopCorners[1]->GetValue(nextCornerIndex))
    {
    }
    input.pointlist[pointListInsertIndex++] = pt0[0];
    input.pointlist[pointListInsertIndex++] = pt0[2];
  }

  // add any "side0" points
  for (int i = sidePoints0->GetNumberOfTuples() - 1; i >= 0; i--)
  {
    if (sidePoints0->GetValue(i) >= 0)
    {
      outputPD->GetPoint(sidePoints0->GetValue(i), pt0);
      triangleToPD.push_back(sidePoints0->GetValue(i));
      ptTransform->TransformPoint(pt0, pt0);
      input.pointlist[pointListInsertIndex++] = pt0[0];
      input.pointlist[pointListInsertIndex++] = pt0[2];
    }
  }
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::SetupSegmentsForTriangle(triangulateio& input,
  int numberOfPointsInLoop0Segment, int numberOfPointsInLoop1Segment, vtkIdTypeArray* sidePoints0,
  vtkIdTypeArray* sidePoints1)
{
  // number of segments is equal to the number of points (since closed loop)
  input.numberofsegments = input.numberofpoints;

  // Allocate Space
  input.segmentlist = static_cast<int*>(tl_alloc(sizeof(int), input.numberofsegments * 2, 0));
  int segmentEndPtIndex = 0;
  for (int j = 0; j < input.numberofsegments; j++)
  {
    input.segmentlist[segmentEndPtIndex++] = j;
    input.segmentlist[segmentEndPtIndex++] = j + 1;
  }
  input.segmentlist[--segmentEndPtIndex] = 0; // close the loop

  input.segmentmarkerlist = static_cast<int*>(tl_alloc(sizeof(int), input.numberofsegments, 0));

  int segmentIndex = 0;

  // segment markers in loop 0
  for (int i = 0; i < numberOfPointsInLoop0Segment - 1; i++)
  {
    // mark as can NOT add points on these boundaries
    input.segmentmarkerlist[segmentIndex++] = 2e9;
  }

  // segment markers for "side1"
  if (sidePoints1->GetNumberOfTuples() == 0)
  {
    // CAN add points to this segment
    input.segmentmarkerlist[segmentIndex++] = 1;
  }
  else if (sidePoints1->GetValue(0) < 0)
  {
    // can NOT add points to this segment
    input.segmentmarkerlist[segmentIndex++] = 2e9;
  }
  else
  {
    for (int i = 0; i <= sidePoints1->GetNumberOfTuples(); i++)
    {
      // can NOT add points to this segment
      input.segmentmarkerlist[segmentIndex++] = 2e9;
    }
  }

  // add points from loop 1
  for (int i = 0; i < numberOfPointsInLoop1Segment - 1; i++)
  {
    // mark as can NOT add points on these boundaries
    input.segmentmarkerlist[segmentIndex++] = 2e9;
  }

  // segment markers for "side0"
  if (sidePoints0->GetNumberOfTuples() == 0)
  {
    // CAN add points to this segment
    input.segmentmarkerlist[segmentIndex++] = 1;
  }
  else if (sidePoints0->GetValue(0) < 0)
  {
    // can NOT add points to this segment
    input.segmentmarkerlist[segmentIndex++] = 2e9;
  }
  else
  {
    for (int i = 0; i <= sidePoints0->GetNumberOfTuples(); i++)
    {
      // can NOT add points to this segment
      input.segmentmarkerlist[segmentIndex++] = 2e9;
    }
  }
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::SetupSidePoints(double* pointList, int inputNumberOfPoints,
  int outputNumberOfPoints, double* startPt, double* endPt, std::vector<vtkIdType>& triangleToPD,
  vtkIdTypeArray* sidePoints, bool fillDescending)
{
  std::map<double, vtkIdType> sidePointsMap;
  double testPt[3] = { 0, 0, 0 };
  // want only the points on the boundary line segment, thus accept only
  // those within 1e-6 of exact segment (actually expect "0")
  double maxDistance2 = vtkMath::Distance2BetweenPoints(startPt, endPt) * 1e-12;
  for (int i = inputNumberOfPoints; i < outputNumberOfPoints; i++)
  {
    testPt[0] = pointList[i * 2];
    testPt[1] = pointList[i * 2 + 1];
    if (vtkLine::DistanceToLine(testPt, startPt, endPt) < maxDistance2)
    {
      sidePointsMap[pointList[i * 2 + 1]] = triangleToPD[i];
    }
  }

  if (sidePointsMap.size() != 0)
  {
    if (fillDescending)
    {
      std::map<double, vtkIdType>::reverse_iterator mapIdIter;
      for (mapIdIter = sidePointsMap.rbegin(); mapIdIter != sidePointsMap.rend(); mapIdIter++)
      {
        sidePoints->InsertNextValue(mapIdIter->second);
      }
    }
    else
    {
      std::map<double, vtkIdType>::iterator mapIdIter;
      for (mapIdIter = sidePointsMap.begin(); mapIdIter != sidePointsMap.end(); mapIdIter++)
      {
        sidePoints->InsertNextValue(mapIdIter->second);
      }
    }
  }
  else
  {
    sidePoints->InsertNextValue(-1);
  }
}

//----------------------------------------------------------------------------
int vtkTINStitcher::FillInputPortInformation(int port, vtkInformation* info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
  {
    return 0;
  }
  if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkTINStitcher::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//-----------------------------------------------------------------------------
void vtkTINStitcher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Use Quads: " << (this->UseQuads ? "On" : "Off");
  os << indent << "Minimum Angle: " << this->MinimumAngle;
  os << indent << "Allow Interior Point Insertion: "
     << (this->AllowInteriorPointInsertion ? "True" : "False");
  os << indent << "Tolerance: " << this->Tolerance;
  os << indent
     << "User Specified TIN Type: " << (this->UserSpecifiedTINType == 0 ? "Auto-Detect" : "Type I");
}
