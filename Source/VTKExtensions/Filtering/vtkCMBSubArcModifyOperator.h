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
// .NAME vtkCMBSubArcModifyOperator - Modify an Arc with certain opeartions
// .SECTION Description
// Operator to modify an arc, straighten (can be used on open whole arc), collapse

#ifndef __vtkCMBSubArcModifyOperator_h
#define __vtkCMBSubArcModifyOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class vtkCMBArc;
class vtkPolyData;

class VTKCMBFILTERING_EXPORT vtkCMBSubArcModifyOperator : public vtkObject
{
public:
  static vtkCMBSubArcModifyOperator * New();
  vtkTypeMacro(vtkCMBSubArcModifyOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  enum EnumOperationType
    {
    OpNONE = 0,
    OpSTRAIGHTEN = 1, // remove all internal points between two ends
    OpCOLLAPSE = 2 // collapse the endPoint into startPoint
    };
  //ETX

  //Description:
  //the arc to do operations
  vtkSetMacro(ArcId, vtkIdType);
  vtkGetMacro(ArcId, vtkIdType);

  //Description:
  //Sets the type of operations, see EnumOperationType
  vtkSetClampMacro(OperationType, int, OpNONE, OpCOLLAPSE);
  vtkGetMacro(OperationType,int);

  //Description:
  //Do operations on the arc based on the given start and end point
  bool Operate(vtkIdType startPointId, vtkIdType endPointId);

protected:
  vtkCMBSubArcModifyOperator();
  virtual ~vtkCMBSubArcModifyOperator();

  // clears the operator back to default values
  void Reset();
  // Update the specified sub-arc with operations
  virtual bool StraightenSubArc(
    vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc);
  virtual bool CollapseSubArc(
    vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc);

  vtkIdType ArcId;
  int OperationType;

private:
  vtkCMBSubArcModifyOperator(const vtkCMBSubArcModifyOperator&);  // Not implemented.
  void operator=(const vtkCMBSubArcModifyOperator&);  // Not implemented.
};

#endif
