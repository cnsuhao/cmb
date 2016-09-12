//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPolyDataSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkCMBPolyDataSource_h
#define __vtkCMBPolyDataSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkCMBPolyDataSource : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBPolyDataSource *New();
  vtkTypeMacro(vtkCMBPolyDataSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  void SetSource(vtkPolyData *source);
  vtkGetObjectMacro(Source, vtkPolyData);

  // Description:
  // Include Source in the MTime for this object
  virtual vtkMTimeType GetMTime();

protected:
  vtkCMBPolyDataSource();
  ~vtkCMBPolyDataSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkPolyData *Source;

private:
  vtkCMBPolyDataSource(const vtkCMBPolyDataSource&);  // Not implemented.
  void operator=(const vtkCMBPolyDataSource&);  // Not implemented.
};

#endif
