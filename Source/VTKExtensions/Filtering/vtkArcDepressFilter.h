//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkArcDepressFilter_h
#define __vtkArcDepressFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

#include <vector>

class DepArcData;

class VTKCMBFILTERING_EXPORT vtkArcDepressFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkArcDepressFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkArcDepressFilter *New();

  void ClearActiveArcPoints(int arc_ind);
  void SetAxis(int axis);
  void AddPointToArc(double arc_ind, double v1, double v2);
  void SetArcAsClosed(int arc_ind);
  void SetManualControlRanges(double arc_ind, double pointId,
                              double minDispDist, double maxDispDist,
                              double minWeightDist, double maxWeightDist);
  void AddArc(int arc_ind);
  void RemoveArc(int arc_ind);
  void SetArcEnable(int arc_ind, int isEnabled);
  void CreateManualFunction(int arc_ind, int funId, int desptFunctionType, int weightFunType,
                            int isRelative, int isSymmetric);
  void CreateWedgeFunction(double arc_ind, double funId, double weightFunType, double relative,
                           double dig, double clamp, double basewidth, double displacement,
                           double slopeLeft, double slopeRight, double maxWidthLeft,
                           double maxWidthRight);
  void SetFunctionToPoint(int arc_ind, int ptId, int funId );
  void AddWeightingFunPoint( double arc_ind, double funId,
                             double x, double y, double m, double s);
  void AddManualDispFunPoint( double arc_ind, double funId, double x, double y, double m, double s);

  void ResizeOrder(int size);
  void SetOrderValue(int loc, int arc_ind);

  void setUseNormalDirection(int);

  //BTX
protected:
  vtkArcDepressFilter();
  ~vtkArcDepressFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int Axis;
  std::vector< DepArcData * > Arcs;
  std::vector< unsigned > ApplyOrder;
  bool UseNormalDirection;

private:
  vtkArcDepressFilter(const vtkArcDepressFilter&):vtkPolyDataAlgorithm()
  {}  // Not implemented.
  void operator=(const vtkArcDepressFilter&){}  // Not implemented.

  bool IsProcessing;
  //ETX
};

#endif
