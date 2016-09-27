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

#ifndef __vtkCMBProjectManagerReader_h
#define __vtkCMBProjectManagerReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkObject.h"
#include "vtkStdString.h"
#include "vtkCMBProjectManager.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBProjectManagerReader : public vtkObject
{
public:
  static vtkCMBProjectManagerReader* New();
  vtkTypeMacro(vtkCMBProjectManagerReader,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(ProjectFileName);
  vtkGetStringMacro(ProjectFileName);

  void ReadProjectFile();

protected:
  vtkCMBProjectManagerReader();
  ~vtkCMBProjectManagerReader() override;

  char* ProjectFileName;
private:
  vtkCMBProjectManagerReader(const vtkCMBProjectManagerReader&);  // Not implemented.
  void operator=(const vtkCMBProjectManagerReader&);  // Not implemented.
};

#endif
