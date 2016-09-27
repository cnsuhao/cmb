//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBContourGroupFilter - creates a vtkMultiBlockDataSet
// .SECTION Description
// Filter to output a vtkMultiBlockDataSet to a file with each block
// as a group of contours appended as a polydata, and each cell (polyline)
// in the polydata represents a contour

#ifndef __vtkCMBContourGroupFilter_h
#define __vtkCMBContourGroupFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"
#include <map>
#include <vector>

class VTKCMBFILTERING_EXPORT vtkCMBContourGroupFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkCMBContourGroupFilter,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct object with PointIds and CellIds on; and ids being generated
  // as scalars.
  static vtkCMBContourGroupFilter *New();

  // Description:
  // Set the active group to modify
  void SetActiveGroupIndex(int i)
  {
    if (i != this->ActiveGroupIdx)
    {
      ActiveGroupIdx = i;
      //Do not want to trigger the filter
      // this->Modified();
    }
  }
  // Description:
  // Set InsideOut flag to the active group.
  void SetGroupInvert(int val);

  // Description:
  // Add/Remove contours
  void AddContour(vtkAlgorithm*);
  void RemoveContour(vtkAlgorithm*);
  void RemoveAllContours();

  // Description:
  // Change the ProjectionNormal/ProjectionPosition of a contour given its index
  // vtkBoundedPlanePointPlacer::enum {
  //  XAxis=0,
  //  YAxis,
  //  ZAxis,
  //  Oblique
  // }
  void SetContourProjectionNormal(int idx, int ApplyPolygon);

  // Description:
  // Change the ProjectionPosition of a contour given its index
  // The position of the bounding plane from the origin along the
  // normal. The origin and normal are defined in the oblique plane
  // when the ProjectionNormal is oblique. For the X, Y, and Z
  // axes projection normals, the normal is the axis direction, and
  // the origin is (0,0,0).
  void SetContourProjectionPosition(int idx, double vaule);

protected:
  vtkCMBContourGroupFilter();
  ~vtkCMBContourGroupFilter() override;

  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *) override;

  int FillInputPortInformation(int port, vtkInformation *info) override;

  // Description:
  // Check if the valid group is valid
  int IsActiveGroupValid();

  int ActiveGroupIdx;
  struct PolygonInfo
  {
    PolygonInfo()
    {
      this->ProjectionNormal = 2; // Z-axis
      this->Polygon = NULL;
      this->ProjectionPosition = 0.0;
    }
    int ProjectionNormal;
    double ProjectionPosition;
    vtkAlgorithm* Polygon;
  };

  std::map<int, std::vector<PolygonInfo*> >Polygons;
  std::map<int, int> GroupInvert;

private:
  vtkCMBContourGroupFilter(const vtkCMBContourGroupFilter&);  // Not implemented.
  void operator=(const vtkCMBContourGroupFilter&);  // Not implemented.
};

#endif
