/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBProjectManagerWriter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBProjectManager - program manager
// .SECTION Description
// This class is used to track a single program information

#ifndef __vtkCMBProjectManagerWriter_h
#define __vtkCMBProjectManagerWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h"
#include "vtkCMBProjectManager.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBProjectManagerWriter : public vtkObject
{
public:
  static vtkCMBProjectManagerWriter* New();
  vtkTypeMacro(vtkCMBProjectManagerWriter,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(ProjectFileName);
  vtkGetStringMacro(ProjectFileName);

  void WriteProjectFile();

protected:
  vtkCMBProjectManagerWriter();
  ~vtkCMBProjectManagerWriter();

  char* ProjectFileName;
private:
  vtkCMBProjectManagerWriter(const vtkCMBProjectManagerWriter&);  // Not implemented.
  void operator=(const vtkCMBProjectManagerWriter&);  // Not implemented.
};

#endif
