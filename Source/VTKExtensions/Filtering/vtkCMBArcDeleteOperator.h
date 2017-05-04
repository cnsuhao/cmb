//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcDeleteOperator - Split an Arc
// .SECTION Description
// Operator to delete a single arc

#ifndef __vtkCMBArcDeleteOperator_h
#define __vtkCMBArcDeleteOperator_h

#include "cmbSystemConfig.h"
#include "vtkABI.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcDeleteOperator : public vtkObject
{
public:
  static vtkCMBArcDeleteOperator* New();
  vtkTypeMacro(vtkCMBArcDeleteOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Set the mode for the delete operator to work in
  //The default mode is real delete which is 0.
  // 1 == marked for delete ( added to the client undo stack )
  // 2 == unmarked for delete ( remove from undo stack )
  vtkSetMacro(DeleteMode, int);

  //Description:
  //Delete the arc id that is passed in
  bool Operate(vtkIdType arcId);

protected:
  vtkCMBArcDeleteOperator();
  ~vtkCMBArcDeleteOperator() override;
  int DeleteMode;

private:
  vtkCMBArcDeleteOperator(const vtkCMBArcDeleteOperator&); // Not implemented.
  void operator=(const vtkCMBArcDeleteOperator&);          // Not implemented.
};

#endif
