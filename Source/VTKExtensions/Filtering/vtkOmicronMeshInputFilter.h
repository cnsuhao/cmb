//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkOmicronMeshInputFilter - final prepraration for Omicron mesh input.
// .SECTION Description
// This filter find material IDs to associate with each object and also
// adds soil and point in soil to output.

#ifndef __vtkOmicronMeshInputFilter_h
#define __vtkOmicronMeshInputFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkMultiBlockDataSet;

class VTKCMBFILTERING_EXPORT vtkOmicronMeshInputFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkOmicronMeshInputFilter* New();
  vtkTypeMacro(vtkOmicronMeshInputFilter, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkMultiBlockDataSet* dataSet);

//BTX
protected:
  vtkOmicronMeshInputFilter();
  ~vtkOmicronMeshInputFilter() override;

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*) override;

private:
  vtkOmicronMeshInputFilter(const vtkOmicronMeshInputFilter&); // Not implemented.
  void operator=(const vtkOmicronMeshInputFilter&); // Not implemented.

//ETX
};

#endif
