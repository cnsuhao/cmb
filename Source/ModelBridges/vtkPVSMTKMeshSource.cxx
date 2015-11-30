//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSMTKMeshSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelManagerWrapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkPVSMTKMeshSource);
vtkCxxSetObjectMacro(vtkPVSMTKMeshSource,ModelManagerWrapper,vtkModelManagerWrapper);

vtkPVSMTKMeshSource::vtkPVSMTKMeshSource()
{
  this->ModelManagerWrapper = NULL;
}

vtkPVSMTKMeshSource::~vtkPVSMTKMeshSource()
{
  this->SetModelManagerWrapper(NULL);
}

void vtkPVSMTKMeshSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ModelManagerWrapper: " << this->ModelManagerWrapper << "\n";
}

void vtkPVSMTKMeshSource::SetModelEntityID(const char* entid)
{
  this->Superclass::SetModelEntityID(entid);
}

char* vtkPVSMTKMeshSource::GetModelEntityID()
{
  return this->Superclass::GetModelEntityID();
}

void vtkPVSMTKMeshSource::SetMeshCollectionID(const char* entid)
{
  this->Superclass::SetMeshCollectionID(entid);
}

char* vtkPVSMTKMeshSource::GetMeshCollectionID()
{
  return this->Superclass::GetMeshCollectionID();
}

int vtkPVSMTKMeshSource::RequestData(
  vtkInformation* request,
  vtkInformationVector** inInfo,
  vtkInformationVector* outInfo)
{
  if (!this->ModelManagerWrapper)
    {
    vtkErrorMacro("No input model manager wrapper!");
    return 0;
    }
  this->SetModelManager(this->ModelManagerWrapper->GetModelManager());
  return this->Superclass::RequestData(request, inInfo, outInfo);
}
