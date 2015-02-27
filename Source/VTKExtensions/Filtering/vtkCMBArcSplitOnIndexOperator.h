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
// .NAME vtkCMBArcSplitOnIndexOperator - Split an Arc
// .SECTION Description
// Operator to split an arc on a given point position
// If you want to split on the Nth internal point of an arc
// you should use vtkCMBArcSplitOnIndexOperator

#ifndef __vtkCMBArcSplitOnIndexOperator_h
#define __vtkCMBArcSplitOnIndexOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"
class VTKCMBFILTERING_EXPORT vtkCMBArcSplitOnIndexOperator : public vtkObject
{
public:
  static vtkCMBArcSplitOnIndexOperator * New();
  vtkTypeMacro(vtkCMBArcSplitOnIndexOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Point Index to split on. If passed in 0 it will
  //split on the first non end node point. If the value
  //is greater than the number of non end node points the spit
  //will fail.
  vtkSetMacro(Index,int);
  vtkGetMacro(Index,int);

  //Description:
  //Split on arc with given Id
  bool Operate(vtkIdType arcId);

  //Description:
  //If the Split work this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId,vtkIdType);

protected:
  vtkCMBArcSplitOnIndexOperator();
  virtual ~vtkCMBArcSplitOnIndexOperator();

  int Index;
  vtkIdType CreatedArcId;
private:
  vtkCMBArcSplitOnIndexOperator(const vtkCMBArcSplitOnIndexOperator&);  // Not implemented.
  void operator=(const vtkCMBArcSplitOnIndexOperator&);  // Not implemented.
//ETX
};

#endif
