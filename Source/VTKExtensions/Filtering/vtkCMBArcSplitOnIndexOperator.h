//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
