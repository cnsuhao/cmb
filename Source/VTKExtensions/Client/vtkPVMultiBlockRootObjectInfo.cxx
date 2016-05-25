//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVMultiBlockRootObjectInfo.h"

#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkStringArray.h"
#include "vtkFieldData.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositeDataIterator.h"

#include "vtkIntArray.h"
#include "vtkMultiBlockWrapper.h"

vtkStandardNewMacro(vtkPVMultiBlockRootObjectInfo);

//----------------------------------------------------------------------------
vtkPVMultiBlockRootObjectInfo::vtkPVMultiBlockRootObjectInfo()
{
  this->MaterialNames = NULL;
  this->ShellNames = NULL;
  this->FaceNames = NULL;
  this->BCNames = NULL;

  this->ShellColors = NULL;
  this->MaterialColors  = NULL;
  this->ModelFaceColors = NULL;
  this->BCColors = NULL;
  this->ShellTranslationPoints = NULL;
}

//----------------------------------------------------------------------------
vtkPVMultiBlockRootObjectInfo::~vtkPVMultiBlockRootObjectInfo()
{
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::CopyFromObject(vtkObject* obj)
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

  // if composite, grab the info from 1st block
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(dataObject);
  if (mds)
    {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    dataObject = iter->GetCurrentDataObject();
    iter->Delete();
    }

  this->MaterialNames = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray(
      vtkMultiBlockWrapper::GetMaterialUserNamesString()) );
  this->ShellNames = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray(
      vtkMultiBlockWrapper::GetShellUserNamesString()) );
  this->FaceNames = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray(
      vtkMultiBlockWrapper::GetModelFaceUserNamesString()) );

  this->BCNames = vtkStringArray::SafeDownCast(
    dataObject->GetFieldData()->GetAbstractArray(
      vtkMultiBlockWrapper::GetBCSUserNamesString()) );

  this->ShellColors = vtkFloatArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray(
    vtkMultiBlockWrapper::GetShellColorsString()));
  this->MaterialColors = vtkFloatArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray(
    vtkMultiBlockWrapper::GetMaterialColorsString()));
  this->ModelFaceColors = vtkFloatArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray(
    vtkMultiBlockWrapper::GetModelFaceColorsString()));
  this->BCColors = vtkFloatArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray(
    vtkMultiBlockWrapper::GetBCSColorsString()));
  this->ShellTranslationPoints = vtkDoubleArray::SafeDownCast(
    dataObject->GetFieldData()->GetArray(
    vtkMultiBlockWrapper::GetShellTranslationPointString()));
}

//----------------------------------------------------------------------------
const char* vtkPVMultiBlockRootObjectInfo::GetMaterialNameWithId(int id)
{
  return this->GetNameArrayValue(this->MaterialNames, id);
}

//----------------------------------------------------------------------------
const char* vtkPVMultiBlockRootObjectInfo::GetShellNameWithId(int id)
{
  return this->GetNameArrayValue(this->ShellNames, id);
}

//----------------------------------------------------------------------------
const char* vtkPVMultiBlockRootObjectInfo::GetFaceNameWithId(int id)
{
  return this->GetNameArrayValue(this->FaceNames, id);
}

//----------------------------------------------------------------------------
const char* vtkPVMultiBlockRootObjectInfo::GetBCNameWithId(int id)
{
  return this->GetNameArrayValue(this->BCNames, id);
}

//----------------------------------------------------------------------------
const char* vtkPVMultiBlockRootObjectInfo::GetNameArrayValue(
  vtkStringArray* nameArray, int id)
{
  if(nameArray && id >=0 && id < nameArray->GetNumberOfTuples())
    {
    return nameArray->GetValue(id).c_str();
    }

  return NULL;
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetShellColorWithId(
  int id, float* rgba)
{
  this->GetColorArrayValue(this->ShellColors, id, rgba);
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetMaterialColorWithId(
  int id, float* rgba)
{
  this->GetColorArrayValue(this->MaterialColors, id, rgba);
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetModelFaceColorWithId(
  int id, float* rgba)
{
  this->GetColorArrayValue(this->ModelFaceColors, id, rgba);
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetModelFaceIds(vtkIdList* list)
{
  if(this->ModelFaceColors)
    {
    list->Reset();
    float vals[4];
    for(int i=0; i<this->ModelFaceColors->GetNumberOfTuples(); i++)
      {
      this->ModelFaceColors->GetTypedTuple(i, vals);
      if(vals[3] != -2)// this is not a hole
        {
        list->InsertNextId(i);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetBCColorWithId(
  int id, float* rgba)
{
  this->GetColorArrayValue(this->BCColors, id, rgba);
}

//----------------------------------------------------------------------------
void vtkPVMultiBlockRootObjectInfo::GetColorArrayValue(
  vtkFloatArray* colorArray, int id, float *rgba)
{
  if(colorArray && id >=0 && id < colorArray->GetNumberOfTuples())
    {
    colorArray->GetTypedTuple(id, rgba);
    }
}

//----------------------------------------------------------------------------
int vtkPVMultiBlockRootObjectInfo::IsShellTranslationPointsLoaded()
{
  if (this->ShellTranslationPoints)
    {
    // check each tuple, see if at least one is valid (pt[3] > 0)
    double *pt;
    for (int i = 0; i < this->ShellTranslationPoints->GetNumberOfTuples(); i++)
      {
      pt = this->ShellTranslationPoints->GetTuple4(i);
      if (pt[3] > 0)
        {
        return 1;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void
vtkPVMultiBlockRootObjectInfo::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void
vtkPVMultiBlockRootObjectInfo::CopyFromStream(const vtkClientServerStream*)
{
}
