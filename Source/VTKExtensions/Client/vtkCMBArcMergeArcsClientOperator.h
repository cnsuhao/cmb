//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcMergeArcsClientOperator
// .SECTION Description
//  Merge the two passed in arcs into a single arc
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkCMBArcMergeArcsClientOperator_h
#define __vtkCMBArcMergeArcsClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkCMBArcMergeArcsClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcMergeArcsClientOperator* New();
  vtkTypeMacro(vtkCMBArcMergeArcsClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  // Returns the arc id of the merged arc
  vtkGetMacro(ArcId,vtkIdType);

  //Description:
  // Returns the arc that needs to be deleted after the merge
  vtkGetMacro(ArcIdToDelete,vtkIdType);

  // Description:
  // Merge the two arcs together
  virtual bool Operate(const vtkIdType& firstArcId, const vtkIdType& secondArcId);

protected:
  vtkCMBArcMergeArcsClientOperator();
  ~vtkCMBArcMergeArcsClientOperator() override;

  vtkIdType ArcId;
  vtkIdType ArcIdToDelete;

private:
  vtkCMBArcMergeArcsClientOperator(const vtkCMBArcMergeArcsClientOperator&); // Not implemented
  void operator=(const vtkCMBArcMergeArcsClientOperator&); // Not implemented
};

#endif
