/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkHydroModelMultiBlockSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkHydroModelMultiBlockSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkHydroModelMultiBlockSource_h
#define __vtkHydroModelMultiBlockSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkHydroModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkHydroModelMultiBlockSource *New();
  vtkTypeMacro(vtkHydroModelMultiBlockSource,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  void CopyData(vtkMultiBlockDataSet *source);
  vtkGetObjectMacro(Source, vtkMultiBlockDataSet);

protected:
  vtkHydroModelMultiBlockSource();
  ~vtkHydroModelMultiBlockSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkMultiBlockDataSet *Source;

private:
  vtkHydroModelMultiBlockSource(const vtkHydroModelMultiBlockSource&);  // Not implemented.
  void operator=(const vtkHydroModelMultiBlockSource&);  // Not implemented.
};

#endif
