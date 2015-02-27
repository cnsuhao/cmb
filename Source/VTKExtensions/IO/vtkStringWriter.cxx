/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStringWriter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStringWriter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkErrorCode.h"

#
#include <sys/types.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkStringWriter);

//-----------------------------------------------------------------------------
vtkStringWriter::vtkStringWriter()
{
  this->FileName = 0;
  this->Text = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkStringWriter::~vtkStringWriter()
{
  this->SetFileName(0);
  this->SetText(0);
}

//----------------------------------------------------------------------------
int vtkStringWriter::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  return 1;
}

//-----------------------------------------------------------------------------
int vtkStringWriter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if ( this->FileName == NULL || this->Text == NULL )
    {
    return 0;
    }

  // make the directory if necessary
  std::string filePath = vtksys::SystemTools::GetFilenamePath(this->FileName);
  if(filePath.empty() == false &&
     vtksys::SystemTools::FileExists(filePath.c_str(), false) == false)
    {
    if(vtksys::SystemTools::MakeDirectory(filePath.c_str()) == false)
      {
      vtkErrorMacro(<<"Could not create directory " << filePath);
      return 0;
      }
    }

  std::ofstream out(this->FileName);
  out.write(this->Text, strlen(this->Text));
  out.close();

  return 1;
}

//-----------------------------------------------------------------------------
void vtkStringWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

