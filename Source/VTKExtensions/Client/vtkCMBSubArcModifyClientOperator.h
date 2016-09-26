//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSubArcModifyClientOperator
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


#ifndef __vtkCMBSubArcModifyClientOperator_h
#define __vtkCMBSubArcModifyClientOperator_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include "vtkCMBSubArcModifyOperator.h"
#include "cmbSystemConfig.h"

class vtkIdTypeArray;
class vtkCMBArcWidgetRepresentation;
class vtkSMNewWidgetRepresentationProxy;
class vtkPolyData;

class VTKCMBCLIENT_EXPORT vtkCMBSubArcModifyClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBSubArcModifyClientOperator* New();
  vtkTypeMacro(vtkCMBSubArcModifyClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //BTX
  enum EnumModificationType
    {
    OpUPDATE = 0, // Default to update
    OpSTRAIGHTEN = 1, // remove all internal points between two ends
    OpCOLLAPSE = 2, // collapse the endPoint into startPoint
    OpMAKEARC = 3 // Make a new arc from StartPoint to EndPoint
    };
  //ETX

  // Description:
  // Update
  virtual bool Operate( vtkIdType arcId,
    vtkSMNewWidgetRepresentationProxy *widgetProxy,
    vtkSMSourceProxy* arcSource,  int OperationType=OpUPDATE);

  vtkGetObjectMacro(CreatedArcs,vtkIdTypeArray);

  //Description:
  // Sets/Gets the range with Start/End point Ids that the input
  // polydata will update.
  // Default values are -1, and whole arc will be updated.
  vtkSetMacro(StartPointId, vtkIdType);
  vtkGetMacro(StartPointId,vtkIdType);
  vtkSetMacro(EndPointId, vtkIdType);
  vtkGetMacro(EndPointId,vtkIdType);

protected:
  vtkCMBSubArcModifyClientOperator();
  ~vtkCMBSubArcModifyClientOperator() override;

  virtual bool UpdateArc( vtkIdType arcId,
    vtkSMNewWidgetRepresentationProxy *widgetProxy,
    vtkSMSourceProxy* arcSource);
  bool FindArcsInWidgetOutput(
    vtkCMBArcWidgetRepresentation *widgetRep, vtkSMSourceProxy* arcSource);
  bool UpdateOperation(const vtkIdType& arcId,
                       vtkSMNewWidgetRepresentationProxy *widgetProxy,
                       vtkCMBArcWidgetRepresentation *widgetRep);
  bool SplitOperation(const vtkIdType& arcId);
  bool ModifyOperation(const vtkIdType& arcId, const int& opType);
  bool MakeArc(vtkIdType arcId, vtkSMSourceProxy* arcSource);

  vtkIdTypeArray* CreatedArcs;
  class InternalInfo;
  InternalInfo *Info;
  vtkIdType StartPointId;
  vtkIdType EndPointId;
  vtkIdType NumOfArcPoints;

private:
  vtkCMBSubArcModifyClientOperator(const vtkCMBSubArcModifyClientOperator&); // Not implemented
  void operator=(const vtkCMBSubArcModifyClientOperator&); // Not implemented
};

#endif
