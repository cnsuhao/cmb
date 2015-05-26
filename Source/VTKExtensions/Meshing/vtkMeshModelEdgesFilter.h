//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkMeshModelEdgesFilter - Mesh polyines according to input cell data.
// .SECTION Description
// This filter meshes input polylines accroding to cell data on the input -
// each polyline (cell) is meshed according to the value in the cell data
// array specified by the TargetSegmentLengthCellArrayName.
//
// There are two modes for creating the mesh.  The 1st (default) creates
// mesh points by summing the length along the input edge (polyline) such
// that the actual distance between the points might be much smaller than
// the specified distance (the straighter the edge is, the closer the
// the distances in the output mesh will be).  For this mode, the target
// segment length is used to compute output the number of segemnts the
// output edge should have, and then the target segment length is adjusted
// such that (along the edge) each segment is of equal length.
//
// The 2nd mode uses the target segment length to search for the next pt
// on the edge that would meet the target segment length criteria.  This
// search is done from both ends of the edge "simultaneously".  The resulting
// mesh will have segments if equal length (as requested), except for "residue"
// in the middle mesh.
//
// An item on the to-do list for this filter is to better handle the
// residue in the middle of the output edge, such that the resulting mesh
// doesn't have a segment signficantly differnt in size than the rest of the
// mesh.  Another is to allow specification of target segment length
// on both ends of the edge; actually the 2nd mode was written with this in
// mind, though it hasn't been tested, and also doesn't current have a method
// of inputing the information (could be done with either point data or cell
// data)

#ifndef __vtkMeshModelEdgesFilter_h
#define __vtkMeshModelEdgesFilter_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBMESHING_EXPORT vtkMeshModelEdgesFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkMeshModelEdgesFilter *New();
  vtkTypeMacro(vtkMeshModelEdgesFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get name of cell array holding desired segment length for each cell.
  vtkSetStringMacro(TargetSegmentLengthCellArrayName);
  vtkGetStringMacro(TargetSegmentLengthCellArrayName);

  // Description:
  // Set/Get whether or not to use length along edge (as opposed to distance
  // between current mesh point and the next) when computing the next mesh pt.
  // Defaults to "true".
  vtkBooleanMacro(UseLengthAlongEdge, bool);
  vtkSetMacro(UseLengthAlongEdge, bool);
  vtkGetMacro(UseLengthAlongEdge, bool);

//BTX
protected:
  vtkMeshModelEdgesFilter();
  ~vtkMeshModelEdgesFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void MeshPolyLine(vtkIdType npts, vtkIdType *pts, vtkPoints *inputPoints,
    double startTargetSegmentLength, double endTargetSegmentLength,
    vtkPoints *outputPoints, vtkIdList *outputPtIds);
  double ComputeEdgeLength(vtkPoints *inputPoints, vtkIdType npts, vtkIdType *pts);
  double ComputeSegmentLength(double s1, double s2, double l, double L, double factor);

  vtkIdType FindRequiredPointOnEdge(double currentPt[3], double segmentLength,
    vtkPoints *inputPoints, vtkPoints *outputPoints, vtkIdType *pts,
    vtkIdType firstIndex, vtkIdType afterLastIndex, vtkIdType searchDirection,
    vtkIdType finalPointId, vtkIdType &nextPtId);
  vtkIdType ComputeRequiredPointAlongLine(vtkPoints *points, double *pt0,
    double *pt1, double t);

private:
  vtkMeshModelEdgesFilter(const vtkMeshModelEdgesFilter&);  // Not implemented.
  void operator=(const vtkMeshModelEdgesFilter&);  // Not implemented.

  vtkIdType EstimateNumberOfOutputPoints(vtkPolyData *input,
    vtkDoubleArray *targetSegmentLengthArray);

  char *TargetSegmentLengthCellArrayName;
  //vtkPoints *TemporaryPoints;
  vtkIdList *MeshPtIdsFromStart;
  vtkIdList *MeshPtIdsFromEnd;
  bool UseLengthAlongEdge;

//ETX
};

#endif
