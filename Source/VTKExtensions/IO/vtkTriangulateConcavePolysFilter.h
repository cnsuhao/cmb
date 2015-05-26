//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkTriangulateConcavePolysFilter - triangulate any concave polygons
// .SECTION Description
// This filter takes input polydata and triangulates any concave polygons,
// passing all other data through (cell data for a concave polygon is copied
// to each of the output triangles resulting from that polygon)

#ifndef __vtkTriangulateConcavePolysFilter_h
#define __vtkTriangulateConcavePolysFilter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBIO_EXPORT vtkTriangulateConcavePolysFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkTriangulateConcavePolysFilter* New();
  vtkTypeMacro(vtkTriangulateConcavePolysFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Tests whether the cell is concave
  static bool IsPolygonConcave(vtkPoints *points, vtkIdType npts, vtkIdType *pts);

//BTX
protected:
  vtkTriangulateConcavePolysFilter() {};
  ~vtkTriangulateConcavePolysFilter() {};

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkTriangulateConcavePolysFilter(const vtkTriangulateConcavePolysFilter&); // Not implemented.
  void operator=(const vtkTriangulateConcavePolysFilter&); // Not implemented.
//ETX
};

#endif


