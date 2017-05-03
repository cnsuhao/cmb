//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkDiscreteLookupTable - a combination of vtkColorTransferFunction and
// vtkLookupTable.
// .SECTION Description
// This is a cross between a vtkColorTransferFunction and a vtkLookupTable
// selectively combining the functionality of both.
// NOTE: One must call Build() after making any changes to the points
// in the ColorTransferFunction to ensure that the discrete and non-discrete
// version match up.

#ifndef __vtkDiscreteLookupTable_h
#define __vtkDiscreteLookupTable_h

#include "cmbSystemConfig.h"
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkDiscretizableColorTransferFunction.h"

class vtkLookupTable;
class vtkColorTransferFunction;

class VTKCMBGENERAL_EXPORT vtkDiscreteLookupTable : public vtkDiscretizableColorTransferFunction
{
public:
  static vtkDiscreteLookupTable* New();
  vtkTypeMacro(vtkDiscreteLookupTable, vtkDiscretizableColorTransferFunction);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Generate discretized lookup table using HSV color space, if applicable.
  // This method must be called after changes to the ColorTransferFunction
  // otherwise the discretized version will be inconsitent with the
  // non-discretized one.
  void Build() override;

  static vtkIdType GetNextIndex(vtkIdType i, vtkUnsignedCharArray* avail);
  static void CalcRGB(double llimit, double ulimit, double& r, double& g, double& b);
  static void CreateLookupTable(vtkLookupTable* lut, double llimit, double ulimit);
  static void CreateLookupTable(vtkLookupTable* lut);

protected:
  vtkDiscreteLookupTable();
  ~vtkDiscreteLookupTable() override;

  double ValueMin;
  double ValueMax;

private:
  vtkDiscreteLookupTable(const vtkDiscreteLookupTable&); // Not implemented.
  void operator=(const vtkDiscreteLookupTable&);         // Not implemented.
};

#endif
