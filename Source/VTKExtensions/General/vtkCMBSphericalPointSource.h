//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSphericalPointSource - generates a set of points inside of a sphere
// .SECTION Description
// vtkCMBSphericalPointSource creates a set of points that lie on or inside of a sphere.
// The points are classified with respects to input.  If a point does not lie inside of a cell
// it will be omitted.  Else the point will be added to the set and the cell's ID will be added
// to the point data of the filter's output.  If the number of R steps > 1 then the result will
// include a point at the sphere's origin, else the result will be points that lie on the surface
// of the sphere's surface.  If the radius is set to 0 then only a single point will be generated
// and tested.

#ifndef __vtkCMBSphericalPointSource_h
#define __vtkCMBSphericalPointSource_h

#include "cmbSystemConfig.h"
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkCellLocator;
class vtkPoints;
class vtkIdTypeArray;

class VTKCMBGENERAL_EXPORT vtkCMBSphericalPointSource : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBSphericalPointSource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct with default position at (0,0,0), a radius of 0, phi Resolution = 3, and theta and r Resolutions = 1
  static vtkCMBSphericalPointSource* New();

  // Description:
  // Set the radius of the sphere.
  // The default is 0
  vtkSetClampMacro(Radius, double, 0.0, VTK_DOUBLE_MAX) vtkGetMacro(Radius, double);

  // Description:
  // Set the number of samples in R.
  vtkSetClampMacro(RResolution, int, 1, VTK_INT_MAX) vtkGetMacro(RResolution, int);

  // Description:
  // Set the number of samples in Theta.
  vtkSetClampMacro(ThetaResolution, int, 1, VTK_INT_MAX) vtkGetMacro(ThetaResolution, int);

  // Description:
  // Set the number of samples in Phi.
  vtkSetClampMacro(PhiResolution, int, 3, VTK_INT_MAX) vtkGetMacro(PhiResolution, int);

  // Description:
  // Set the center of the sphere.
  // The default is 0,0,0.
  vtkSetVector3Macro(Center, double);
  vtkGetVectorMacro(Center, double, 3);

  //Description:
  // Indicate if Phi should be ignored which will result in a Disc of Points.
  // The Default is false
  vtkSetMacro(IgnorePhi, bool);
  vtkGetMacro(IgnorePhi, bool);
  vtkBooleanMacro(IgnorePhi, bool);

protected:
  vtkCMBSphericalPointSource();
  ~vtkCMBSphericalPointSource() override {}

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  double Radius;
  double Center[3];
  double RResolution;
  double ThetaResolution;
  double PhiResolution;
  bool IgnorePhi;

private:
  vtkCMBSphericalPointSource(const vtkCMBSphericalPointSource&); // Not implemented.
  void operator=(const vtkCMBSphericalPointSource&);             // Not implemented.
};

#endif
