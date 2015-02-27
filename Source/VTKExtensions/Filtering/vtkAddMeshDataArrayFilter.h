/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAddMeshDataArrayFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAddMeshDataArrayFilter
// .SECTION Description

#ifndef __vtkAddMaterialCellArrayFilter_h
#define __vtkAddMaterialCellArrayFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdList;

class VTKCMBFILTERING_EXPORT vtkAddMeshDataArrayFilter : public vtkDataSetAlgorithm
{
public:
  static vtkAddMeshDataArrayFilter* New();
  vtkTypeMacro(vtkAddMeshDataArrayFilter, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
protected:
  vtkAddMeshDataArrayFilter();
  ~vtkAddMeshDataArrayFilter();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkAddMeshDataArrayFilter(const vtkAddMeshDataArrayFilter&); // Not implemented.
  void operator=(const vtkAddMeshDataArrayFilter&); // Not implemented.


  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif
