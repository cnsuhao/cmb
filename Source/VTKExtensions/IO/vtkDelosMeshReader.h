//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkDelosMeshReader - Reader for Delos Meshes

#ifndef __vtkDelosMeshReader_h
#define __vtkDelosMeshReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBIO_EXPORT vtkDelosMeshReader : public vtkPolyDataAlgorithm
{
public:

  static vtkDelosMeshReader *New();
  vtkTypeMacro(vtkDelosMeshReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Determine whether the file can be read by this reader.
  int CanReadFile(const char *);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkDelosMeshReader();
  ~vtkDelosMeshReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  char * FileName;

private:
  vtkDelosMeshReader(const vtkDelosMeshReader&);  // Not implemented.
  void operator=(const vtkDelosMeshReader&);  // Not implemented.
};
#endif
