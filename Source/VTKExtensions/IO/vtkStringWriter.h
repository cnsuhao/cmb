//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkStringWriter - writes the contents of a string into a file
// .SECTION Description

#ifndef __StringWriter_h
#define __StringWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"
#include <string>
#include <vector>
#include <map>

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkStringWriter : public vtkPolyDataAlgorithm
{
public:
  static vtkStringWriter *New();
  vtkTypeMacro(vtkStringWriter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/Get the text to be written in the file.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

protected:
  vtkStringWriter();
  ~vtkStringWriter() override;

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

  char *FileName;
  char *Text;
private:
  vtkStringWriter(const vtkStringWriter&);  // Not implemented.
  void operator=(const vtkStringWriter&);  // Not implemented.

};

#endif
