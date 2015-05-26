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
  void SetControlRanges(double arc_ind,
                        double minDispDist, double maxDispDist,
                        double minWeightDist, double maxWeightDist);
  void AddArc(int arc_ind);
  void RemoveArc(int arc_ind);
  void SetArcEnable(int arc_ind, int isEnabled);
  void SetFunctionModes(int arc_ind, int isRelative, int isSymmetric);
  void ClearFunctions( int arc_ind );
  void AddWeightingFunPoint( double arc_ind, double x, double y, double m, double s);
  void AddDispFunPoint( double arc_ind, double x, double y, double m, double s);
  void SelectFunctionType( int arc_ind, int weightT, int dispT);

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
