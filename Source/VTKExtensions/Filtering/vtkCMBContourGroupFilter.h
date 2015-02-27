/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
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
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkCMBContourGroupFilter();

  virtual int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);

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
