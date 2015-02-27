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
// .NAME vtkCMBArcPickPointOperator - Split an Arc
// .SECTION Description
// Operator to merge multiple arcs into a single arc.

#ifndef __vtkCMBArcPickPointOperator_h
#define __vtkCMBArcPickPointOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class vtkPVExtractSelection;
class VTKCMBFILTERING_EXPORT vtkCMBArcPickPointOperator : public vtkObject
{
public:
  static vtkCMBArcPickPointOperator * New();
  vtkTypeMacro(vtkCMBArcPickPointOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Convert the passed in the selection into a selection that only has
  //a single point.
  bool Operate(vtkPVExtractSelection *source);
  vtkSetMacro(ArcId,vtkIdType);
  vtkGetMacro(ArcId,vtkIdType);

  vtkGetMacro(PickedPointId,vtkIdType);
protected:
  vtkCMBArcPickPointOperator();
  virtual ~vtkCMBArcPickPointOperator();

  vtkIdType ArcId;
  vtkIdType PickedPointId;

private:
  vtkCMBArcPickPointOperator(const vtkCMBArcPickPointOperator&);  // Not implemented.
  void operator=(const vtkCMBArcPickPointOperator&);  // Not implemented.
};

#endif
