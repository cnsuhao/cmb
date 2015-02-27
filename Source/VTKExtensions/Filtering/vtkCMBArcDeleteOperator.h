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
// .NAME vtkCMBArcDeleteOperator - Split an Arc
// .SECTION Description
// Operator to delete a single arc

#ifndef __vtkCMBArcDeleteOperator_h
#define __vtkCMBArcDeleteOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcDeleteOperator : public vtkObject
{
public:
  static vtkCMBArcDeleteOperator * New();
  vtkTypeMacro(vtkCMBArcDeleteOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Set the mode for the delete operator to work in
  //The default mode is real delete which is 0.
  // 1 == marked for delete ( added to the client undo stack )
  // 2 == unmarked for delete ( remove from undo stack )
  vtkSetMacro(DeleteMode,int);

  //Description:
  //Delete the arc id that is passed in
  bool Operate(vtkIdType arcId);

protected:
  vtkCMBArcDeleteOperator();
  virtual ~vtkCMBArcDeleteOperator();
  int DeleteMode;

private:
  vtkCMBArcDeleteOperator(const vtkCMBArcDeleteOperator&);  // Not implemented.
  void operator=(const vtkCMBArcDeleteOperator&);  // Not implemented.
};

#endif
