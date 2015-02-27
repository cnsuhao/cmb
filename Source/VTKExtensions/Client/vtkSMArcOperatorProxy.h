/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
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
