//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBProjectManager - program manager
// .SECTION Description
// This class is used to track a single program information

#ifndef __vtkCMBProgramManager_h
#define __vtkCMBProgramManager_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h"
#include "vtkCMBProjectManager.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkCMBProgramManager : public vtkObject
{
public:
  static vtkCMBProgramManager* New();
  vtkTypeMacro(vtkCMBProgramManager,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  //Description:
  //Get the path to the this program directory in the project
  //manager file structure
  vtkStdString const& GetDirectoryPath() const;

  //Description:
  //Get program name
  vtkStdString const& GetProgramName() const;


  //Description:
  //Get a string the represents the program version
  vtkStdString const& GetProgramVersion() const;
  //ETX


  //Description:
  //Get the major version number
  int const& GetVersionMajor() const;

  //Description:
  //Get the minor version number
  int const& GetVersionMinor() const;

protected:
  vtkCMBProgramManager();
  ~vtkCMBProgramManager();

  friend class vtkCMBProjectManager;


  void SetProgram(vtkCMBProjectManager::PROGRAM program);
  void SetDirectoryPath(const vtkStdString &path);
  void SetProgramVersion(const int &major, const int &minor);

  vtkCMBProjectManager::PROGRAM Program;
  int VersionMajor;
  int VersionMinor;

  vtkStdString ProgramName;
  vtkStdString DirectoryPath;
  vtkStdString ProgramVersion;


private:
  vtkCMBProgramManager(const vtkCMBProgramManager&);  // Not implemented.
  void operator=(const vtkCMBProgramManager&);  // Not implemented.
};

#endif
