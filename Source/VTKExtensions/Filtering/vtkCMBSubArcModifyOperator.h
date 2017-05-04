//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSubArcModifyOperator - Modify an Arc with certain opeartions
// .SECTION Description
// Operator to modify an arc, straighten (can be used on open whole arc), collapse

#ifndef __vtkCMBSubArcModifyOperator_h
#define __vtkCMBSubArcModifyOperator_h

#include "cmbSystemConfig.h"
#include "vtkABI.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"

class vtkCMBArc;
class vtkPolyData;

class VTKCMBFILTERING_EXPORT vtkCMBSubArcModifyOperator : public vtkObject
{
public:
  static vtkCMBSubArcModifyOperator* New();
  vtkTypeMacro(vtkCMBSubArcModifyOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //BTX
  enum EnumOperationType
  {
    OpNONE = 0,
    OpSTRAIGHTEN = 1, // remove all internal points between two ends
    OpCOLLAPSE = 2    // collapse the endPoint into startPoint
  };
  //ETX

  //Description:
  //the arc to do operations
  vtkSetMacro(ArcId, vtkIdType);
  vtkGetMacro(ArcId, vtkIdType);

  //Description:
  //Sets the type of operations, see EnumOperationType
  vtkSetClampMacro(OperationType, int, OpNONE, OpCOLLAPSE);
  vtkGetMacro(OperationType, int);

  //Description:
  //Do operations on the arc based on the given start and end point
  bool Operate(vtkIdType startPointId, vtkIdType endPointId);

protected:
  vtkCMBSubArcModifyOperator();
  ~vtkCMBSubArcModifyOperator() override;

  // clears the operator back to default values
  void Reset();
  // Update the specified sub-arc with operations
  virtual bool StraightenSubArc(
    vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc);
  virtual bool CollapseSubArc(vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc);

  vtkIdType ArcId;
  int OperationType;

private:
  vtkCMBSubArcModifyOperator(const vtkCMBSubArcModifyOperator&); // Not implemented.
  void operator=(const vtkCMBSubArcModifyOperator&);             // Not implemented.
};

#endif
