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

#ifndef __vtkCMBProjectManagerWriter_h
#define __vtkCMBProjectManagerWriter_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkCMBProjectManager.h"
#include "vtkObject.h"
#include "vtkStdString.h"

class VTKCMBIO_EXPORT vtkCMBProjectManagerWriter : public vtkObject
{
public:
  static vtkCMBProjectManagerWriter* New();
  vtkTypeMacro(vtkCMBProjectManagerWriter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(ProjectFileName);
  vtkGetStringMacro(ProjectFileName);

  void WriteProjectFile();

protected:
  vtkCMBProjectManagerWriter();
  ~vtkCMBProjectManagerWriter() override;

  char* ProjectFileName;

private:
  vtkCMBProjectManagerWriter(const vtkCMBProjectManagerWriter&); // Not implemented.
  void operator=(const vtkCMBProjectManagerWriter&);             // Not implemented.
};

#endif
