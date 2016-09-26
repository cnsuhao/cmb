//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  const char *GetFileContents() { return this->FileContents.c_str(); }

  //BTX

protected:
  vtkStringReader();
  ~vtkStringReader() override;

  char *FileName;
  std::string FileContents;
  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

private:
  vtkStringReader(const vtkStringReader&);  // Not implemented.
  void operator=(const vtkStringReader&);  // Not implemented.

  //ETX
};

#endif
