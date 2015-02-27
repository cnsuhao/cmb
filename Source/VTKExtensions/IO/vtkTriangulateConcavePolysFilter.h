/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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


