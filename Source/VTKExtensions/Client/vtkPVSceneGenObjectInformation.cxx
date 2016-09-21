//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSceneGenObjectInformation.h"

#include "vtkTransform.h"
#include "vtkPointData.h"
#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkFieldData.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositeDataIterator.h"


vtkStandardNewMacro(vtkPVSceneGenObjectInformation);

//----------------------------------------------------------------------------
vtkPVSceneGenObjectInformation::vtkPVSceneGenObjectInformation()
{
  this->Transform = vtkTransform::New();
  this->Translation[0] = this->Translation[1] = this->Translation[2] = 0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0;
  this->Scale[0] = this->Scale[1] = this->Scale[2] = 1;
  this->Color[0] = this->Color[1] = this->Color[2] = 1;
  this->NumberOfPoints = -1;
  this->HasColorPointData = false;
}

//----------------------------------------------------------------------------
vtkPVSceneGenObjectInformation::~vtkPVSceneGenObjectInformation()
{
  this->Transform->Delete();
}

//----------------------------------------------------------------------------
void vtkPVSceneGenObjectInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->Transform->PrintSelf(os, indent);
  os << indent << "Translation: " << this->Translation[0] << ", " <<
    this->Translation[1] << ", " << this->Translation[2]  << endl;
  os << indent << "Orientation: " << this->Orientation[0] << ", " <<
    this->Orientation[1] << ", " << this->Orientation[2]  << endl;
  os << indent << "Scale: " << this->Scale[0] << ", " <<
    this->Scale[1] << ", " << this->Scale[2]  << endl;
  os << indent << "Color: " << this->Color[0] << ", " <<
    this->Color[1] << ", " << this->Color[2]  << endl;
  os << indent << "NumberOfPoints: " << this->NumberOfPoints << endl;
  os << indent << "HasColorPointData: " <<
    (this->HasColorPointData ? "true" : "false") << endl;
}

//----------------------------------------------------------------------------
void vtkPVSceneGenObjectInformation::CopyFromObject(vtkObject* obj)
{
  vtkDataObject *dataObject = vtkDataObject::SafeDownCast( obj );

  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
    {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast( obj );
    if (algOutput && algOutput->GetProducer())
      {
      dataObject = algOutput->GetProducer()->GetOutputDataObject(
        algOutput->GetIndex() );
      }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast( obj );
    if (alg)
      {
      dataObject = alg->GetOutputDataObject( 0 );
      }
    if (!dataObject)
      {
      vtkErrorMacro("Unable to get data object from object!");
      return;
      }
    }

  // if composite, grab transform from 1st block
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(dataObject);
  if (mds)
    {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    dataObject = iter->GetCurrentDataObject();
    iter->Delete();
    }

  if (vtkDataSet::SafeDownCast(dataObject))
    {
    this->NumberOfPoints = vtkDataSet::SafeDownCast(dataObject)->GetNumberOfPoints();
    }
  else
    {
    this->NumberOfPoints = -1;
    }

  vtkStringArray *objectType = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray( "ObjectType" ) );
  if (objectType)
    {
    this->ObjectType = objectType->GetValue(0);
    }
  else
    {
    this->ObjectType = "MissingField";
    }

  vtkStringArray *objectName = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray( "ObjectName" ) );
  if (objectName)
    {
    this->ObjectName = objectName->GetValue(0);
    }
  else
    {
    this->ObjectName = "MissingField";
    }

  vtkDoubleArray *translation = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray( "Translation" ) );
  if (translation)
    {
    translation->GetTuple(0, this->Translation);
    }

  vtkDoubleArray *orientation = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray( "Rotation" ) );
  if (orientation)
    {
    orientation->GetTuple(0, this->Orientation);
    }

  vtkDoubleArray *scale = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray( "Scale" ) );
  if (scale)
    {
    scale->GetTuple(0, this->Scale);
    }

  if (this->ObjectType == "RegionOfInterest")
    {
    return;
    }

  vtkStringArray *objectFileName = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray( "FileName" ) );
  if (objectFileName)
    {
    this->ObjectFileName = objectFileName->GetValue(0);
    }
  else
    {
    this->ObjectFileName = "MissingField";
    }

  // if we have a transformation, use it... otherwise set to identity
  vtkDoubleArray *transformation = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray( "Transformation" ) );
  if (transformation)
    {
    double elements[16];
    transformation->GetTuple(0, elements);
    this->Transform->SetMatrix( elements );
    }
  else
    {
    this->Transform->Identity();
    }

  vtkDoubleArray *color = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray( "Color" ) );
  if (color)
    {
    color->GetTuple(0, this->Color);
    }

  if (vtkPolyData::SafeDownCast(dataObject) &&
    vtkPolyData::SafeDownCast(dataObject)->GetPointData()->GetArray("Color"))
    {
    this->HasColorPointData = true;
    }
  else
    {
    this->HasColorPointData = false;
    }
  }

//----------------------------------------------------------------------------
void vtkPVSceneGenObjectInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVSceneGenObjectInformation *sceneGenInfo =
    vtkPVSceneGenObjectInformation::SafeDownCast(info);
  if (sceneGenInfo)
    {
    this->Transform->DeepCopy( sceneGenInfo->GetTransform() );
    sceneGenInfo->GetTranslation(this->Translation);
    sceneGenInfo->GetOrientation(this->Orientation);
    sceneGenInfo->GetScale(this->Scale);
    sceneGenInfo->GetColor(this->Color);
    this->NumberOfPoints = sceneGenInfo->GetNumberOfPoints();
    this->ObjectType = sceneGenInfo->GetObjectType();
    this->HasColorPointData = sceneGenInfo->GetHasColorPointData();
    }
}

//----------------------------------------------------------------------------
void
vtkPVSceneGenObjectInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  vtkMatrix4x4 *matrix = this->Transform->GetMatrix();
  double transformData[16];
  vtkMatrix4x4::DeepCopy(transformData, matrix);
  *css << vtkClientServerStream::InsertArray(transformData, 16) <<
    vtkClientServerStream::InsertArray(this->Translation, 3) <<
    vtkClientServerStream::InsertArray(this->Orientation, 3) <<
    vtkClientServerStream::InsertArray(this->Scale, 3) <<
    vtkClientServerStream::InsertArray(this->Color, 3) <<
    this->NumberOfPoints <<
    this->ObjectType.c_str() <<
    this->HasColorPointData <<
    vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void
vtkPVSceneGenObjectInformation::CopyFromStream(const vtkClientServerStream* css)
{
  double elements[16];
  css->GetArgument(0, 0, elements, 16);
  this->Transform->SetMatrix( elements );
  css->GetArgument(0, 1, this->Translation, 3);
  css->GetArgument(0, 2, this->Orientation, 3);
  css->GetArgument(0, 3, this->Scale, 3);
  css->GetArgument(0, 4, this->Color, 3);
  css->GetArgument(0, 5, &this->NumberOfPoints);
  char buffer[64];
  css->GetArgument(0, 6, buffer);
  this->ObjectType = buffer;
  css->GetArgument(0, 7, &this->HasColorPointData);
}
