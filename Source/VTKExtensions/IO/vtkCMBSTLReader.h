//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSTLReader - Read in a stl file into CMB.
// .SECTION Description
// Read in a stl file into CMB.

#ifndef __vtkCMBSTLReader_h
#define __vtkCMBSTLReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBSTLReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBSTLReader * New();
  vtkTypeMacro(vtkCMBSTLReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCMBSTLReader();
  virtual ~vtkCMBSTLReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkCMBSTLReader(const vtkCMBSTLReader&);  // Not implemented.
  void operator=(const vtkCMBSTLReader&);  // Not implemented.

  // Description:
  // The name of the file to be read in.
  char* FileName;
};

#endif
