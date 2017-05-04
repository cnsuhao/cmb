//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcAutoConnectOperator - Split an Arc
// .SECTION Description
// Operator to create a third arc between two existing arcs

#ifndef __vtkCMBArcAutoConnectOperator_h
#define __vtkCMBArcAutoConnectOperator_h

#include "cmbSystemConfig.h"
#include "vtkABI.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcAutoConnectOperator : public vtkObject
{
public:
  static vtkCMBArcAutoConnectOperator* New();
  vtkTypeMacro(vtkCMBArcAutoConnectOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool Operate(vtkIdType firstId, vtkIdType secondId);

  //Description:
  //If the auto connect works this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId, vtkIdType);

protected:
  vtkCMBArcAutoConnectOperator();
  ~vtkCMBArcAutoConnectOperator() override;

  vtkIdType CreatedArcId;

private:
  vtkCMBArcAutoConnectOperator(const vtkCMBArcAutoConnectOperator&); // Not implemented.
  void operator=(const vtkCMBArcAutoConnectOperator&);               // Not implemented.
  //ETX
};

#endif
