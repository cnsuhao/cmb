//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBScalarLineSource.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkCMBScalarLineSource);

vtkCMBScalarLineSource::vtkCMBScalarLineSource()
{
  this->Point1[0] = this->Point1[1] = 0.0;
  this->Point1[2] = 0.0;

  this->Point2[0] = this->Point2[1] = 0.0;
  this->Point2[2] = 1.0;

  this->Scalar1 = this->Scalar2 = 1.0;
  this->SetNumberOfInputPorts(0);
}

vtkCMBScalarLineSource::~vtkCMBScalarLineSource()
{
}

int vtkCMBScalarLineSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the info object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints* newPoints;
  vtkDoubleArray* newData;
  vtkCellArray* newLines;

  if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
  {
    return 1;
  }

  newPoints = vtkPoints::New();
  newPoints->SetNumberOfPoints(2);
  newData = vtkDoubleArray::New();
  newData->SetNumberOfValues(2);
  newData->SetName("Scalar Data");
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(1, 2));

  newPoints->SetPoint(0, this->Point1);
  newPoints->SetPoint(1, this->Point2);
  newData->SetValue(0, this->Scalar1);
  newData->SetValue(1, this->Scalar2);
  newLines->InsertNextCell(2);
  newLines->InsertCellPoint(0);
  newLines->InsertCellPoint(1);
  //
  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  // Set newData to be the active scalar array
  output->GetPointData()->AddArray(newData);
  output->GetPointData()->SetActiveScalars("Scalar Data");
  newData->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  return 1;
}

void vtkCMBScalarLineSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Point1: " << this->Point1[0] << ", " << this->Point1[1] << ", "
     << this->Point1[2] << "\n";
  os << indent << "Point2: " << this->Point2[0] << ", " << this->Point2[1] << ", "
     << this->Point2[2] << "\n";
  os << indent << "Scalar1: " << this->Scalar1 << "\n";
  os << indent << "Scalar2: " << this->Scalar2 << "\n";
}
