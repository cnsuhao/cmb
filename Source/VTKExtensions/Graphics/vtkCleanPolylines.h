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
// .NAME vtkCleanPolylines - filter removes polylines below threshold length
// .SECTION Description
// The filter removes polyline below threshold length.  Before analysis,
// duplicate points are merged, and then any input lines are stripped.  Then,
// before discarding any polylines, the polylines are appended to adjacent
// polylines where possible.

#ifndef __vtkCleanPolylines_h
#define __vtkCleanPolylines_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGRAPHICS_EXPORT vtkCleanPolylines : public vtkPolyDataAlgorithm
{
public:
  static vtkCleanPolylines *New();
  vtkTypeMacro(vtkCleanPolylines,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetClampMacro(MinimumLineLength, double, 0, VTK_FLOAT_MAX);
  vtkGetMacro(MinimumLineLength, double);

  // Description:
  // If "on", the average line length is multiplied by the MinimumLineLength
  // to determine the line test length
  vtkBooleanMacro(UseRelativeLineLength, bool);
  vtkSetMacro(UseRelativeLineLength, bool);
  vtkGetMacro(UseRelativeLineLength, bool);

//BTX
protected:
  vtkCleanPolylines();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void TraverseLine(vtkIdType startPid, vtkIdType startCellId,
        vtkPolyData *input, unsigned char *marks,
        vtkIdList *ids, double *length,
                    vtkIdType *lastLineId);
  void StripLines(vtkPolyData *input, vtkPolyData *result,
                  vtkDoubleArray *lengths);
  void RemoveNonManifoldFeatures(vtkPolyData *input, vtkDoubleArray *lengths,
                                 vtkPolyData *result,
                                 vtkDoubleArray *newLengths);
  void TraversePolyLine(vtkIdType startPid, vtkIdType startCellId,
                        vtkPolyData *input, vtkDoubleArray *lengths,
                        unsigned char *marks,
                        vtkIdList *ids, double *length);

private:
  vtkCleanPolylines(const vtkCleanPolylines&);  // Not implemented.
  void operator=(const vtkCleanPolylines&);  // Not implemented.

  double MinimumLineLength;
  bool UseRelativeLineLength;
//ETX
};

#endif
