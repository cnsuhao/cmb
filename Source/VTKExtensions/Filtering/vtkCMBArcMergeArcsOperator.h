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
// .NAME vtkCMBArcMergeArcsOperator - Split an Arc
// .SECTION Description
// Operator to merge multiple arcs into a single arc.

#ifndef __vtkCMBArcMergeArcsOperator_h
#define __vtkCMBArcMergeArcsOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcMergeArcsOperator : public vtkObject
{
public:
  static vtkCMBArcMergeArcsOperator * New();
  vtkTypeMacro(vtkCMBArcMergeArcsOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  bool Operate(vtkIdType firstId, vtkIdType secondId);

  //Description:
  //If the merge work this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId,vtkIdType);

  //Description:
  //If the merge work this is the ArcId that needs to be deleted
  vtkGetMacro(ArcIdToDelete,vtkIdType);


protected:
  vtkCMBArcMergeArcsOperator();
  virtual ~vtkCMBArcMergeArcsOperator();

  vtkIdType CreatedArcId;
  vtkIdType ArcIdToDelete;

private:
  vtkCMBArcMergeArcsOperator(const vtkCMBArcMergeArcsOperator&);  // Not implemented.
  void operator=(const vtkCMBArcMergeArcsOperator&);  // Not implemented.
//ETX
};

#endif
