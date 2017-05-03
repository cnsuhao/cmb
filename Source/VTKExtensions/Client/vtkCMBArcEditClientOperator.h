//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcEditClientOperator - Move an Arcs shape to the edit widget
// .SECTION Description
// Create a widget representation for editing by using the
// server side representation of the arc
// by the vtk.
// .SECTION See Also
// vtkSMSourceProxy

#ifndef __vtkCMBArcEditClientOperator_h
#define __vtkCMBArcEditClientOperator_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"

class vtkSMProxy;
class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkCMBArcEditClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcEditClientOperator* New();
  vtkTypeMacro(vtkCMBArcEditClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Copy the proxy poly data into the widget representation
  virtual bool Operate(vtkSMProxy* sourceProxy, vtkSMNewWidgetRepresentationProxy* widgetProxy);

  //Description:
  //Set if the arc is a closed loop, this is needed
  //to properly setup the state of the widget.
  //Default is false
  vtkSetMacro(ArcIsClosed, bool);

protected:
  vtkCMBArcEditClientOperator();
  ~vtkCMBArcEditClientOperator() override;
  bool ArcIsClosed;

private:
  vtkCMBArcEditClientOperator(const vtkCMBArcEditClientOperator&); // Not implemented
  void operator=(const vtkCMBArcEditClientOperator&);              // Not implemented
};

#endif
