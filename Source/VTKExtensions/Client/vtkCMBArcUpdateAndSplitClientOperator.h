//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcUpdateAndSplitClientOperator
// .SECTION Description
// Take a widget representation and convert it down to the arcs it represents.
// This requires taking the shape and updating the passed in arc with that shape.
// Than it requires taking that arc and splitting it on each index that the
// user has selected to be an end node
// by the vtk.
// Note:
// This will create new arcs on the server so it will return to the caller
// the arc ids it has created
// .SECTION See Also
// vtkSMSourceProxy vtkCMBArcUpdateOperator vtkCMBArcSplitOnIndexOperator


#ifndef __vtkCMBArcUpdateAndSplitClientOperator_h
#define __vtkCMBArcUpdateAndSplitClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"

class vtkIdTypeArray;
class vtkSMTKArcRepresentation;
class vtkSMNewWidgetRepresentationProxy;
class vtkPolyData;

class VTKCMBCLIENT_EXPORT vtkCMBArcUpdateAndSplitClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcUpdateAndSplitClientOperator* New();
  vtkTypeMacro(vtkCMBArcUpdateAndSplitClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Update
  virtual bool Operate( vtkIdType arcId, vtkSMNewWidgetRepresentationProxy *widgetProxy);

  vtkGetObjectMacro(CreatedArcs,vtkIdTypeArray);
protected:
  vtkCMBArcUpdateAndSplitClientOperator();
  ~vtkCMBArcUpdateAndSplitClientOperator() override;

  bool FindArcsInWidgetOutput(vtkSMTKArcRepresentation *widgetRep);
  bool UpdateOperation(const vtkIdType& arcId,
                       vtkSMNewWidgetRepresentationProxy *widgetProxy,
                       vtkSMTKArcRepresentation *widgetRep);
  bool SplitOperation(const vtkIdType& arcId,
                      vtkSMTKArcRepresentation *widgetRep);

  vtkIdTypeArray* CreatedArcs;
  class InternalInfo;
  InternalInfo *Info;

private:
  vtkCMBArcUpdateAndSplitClientOperator(const vtkCMBArcUpdateAndSplitClientOperator&); // Not implemented
  void operator=(const vtkCMBArcUpdateAndSplitClientOperator&); // Not implemented
};

#endif
