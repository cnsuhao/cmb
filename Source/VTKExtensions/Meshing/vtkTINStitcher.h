//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkTINStitcher - stitch two TINs/meshes together
// .SECTION Description
// vtkTINStitcher is a filter that stitches TINS.  Currently it handles both
// Type I and Type II TINs (it does not handle Type III TINs).  If Type II,
// Triangle is used to stitch the TINs.  If Type I, can use Quads (if UseQuads
// is ON), Triangle (if UseTriangleForTypeI is ON and UseQuads is Off), or
// split quads (triangles) if both are OFF, to stitch the TINs.
//
// In its current form, the filter is expecting two (and only two) meshes/TINs
// appended together as input, and of type vtkPolyData.  The output is a
// vtkPolyData of the TINs stitched together.  Note also that any CellData
// is removed when running this filter, and that PointData only is present
// on output if no points are added durign the stitching process.
// .SECTION See Also

#ifndef __vtkTINStitcher_h
#define __vtkTINStitcher_h

#include "cmbSystemConfig.h"
#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPointSetAlgorithm.h"
#include <vector>

class vtkCellArray;
class vtkIdTypeArray;
class vtkTransform;
struct triangulateio; // for Triangle

class VTKCMBMESHING_EXPORT vtkTINStitcher : public vtkPointSetAlgorithm
{
public:
  vtkTypeMacro(vtkTINStitcher, vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkTINStitcher* New();

  // Description:
  // Set 2nd input input to the filter (required)
  void Set2ndInputData(vtkUnstructuredGrid* input);
  void Set2ndInputData(vtkPolyData* input);
  void Set2ndInputConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // Return TIN type (1, 2, or 3)
  int GetTINType();

  // Description:
  // Indicate the minimum target angle to pass to Triangle if Type 2 TIN
  // (or Type I using Triangle to stitch).
  vtkSetClampMacro(MinimumAngle, double, 0, 34);
  vtkGetMacro(MinimumAngle, double);
  vtkBooleanMacro(MinimumAngle, double);

  // Description:
  // Indicate whether the resulting mesh should be quads if possible.
  // This only used if the loops contain the same number of points (Type I TIN).
  vtkSetMacro(UseQuads, bool);
  vtkGetMacro(UseQuads, bool);
  vtkBooleanMacro(UseQuads, bool);

  // Description:
  // Specifies whether Triangle is allowd to do point insertion on the interior
  // in order to meet minimum angle criteria.
  // Note, if a Type I TIN and UseQuads is TRUE, this is ignored.
  vtkSetMacro(AllowInteriorPointInsertion, bool);
  vtkGetMacro(AllowInteriorPointInsertion, bool);
  vtkBooleanMacro(AllowInteriorPointInsertion, bool);

  // Description:
  // Set/Get tolerance value, which is multiplied by the short side of the
  // input bounding box to compute a MaxDistance used when determining
  // TIN "sides" as well as Type I versus Type II.  Defaults to 1e-6.
  vtkSetClampMacro(Tolerance, double, 0, 0.05);
  vtkGetMacro(Tolerance, double);

  // Description:
  // Set/Get user specified TIN type.  If 0, auto-detect the TIN type and
  // stitch accordingly (only type I or type II right now); if 1, will stitch
  // as type I (if possible).  In auto-detect mode, determining TIN type is
  // 1st attempted using the member Tolerance as the tolerance factor.  If
  // not classified at type I or II at that tolerance level it is successively
  // increased by an order of magnitude, and auto-detection repeated, up to
  // maximum value of 0.05
  vtkSetClampMacro(UserSpecifiedTINType, int, 0, 1);
  vtkGetMacro(UserSpecifiedTINType, int);

  //BTX
protected:
  vtkTINStitcher();
  ~vtkTINStitcher();

  // Usual data generation method
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int FillInputPortInformation(int, vtkInformation*);
  virtual int FillOutputPortInformation(int, vtkInformation*);

private:
  vtkTINStitcher(const vtkTINStitcher&); // Not implemented.
  void operator=(const vtkTINStitcher&); // Not implemented.

  bool UseQuads;
  double MinimumAngle;
  bool AllowInteriorPointInsertion;
  double Tolerance;
  int UserSpecifiedTINType;
  int TINType;

  double MaxDistance;
  double MaxDistance2;

  vtkCellArray* LoopLines;
#ifndef __WRAP__
  vtkIdTypeArray*(LoopCorners[2]);
#endif
  vtkPolyData* AppendedPolyData;
  vtkPolyData* PreppedStitchingInput;
  vtkIdType LoopNPts[2];
#ifndef __WRAP__
  vtkIdType*(LoopPts[2]);
#endif

  bool AreInputsOK();

  int PrepInputsForStitching();

  void FindPolyLineCorners(vtkPolyData* input, vtkIdType npts, vtkIdType* pts,
    vtkIdTypeArray* corners, double maxDistance2);

  int SetupToStitchAsType1();
  int SetupToStitchUsingAutoDetect(double maxDistance, double maxDistance2);

  int ReorderPolyLine(vtkCellArray* newLines, vtkIdTypeArray* corners, vtkIdType npts,
    vtkIdType* pts, vtkIdType startCorner);

  void MapLoopLinesToAppendedData();

  void CreateQuadStitching(vtkPolyData* outputPD);

  void CreateTriStitching(vtkPolyData* outputPD);

  void ProcessSegmentWithTriangle(vtkPolyData* outputPD, vtkIdType startCornerIndex,
    vtkIdTypeArray* side0ExtraPoints, vtkIdTypeArray* side1ExtraPoints);

  void SetupPointsForTriangle(triangulateio& input, vtkTransform* ptTransform,
    vtkPolyData* outputPD, vtkIdType startCornerIndex, vtkIdTypeArray* sidePoints0,
    vtkIdTypeArray* sidePoints1, int& numberOfPointsInLoop0Segment,
    int& numberOfPointsInLoop1Segment, bool& loop0HasGreaterZ,
    std::vector<vtkIdType>& triangleToPD);

  void SetupSegmentsForTriangle(triangulateio& input, int numberOfPointsInLoop0Segment,
    int numberOfPointsInLoop1Segment, vtkIdTypeArray* sidePoints0, vtkIdTypeArray* sidePoints1);

  void SetupSidePoints(double* pointList, int inputNumberOfPoints, int outputNumberOfPoints,
    double* startPt, double* endPt, std::vector<vtkIdType>& triangleToPD,
    vtkIdTypeArray* sidePoints, bool fillDescending);

  //ETX
};

#endif
