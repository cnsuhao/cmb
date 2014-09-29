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
// .NAME vtkSMOperatorProxy
// .SECTION Description
//

#ifndef __vtkSMOperatorProxy_h
#define __vtkSMOperatorProxy_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSMProxy.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkSMOperatorProxy : public vtkSMProxy
{
public:
  static vtkSMOperatorProxy* New();
  vtkTypeMacro(vtkSMOperatorProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
  // Description:
  // Operate on the model on the server.
  virtual void Operate(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Operate on the model on the server with a given input proxy.
  virtual void Operate(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy,
                       vtkSMProxy* InputProxy);

  // Description:
  // Build an object on the model on the server.
  virtual vtkIdType Build(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Destroy, if possible, an object on the model on the server.
  virtual bool Destroy(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

protected:
  vtkSMOperatorProxy();
  ~vtkSMOperatorProxy();

private:
  vtkSMOperatorProxy(const vtkSMOperatorProxy&); // Not implemented
  void operator=(const vtkSMOperatorProxy&); // Not implemented
//ETX
};

#endif
