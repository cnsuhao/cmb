/*=========================================================================

  Program:   ParaView
  Module:    vtkDiscreteLookupTable.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDiscreteLookupTable - a combination of vtkColorTransferFunction and
// vtkLookupTable.
// .SECTION Description
// This is a cross between a vtkColorTransferFunction and a vtkLookupTable
// selectively combiniting the functionality of both.
// NOTE: One must call Build() after making any changes to the points
// in the ColorTransferFunction to ensure that the discrete and non-discrete
// version match up.

#ifndef __vtkDiscreteLookupTable_h
#define __vtkDiscreteLookupTable_h

#include "vtkPVLookupTable.h"
#include "cmbSystemConfig.h"

class vtkLookupTable;
class vtkColorTransferFunction;

class VTK_EXPORT vtkDiscreteLookupTable : public vtkPVLookupTable
{
public:
  static vtkDiscreteLookupTable* New();
  vtkTypeMacro(vtkDiscreteLookupTable, vtkPVLookupTable);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Generate discretized lookup table using HSV color space, if applicable.
  // This method must be called after changes to the ColorTransferFunction
  // otherwise the discretized version will be inconsitent with the
  // non-discretized one.
  virtual void Build();

  // Description:
  // Set/Get the delta intervals of H/S/V while generating *NumberOfValues*
  // colors to build the discrete LookupTable.
  vtkSetMacro(HueDelta, double);
  vtkGetMacro(HueDelta, double);
  vtkSetMacro(SaturationDelta, double);
  vtkGetMacro(SaturationDelta, double);
  vtkSetMacro(ValueDelta, double);
  vtkGetMacro(ValueDelta, double);

protected:
  vtkDiscreteLookupTable();
  ~vtkDiscreteLookupTable();

  double HueDelta;
  double SaturationDelta;
  double ValueDelta;

  double SaturationMin;
  double SaturationMax;
  double ValueMin;
  double ValueMax;
private:
  vtkDiscreteLookupTable(const vtkDiscreteLookupTable&); // Not implemented.
  void operator=(const vtkDiscreteLookupTable&); // Not implemented.
};

#endif

