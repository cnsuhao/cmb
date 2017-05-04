//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVContourGroupInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCellData.h"
#include "vtkClientServerStream.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkPVContourGroupInformation);

vtkPVContourGroupInformation::vtkPVContourGroupInformation()
{
  this->ProjectionPositionArray = NULL;
  this->ProjectionPlaneArray = NULL;
}

vtkPVContourGroupInformation::~vtkPVContourGroupInformation()
{
  if (this->ProjectionPositionArray)
  {
    this->ProjectionPositionArray->Delete();
  }
  if (this->ProjectionPlaneArray)
  {
    this->ProjectionPlaneArray->Delete();
  }
}

void vtkPVContourGroupInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ProjectionPositionArray: " << this->ProjectionPositionArray << endl;
  os << indent << "ProjectionPlaneArray: " << this->ProjectionPlaneArray << endl;
}

void vtkPVContourGroupInformation::CopyFromObject(vtkObject* obj)
{
  vtkDataSet* dataObject = vtkDataSet::SafeDownCast(obj);

  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
  {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast(obj);
    if (algOutput && algOutput->GetProducer())
    {
      dataObject = vtkDataSet::SafeDownCast(
        algOutput->GetProducer()->GetOutputDataObject(algOutput->GetIndex()));
    }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast(obj);
    if (alg)
    {
      dataObject = vtkDataSet::SafeDownCast(alg->GetOutputDataObject(0));
    }
    if (!dataObject)
    {
      vtkErrorMacro("Unable to get data object from input!");
      return;
    }
  }
  if (this->ProjectionPositionArray)
  {
    this->ProjectionPositionArray->Delete();
  }
  vtkDataArray* posArray = dataObject->GetFieldData()->GetArray("ProjectionPosition");
  if (posArray)
  {
    this->ProjectionPositionArray = vtkDoubleArray::SafeDownCast(posArray->NewInstance());
    this->ProjectionPositionArray->DeepCopy(posArray);
  }

  if (this->ProjectionPlaneArray)
  {
    this->ProjectionPlaneArray->Delete();
    this->ProjectionPlaneArray = NULL;
  }

  vtkDataArray* planeArray = dataObject->GetFieldData()->GetArray("ProjectionNormal");
  if (planeArray)
  {
    this->ProjectionPlaneArray = vtkIntArray::SafeDownCast(planeArray->NewInstance());
    this->ProjectionPlaneArray->DeepCopy(planeArray);
  }
}

void vtkPVContourGroupInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVContourGroupInformation* ContourArrayInfo = vtkPVContourGroupInformation::SafeDownCast(info);
  if (ContourArrayInfo && ContourArrayInfo->GetProjectionPositionArray())
  {
    if (this->ProjectionPositionArray)
    {
      this->ProjectionPositionArray->Delete();
    }
    this->ProjectionPositionArray = ContourArrayInfo->GetProjectionPositionArray()->NewInstance();
    this->ProjectionPositionArray->DeepCopy(ContourArrayInfo->GetProjectionPositionArray());
  }
  if (this->ProjectionPlaneArray)
  {
    this->ProjectionPlaneArray->Delete();
    this->ProjectionPlaneArray = NULL;
  }
  if (ContourArrayInfo->GetProjectionPlaneArray())
  {
    this->ProjectionPlaneArray = ContourArrayInfo->GetProjectionPlaneArray()->NewInstance();
    this->ProjectionPlaneArray->DeepCopy(ContourArrayInfo->GetProjectionPlaneArray());
  }
}
