//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSceneGenV2ContourWriter - outputs all the contours in XML format
// .SECTION Description

#ifndef __vtkSceneGenV2ContourWriter_h
#define __vtkSceneGenV2ContourWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkSceneGenV2ContourWriter : public vtkPolyDataAlgorithm
{
public:
  static vtkSceneGenV2ContourWriter *New();
  vtkTypeMacro(vtkSceneGenV2ContourWriter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

protected:
  vtkSceneGenV2ContourWriter();
  ~vtkSceneGenV2ContourWriter();

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  char* FileName;
private:
  vtkSceneGenV2ContourWriter(const vtkSceneGenV2ContourWriter&);  // Not implemented.
  void operator=(const vtkSceneGenV2ContourWriter&);  // Not implemented.
};

#endif
