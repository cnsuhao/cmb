/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkHydroModelCreator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkHydroModelCreator - creates a vtkMultiBlockDataSet
// .SECTION Description
// Filter to output a vtkMultiBlockDataSet with the classification info
// from the incoming vtkPolyData (requires "shell" information) in
// other vtkPolyDatas.

#ifndef __vtkHydroModelCreator_h
#define __vtkHydroModelCreator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkHydroModelCreator : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkHydroModelCreator *New();
  vtkTypeMacro(vtkHydroModelCreator,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);



protected:
  vtkHydroModelCreator();
  ~vtkHydroModelCreator();

  virtual int FillInputPortInformation(int, vtkInformation* info);
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkHydroModelCreator(const vtkHydroModelCreator&);  // Not implemented.
  void operator=(const vtkHydroModelCreator&);  // Not implemented.
};

#endif
