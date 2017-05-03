//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVCMBSceneV2WriterInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCMBSceneV2WriterHelper.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkFieldData.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkStringList.h"

vtkStandardNewMacro(vtkPVCMBSceneV2WriterInformation);

//----------------------------------------------------------------------------
vtkPVCMBSceneV2WriterInformation::vtkPVCMBSceneV2WriterInformation()
{
  this->ObjectFileNames = NULL;
}

//----------------------------------------------------------------------------
vtkPVCMBSceneV2WriterInformation::~vtkPVCMBSceneV2WriterInformation()
{
  if (this->ObjectFileNames)
  {
    this->ObjectFileNames->Delete();
    this->ObjectFileNames = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkPVCMBSceneV2WriterInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
const char* vtkPVCMBSceneV2WriterInformation::GetObjectFileName(int index)
{
  if (!this->ObjectFileNames)
  {
    return NULL;
  }
  return this->ObjectFileNames->GetString(index);
}

//----------------------------------------------------------------------------
int vtkPVCMBSceneV2WriterInformation::GetNumberOfObjectFileNames()
{
  return (this->ObjectFileNames) ? this->ObjectFileNames->GetNumberOfStrings() : 0;
}

//----------------------------------------------------------------------------
void vtkPVCMBSceneV2WriterInformation::CopyFromObject(vtkObject* obj)
{
  vtkCMBSceneV2WriterHelper* dataObject = vtkCMBSceneV2WriterHelper::SafeDownCast(obj);

  if (!dataObject)
  {
    vtkErrorMacro("Object is not a SceneGen V2WriterHelper!");
    return;
  }

  if (this->ObjectFileNames)
  {
    this->ObjectFileNames->Delete();
    this->ObjectFileNames = NULL;
  }
  this->ObjectFileNames = vtkStringList::New();

  int num = dataObject->GetNumberOfObjectFileNames();
  for (int i = 0; i < num; ++i)
  {
    this->ObjectFileNames->AddString(dataObject->GetObjectFileName(i));
  }
}

//----------------------------------------------------------------------------
void vtkPVCMBSceneV2WriterInformation::AddInformation(vtkPVInformation* /*info*/)
{
}

//----------------------------------------------------------------------------
void vtkPVCMBSceneV2WriterInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;

  if (this->ObjectFileNames)
  {
    int num = this->ObjectFileNames->GetNumberOfStrings();
    *css << num;
    for (int i = 0; i < num; ++i)
    {
      const char* name = this->ObjectFileNames->GetString(i);
      *css << strlen(name) << name;
    }
  }
  *css << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void vtkPVCMBSceneV2WriterInformation::CopyFromStream(const vtkClientServerStream* css)
{
  if (this->ObjectFileNames)
  {
    this->ObjectFileNames->Delete();
    this->ObjectFileNames = NULL;
  }
  this->ObjectFileNames = vtkStringList::New();

  int size;
  css->GetArgument(0, 0, &size);

  int len;
  for (int i = 0; i < size; ++i)
  {
    //position 1 is length, 2 is string, 3 is length etc
    css->GetArgument(0, (i * 2) + 1, &len);
    char* buffer = new char[len];
    css->GetArgument(0, (i * 2) + 2, buffer, len);
    this->ObjectFileNames->AddString(buffer);
    delete[] buffer;
  }
}
