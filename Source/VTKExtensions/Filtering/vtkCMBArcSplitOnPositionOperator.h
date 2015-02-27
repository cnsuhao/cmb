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
// .NAME vtkCMBArcSplitOnPositionOperator - Split an Arc
// .SECTION Description
// Operator to split an arc on a given point position
// If you want to split on the Nth internal point of an arc
// you should use vtkCMBArcSplitOnIndexOperator

#ifndef __vtkCMBArcSplitOnPositionOperator_h
#define __vtkCMBArcSplitOnPositionOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkCMBArcSplitOnPositionOperator : public vtkObject
{
public:
  static vtkCMBArcSplitOnPositionOperator * New();
  vtkTypeMacro(vtkCMBArcSplitOnPositionOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Tolerance to consider the split position equal to a
  //point on the Arc. Default is 1.0e-05
  vtkSetMacro(PositionTolerance,double);
  vtkGetMacro(PositionTolerance,double);

  void SetSplitPosition(double x, double y, double z);
  void SetSplitPosition(double pos[3])
    {
    this->SetSplitPosition(pos[0],pos[1],pos[2]);
    }
  vtkGetVector3Macro(SplitPosition,double);

  bool Operate(vtkIdType arcId);

  //Description:
  //If the Split work this is the ArcId for the newly
  //created arc
  vtkGetMacro(CreatedArcId,vtkIdType);

protected:
  vtkCMBArcSplitOnPositionOperator();
  virtual ~vtkCMBArcSplitOnPositionOperator();

  double PositionTolerance;
  double SplitPosition[3];
  bool ValidPosition;

  vtkIdType CreatedArcId;
private:
  vtkCMBArcSplitOnPositionOperator(const vtkCMBArcSplitOnPositionOperator&);  // Not implemented.
  void operator=(const vtkCMBArcSplitOnPositionOperator&);  // Not implemented.
//ETX
};

#endif
