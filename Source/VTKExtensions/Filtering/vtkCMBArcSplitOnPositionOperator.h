//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
