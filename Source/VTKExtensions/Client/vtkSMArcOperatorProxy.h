//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMArcOperatorProxy
// .SECTION Description
//

#ifndef __vtkSMArcOperatorProxy_h
#define __vtkSMArcOperatorProxy_h

#include "vtkSMProxy.h"
#include "vtkCMBClientModule.h" // For export macro
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;
class vtkSMOutputPort;
class vtkSMSourceProxy;

class VTKCMBCLIENT_EXPORT vtkSMArcOperatorProxy : public vtkSMProxy
{
public:
  static vtkSMArcOperatorProxy* New();
  vtkTypeMacro(vtkSMArcOperatorProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Operate on the source proxy on the server
  //this is used by update and create operators
  virtual bool Operate(vtkSMSourceProxy* sourceProxy);

  // Description:
  // Operate on the source proxy on the server
  //this is used by update and create operators
  virtual bool Operate(vtkSMOutputPort* sourceOutputPort);

  // Description:
  // Operate on the source proxy on the server
  //this is used by update and create operators
  virtual bool Operate(vtkSMNewWidgetRepresentationProxy* sourceProxy);

  // Description:
  // Operate on the arc on the server represented by this id
  virtual bool Operate(vtkIdType arcId);

  // Description:
  // Operate on the arc on the server represented by the first id
  // passing in the second id as another arc to work with.
  // This is used by auto connect and merge arcs
  // NEW ADDITION:
  // Operate on the sub-arc on the server represented start and end point id
  // This is used by sub-arc modify Operator
  virtual bool Operate(vtkIdType firstId, vtkIdType secondId);

  // Description:
  // Call Operate on the server
  // used by grow arcs
  virtual bool Operate();

protected:
  vtkSMArcOperatorProxy();
  ~vtkSMArcOperatorProxy();

private:
  vtkSMArcOperatorProxy(const vtkSMArcOperatorProxy&); // Not implemented
  void operator=(const vtkSMArcOperatorProxy&); // Not implemented
};

#endif
