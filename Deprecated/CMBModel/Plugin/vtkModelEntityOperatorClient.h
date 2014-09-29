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
// .NAME vtkModelEntityOperatorClient - Change properties of model entities.
// .SECTION Description
// Operator to change the color (RGBA), the user name and/or visibility of a
// vtkModelEntity on the client.

#ifndef __vtkModelEntityOperatorClient_h
#define __vtkModelEntityOperatorClient_h

#include "vtkModelEntityOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEntityOperatorClient : public vtkModelEntityOperatorBase
{
public:
  static vtkModelEntityOperatorClient * New();
  vtkTypeMacro(vtkModelEntityOperatorClient,vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, username, and/or the visibility of an object.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEntityOperatorClient();
  virtual ~vtkModelEntityOperatorClient();

private:
  vtkModelEntityOperatorClient(const vtkModelEntityOperatorClient&);  // Not implemented.
  void operator=(const vtkModelEntityOperatorClient&);  // Not implemented.
};

#endif