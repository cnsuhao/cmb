//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBProjectManagerWriter.h"
#include "vtkCMBProjectManager.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"

#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBProjectManagerWriter);

//-----------------------------------------------------------------------------
vtkCMBProjectManagerWriter::vtkCMBProjectManagerWriter()
{
  this->ProjectFileName = 0;
}

//-----------------------------------------------------------------------------
vtkCMBProjectManagerWriter::~vtkCMBProjectManagerWriter()
{
  this->SetProjectFileName(0);
}

//-----------------------------------------------------------------------------
void vtkCMBProjectManagerWriter::WriteProjectFile()
{
  if (!this->ProjectFileName)
    {
    return;
    }

  std::stringstream buffer;
  vtkCMBProjectManager *projectManager = vtkCMBProjectManager::GetInstance();

  vtkSmartPointer<vtkXMLDataElement> projectFile =
    vtkSmartPointer<vtkXMLDataElement>::New();
   projectFile->SetName("Project");

  projectFile->SetIntAttribute("VersionMajor",
                  projectManager->GetVersionMajor());
  projectFile->SetIntAttribute("VersionMinor",
                  projectManager->GetVersionMinor());


  for ( int i=0; i < vtkCMBProjectManager::NUM_PROGRAMS; ++i)
    {
    //add the data element for each program if it exists
    projectManager->SetActiveProgram(i);
    const char* directory = projectManager->GetActiveProgramDirectory();
    if (!directory )
      {
      continue;
      }
    //write out this program entry
    vtkSmartPointer<vtkXMLDataElement> program =
        vtkSmartPointer<vtkXMLDataElement>::New();
    program->SetName("Program");
    program->SetAttribute("Name",vtkCMBProjectManager::GetProgramName(
                          vtkCMBProjectManager::PROGRAM(i)));

    vtkSmartPointer<vtkXMLDataElement> directoryPath =
        vtkSmartPointer<vtkXMLDataElement>::New();
    directoryPath->SetName("Directory");
    directoryPath->SetAttribute("Path",directory);
    program->AddNestedElement(directoryPath);
    projectFile->AddNestedElement(program);
    }

  vtkIndent indent;
  vtkXMLUtilities::WriteElementToFile(
    projectFile, this->GetProjectFileName(), &indent);
  return;
}

//-----------------------------------------------------------------------------
void vtkCMBProjectManagerWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ProjectFileName: " <<
    (this->ProjectFileName ? this->ProjectFileName : "(none)") << "\n";
}
