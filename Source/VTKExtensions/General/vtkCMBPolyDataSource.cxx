//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPolyDataSource.h"

#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBPolyDataSource);
vtkCxxSetObjectMacro(vtkCMBPolyDataSource,Source,vtkPolyData);


//-----------------------------------------------------------------------------
vtkCMBPolyDataSource::vtkCMBPolyDataSource()
{
  this->Source = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkCMBPolyDataSource::~vtkCMBPolyDataSource()
{
  this->SetSource( 0 );
}

//-----------------------------------------------------------------------------
int vtkCMBPolyDataSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (this->Source == 0)
    {
    vtkErrorMacro("Must set Source!");
    return 0;
    }

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // now move the input through to the output
  output->ShallowCopy( this->Source );

  return 1;
}



//----------------------------------------------------------------------------
unsigned long vtkCMBPolyDataSource::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();

  if (this->Source && this->Source->GetMTime() > mTime)
    {
    return this->Source->GetMTime();
    }

  return mTime;
}

//-----------------------------------------------------------------------------
void vtkCMBPolyDataSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if(this->Source)
    {
    os << indent << "Source: " << this->Source << "\n";
    }
  else
    {
    os << indent << "Source: (none)\n";
    }
}
