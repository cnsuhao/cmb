//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcMergeArcsOperator - Split an Arc
// .SECTION Description
// Operator to merge multiple arcs into a single arc.

#ifndef __vtkCMBArcMergeArcsOperator_h
#define __vtkCMBArcMergeArcsOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcMergeArcsOperator : public vtkObject
{
public:
  static vtkCMBArcMergeArcsOperator * New();
  vtkTypeMacro(vtkCMBArcMergeArcsOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  bool Operate(vtkIdType firstId, vtkIdType secondId);

  //Description:
  //If the merge work this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId,vtkIdType);

  //Description:
  //If the merge work this is the ArcId that needs to be deleted
  vtkGetMacro(ArcIdToDelete,vtkIdType);


protected:
  vtkCMBArcMergeArcsOperator();
  virtual ~vtkCMBArcMergeArcsOperator();

  vtkIdType CreatedArcId;
  vtkIdType ArcIdToDelete;

private:
  vtkCMBArcMergeArcsOperator(const vtkCMBArcMergeArcsOperator&);  // Not implemented.
  void operator=(const vtkCMBArcMergeArcsOperator&);  // Not implemented.
//ETX
};

#endif
