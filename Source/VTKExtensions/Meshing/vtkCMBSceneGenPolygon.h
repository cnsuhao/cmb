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
// .NAME vtkCMBSceneGenPolygon
// .SECTION Description

#ifndef __vtkCMBSceneGenPolygon_h
#define __vtkCMBSceneGenPolygon_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class vtkCellLocator;
class vtkGenericCell;

class VTKCMBMESHING_EXPORT vtkCMBSceneGenPolygon : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBSceneGenPolygon *New();
  vtkTypeMacro(vtkCMBSceneGenPolygon,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkCMBSceneGenPolygon();
  ~vtkCMBSceneGenPolygon();

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  bool PrepForMeshing(vtkInformationVector* input, vtkPolyData *mesh);
  void AppendArcSets(vtkInformationVector* input, vtkPolyData *mesh);
  bool DeterminePolygon(vtkPolyData *mesh);

private:

  bool FindStartingCellForMesh(vtkCellLocator *locator,vtkGenericCell *cell,vtkIdType &cellId, double bounds[6]);

  vtkCMBSceneGenPolygon(const vtkCMBSceneGenPolygon&);  // Not implemented.
  void operator=(const vtkCMBSceneGenPolygon&);  // Not implemented.
};

#endif
