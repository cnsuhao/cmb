//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVMeshDataInformation.h"

#include "vtkCellData.h"
#include "vtkClientServerStream.h"
#include "vtkDataSet.h"
#include "vtkCellTypes.h"
#include "vtkObjectFactory.h"
#include "vtkFieldData.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkMultiBlockWrapper.h"

vtkStandardNewMacro(vtkPVMeshDataInformation);

//----------------------------------------------------------------------------
vtkPVMeshDataInformation::vtkPVMeshDataInformation()
{
  this->CellTypes = NULL;
  this->RegionArray = NULL;
}

//----------------------------------------------------------------------------
vtkPVMeshDataInformation::~vtkPVMeshDataInformation()
{
  if(this->CellTypes)
    {
    this->CellTypes->Delete();
    }
  if(this->RegionArray)
    {
    this->RegionArray->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkPVMeshDataInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CellTypes: " << this->CellTypes << endl;
  os << indent << "RegionArray: " << this->RegionArray << endl;
}

//----------------------------------------------------------------------------
void vtkPVMeshDataInformation::CopyFromObject(vtkObject* obj)
{
  vtkDataSet *dataObject = vtkDataSet::SafeDownCast( obj );

  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
    {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast( obj );
    if (algOutput && algOutput->GetProducer())
      {
      dataObject = vtkDataSet::SafeDownCast(
        algOutput->GetProducer()->GetOutputDataObject(
        algOutput->GetIndex() ));
      }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast( obj );
    if (alg)
      {
      dataObject = vtkDataSet::SafeDownCast(
        alg->GetOutputDataObject( 0 ));
      }
    if (!dataObject)
      {
      vtkErrorMacro("Unable to get data object from input!");
      return;
      }
  }
  if(this->CellTypes)
    {
    this->CellTypes->Delete();
    }
  this->CellTypes = vtkCellTypes::New();
  dataObject->GetCellTypes(this->CellTypes);

  if(this->RegionArray)
    {
    this->RegionArray->Delete();
    this->RegionArray = NULL;
    }
  vtkDataArray* dataArray = dataObject->GetCellData()->GetArray(
    vtkMultiBlockWrapper::GetShellTagName());
  if(dataArray)
    {
    this->RegionArray = vtkIntArray::SafeDownCast(dataArray->NewInstance());
    this->RegionArray->DeepCopy(dataArray);
    }
}

//----------------------------------------------------------------------------
void vtkPVMeshDataInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVMeshDataInformation *CellTypesInfo =
    vtkPVMeshDataInformation::SafeDownCast(info);
  if (CellTypesInfo && CellTypesInfo->GetCellTypes())
    {
    if(this->CellTypes)
      {
      this->CellTypes->Delete();
      }
    this->CellTypes = CellTypesInfo->GetCellTypes()->NewInstance();
    this->CellTypes->DeepCopy(CellTypesInfo->GetCellTypes());
    }
  if(this->RegionArray)
    {
    this->RegionArray->Delete();
    this->RegionArray = NULL;
    }
  if(CellTypesInfo->GetRegionArray())
    {
    this->RegionArray = CellTypesInfo->GetRegionArray()->NewInstance();
    this->RegionArray->DeepCopy(CellTypesInfo->GetRegionArray());
    }
}
