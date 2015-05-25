//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBClassifyPointsFilter.h"

#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkCellLocator.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include <math.h>

vtkStandardNewMacro(vtkCMBClassifyPointsFilter);

//----------------------------------------------------------------------------
// Construct with defaults
vtkCMBClassifyPointsFilter::vtkCMBClassifyPointsFilter()
{
  this->SetNumberOfInputPorts(2);
}

//----------------------------------------------------------------------------
void vtkCMBClassifyPointsFilter::SetSolidConnection(vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

//----------------------------------------------------------------------------
int vtkCMBClassifyPointsFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBClassifyPointsFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *meshInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDataSet *mesh = vtkDataSet::SafeDownCast(
    meshInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Create a Cell Locator
  vtkSmartPointer<vtkCellLocator> locator =
    vtkSmartPointer<vtkCellLocator>::New();
  locator->SetDataSet(mesh);
  locator->BuildLocator();

  vtkIdType i, n = input->GetNumberOfPoints();
  double p[3];

  // Lets allocate the arrays
  vtkSmartPointer<vtkPoints> newPoints;
  newPoints = vtkSmartPointer<vtkPoints>::New();
  newPoints->SetDataTypeToDouble();
  newPoints->Allocate(n);
  output->SetPoints(newPoints);
  vtkSmartPointer<vtkIdTypeArray> ids;
  ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfComponents(1);
  ids->Allocate(n);
  ids->SetName("GridCellIds");
  vtkPointData *pdata = output->GetPointData();
  pdata->SetScalars(ids);

  vtkIdType cellId;

  for (i = 0; i < n; i++)
    {
    input->GetPoint(i, p);
    cellId = locator->FindCell(p);
    if (cellId == -1)
      {
      continue;
      }
    newPoints->InsertNextPoint(p);
    ids->InsertNextTupleValue(&cellId);
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBClassifyPointsFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
