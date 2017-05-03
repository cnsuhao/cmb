//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkAddMeshDataArrayFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkAddMeshDataArrayFilter);

//----------------------------------------------------------------------------
vtkAddMeshDataArrayFilter::vtkAddMeshDataArrayFilter()
{
}

//----------------------------------------------------------------------------
vtkAddMeshDataArrayFilter::~vtkAddMeshDataArrayFilter()
{
}

//----------------------------------------------------------------------------
int vtkAddMeshDataArrayFilter::RequestData(vtkInformation* vtkNotUsed(request),
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
    regionIDs->SetNumberOfComponents(1);
    regionIDs->SetNumberOfTuples(input->GetNumberOfCells());
    regionIDs->SetName(vtkMultiBlockWrapper::GetShellTagName());
    for (vtkIdType i = 0; i < regionIDs->GetNumberOfTuples(); i++)
    {
      regionIDs->SetValue(i, 0);
    }
    cellData->AddArray(regionIDs);
    regionIDs->Delete();
  }

  // Get the original "Cell ID" array
  vtkIdTypeArray* meshCellIdArray =
    vtkIdTypeArray::SafeDownCast(input->GetCellData()->GetArray("Mesh Cell ID"));
  if (!meshCellIdArray)
  {
    meshCellIdArray = vtkIdTypeArray::New();
    meshCellIdArray->SetNumberOfComponents(1);
    meshCellIdArray->SetNumberOfTuples(input->GetNumberOfCells());
    meshCellIdArray->SetName("Mesh Cell ID");
    for (vtkIdType i = 0; i < meshCellIdArray->GetNumberOfTuples(); i++)
    {
      meshCellIdArray->SetValue(i, i);
    }
    cellData->AddArray(meshCellIdArray);
    meshCellIdArray->Delete();
  }

  vtkIdTypeArray* meshNodeIdArray =
    vtkIdTypeArray::SafeDownCast(input->GetPointData()->GetArray("Mesh Node ID"));
  if (!meshNodeIdArray)
  {
    meshNodeIdArray = vtkIdTypeArray::New();
    meshNodeIdArray->SetNumberOfComponents(1);
    meshNodeIdArray->SetNumberOfTuples(input->GetNumberOfPoints());
    meshNodeIdArray->SetName("Mesh Node ID");
    for (vtkIdType i = 0; i < meshNodeIdArray->GetNumberOfTuples(); i++)
    {
      meshNodeIdArray->SetValue(i, i);
    }
    output->GetPointData()->AddArray(meshNodeIdArray);
    meshNodeIdArray->Delete();
  }

  return 1;
}

//----------------------------------------------------------------------------
void vtkAddMeshDataArrayFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
