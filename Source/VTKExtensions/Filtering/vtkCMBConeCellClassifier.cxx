//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBConeCellClassifier.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkNew.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkCMBConeCellClassifier);
//-----------------------------------------------------------------------------
vtkCMBConeCellClassifier::vtkCMBConeCellClassifier()
{
  this->ClassificationMode = 0;
  this->OriginalCellValue = 0;
  this->NewCellValue = 1;
  this->BaseCenter[0] = this->BaseCenter[1] = this->BaseCenter[2] = 0.0;
  this->AxisDirection[0] = this->AxisDirection[1] = 0.0;
  this->AxisDirection[2] = 1.0;
  this->Height = 1.0;
  this->BaseRadius = 0.5;
  this->TopRadius = 0.0;
  this->Translation[0] = this->Translation[1] = this->Translation[2] = 0.0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0.0;
  this->Scaling[0] = this->Scaling[1] = this->Scaling[2] = 1.0;
  this->ClassificationMode = 0;
  this->OriginalCellValue = 0;
  this->NewCellValue = 1;
  this->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_CELLS,
    vtkDataSetAttributes::SCALARS);
}

//-----------------------------------------------------------------------------
vtkCMBConeCellClassifier::~vtkCMBConeCellClassifier()
{
}


//-----------------------------------------------------------------------------
int vtkCMBConeCellClassifier::RequestData(vtkInformation* vtkNotUsed(request),
                                          vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector)
{
    // get the info and input data
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input)
    {
    vtkErrorMacro("vtkUnstructuredGrid input is required.");
    return 0;
    }

  output->CopyStructure(input);
  vtkIntArray *values = vtkIntArray::SafeDownCast(this->GetInputArrayToProcess(0, input));
  vtkNew<vtkIntArray> newVals;
  vtkIdType numCells = input->GetNumberOfCells();
  newVals->SetNumberOfComponents(1);
  newVals->SetNumberOfTuples(numCells);
  newVals->SetName("NewIds");
  output->GetCellData()->AddArray(newVals.GetPointer());
  output->GetCellData()->SetScalars(newVals.GetPointer());

  // Process all of the cones
  vtkIdType i, j, cell;
  double p[3],transP[3];
  int cellVal;
  vtkIdType numPts;
  int maxSize = input->GetMaxCellSize();
  vtkIdType *pids = new vtkIdType[maxSize];
  vtkCellArray *cells = input->GetCells();
  vtkPoints *points = input->GetPoints();
  // We need to get the unit vector based on the Cone's Axis Direction
  this->AxisUnitDir[0] = this->AxisDirection[0];
  this->AxisUnitDir[1] = this->AxisDirection[1];
  this->AxisUnitDir[2] = this->AxisDirection[2];
  vtkMath::Normalize(this->AxisUnitDir);

  // Create a transform for transforming the test points into
  // the coordinate system of the cone
  vtkNew<vtkTransform> transform;
  transform->Identity();
  transform->PreMultiply();
  transform->Translate(this->Translation);
  transform->RotateZ(this->Orientation[2]);
  transform->RotateX(this->Orientation[0]);
  transform->RotateY(this->Orientation[1]);
  transform->Scale(this->Scaling);
  transform->Inverse();
  transform->Update();
  // First copy the original values of the cell data
  for (i = 0; i < numCells; i++)
    {
    newVals->SetValue(i, values->GetValue(i));
    }

  cells->InitTraversal();
  for (cell = 0; cells->GetNextCell(numPts, pids); cell++)
    {
    cellVal = values->GetValue(cell);
    if (cellVal != this->OriginalCellValue)
      {
      // if the cell doesn't have the value we are look to change
      // skip it
      continue;
      }
    // Fully inside mode
    if (this->ClassificationMode == 1)
      {
      for (j = 0; j < numPts; j++)
        {
        points->GetPoint(pids[j], p);
        transform->InternalTransformPoint(p, transP);
        if (!this->IsInside(transP))
          {
          break;
          }
        }
      // Did all the points pass?
      if (j == numPts)
        {
        newVals->SetValue(cell, this->NewCellValue);
        }
      }
    // Partially inside mode
    else if (this->ClassificationMode == 0)
      {
      for (j = 0; j < numPts; j++)
        {
        points->GetPoint(pids[j], p);
        transform->InternalTransformPoint(p, transP);
        if (this->IsInside(transP))
          {
          break;
          }
        }
      // Did we find a point inside?
      if (j != numPts)
        {
        newVals->SetValue(cell, this->NewCellValue);
        }
      }
    else // Only cells that intersect
      {
      bool foundInside = false;
      bool foundOutside = false;
      for (j = 0; j < numPts; j++)
        {
        points->GetPoint(pids[j], p);
        transform->InternalTransformPoint(p, transP);
        if (this->IsInside(transP))
          {
          foundInside = true;
          }
        else
          {
          foundOutside = true;
          }
        if (foundInside && foundOutside)
          {
          break;
          }
        }
      // Did we find a point inside?
      if (foundInside && foundOutside)
        {
        newVals->SetValue(cell, this->NewCellValue);
        }
      }
    }
  delete[] pids;
  return 1;
}
//----------------------------------------------------------------------------
bool vtkCMBConeCellClassifier::IsInside(const double p[3])
{
  double vec[3];
  vtkMath::Subtract(p, this->BaseCenter, vec);
  // Get the dot product to see if the
  // projection of p onto the cone axis lies between 0 and coneLength
  double l = vtkMath::Dot(vec, this->AxisUnitDir);
  if ((l < 0.0) || (l > this->Height))
    {
    return false; // point is outside of the cone
    }
  // Now see what the radius at that point along the cone
  // should be based on interpolating between the radii of the cone
  double r2 = this->BaseRadius + ((this->TopRadius - this->BaseRadius) * l / this->Height);
  // Square the result - we will need it in a min.
  r2 *= r2;
  // Calculate the perpendicular distance squared from the point and the cone
  // axis - this is the distance between the point and p0 squared minus the projected
  // length squared
  double dist2 = vtkMath::Dot(vec, vec) - (l*l);
  // if the perp dist is greater than the radius value we calculated then we know the point
  // is outside the cone (else its inside)
  if (dist2 > r2)
    {
    return false; // point is outside of the cone
    }
  return true;
}
//----------------------------------------------------------------------------
void vtkCMBConeCellClassifier::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
//-----------------------------------------------------------------------------

