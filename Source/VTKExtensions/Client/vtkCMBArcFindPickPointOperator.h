//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcFindPickPointOperator
// .SECTION Description
//  Condense the selection on an Arc to be only the id that is in the middle
//  off the selection.
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkCMBArcFindPickPointOperator_h
#define __vtkCMBArcFindPickPointOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"
class vtkSMOutputPort;

class VTKCMBCLIENT_EXPORT vtkCMBArcFindPickPointOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcFindPickPointOperator* New();
  vtkTypeMacro(vtkCMBArcFindPickPointOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // convert the multi point selection to the middle point of the selection.
  virtual bool Operate(const vtkIdType& arcId, vtkSMOutputPort *selectionPort);

protected:
  vtkCMBArcFindPickPointOperator();
  ~vtkCMBArcFindPickPointOperator() override;
  vtkIdType PickedPointId;

private:
  vtkCMBArcFindPickPointOperator(const vtkCMBArcFindPickPointOperator&); // Not implemented
  void operator=(const vtkCMBArcFindPickPointOperator&); // Not implemented
};

#endif
