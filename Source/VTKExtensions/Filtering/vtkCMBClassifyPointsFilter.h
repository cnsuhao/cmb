/*=========================================================================

Copyright (c) 1998-2010 Kitware Inc. 28 Corporate Drive, Suite 204,
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
// .NAME vtkCMBClassifyPointsFilter - classifies a set points with respects to a solid mesh
// .SECTION Description
// vtkCMBClassifyPointsFilter classifies a set of points with respects to input.  If a point does not lie inside of a cell
// it will be omitted.  Else the point will be added to the set and the cell's ID will be added
// to the point data of the filter's output.

#ifndef __vtkCMBClassifyPointsFilter_h
#define __vtkCMBClassifyPointsFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkPoints;
class vtkIdTypeArray;

class VTKCMBFILTERING_EXPORT vtkCMBClassifyPointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBClassifyPointsFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkCMBClassifyPointsFilter *New();

  // Description:
  // Specify the solid mesh to be used. Any geometry
  // can be used. New style. Equivalent to SetInputConnection(1, algOutput).
  void SetSolidConnection(vtkAlgorithmOutput* algOutput);

protected:
  vtkCMBClassifyPointsFilter();
  ~vtkCMBClassifyPointsFilter() {}
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
private:
  vtkCMBClassifyPointsFilter(const vtkCMBClassifyPointsFilter&);  // Not implemented.
  void operator=(const vtkCMBClassifyPointsFilter&);  // Not implemented.
};

#endif


