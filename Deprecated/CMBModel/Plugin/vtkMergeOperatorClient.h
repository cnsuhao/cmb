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
// .NAME vtkMergeOperatorClient - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the client.  It also calls the operator
// on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkMergeOperatorClient_h
#define __vtkMergeOperatorClient_h

#include "vtkMergeOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkMergeOperatorClient : public vtkMergeOperatorBase
{
public:
  static vtkMergeOperatorClient * New();
  vtkTypeMacro(vtkMergeOperatorClient,vtkMergeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Merge the specified geometric model entities on both the client
  // and the server.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkMergeOperatorClient();
  virtual ~vtkMergeOperatorClient();

private:
  vtkMergeOperatorClient(const vtkMergeOperatorClient&);  // Not implemented.
  void operator=(const vtkMergeOperatorClient&);  // Not implemented.
};

#endif