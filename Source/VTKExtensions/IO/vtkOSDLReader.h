//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkOSDLReader();
  ~vtkOSDLReader() override;

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *) override;
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *) override;

  void AppendBlocks( vtkMultiBlockDataSet *output, vtkDataObject *dataObject );
  char *FileName;

private:
  vtkOSDLReader(const vtkOSDLReader&);  // Not implemented.
  void operator=(const vtkOSDLReader&);  // Not implemented.
};

#endif
