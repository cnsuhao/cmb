//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkLoopsMesher.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"

vtkStandardNewMacro(vtkLoopsMesher);

vtkLoopsMesher::vtkLoopsMesher()
{
  this->UseSubLoops = 0;
  this->UseQuads = 0;
  this->PassLines = 0;
  this->OrientLoops = 0;
  this->Ids = vtkIdList::New();
  this->Ids->SetNumberOfIds(4);
}

vtkLoopsMesher::~vtkLoopsMesher()
{
  this->Ids->Delete();
}

int vtkLoopsMesher::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints* inPts;
  vtkIdType i, numPts, numLines;
  vtkCellArray *inLines, *newPolys;
  vtkIdType* pts = 0;
  vtkIdType* pts2 = 0;
  vtkIdType npts = 0;
  vtkIdType npts2 = 0;

  // Check input, pass data if requested
  //
  vtkDebugMacro(<< "Creating a ruled surface");

  inPts = input->GetPoints();
  inLines = input->GetLines();
  if (!inPts || !inLines)
  {
    return 1;
  }
  numLines = inLines->GetNumberOfCells();
  numPts = inPts->GetNumberOfPoints();
  if (numPts < 1 || numLines < 2)
  {
    return 1;
  }

  if (this->PassLines)
  {
    output->SetLines(inLines);
  }

  output->SetPoints(inPts);
  output->GetPointData()->PassData(input->GetPointData());
  newPolys = vtkCellArray::New();
  newPolys->Allocate(2 * numPts);
  output->SetPolys(newPolys);
  newPolys->Delete();

  // For each pair of lines (as selected by Offset and OnRatio), create a
  // stripe (a ruled surfac between two lines).
  //
  inLines->InitTraversal();
  inLines->GetNextCell(npts, pts);
  for (i = 0; i < numLines; i++)
  {
    //abort/progress methods
    this->UpdateProgress(static_cast<double>(i) / numLines);
    if (this->GetAbortExecute())
    {
      break; //out of line loop
    }

    inLines->GetNextCell(npts2, pts2); //get the next edge

    this->PointWalk(output, inPts, npts, pts, npts2, pts2);

    //Get the next line for generating the next stripe
    npts = npts2;
    pts = pts2;
    if (i == (numLines - 2))
    {
      i++; //will cause the loop to end
    }      //add far boundary of surface
  }        //for all selected line pairs

  return 1;
}

void vtkLoopsMesher::PointWalk(
  vtkPolyData* output, vtkPoints* inPts, int npts, vtkIdType* pts, int npts2, vtkIdType* pts2)
{
  int loc, loc2, next2;
  vtkCellArray* newPolys = output->GetPolys();
  double x[3], y[3], a[3], b[3], xb, ya, distance2;

  // Compute distance factor based on first two points
  //

  vtkIdType endLoop2, startLoop2, i;

  if (!this->OrientLoops)
  {
    endLoop2 = npts2 - 1;
    startLoop2 = 0;
    inPts->GetPoint(pts[0], x);
    inPts->GetPoint(pts2[0], y);
  }
  else
  {
    double minD2;
    startLoop2 = 0;
    inPts->GetPoint(pts[0], x);
    inPts->GetPoint(pts2[0], y);
    minD2 = vtkMath::Distance2BetweenPoints(x, y);
    for (i = 1; i != npts2; i++)
    {
      inPts->GetPoint(pts2[i], y);
      distance2 = vtkMath::Distance2BetweenPoints(x, y);
      if (distance2 < minD2)
      {
        minD2 = distance2;
        startLoop2 = i;
      }
    }

    // If the starting point is not 0 then the end is behind us
    if (startLoop2)
    {
      endLoop2 = startLoop2 - 1;
    }
    else
    {
      endLoop2 = npts2 - 1;
    }
  }

  // Walk "edge" along the two lines maintaining closest distance
  // and generating triangles as we go.
  loc = 0;
  loc2 = startLoop2;
  bool endOfLoop2 = false;
  while (loc < (npts - 1) || (!endOfLoop2))
  {

    // Determine the next point in loop 2
    next2 = loc2 + 1;
    if ((!startLoop2) && (next2 == endLoop2))
    {
      // If we started 0 then when we hit the end of the loop
      // we are done
      endOfLoop2 = true;
    }
    else if (next2 == startLoop2)
    {
      // If we are here we have reached the end of the loop
      // though we need to still process the starting point a second time
      // to close the surface
      endOfLoop2 = true;
    }
    else if (next2 == npts2)
    {
      // The only way we would reach the end of the original
      // loop is if we did not start with the 0th point - since
      // this point is repeated (its the same at the npts2-1 point
      // we need to skip it
      next2 = 1;
    }

    if (loc >= (npts - 1)) //clamped at end of first line
    {
      inPts->GetPoint(pts[loc], x);
      inPts->GetPoint(pts2[loc2], a);
      inPts->GetPoint(pts2[next2], b);
      xb = vtkMath::Distance2BetweenPoints(x, b);
      newPolys->InsertNextCell(3);
      newPolys->InsertCellPoint(pts[loc]);    //x
      newPolys->InsertCellPoint(pts2[next2]); //b
      newPolys->InsertCellPoint(pts2[loc2]);  //a

      loc2 = next2;
    }
    else if (loc2 == endLoop2) //clamped at end of second line
    {
      inPts->GetPoint(pts[loc], x);
      inPts->GetPoint(pts[loc + 1], y);
      inPts->GetPoint(pts2[loc2], a);
      ya = vtkMath::Distance2BetweenPoints(y, a);
      newPolys->InsertNextCell(3);
      newPolys->InsertCellPoint(pts[loc]);     //x
      newPolys->InsertCellPoint(pts[loc + 1]); //y
      newPolys->InsertCellPoint(pts2[loc2]);   //a
      loc++;
    }
    else //not at either end
    {
      inPts->GetPoint(pts[loc], x);
      inPts->GetPoint(pts[loc + 1], y);
      inPts->GetPoint(pts2[loc2], a);
      inPts->GetPoint(pts2[next2], b);
      xb = vtkMath::Distance2BetweenPoints(x, b);
      ya = vtkMath::Distance2BetweenPoints(a, y);
      if (xb <= ya)
      {
        newPolys->InsertNextCell(3);
        newPolys->InsertCellPoint(pts[loc]);    //x
        newPolys->InsertCellPoint(pts2[next2]); //b
        newPolys->InsertCellPoint(pts2[loc2]);  //a

        loc2 = next2;
      }
      else
      {
        newPolys->InsertNextCell(3);
        newPolys->InsertCellPoint(pts[loc]);     //x
        newPolys->InsertCellPoint(pts[loc + 1]); //y
        newPolys->InsertCellPoint(pts2[loc2]);   //a

        loc++;
      }
    } //where in the lines
  }   //while still building the stripe
}

void vtkLoopsMesher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Use Quads: " << (this->UseQuads ? "On\n" : "Off\n");
  os << indent << "Use SubLoops: " << (this->UseSubLoops ? "On\n" : "Off\n");
  os << indent << "Pass Lines: " << (this->PassLines ? "On\n" : "Off\n");
  os << indent << "Orient Loops: " << (this->OrientLoops ? "On\n" : "Off\n");
}
