//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBProjectManager.h"
#include "vtkCMBProgramManager.h"
#include "vtkDebugLeaks.h"
#include "vtkDebugLeaksManager.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

vtkCMBProjectManager* vtkCMBProjectManager::Instance = 0;
vtkCMBProjectManagerCleanup vtkCMBProjectManager::Cleanup;

vtkCMBProjectManagerCleanup::vtkCMBProjectManagerCleanup()
{
}

vtkCMBProjectManagerCleanup::~vtkCMBProjectManagerCleanup()
{
  // Destroy any remaining output window.
  vtkCMBProjectManager::SetInstance(NULL);
}

vtkCMBProjectManager::vtkCMBProjectManager()
  : ProjectFilePath("")
  , ActiveProgram(NUM_PROGRAMS)
  , VersionMajor(0)
  , VersionMinor(2)
{
  for (int i = 0; i < NUM_PROGRAMS; ++i)
  {
    this->Programs[i] = NULL;
  }
}

vtkCMBProjectManager::~vtkCMBProjectManager()
{
  for (int i = 0; i < NUM_PROGRAMS; ++i)
  {
    if (this->Programs[i])
    {
      this->Programs[i]->Delete();
    }
  }
}

void vtkCMBProjectManager::ResetProjectManager()
{
  for (int i = 0; i < NUM_PROGRAMS; ++i)
  {
    if (this->Programs[i])
    {
      this->Programs[i]->Delete();
      this->Programs[i] = NULL;
    }
  }

  this->ProjectFilePath = "";
  this->ActiveProgram = NUM_PROGRAMS;
  this->VersionMajor = 0, this->VersionMinor = 2;
}

// Up the reference count so it behaves like New
vtkCMBProjectManager* vtkCMBProjectManager::New()
{
  vtkCMBProjectManager* ret = vtkCMBProjectManager::GetInstance();
  ret->Register(NULL);
  return ret;
}

// Return the single instance of the vtkCMBProjectManager
vtkCMBProjectManager* vtkCMBProjectManager::GetInstance()
{
  if (!vtkCMBProjectManager::Instance)
  {
    // Try the factory first
    vtkCMBProjectManager::Instance =
      static_cast<vtkCMBProjectManager*>(vtkObjectFactory::CreateInstance("vtkCMBProjectManager"));
    // if the factory did not provide one, then create it here
    if (!vtkCMBProjectManager::Instance)
    {
      // if the factory failed to create the object,
      // then destroy it now, as vtkDebugLeaks::ConstructClass was called
      // with "vtkCMBProjectManager", and not the real name of the class
      vtkCMBProjectManager::Instance = new vtkCMBProjectManager;
    }
  }
  // return the instance
  return vtkCMBProjectManager::Instance;
}

void vtkCMBProjectManager::SetInstance(vtkCMBProjectManager* instance)
{
  if (vtkCMBProjectManager::Instance == instance)
  {
    return;
  }
  // preferably this will be NULL
  if (vtkCMBProjectManager::Instance)
  {
    vtkCMBProjectManager::Instance->Delete();
  }
  vtkCMBProjectManager::Instance = instance;
  if (!instance)
  {
    return;
  }
  // user will call ->Delete() after setting instance
  instance->Register(NULL);
}

vtkCMBProjectManager::PROGRAM vtkCMBProjectManager::GetProgramType(const char* name)
{
  if (strcmp(name, "Points Builder") == 0)
  {
    return vtkCMBProjectManager::PointsBuilder;
  }
  else if (strcmp(name, "Scene Builder") == 0)
  {
    return vtkCMBProjectManager::SceneBuilder;
  }
  else if (strcmp(name, "Model Builder") == 0)
  {
    return vtkCMBProjectManager::ModelBuilder;
  }
  else if (strcmp(name, "Simulation Builder") == 0)
  {
    return vtkCMBProjectManager::SimulationBuilder;
  }
  return vtkCMBProjectManager::NUM_PROGRAMS;
}

const char* vtkCMBProjectManager::GetProgramName(vtkCMBProjectManager::PROGRAM const& program)
{
  switch (program)
  {
    case vtkCMBProjectManager::PointsBuilder:
      return "Points Builder";
    case vtkCMBProjectManager::SceneBuilder:
      return "Scene Builder";
    case vtkCMBProjectManager::ModelBuilder:
      return "Model Builder";
    case vtkCMBProjectManager::SimulationBuilder:
      return "Simulation Builder";
    case vtkCMBProjectManager::NUM_PROGRAMS:
    default:
      return NULL;
  }
  return NULL;
}

void vtkCMBProjectManager::RegisterProgram(
  vtkCMBProjectManager::PROGRAM const& program, vtkStdString const& directory)
{
  //currently do not support changing a programs directory
  if (this->Programs[program] == NULL)
  {
    this->Programs[program] = vtkCMBProgramManager::New();
    this->Programs[program]->SetProgram(program);

    //note, this will also create the directory if it doesn't exist
    this->Programs[program]->SetDirectoryPath(directory);
  }
}

vtkCMBProgramManager const* vtkCMBProjectManager::GetProgramManager(
  vtkCMBProjectManager::PROGRAM const& program) const
{
  int index = static_cast<int>(program);
  return this->Programs[index];
}

void vtkCMBProjectManager::SetActiveProgram(int program)
{
  if (program >= vtkCMBProjectManager::PointsBuilder &&
    program < vtkCMBProjectManager::NUM_PROGRAMS && program != this->ActiveProgram)
  {
    this->ActiveProgram = vtkCMBProjectManager::PROGRAM(program);
  }
}

const char* vtkCMBProjectManager::GetActiveDefaultProgramDirectory()
{
  return this->GetDefaultFolder(this->ActiveProgram);
}

const char* vtkCMBProjectManager::GetDefaultFolder(const vtkCMBProjectManager::PROGRAM& program)
{
  std::string folder;
  switch (program)
  {
    case vtkCMBProjectManager::PointsBuilder:
      folder = "Points";
      break;
    case vtkCMBProjectManager::SceneBuilder:
      folder = "Scene";
      break;
    case vtkCMBProjectManager::ModelBuilder:
      folder = "Models";
      break;
    case vtkCMBProjectManager::SimulationBuilder:
      folder = "Simulations";
      break;
    case vtkCMBProjectManager::NUM_PROGRAMS:
    default:
      break;
  }
  if (folder.size() == 0)
  {
    //we don't have a default folder for the project passed in
    return NULL;
  }

  //we don't want to change the directory path
  std::string dir = vtksys::SystemTools::GetParentDirectory(this->ProjectFilePath);
  if (dir.size() == 0)
  {
    //we are creating a new project, so lets
    //use the current working directory
    dir = vtksys::SystemTools::GetCurrentWorkingDirectory();
  }

  //need to always add the separator
  dir += "/";

  std::vector<std::string> path;
  path.push_back(dir);
  path.push_back(folder);

  this->ProgramDefaultFolder[program] = vtksys::SystemTools::JoinPath(path);
  return this->ProgramDefaultFolder[program].c_str();
}

const char* vtkCMBProjectManager::GetActiveProgramDirectory() const
{
  return this->GetProgramDirectory(this->ActiveProgram);
}

const char* vtkCMBProjectManager::GetProgramDirectory(
  vtkCMBProjectManager::PROGRAM const& program) const
{
  if (!this->Programs[program])
  {
    return NULL;
  }
  return this->Programs[program]->GetDirectoryPath().c_str();
}

void vtkCMBProjectManager::SetActiveProgramDirectory(const char* dir)
{
  if (dir == NULL || strlen(dir) < 1 || this->ActiveProgram == NUM_PROGRAMS)
  {
    return;
  }
  vtkCMBProjectManager::RegisterProgram(this->ActiveProgram, dir);
}

int const& vtkCMBProjectManager::GetVersionMajor() const
{
  return this->VersionMajor;
}

int const& vtkCMBProjectManager::GetVersionMinor() const
{
  return this->VersionMinor;
}

void vtkCMBProjectManager::SetVersionMajor(int const& major)
{
  if (major != this->VersionMajor)
  {
    this->VersionMajor = major;
  }
}

void vtkCMBProjectManager::SetVersionMinor(int const& minor)
{
  if (minor != this->VersionMinor)
  {
    this->VersionMinor = minor;
  }
}

void vtkCMBProjectManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "vtkCMBProjectManager Single instance = "
     << static_cast<void*>(vtkCMBProjectManager::Instance) << endl;
  os << indent << "Version Major  = " << this->VersionMajor << endl;
  os << indent << "Version Minor  = " << this->VersionMinor << endl;
  for (int i = 0; i < vtkCMBProjectManager::NUM_PROGRAMS; ++i)
  {
    if (this->Programs[i] != NULL)
    {
      this->Programs[i]->PrintSelf(os, indent.GetNextIndent());
    }
  }
}
