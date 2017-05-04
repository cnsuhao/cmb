//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkAddCellDataFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkIntArray.h"

vtkStandardNewMacro(vtkAddCellDataFilter);

vtkAddCellDataFilter::vtkAddCellDataFilter()
{
}

vtkAddCellDataFilter::~vtkAddCellDataFilter()
{
}

int vtkAddCellDataFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet* input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet* output = vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->ShallowCopy(input);

  vtkCellData* cellData = output->GetCellData();
  vtkIntArray* regionIDs =
    vtkIntArray::SafeDownCast(cellData->GetArray(vtkMultiBlockWrapper::GetShellTagName()));
  if (!regionIDs) // intialize to be a single region
  {
    regionIDs = vtkIntArray::New();
    regionIDs->SetNumberOfTuples(output->GetNumberOfCells());
    regionIDs->SetName(vtkMultiBlockWrapper::GetShellTagName());
    for (int i = 0; i < regionIDs->GetNumberOfTuples(); i++)
    {
      regionIDs->SetValue(i, 0);
    }
    cellData->AddArray(regionIDs);
    regionIDs->Delete();
  }

  // if "Material" not present, copy Region
  vtkIntArray* materialIDs =
    vtkIntArray::SafeDownCast(cellData->GetArray(vtkMultiBlockWrapper::GetMaterialTagName()));
  if (!materialIDs)
  {
    materialIDs = vtkIntArray::New();
    materialIDs->DeepCopy(regionIDs);
    materialIDs->SetName(vtkMultiBlockWrapper::GetMaterialTagName());
    cellData->AddArray(materialIDs);
    materialIDs->Delete();
  }

  // if "Model Face" not present, copy Region
  vtkIntArray* modelFaceIDs =
    vtkIntArray::SafeDownCast(cellData->GetArray(vtkMultiBlockWrapper::GetModelFaceTagName()));
  if (!modelFaceIDs)
  {
    modelFaceIDs = vtkIntArray::New();
    modelFaceIDs->DeepCopy(regionIDs);
    modelFaceIDs->SetName(vtkMultiBlockWrapper::GetModelFaceTagName());
    cellData->AddArray(modelFaceIDs);
    modelFaceIDs->Delete();
  }

  return 1;
}

void vtkAddCellDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
