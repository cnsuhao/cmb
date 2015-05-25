//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcSnapClientOperator - Create a vtkCMBArc on the server
// .SECTION Description
// Create a vtkCMBArc on the server by using the polydata from
// by the vtkCMBArcSnapClientOperator.
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkCMBArcSnapClientOperator_h
#define __vtkCMBArcSnapClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkCMBArcSnapClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcSnapClientOperator* New();
  vtkTypeMacro(vtkCMBArcSnapClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  // Returns the radius of the snapping setting.
  // A radius of zero or less means snapping isn't active
  double GetCurrentRadius();

  // Description:
  // Set the server snapping radius to value passed in.
  // A radius of zero or less will disable snapping
  virtual bool Operate(const double& radius);

protected:
  vtkCMBArcSnapClientOperator();
  ~vtkCMBArcSnapClientOperator();

private:
  vtkCMBArcSnapClientOperator(const vtkCMBArcSnapClientOperator&); // Not implemented
  void operator=(const vtkCMBArcSnapClientOperator&); // Not implemented
};

#endif
