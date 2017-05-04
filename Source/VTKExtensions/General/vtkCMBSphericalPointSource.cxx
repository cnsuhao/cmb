//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBSphericalPointSource.h"

#include "vtkCellArray.h"
#include "vtkCellLocator.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include <math.h>

vtkStandardNewMacro(vtkCMBSphericalPointSource);

// Construct with defaults
vtkCMBSphericalPointSource::vtkCMBSphericalPointSource()
{
  this->Radius = 0.0;
  this->RResolution = 1;
  this->ThetaResolution = 1;
  this->PhiResolution = 1;
  this->IgnorePhi = false;
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
  this->SetNumberOfInputPorts(0);
}

int vtkCMBSphericalPointSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Determine the number of points we will be evaluating as well as the deltas
  int size;
  double deltaR, deltaTheta, deltaPhi;
  const double PI = 3.141592654;
  deltaTheta = 2.0 * PI / static_cast<double>(this->ThetaResolution);
  deltaPhi = PI / static_cast<double>(this->PhiResolution - 1);
  deltaR = this->Radius;

  if (this->IgnorePhi) // Evaluating a disk
  {
    if ((this->Radius == 0.0) || ((this->RResolution == 1) && (this->ThetaResolution == 1)))
    {
      size = 1;
    }
    else
    {
      if (this->RResolution > 1)
      {
        size = ((this->RResolution - 1) * this->ThetaResolution) + 1;
        deltaR = this->Radius / static_cast<double>(this->RResolution - 1);
      }
      else
      {
        size = this->ThetaResolution;
      }
    }
  }
  else // Evaluating a Sphere
  {
    if (this->RResolution > 1)
    {
      // We need to add points along the the z axis which is why there is a + 2 as well as the
      // single point at the origin (the reason for the +1)
      size = (this->RResolution - 1) * (this->ThetaResolution * (this->PhiResolution - 2) + 2) + 1;
      deltaR = this->Radius / static_cast<double>(this->RResolution - 1);
    }
    else
    {
      size = (this->ThetaResolution * (this->PhiResolution - 2)) + 2;
    }
  }

  // Lets allocate the arrays
  vtkSmartPointer<vtkPoints> newPoints;
  newPoints = vtkSmartPointer<vtkPoints>::New();
  newPoints->SetDataTypeToDouble(); //used later during transformation
  newPoints->Allocate(size);
  output->SetPoints(newPoints);
  vtkIdType nextPt;
  vtkSmartPointer<vtkCellArray> outVerts = vtkSmartPointer<vtkCellArray>::New();
  output->SetVerts(outVerts);

  // Populate the data
  if (this->Radius == 0.0)
  {
    // Just test the center Point
    nextPt = newPoints->InsertNextPoint(this->Center);
    outVerts->InsertNextCell(1, &nextPt);
    return 1;
  }

  double theta, r, phi, p[3];
  int iTheta, iPhi, iR = 0;

  // Insert Center if needed
  if (this->RResolution > 1)
  {
    nextPt = newPoints->InsertNextPoint(this->Center);
    outVerts->InsertNextCell(1, &nextPt);
    iR = 1;
  }
  if (this->IgnorePhi)
  {
    p[2] = this->Center[2];
    for (r = deltaR; iR < this->RResolution; iR++, r += deltaR)
    {
      for (theta = 0.0, iTheta = 0; iTheta < this->ThetaResolution; iTheta++, theta += deltaTheta)
      {
        p[0] = this->Center[0] + (r * cos(theta));
        p[1] = this->Center[1] + (r * sin(theta));
        nextPt = newPoints->InsertNextPoint(p);
        outVerts->InsertNextCell(1, &nextPt);
      }
    }
  }
  else
  {
    // First insert the points along the Phi = 0 axis
    p[0] = this->Center[0];
    p[1] = this->Center[1];
    //Are we inserting points in the r direction?
    if (iR == 1)
    {
      for (r = deltaR; iR < this->RResolution; iR++, r += deltaR)
      {
        p[2] = this->Center[2] + r;
        nextPt = newPoints->InsertNextPoint(p);
        outVerts->InsertNextCell(1, &nextPt);
        p[2] = this->Center[2] - r;
        nextPt = newPoints->InsertNextPoint(p);
        outVerts->InsertNextCell(1, &nextPt);
      }
      iR = 1;
    }
    else
    {
      // Just insert points at the poles
      p[2] = this->Center[2] + this->Radius;
      nextPt = newPoints->InsertNextPoint(p);
      outVerts->InsertNextCell(1, &nextPt);
      p[2] = this->Center[2] - this->Radius;
      nextPt = newPoints->InsertNextPoint(p);
      outVerts->InsertNextCell(1, &nextPt);
    }
    double sphi;
    for (r = deltaR; iR < this->RResolution; iR++, r += deltaR)
    {
      for (phi = deltaPhi, iPhi = 2; iPhi < this->PhiResolution; iPhi++, phi += deltaPhi)
      {
        p[2] = this->Center[2] + (r * cos(phi));
        sphi = (r * sin(phi));

        for (theta = 0.0, iTheta = 0; iTheta < this->ThetaResolution; iTheta++, theta += deltaTheta)
        {
          p[0] = this->Center[0] + (sphi * cos(theta));
          p[1] = this->Center[1] + (sphi * sin(theta));
          nextPt = newPoints->InsertNextPoint(p);
          outVerts->InsertNextCell(1, &nextPt);
        }
      }
    }
  }
  return 1;
}

void vtkCMBSphericalPointSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "RResolution: " << this->RResolution << "\n";
  os << indent << "ThetaResolution: " << this->ThetaResolution << "\n";
  os << indent << "PhiResolution: " << this->PhiResolution << "\n";
  os << indent << "IgnorePhi: " << (this->IgnorePhi ? "On\n" : "Off\n");
  os << indent << "Center: (" << this->Center[0] << ", " << this->Center[1] << ", "
     << this->Center[2] << ")\n";
}
