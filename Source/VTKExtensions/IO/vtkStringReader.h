/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStringReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkStringReader - Reads the contents of a file into a string
// .SECTION Description
// Generic reader to get contents of a file into a string.

#ifndef __StringReader_h
#define __StringReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"
#include <string>
#include <vector>
#include <map>

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkStringReader : public vtkPolyDataAlgorithm
{
public:
  static vtkStringReader *New();
  vtkTypeMacro(vtkStringReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  const char *GetFileContents() { return this->FileContents.c_str(); }

  //BTX

protected:
  vtkStringReader();
  ~vtkStringReader();

  char *FileName;
  std::string FileContents;
  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:
  vtkStringReader(const vtkStringReader&);  // Not implemented.
  void operator=(const vtkStringReader&);  // Not implemented.

  //ETX
};

#endif
