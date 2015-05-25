//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcCreateOperator - Split an Arc
// .SECTION Description
// Operator to merge multiple arcs into a single arc.

#ifndef __vtkCMBArcCreateOperator_h
#define __vtkCMBArcCreateOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class vtkPolyData;
class VTKCMBFILTERING_EXPORT vtkCMBArcCreateOperator : public vtkObject
{
public:
  static vtkCMBArcCreateOperator * New();
  vtkTypeMacro(vtkCMBArcCreateOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Convert the passed in the polydata into an arc
  bool Operate(vtkPolyData *source);

  //Description:
  //If the merge work this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId,vtkIdType);

protected:
  vtkCMBArcCreateOperator();
  virtual ~vtkCMBArcCreateOperator();

  vtkIdType CreatedArcId;
private:
  vtkCMBArcCreateOperator(const vtkCMBArcCreateOperator&);  // Not implemented.
  void operator=(const vtkCMBArcCreateOperator&);  // Not implemented.
};

#endif
