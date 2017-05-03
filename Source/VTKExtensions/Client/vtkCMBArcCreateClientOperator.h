//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcCreateClientOperator - Create a vtkCMBArc on the server
// .SECTION Description
// Create a vtkCMBArc on the server by using the polydata from
// by the vtkCMBArcCreateClientOperator.
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy

#ifndef __vtkCMBArcCreateClientOperator_h
#define __vtkCMBArcCreateClientOperator_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkCMBArcCreateClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcCreateClientOperator* New();
  vtkTypeMacro(vtkCMBArcCreateClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  // Returns the arc id of the newly created arc
  vtkGetMacro(ArcId, vtkIdType);

  // Description:
  // Copies data from a widget proxy to a vtkCMBArc
  virtual bool Create(vtkSMNewWidgetRepresentationProxy* widgetProxy);

  // Description:
  // Copies data from a source proxy to a vtkCMBArc
  // This is generally used to copy from some reader / filter
  // that generates polydata output
  virtual bool Create(vtkSMSourceProxy* sourceProxy);

protected:
  vtkCMBArcCreateClientOperator();
  ~vtkCMBArcCreateClientOperator() override;
  vtkIdType ArcId;

private:
  vtkCMBArcCreateClientOperator(const vtkCMBArcCreateClientOperator&); // Not implemented
  void operator=(const vtkCMBArcCreateClientOperator&);                // Not implemented
};

#endif
