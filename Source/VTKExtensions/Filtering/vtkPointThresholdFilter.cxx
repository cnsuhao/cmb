//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPointThresholdFilter.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkPointThresholdFilter);
vtkCxxSetObjectMacro(vtkPointThresholdFilter, Transform, vtkTransform);

//--------------------------------------------------------------------
vtkPointThresholdFilter::vtkPointThresholdFilter()
{
  this->ActiveFilterIndex = -1;
  this->Transform = 0;
  this->TransformOutputData = false;
}
//--------------------------------------------------------------------
vtkPointThresholdFilter::~vtkPointThresholdFilter()
{
  FilterList.clear();
  this->SetTransform(static_cast<vtkTransform*>(0));
}
//--------------------------------------------------------------------
void vtkPointThresholdFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkPointThresholdFilter::SetTransform(double elements[16])
{
  vtkTransform* tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}

//--------------------------------------------------------------------
int vtkPointThresholdFilter::RequestData(vtkInformation* /*request*/,
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]);
  if (input == 0)
  {
    vtkErrorMacro("Must set Input!");
    return 0;
  }

  //Setup Output
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  bool IsAnyFilterActive = false;

  for (std::vector<PointThreshold*>::iterator it = FilterList.begin(); it != FilterList.end(); ++it)
  {
    //If this filter won't do anything then the input is the output
    if (((*it)->UseMinX || (*it)->UseMinY || (*it)->UseMinZ || (*it)->UseMaxX || (*it)->UseMaxY ||
          (*it)->UseMaxZ || (*it)->UseMinRGB || (*it)->UseMaxRGB || (*it)->Invert) &&
      (*it)->UseFilter)
    {
      IsAnyFilterActive = true;
      break;
    }
  }
  if (!IsAnyFilterActive)
  {
    output->ShallowCopy(input);
    return 1;
  }

  vtkPoints* newPoints = vtkPoints::New();
  vtkCellArray* newVerts = vtkCellArray::New();

  newPoints->SetDataTypeToFloat();

  output->SetPoints(newPoints);
  output->SetVerts(newVerts);

  newPoints->Allocate(input->GetPoints()->GetNumberOfPoints());
  newVerts->Allocate(input->GetPoints()->GetNumberOfPoints());

  output->GetPointData()->CopyAllocate(input->GetPointData());

  newPoints->UnRegister(this);
  newVerts->UnRegister(this);

  //Populate Output
  vtkPoints* points = input->GetPoints();
  vtkUnsignedCharArray* scalars =
    static_cast<vtkUnsignedCharArray*>(input->GetPointData()->GetScalars("Color"));

  unsigned char* color = 0;
  if (scalars)
  {
    color = scalars->GetPointer(0);
  }

  unsigned newNumPoints = 0;
  double transformedPoint[3], *pointPtr;
  for (int i = 0; i < input->GetPoints()->GetNumberOfPoints(); ++i)
  {
    double rgb[3] = { 0, 0, 0 };
    double point[3];

    if (color)
    {
      rgb[0] = static_cast<double>(color[(i * 3)]);
      rgb[1] = static_cast<double>(color[(i * 3) + 1]);
      rgb[2] = static_cast<double>(color[(i * 3) + 2]);
    }

    points->GetPoint(i, point);

    if (this->Transform)
    {
      this->Transform->TransformPoint(point, transformedPoint);
      pointPtr = transformedPoint;
    }
    else
    {
      pointPtr = point;
    }

    bool pointIsIn = true;
    //Iterate over all filters and if it fails one the point is not in
    for (std::vector<PointThreshold*>::iterator it = FilterList.begin(); it != FilterList.end();
         ++it)
    {
      //Skip disabled filters
      if (!(*it)->UseFilter)
      {
        continue;
      }
      //Filter points to add
      if (((*it)->UseMinX && (pointPtr[0] < (*it)->MinX)) ||
        ((*it)->UseMinY && (pointPtr[1] < (*it)->MinY)) ||
        ((*it)->UseMinZ && (pointPtr[2] < (*it)->MinZ)) ||
        ((*it)->UseMaxX && (pointPtr[0] > (*it)->MaxX)) ||
        ((*it)->UseMaxY && (pointPtr[1] > (*it)->MaxY)) ||
        ((*it)->UseMaxZ && (pointPtr[2] > (*it)->MaxZ)))
      {
        pointIsIn = false;
      }

      if (color &&
        (((*it)->UseMinRGB && (static_cast</*REPLACED*/ int>(rgb[0]) < (*it)->MinRGB[0] ||
                                static_cast</*REPLACED*/ int>(rgb[1]) < (*it)->MinRGB[1] ||
                                static_cast</*REPLACED*/ int>(rgb[2]) < (*it)->MinRGB[2])) ||
            ((*it)->UseMaxRGB && (static_cast</*REPLACED*/ int>(rgb[0]) > (*it)->MaxRGB[0] ||
                                   static_cast</*REPLACED*/ int>(rgb[1]) > (*it)->MaxRGB[1] ||
                                   static_cast</*REPLACED*/ int>(rgb[2]) > (*it)->MaxRGB[2]))))
      {
        pointIsIn = false;
      }

      if ((*it)->Invert)
      {
        pointIsIn = !pointIsIn;
      }
      if (!pointIsIn)
      {
        break;
      }
    }
    if (!pointIsIn)
    {
      continue;
    }

    vtkIdType idx;
    if (this->Transform && this->TransformOutputData)
    {
      idx = newPoints->InsertNextPoint(transformedPoint);
    }
    else
    {
      idx = newPoints->InsertNextPoint(point);
    }
    newVerts->InsertNextCell(1, &idx);
    output->GetPointData()->CopyData(input->GetPointData(), i, newNumPoints++);
  }

  newVerts->Squeeze();
  newPoints->Squeeze();
  output->GetPointData()->Squeeze();
  return 1;
}
