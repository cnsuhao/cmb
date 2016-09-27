//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcDeleteClientOperator - Delete a vtkCMBArc on the server
// .SECTION Description
// Delete a vtkCMBArc on the server
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkCMBArcDeleteClientOperator_h
#define __vtkCMBArcDeleteClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkCMBArcDeleteClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcDeleteClientOperator* New();
  vtkTypeMacro(vtkCMBArcDeleteClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Delete the arc with the provided id on the server
  virtual bool DeleteArc(const vtkIdType& arcId);

  // Description:
  // Remove the arc from the manager but don't actually delete this.
  // This is means the arc is on the undo redo stack of the client
  virtual bool SetMarkedForDeletion(const vtkIdType& arcId);

  // Description:
  // Return the arc to the manager but don't actually delete it.
  // This is means the arc is off the undo stack on the client
  virtual bool SetUnMarkedForDeletion(const vtkIdType& arcId);

protected:
  vtkCMBArcDeleteClientOperator();
  ~vtkCMBArcDeleteClientOperator() override;

  enum Mode{
    Delete_Mode = 0,
    Mark_Mode = 1,
    UnMark_Mode = 2
    };

  virtual bool Operate(const vtkIdType& arcId, vtkCMBArcDeleteClientOperator::Mode mode);

private:
  vtkCMBArcDeleteClientOperator(const vtkCMBArcDeleteClientOperator&); // Not implemented
  void operator=(const vtkCMBArcDeleteClientOperator&); // Not implemented
};

#endif
