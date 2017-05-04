//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVLASOutputBlockInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"
//#include "vtkCompositeDataSet.h"
//#include "vtkCompositeDataIterator.h"

vtkStandardNewMacro(vtkPVLASOutputBlockInformation);

//----------------------------------------------------------------------------
vtkPVLASOutputBlockInformation::vtkPVLASOutputBlockInformation()
{
  this->NumberOfPoints = -1;
  this->NumberOfPointsInClassification = -1;
  this->Classification = 255;
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = 1;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -1;
}

//----------------------------------------------------------------------------
vtkPVLASOutputBlockInformation::~vtkPVLASOutputBlockInformation()
{
}

//----------------------------------------------------------------------------
void vtkPVLASOutputBlockInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfPoints: " << this->NumberOfPoints << endl;
  os << indent << "Bounds: " << this->Bounds[0] << ", " << this->Bounds[1] << ", "
     << this->Bounds[2] << ", " << this->Bounds[3] << ", " << this->Bounds[4] << ", "
     << this->Bounds[5] << endl;
  os << indent << "NumberOfPointsInClassification: " << this->NumberOfPointsInClassification
     << endl;
  os << indent << "Classification: " << this->Classification << endl;
  os << indent << "ClassificationName: " << this->ClassificationName << endl;
}

//----------------------------------------------------------------------------
void vtkPVLASOutputBlockInformation::CopyFromObject(vtkObject* obj)
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

  vtkIdTypeArray* pointsInClassification = vtkIdTypeArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray("NumberOfPointsInClassification"));
  if (pointsInClassification)
  {
    this->NumberOfPointsInClassification = pointsInClassification->GetValue(0);
  }
  else
  {
    this->NumberOfPointsInClassification = -1;
  }

  if (vtkDataSet::SafeDownCast(dataObject))
  {
    this->NumberOfPoints = vtkDataSet::SafeDownCast(dataObject)->GetNumberOfPoints();
    vtkDataSet::SafeDownCast(dataObject)->GetBounds(this->Bounds);
  }
  else
  {
    this->NumberOfPoints = -1;
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = 1;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -1;
  }

  vtkUnsignedCharArray* classification =
    vtkUnsignedCharArray::SafeDownCast(dataObject->GetFieldData()->GetArray("Classification"));
  if (classification)
  {
    this->Classification = classification->GetValue(0);
  }
  else
  {
    // 255 is currently unused value; actually, as currently saving the data
    // we only save tha value from btis 0-4, so anything above 31 is unused
    this->Classification = 255;
  }

  vtkStringArray* classificationName = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray("ClassificationName"));
  if (classificationName)
  {
    this->ClassificationName = classificationName->GetValue(0);
  }
  else
  {
    this->ClassificationName = "MissingField";
  }
}

//----------------------------------------------------------------------------
void vtkPVLASOutputBlockInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVLASOutputBlockInformation* outputBlockInfo =
    vtkPVLASOutputBlockInformation::SafeDownCast(info);
  if (outputBlockInfo)
  {
    this->NumberOfPoints = outputBlockInfo->GetNumberOfPoints();
    outputBlockInfo->GetBounds(this->Bounds);
    this->NumberOfPointsInClassification = outputBlockInfo->GetNumberOfPointsInClassification();
    this->Classification = outputBlockInfo->GetClassification();
    this->ClassificationName = outputBlockInfo->GetClassificationName();
  }
}

//----------------------------------------------------------------------------
void vtkPVLASOutputBlockInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << this->NumberOfPoints << vtkClientServerStream::InsertArray(this->Bounds, 6)
       << this->NumberOfPointsInClassification << this->Classification
       << this->ClassificationName.c_str() << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void vtkPVLASOutputBlockInformation::CopyFromStream(const vtkClientServerStream* css)
{
  css->GetArgument(0, 0, &this->NumberOfPoints);
  css->GetArgument(0, 1, this->Bounds, 6);
  css->GetArgument(0, 2, &this->NumberOfPointsInClassification);
  css->GetArgument(0, 3, &this->Classification);
  char buffer[64];
  css->GetArgument(0, 4, buffer);
  this->ClassificationName = buffer;
}
