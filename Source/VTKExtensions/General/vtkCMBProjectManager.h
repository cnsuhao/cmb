//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBProjectManager - Project manager
// .SECTION Description
// This class is used to track the project information
// Note each Program can be used by multiple Projects


#ifndef __vtkCMBProjectManager_h
#define __vtkCMBProjectManager_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h"
#include "cmbSystemConfig.h"

class vtkCMBProgramManager;

//BTX
class VTKCMBGENERAL_EXPORT vtkCMBProjectManagerCleanup
{
public:
  vtkCMBProjectManagerCleanup();
  ~vtkCMBProjectManagerCleanup();
};
//ETX

class VTKCMBGENERAL_EXPORT vtkCMBProjectManager : public vtkObject
{
friend class vtkCMBProjectManagerReader;
public:
// Methods from vtkObject
  vtkTypeMacro(vtkCMBProjectManager,vtkObject);
  // Description:
  // Print ObjectFactor to stream.
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // This is a singleton pattern New.  There will only be ONE
  // reference to a vtkCMBProjectManager object per process.  Clients that
  // call this must call Delete on the object so that the reference
  // counting will work.   The single instance will be unreferenced when
  // the program exits.
  static vtkCMBProjectManager* New();
  // Description:
  // Return the singleton instance with no reference counting.
  static vtkCMBProjectManager* GetInstance();
  // Description:
  // Supply a user defined output window. Call ->Delete() on the supplied
  // instance after setting it.
  static void SetInstance(vtkCMBProjectManager *instance);

//BTX
  // use this as a way of memory management when the
  // program exits the SmartPointer will be deleted which
  // will delete the Instance singleton
  static vtkCMBProjectManagerCleanup Cleanup;
//ETX

  //Description
  //Reset the internal state so that it is tracking no programs
  void ResetProjectManager();

  //Description:
  //Get the major version number
  int const& GetVersionMajor() const;

  //Description:
  //Get the minor version number
  int const& GetVersionMinor() const;

  enum PROGRAM
  {
  PointsBuilder = 0,
  SceneBuilder,
  ModelBuilder,
  SimulationBuilder,
  NUM_PROGRAMS
  };

  static vtkCMBProjectManager::PROGRAM GetProgramType(const char *name);
  static const char* GetProgramName(vtkCMBProjectManager::PROGRAM const &program);

  // Description
  // For a given program get back that exact program manager.
  vtkCMBProgramManager const* GetProgramManager(vtkCMBProjectManager::PROGRAM const& program) const;

  //Description
  //Set the Active program so you can get back information about that program
  void SetActiveProgram(int program);

  const char* GetActiveProgramDirectory() const;
  const char* GetActiveDefaultProgramDirectory();

  //Description sets the active program directory, only
  //if doesn't exist already.
  void SetActiveProgramDirectory( const char* dir );


  int GetManagerActive() const { return (ProjectFilePath.size() != 0); }
  const char* GetProjectFilePath() const { return ProjectFilePath.c_str(); }

protected:
  vtkCMBProjectManager();
  ~vtkCMBProjectManager() override;

  // Description:
  // Returns the default folder for the give program, this is used
  // when the program hasn't specified where it exists. All paths
  // need to be absolute.
  const char* GetDefaultFolder(const vtkCMBProjectManager::PROGRAM &program);

  // Set the directory path, which only friend classes can do
  void SetProjectFilePath(vtkStdString const& path ){ ProjectFilePath = path;}

  // Description:
  // Get the programs directory if it is registered.
  const char* GetProgramDirectory(vtkCMBProjectManager::PROGRAM const& program) const;

  // Description:
  // Register the information object for a CMB program to the project manager
  void RegisterProgram(vtkCMBProjectManager::PROGRAM const& program, vtkStdString const& directory);

  void SetVersionMajor(int const& major);
  void SetVersionMinor(int const& minor);

private:
  vtkCMBProgramManager* Programs[NUM_PROGRAMS];

  //The file this project was loaded from
  vtkStdString ProjectFilePath;

  //default paths for each program
  //only populated when requested
  vtkStdString ProgramDefaultFolder[NUM_PROGRAMS];

  //stores the active program that was set by a user of the manager
  vtkCMBProjectManager::PROGRAM ActiveProgram;

  int VersionMajor;
  int VersionMinor;

  static vtkCMBProjectManager* Instance;
  vtkCMBProjectManager(const vtkCMBProjectManager&);  // Not implemented.
  void operator=(const vtkCMBProjectManager&);  // Not implemented.
};

#endif
