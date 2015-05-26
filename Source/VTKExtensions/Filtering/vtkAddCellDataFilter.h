//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkAddCellDataFilter
// .SECTION Description

#ifndef __vtkAddCellDataFilter_h
#define __vtkAddCellDataFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdList;

class VTKCMBFILTERING_EXPORT vtkAddCellDataFilter : public vtkDataSetAlgorithm
{
public:
  static vtkAddCellDataFilter* New();
  vtkTypeMacro(vtkAddCellDataFilter, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
protected:
  vtkAddCellDataFilter();
  ~vtkAddCellDataFilter();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkAddCellDataFilter(const vtkAddCellDataFilter&); // Not implemented.
  void operator=(const vtkAddCellDataFilter&); // Not implemented.


  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif


