//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBProjectManagerReader.h"
#include "vtkCMBProjectManager.h"
#include "vtkDebugLeaks.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

#include <iostream>
#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBProjectManagerReader);

vtkCMBProjectManagerReader::vtkCMBProjectManagerReader()
{
  this->ProjectFileName = 0;
}

vtkCMBProjectManagerReader::~vtkCMBProjectManagerReader()
{
  this->SetProjectFileName(0);
}

void vtkCMBProjectManagerReader::ReadProjectFile()
{
  if (!this->ProjectFileName)
  {
    return;
  }

  ifstream fileStream(this->ProjectFileName, ios::in);
  vtkXMLDataParser* parser = vtkXMLDataParser::New();
  parser->SetStream(&fileStream);
  int parsedPassed = parser->Parse();

  fileStream.close();
  if (!parsedPassed)
  {
    parser->Delete();
    return;
  }

  //Parse the file for all the information it has
  vtkXMLDataElement* project = parser->GetRootElement();

  if (project)
  {
    //setup the project directory
    vtkCMBProjectManager* projectManager = vtkCMBProjectManager::GetInstance();
    projectManager->SetProjectFilePath(this->ProjectFileName);

    //setup the version of the project file
    int value = 0, found = 0;
    found = project->GetScalarAttribute("VersionMajor", value);
    if (found)
    {
      projectManager->SetVersionMajor(value);
    }
    found = project->GetScalarAttribute("VersionMinor", value);
    if (found)
    {
      projectManager->SetVersionMinor(value);
    }

    //read in each sub program
    vtkXMLDataElement* program = NULL;
    vtkXMLDataElement* programPath = NULL;
    int size = project->GetNumberOfNestedElements();
    for (int i = 0; i < size; ++i)
    {
      program = project->GetNestedElement(i);
      if (program && strcmp(program->GetName(), "Program") == 0)
      {
        //lookup the name
        const char* name = program->GetAttribute("Name");
        vtkCMBProjectManager::PROGRAM type = vtkCMBProjectManager::GetProgramType(name);

        //get the path to this program folder
        programPath = program->FindNestedElementWithName("Directory");
        if (programPath)
        {
          vtkStdString programDirectoryPath = programPath->GetAttribute("Path");

          //register this program
          projectManager->RegisterProgram(type, programDirectoryPath);
        }
      }
    }
  }

  parser->Delete();
  return;
}

void vtkCMBProjectManagerReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ProjectFileName: " << (this->ProjectFileName ? this->ProjectFileName : "(none)")
     << "\n";
}
