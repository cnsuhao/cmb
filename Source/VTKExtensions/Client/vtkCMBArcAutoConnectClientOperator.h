//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcAutoConnectClientOperator
// .SECTION Description
//  Create a third arc that connects the two passed in arcs
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy

#ifndef __vtkCMBArcAutoConnectClientOperator_h
#define __vtkCMBArcAutoConnectClientOperator_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"

class VTKCMBCLIENT_EXPORT vtkCMBArcAutoConnectClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcAutoConnectClientOperator* New();
  vtkTypeMacro(vtkCMBArcAutoConnectClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  // Returns the arc id of the newly created arc
  vtkGetMacro(ArcId, vtkIdType);

  // Description:
  // Connect the two arcs with the provided id on the server
  virtual bool Operate(const vtkIdType& firstArcId, const vtkIdType& secondArcId);

protected:
  vtkCMBArcAutoConnectClientOperator();
  ~vtkCMBArcAutoConnectClientOperator() override;
  vtkIdType ArcId;

private:
  vtkCMBArcAutoConnectClientOperator(const vtkCMBArcAutoConnectClientOperator&); // Not implemented
  void operator=(const vtkCMBArcAutoConnectClientOperator&);                     // Not implemented
};

#endif
