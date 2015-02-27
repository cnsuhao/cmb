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
// .NAME vtkCMBArcGrowOperator - Split an Arc
// .SECTION Description
// Operator to grow the current 'selection' of arcs

#ifndef __vtkCMBArcGrowOperator_h
#define __vtkCMBArcGrowOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"
#include <set>

class vtkIdTypeArray;
class VTKCMBFILTERING_EXPORT vtkCMBArcGrowOperator : public vtkObject
{
public:
  static vtkCMBArcGrowOperator * New();
  vtkTypeMacro(vtkCMBArcGrowOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Add an Id of an arc to the set of arcs to grow from
  void AddArc(vtkIdType arcId);

  //Description:
  //Clear the input arcid collection
  void ClearInputArcs();

  //Description:
  //Calling this will clear the input arcs
  bool Operate();

  //Description:
  //If the grow works this these are the arc ids of the grow
  vtkGetObjectMacro(GrownArcSetIds,vtkIdTypeArray);



protected:
  vtkCMBArcGrowOperator();
  virtual ~vtkCMBArcGrowOperator();

  typedef std::set<vtkIdType> ArcSet;
  ArcSet InputArcs;
  vtkIdTypeArray* GrownArcSetIds;

private:
  vtkCMBArcGrowOperator(const vtkCMBArcGrowOperator&);  // Not implemented.
  void operator=(const vtkCMBArcGrowOperator&);  // Not implemented.
//ETX
};

#endif
