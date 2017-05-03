//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcPickPointOperator - Split an Arc
// .SECTION Description
// Operator to merge multiple arcs into a single arc.

#ifndef __vtkCMBArcPickPointOperator_h
#define __vtkCMBArcPickPointOperator_h

#include "cmbSystemConfig.h"
#include "vtkABI.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"

class vtkPVExtractSelection;
class VTKCMBFILTERING_EXPORT vtkCMBArcPickPointOperator : public vtkObject
{
public:
  static vtkCMBArcPickPointOperator* New();
  vtkTypeMacro(vtkCMBArcPickPointOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Convert the passed in the selection into a selection that only has
  //a single point.
  bool Operate(vtkPVExtractSelection* source);
  vtkSetMacro(ArcId, vtkIdType);
  vtkGetMacro(ArcId, vtkIdType);

  vtkGetMacro(PickedPointId, vtkIdType);

protected:
  vtkCMBArcPickPointOperator();
  ~vtkCMBArcPickPointOperator() override;

  vtkIdType ArcId;
  vtkIdType PickedPointId;

private:
  vtkCMBArcPickPointOperator(const vtkCMBArcPickPointOperator&); // Not implemented.
  void operator=(const vtkCMBArcPickPointOperator&);             // Not implemented.
};

#endif
