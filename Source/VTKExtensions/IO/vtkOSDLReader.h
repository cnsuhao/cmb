/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSDLReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSDLReader - Reader for OSDL SceneGen file
// .SECTION Description
// Reader for main SceneGen file

#ifndef __SceneGenReader_h
#define __SceneGenReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkMultiBlockDataSet;
class vtkDataSet;

class VTKCMBIO_EXPORT vtkOSDLReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkOSDLReader *New();
  vtkTypeMacro(vtkOSDLReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkOSDLReader();
  ~vtkOSDLReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  void AppendBlocks( vtkMultiBlockDataSet *output, vtkDataObject *dataObject );
  char *FileName;

private:
  vtkOSDLReader(const vtkOSDLReader&);  // Not implemented.
  void operator=(const vtkOSDLReader&);  // Not implemented.
};

#endif
