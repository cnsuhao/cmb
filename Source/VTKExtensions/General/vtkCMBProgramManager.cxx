//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBProgramManager.h"
#include "vtkObjectFactory.h"

#include <iostream>
#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBProgramManager);

vtkCMBProgramManager::vtkCMBProgramManager()
{
}

vtkCMBProgramManager::~vtkCMBProgramManager()
{
}

vtkStdString const& vtkCMBProgramManager::GetDirectoryPath() const
{
  return this->DirectoryPath;
}

vtkStdString const& vtkCMBProgramManager::GetProgramName() const
{
  return this->ProgramName;
}

vtkStdString const& vtkCMBProgramManager::GetProgramVersion() const
{
  return this->ProgramVersion;
}

int const& vtkCMBProgramManager::GetVersionMajor() const
{
  return this->VersionMajor;
}

int const& vtkCMBProgramManager::GetVersionMinor() const
{
  return this->VersionMinor;
}

void vtkCMBProgramManager::SetDirectoryPath(const vtkStdString& path)
{
  this->DirectoryPath = path;

  //if the directory does not exist at this point create it
  vtksys::SystemTools::MakeDirectory(path.c_str());
}

void vtkCMBProgramManager::SetProgram(vtkCMBProjectManager::PROGRAM program)
{
  this->Program = program;
  this->ProgramName = vtkCMBProjectManager::GetProgramName(program);
}

void vtkCMBProgramManager::SetProgramVersion(const int& major, const int& minor)
{
  this->VersionMajor = major;
  this->VersionMinor = minor;

  //create the string of the version once, instead of each time
  //it is requested.
  std::stringstream buff(std::stringstream::in | std::stringstream::out);
  buff << major << "." << minor;
  this->ProgramVersion = buff.str();
}

void vtkCMBProgramManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Program ENUM   = " << this->Program << endl;
  os << indent << "Program Name   = " << this->ProgramName << endl;
  os << indent << "Directory Path = " << this->DirectoryPath << endl;
  os << indent << "Version String = " << this->ProgramVersion << endl;
  os << indent << "Version Major  = " << this->VersionMajor << endl;
  os << indent << "Version Minor  = " << this->VersionMinor << endl;
}
