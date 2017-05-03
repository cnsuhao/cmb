//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVModelVertexObjectInformation.h"

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkClientServerStream.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkPVModelVertexObjectInformation);

vtkPVModelVertexObjectInformation::vtkPVModelVertexObjectInformation()
{
  this->Location[0] = this->Location[1] = this->Location[2] = 0;
  this->IsInfoValid = 0;
  this->PointId = -1;
}

vtkPVModelVertexObjectInformation::~vtkPVModelVertexObjectInformation()
{
}

void vtkPVModelVertexObjectInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Location: " << this->Location[0] << ", " << this->Location[1] << ", "
     << this->Location[2] << endl;
  os << indent << "IsInfoValid: " << this->IsInfoValid << endl;
  os << indent << "PointId: " << this->PointId << endl;
}

void vtkPVModelVertexObjectInformation::CopyFromObject(vtkObject* obj)
{
  vtkDataObject* dataObject = vtkDataObject::SafeDownCast(obj);
  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
  {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast(obj);
    if (algOutput && algOutput->GetProducer())
    {
      dataObject = algOutput->GetProducer()->GetOutputDataObject(algOutput->GetIndex());
    }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast(obj);
    if (alg)
    {
      dataObject = alg->GetOutputDataObject(0);
    }
    if (!dataObject)
    {
      vtkErrorMacro("Unable to get data object from object!");
      return;
    }
  }
  if (vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataObject))
  {
    if (polyData->GetNumberOfCells() != 1 || polyData->GetCellType(0) != VTK_VERTEX)
    {
      vtkErrorMacro("Wrong vtkPolyData to get information from.");
      return;
    }
    vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
    polyData->GetCellPoints(0, pointIds);
    this->PointId = pointIds->GetId(0);
    polyData->GetPoint(this->PointId, this->Location);
    this->IsInfoValid = 1;
    return;
  }
  vtkErrorMacro("Data object is not a vtkPolyData!");
  return;
}

void vtkPVModelVertexObjectInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVModelVertexObjectInformation* modelVertexInfo =
    vtkPVModelVertexObjectInformation::SafeDownCast(info);
  if (modelVertexInfo)
  {
    modelVertexInfo->GetLocation(this->Location);
    this->PointId = modelVertexInfo->GetPointId();
  }
}

void vtkPVModelVertexObjectInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << vtkClientServerStream::InsertArray(this->Location, 3) << this->PointId
       << vtkClientServerStream::End;
}

void vtkPVModelVertexObjectInformation::CopyFromStream(const vtkClientServerStream* css)
{
  css->GetArgument(0, 0, this->Location, 3);
  css->GetArgument(0, 1, &this->PointId);
}
