//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkStringReader.h"

#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

#
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkStringReader);

//-----------------------------------------------------------------------------
vtkStringReader::vtkStringReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkStringReader::~vtkStringReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkStringReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  unsigned long fsize = vtksys::SystemTools::FileLength(this->FileName);
  FILE* fin;
  fin = fopen(this->FileName, "r");

  if (fin == NULL)
  {
    this->FileContents = "";
    this->SetErrorCode(vtkErrorCode::FileNotFoundError);
    return VTK_ERROR;
  }

  char* buffer = new char[fsize + 1];
  size_t size = fread(buffer, 1, fsize, fin);
  fclose(fin);
  buffer[size] = '\0';
  this->FileContents = buffer;
  delete[] buffer;
  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkStringReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkStringReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return VTK_ERROR;
  }

  return VTK_OK;
}
