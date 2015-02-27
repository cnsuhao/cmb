/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkClosestPointFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkClosestPointFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkErrorCode.h"
#include "vtkTransform.h"
#include "vtkCellLocator.h"
#
#include <sys/types.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkClosestPointFilter);

//-----------------------------------------------------------------------------
vtkClosestPointFilter::vtkClosestPointFilter()
{
  this->Translation[0] = this->Translation[1] = this->Translation[2] = 0.0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0.0;
  this->Scale[0] = this->Scale[1] = this->Scale[2] = 1.0;
  this->Transform = vtkTransform::New();
  this->Locator = vtkCellLocator::New();
  this->SetNumberOfInputPorts(1);
  this->PointMode = true;
  this->TestPoint[0] = this->TestPoint[1] = this->TestPoint[2] = 0.0;
  this->ClosestPoint[0] = this->ClosestPoint[1] = this->ClosestPoint[2] = 0.0;
  this->TransformInverse = NULL;
}

//-----------------------------------------------------------------------------
vtkClosestPointFilter::~vtkClosestPointFilter()
{
  this->Locator->Delete();
  this->Transform->Delete();
}

//-----------------------------------------------------------------------------
int vtkClosestPointFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *vtkNotUsed(outputVector))
{
  // get the info and input data
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->Locator->SetDataSet(input);
  this->Locator->BuildLocator();

//   // See if we need to update the transform
//   if (this->BuildTime < this->MTime)
//     {
    this->Transform->Identity();
    this->Transform->PreMultiply();
    this->Transform->Translate(this->Translation);
    this->Transform->RotateZ(this->Orientation[2]);
    this->Transform->RotateX(this->Orientation[0]);
    this->Transform->RotateY(this->Orientation[1]);
    this->Transform->Scale(this->Scale[0], this->Scale[1], this->Scale[2]);

    this->TransformInverse = this->Transform->GetInverse();
    this->BuildTime.Modified();
//    }

  double tp[3], ctp[3];
  int subId;
  double d2;
  vtkIdType cellId;

  if (this->PointMode)
    {
    // Transform the test point to the data's coordinate system
    this->TransformInverse->TransformPoint(this->TestPoint, tp);

    // Now calculate the closest point
    this->Locator->FindClosestPoint(tp, ctp, cellId, subId, d2);
    }
  else
    {
    double tp2[3], t, pcoords[3];
    // Transform the test line to the data's coordinate system
    this->TransformInverse->TransformPoint(this->TestLine, tp);
    this->TransformInverse->TransformPoint(&(this->TestLine[3]), tp2);

    // Now calculate the closest point
    if (!this->Locator->IntersectWithLine(tp, tp2, 0.0, t, ctp, pcoords, subId))
      {
      // Well the intersection failed try closestpoint
      this->Locator->FindClosestPoint(tp, ctp, cellId, subId, d2);
      }
    }

  // Transform the point back to world space
  this->Transform->TransformPoint(ctp, this->ClosestPoint);

  return VTK_OK;
}


//-----------------------------------------------------------------------------
void vtkClosestPointFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->PointMode)
    {
    os << indent << "Test Point: (" << this->TestPoint[0] << ", "
       << this->TestPoint[1] << ", "  << this->TestPoint[2] << ")\n";
    os << indent << "Test Line: (Not Set)\n";
    }
  else
    {
    os << indent << "Test Line: ((" << this->TestLine[0] << ", "
       << this->TestLine[1] << ", "  << this->TestLine[2] << ") ("
       << this->TestLine[3] << ", " << this->TestLine[4] << ", "
       << this->TestLine[5]<< "))\n";
    os << indent << "Test Point: (Not Set)\n";
    }

  os << indent << "Closest Point: (" << this->ClosestPoint[0] << ", "
     << this->ClosestPoint[1] << ", "  << this->ClosestPoint[2] << ")\n";
  os << indent << "Translation: (" << this->Translation[0] << ", "
     << this->Translation[1] << ", "  << this->Translation[2] << ")\n";
  os << indent << "Orientation: (" << this->Orientation[0] << ", "
     << this->Orientation[1] << ", "  << this->Orientation[2] << ")\n";
  os << indent << "Scale: (" << this->Scale[0] << ", "
     << this->Scale[1] << ", "  << this->Scale[2] << ")\n";
}


//----------------------------------------------------------------------------
