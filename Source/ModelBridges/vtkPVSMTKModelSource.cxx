//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSMTKModelSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelManagerWrapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkPVSMTKModelSource);
vtkCxxSetObjectMacro(vtkPVSMTKModelSource,ModelManagerWrapper,vtkModelManagerWrapper);

vtkPVSMTKModelSource::vtkPVSMTKModelSource()
{
  this->ModelManagerWrapper = NULL;
}

vtkPVSMTKModelSource::~vtkPVSMTKModelSource()
{
  this->SetModelManagerWrapper(NULL);
}

void vtkPVSMTKModelSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ModelManagerWrapper: " << this->ModelManagerWrapper << "\n";
}

void vtkPVSMTKModelSource::SetModelEntityID(const char* entid)
{
  this->Superclass::SetModelEntityID(entid);
}

char* vtkPVSMTKModelSource::GetModelEntityID()
{
  this->Superclass::GetModelEntityID();
}

int vtkPVSMTKModelSource::RequestData(
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
