//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  void PrintSelf(ostream& os, vtkIndent indent) override;

//BTX
protected:
  vtkAddMeshDataArrayFilter();
  ~vtkAddMeshDataArrayFilter() override;

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*) override;

private:
  vtkAddMeshDataArrayFilter(const vtkAddMeshDataArrayFilter&); // Not implemented.
  void operator=(const vtkAddMeshDataArrayFilter&); // Not implemented.


  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif
