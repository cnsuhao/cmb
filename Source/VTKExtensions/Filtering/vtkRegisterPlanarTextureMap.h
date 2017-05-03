//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkRegisterPlanarTextureMap - generate texture coordinates by mapping points to plane
// .SECTION Description
// vtkRegisterPlanarTextureMap is a filter that generates 2D texture coordinates
// by mapping input dataset points onto a plane. The plane can either be
// user specified or generated automatically. (A least squares method is
// used to generate the plane automatically.)
//
// There are two ways you can specify the plane. The first is to provide a
// plane normal. In this case the points are projected to a plane, and the
// points are then mapped into the user specified s-t coordinate range. For
// more control, you can specify a plane with three points: an origin and two
// points defining the two axes of the plane. (This is compatible with the
// vtkPlaneSource.) Using the second method, the SRange and TRange vectors
// are ignored, since the presumption is that the user does not want to scale
// the texture coordinates; and you can adjust the origin and axes points to
// achieve the texture coordinate scaling you need. Note also that using the
// three point method the axes do not have to be orthogonal.

// .SECTION See Also
//  vtkPlaneSource vtkTextureMapToCylinder
// vtkTextureMapToSphere vtkThresholdTextureCoords

#ifndef __vtkRegisterPlanarTextureMap_h
#define __vtkRegisterPlanarTextureMap_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkRegisterPlanarTextureMap : public vtkDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkRegisterPlanarTextureMap, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct with identity mapping with clipping off.
  static vtkRegisterPlanarTextureMap* New();

  // Description:
  // Define Registration based on 2 points
  int SetTwoPointRegistration(double xy1[2], double st1[2], double xy2[2], double st2[2]);
  // Description:
  // Define Registration based on 2 points - format is x1 y1 s1 t1 x2 y2 s2 t2
  int SetTwoPointRegistration(double info[8]);

  // Description:
  // Get Registration based on 2 points - format is x1 y1 s1 t1 x2 y2 s2 t2
  void GetTwoPointRegistration(double info[8]);

  // Description:
  // Define Registration based on 3 points
  int SetThreePointRegistration(
    double xy1[2], double st1[2], double xy2[2], double st2[2], double xy3[2], double st3[2]);
  // Description:
  // Define Registration based on 3 points - format is x1 y1 s1 t1 x2 y2 s2 t2
  // x3 y3 s3 t3
  int SetThreePointRegistration(double info[12]);

  // Description:
  // Get Registration based on 3 points - format is x1 y1 s1 t1 x2 y2 s2 t2
  // x3 y3 s3 t3
  void GetThreePointRegistration(double info[12]);

  vtkGetMacro(NumberOfRegisterPoints, int);

  // Description:
  // Specify x-coordinate range for texture coordinate pair.
  vtkSetVector2Macro(XRange, double);
  vtkGetVectorMacro(XRange, double, 2);

  // Description:
  // Specify y-coordinate range for texture s-t coordinate pair.
  vtkSetVector2Macro(YRange, double);
  vtkGetVectorMacro(YRange, double, 2);

  // Description:
  // Specify s-coordinate range for texture s-t coordinate pair.
  vtkSetVector2Macro(SRange, double);
  vtkGetVectorMacro(SRange, double, 2);

  // Description:
  // Specify t-coordinate range for texture s-t coordinate pair.
  vtkSetVector2Macro(TRange, double);
  vtkGetVectorMacro(TRange, double, 2);

  // Description:
  // Turn on/off texture clipping.
  vtkSetMacro(ClipXY, int);
  vtkGetMacro(ClipXY, int);
  vtkBooleanMacro(ClipXY, int);

  // Description:
  // Turn on/off the filter's ability of generating texture coordinates.
  // This is on by default
  vtkSetMacro(GenerateCoordinates, int);
  vtkGetMacro(GenerateCoordinates, int);
  vtkBooleanMacro(GenerateCoordinates, int);

  // Description:
  // Static method to compute the texture coordinate for a point in same
  // manner as is done in the RequestData member fn.
  static void ComputeTextureCoordinate(double pt[2], double sRange[2], double tRange[2],
    double sMap[3], double tMap[3], double* tCoords);

protected:
  vtkRegisterPlanarTextureMap();
  ~vtkRegisterPlanarTextureMap() override{};

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  double SMap[3];
  double TMap[3];
  double XRange[2];
  double YRange[2];
  double SRange[2];
  double TRange[2];
  double RegisterXYPoints[3][2];
  double RegisterSTPoints[3][2];
  int NumberOfRegisterPoints;
  int ClipXY;
  int GenerateCoordinates;

private:
  vtkRegisterPlanarTextureMap(const vtkRegisterPlanarTextureMap&); // Not implemented.
  void operator=(const vtkRegisterPlanarTextureMap&);              // Not implemented.
};

#endif
