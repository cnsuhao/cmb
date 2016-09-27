//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBClassifyPointsFilter - classifies a set points with respects to a solid mesh
// .SECTION Description
// vtkCMBClassifyPointsFilter classifies a set of points with respects to input.  If a point does not lie inside of a cell
// it will be omitted.  Else the point will be added to the set and the cell's ID will be added
// to the point data of the filter's output.

#ifndef __vtkCMBClassifyPointsFilter_h
#define __vtkCMBClassifyPointsFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkPoints;
class vtkIdTypeArray;

class VTKCMBFILTERING_EXPORT vtkCMBClassifyPointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBClassifyPointsFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkCMBClassifyPointsFilter *New();

  // Description:
  // Specify the solid mesh to be used. Any geometry
  // can be used. New style. Equivalent to SetInputConnection(1, algOutput).
  void SetSolidConnection(vtkAlgorithmOutput* algOutput);

protected:
  vtkCMBClassifyPointsFilter();
  ~vtkCMBClassifyPointsFilter() override {}
  int FillInputPortInformation(int port, vtkInformation* info) override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
private:
  vtkCMBClassifyPointsFilter(const vtkCMBClassifyPointsFilter&);  // Not implemented.
  void operator=(const vtkCMBClassifyPointsFilter&);  // Not implemented.
};

#endif


