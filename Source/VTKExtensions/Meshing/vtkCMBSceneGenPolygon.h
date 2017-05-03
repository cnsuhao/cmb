//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSceneGenPolygon
// .SECTION Description

#ifndef __vtkCMBSceneGenPolygon_h
#define __vtkCMBSceneGenPolygon_h

#include "cmbSystemConfig.h"
#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkCellLocator;
class vtkGenericCell;

class VTKCMBMESHING_EXPORT vtkCMBSceneGenPolygon : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBSceneGenPolygon* New();
  vtkTypeMacro(vtkCMBSceneGenPolygon, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkCMBSceneGenPolygon();
  ~vtkCMBSceneGenPolygon() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  bool PrepForMeshing(vtkInformationVector* input, vtkPolyData* mesh);
  void AppendArcSets(vtkInformationVector* input, vtkPolyData* mesh);
  bool DeterminePolygon(vtkPolyData* mesh);

private:
  bool FindStartingCellForMesh(
    vtkCellLocator* locator, vtkGenericCell* cell, vtkIdType& cellId, double bounds[6]);

  vtkCMBSceneGenPolygon(const vtkCMBSceneGenPolygon&); // Not implemented.
  void operator=(const vtkCMBSceneGenPolygon&);        // Not implemented.
};

#endif
