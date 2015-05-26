//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcGrowClientOperator
// .SECTION Description
//  Create a third arc that connects the two passed in arcs
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkCMBArcGrowClientOperator_h
#define __vtkCMBArcGrowClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"
#include <list>

class vtkIdTypeArray;

class VTKCMBCLIENT_EXPORT vtkCMBArcGrowClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcGrowClientOperator* New();
  vtkTypeMacro(vtkCMBArcGrowClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //If the grow works this these are the arc ids of the grow
  vtkGetObjectMacro(GrownArcIds,vtkIdTypeArray);

  // Description:
  // Grow
  virtual bool Operate(std::list<vtkIdType> arcIds);

protected:
  vtkCMBArcGrowClientOperator();
  ~vtkCMBArcGrowClientOperator();

  vtkIdTypeArray* GrownArcIds;

private:
  vtkCMBArcGrowClientOperator(const vtkCMBArcGrowClientOperator&); // Not implemented
  void operator=(const vtkCMBArcGrowClientOperator&); // Not implemented
};

#endif
