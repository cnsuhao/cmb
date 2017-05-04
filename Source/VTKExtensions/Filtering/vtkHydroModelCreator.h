//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkHydroModelCreator - creates a vtkMultiBlockDataSet
// .SECTION Description
// Filter to output a vtkMultiBlockDataSet with the classification info
// from the incoming vtkPolyData (requires "shell" information) in
// other vtkPolyDatas.

#ifndef __vtkHydroModelCreator_h
#define __vtkHydroModelCreator_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkHydroModelCreator : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkHydroModelCreator* New();
  vtkTypeMacro(vtkHydroModelCreator, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkHydroModelCreator();
  ~vtkHydroModelCreator() override;

  int FillInputPortInformation(int, vtkInformation* info) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkHydroModelCreator(const vtkHydroModelCreator&); // Not implemented.
  void operator=(const vtkHydroModelCreator&);       // Not implemented.
};

#endif
