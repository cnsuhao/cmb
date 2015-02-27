/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkHydroModelPolySource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkHydroModelPolySource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkHydroModelPolySource_h
#define __vtkHydroModelPolySource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkHydroModelPolySource : public vtkPolyDataAlgorithm
{
public:
  static vtkHydroModelPolySource *New();
  vtkTypeMacro(vtkHydroModelPolySource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  void CopyData(vtkPolyData *source);
  vtkGetObjectMacro(Source, vtkPolyData);

protected:
  vtkHydroModelPolySource();
  ~vtkHydroModelPolySource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkPolyData *Source;

private:
  vtkHydroModelPolySource(const vtkHydroModelPolySource&);  // Not implemented.
  void operator=(const vtkHydroModelPolySource&);  // Not implemented.
};

#endif
