//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBConeSource - generate polygonal conical shell
// .SECTION Description
// vtkCMBConeSource creates a cone centered at a specified point and pointing in
// a specified direction. The BaseCenter defines the center of cone's Base and
// the BaseRadius defines the radius of base.  The Height and Direction defines
// the position of the "top" of the conical shell.  If the TopRadius is set to be
// 0, the shell will come to a point at the top, else it will have a circular area.

#ifndef __vtkCMBConeSource_h
#define __vtkCMBConeSource_h

#include "cmbSystemConfig.h"
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

class VTKCMBGENERAL_EXPORT vtkCMBConeSource : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBConeSource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct with default resolution 6, height 1.0, radius 0.5, and
  // capping on. The base cone is centered at the origin and points down
  // the x-axis.
  static vtkCMBConeSource* New();

  // Description:
  // Set the height of the cone. This is the height along the cone in
  // its specified direction.
  vtkSetClampMacro(Height, double, 0.0, VTK_DOUBLE_MAX) vtkGetMacro(Height, double);

  // Description:
  // Set the base radius of the cone.
  vtkSetClampMacro(BaseRadius, double, 0.0, VTK_DOUBLE_MAX) vtkGetMacro(BaseRadius, double);

  // Description:
  // Set the top radius of the cone.
  vtkSetClampMacro(TopRadius, double, 0.0, VTK_DOUBLE_MAX) vtkGetMacro(TopRadius, double);

  // Description:
  // Set the number of facets used to represent the cone.
  vtkSetClampMacro(Resolution, int, 0, VTK_CELL_SIZE) vtkGetMacro(Resolution, int);

  // Description:
  // Set the base center of the cone.
  // The default is 0,0,0.
  vtkSetVector3Macro(BaseCenter, double);
  vtkGetVectorMacro(BaseCenter, double, 3);

  // Description:
  // Set the orientation vector of the cone. The vector does not have
  // to be normalized. The direction goes from the center of the base toward
  // the apex. The default is (1,0,0).
  vtkSetVector3Macro(Direction, double);
  vtkGetVectorMacro(Direction, double, 3);

  // Description:
  // Turn on/off whether to cap the cone with a polygon.
  vtkSetMacro(Capping, int);
  vtkGetMacro(Capping, int);
  vtkBooleanMacro(Capping, int);

protected:
  vtkCMBConeSource(int res = 6);
  ~vtkCMBConeSource() override {}

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  double Height;
  double BaseRadius;
  double TopRadius;
  int Resolution;
  int Capping;
  double BaseCenter[3];
  double Direction[3];

private:
  vtkCMBConeSource(const vtkCMBConeSource&); // Not implemented.
  void operator=(const vtkCMBConeSource&);   // Not implemented.
};

#endif
