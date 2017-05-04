//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBSubArcModifyClientOperator.h"

#include "vtkCellArray.h"
#include "vtkClientServerStream.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"

#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "vtkCMBArc.h"
#include "vtkContourWidget.h"
#include "vtkNew.h"
#include "vtkPVArcInfo.h"
#include "vtkSMArcOperatorProxy.h"
#include <vector>

namespace
{
enum ModifiedPointFlags
{
  Point_Moved = 1 << 1,
  Point_Deleted = 1 << 2,
  Point_Inserted = 1 << 4,
  Point_Original = 1 << 8
};
}
class vtkCMBSubArcModifyClientOperator::InternalInfo
{
public:
  InternalInfo()
    : WidgetSize(0)
    , SplitIndices()
  {
  }

  ~InternalInfo() {}

  void Reset()
  {
    this->WidgetSize = 0;
    this->SplitIndices.clear();
  }
  int WidgetSize;
  std::vector<int> SplitIndices;
};

vtkStandardNewMacro(vtkCMBSubArcModifyClientOperator);

vtkCMBSubArcModifyClientOperator::vtkCMBSubArcModifyClientOperator()
{
  this->CreatedArcs = NULL;
  this->Info = new vtkCMBSubArcModifyClientOperator::InternalInfo();
  this->StartPointId = -1;
  this->EndPointId = -1;
}

vtkCMBSubArcModifyClientOperator::~vtkCMBSubArcModifyClientOperator()
{
  if (this->CreatedArcs)
  {
    this->CreatedArcs->Delete();
    this->CreatedArcs = NULL;
  }
  if (this->Info)
  {
    delete this->Info;
  }
}

bool vtkCMBSubArcModifyClientOperator::Operate(vtkIdType arcId,
  vtkSMNewWidgetRepresentationProxy* widgetProxy, vtkSMSourceProxy* arcSource, int OperationType)
{
  bool result = false;
  if (OperationType == OpUPDATE)
  {
    result = this->UpdateArc(arcId, widgetProxy, arcSource);
  }
  else if (OperationType == OpSTRAIGHTEN || OperationType == OpCOLLAPSE)
  {
    // Note, OpSTRAIGHTEN and OpCOLLAPSE, should match the ones defined
    // in vtkCMBSubArcModifyOperator.h
    int opType = OperationType == OpSTRAIGHTEN ? vtkCMBSubArcModifyOperator::OpSTRAIGHTEN
                                               : vtkCMBSubArcModifyOperator::OpCOLLAPSE;
    result = this->ModifyOperation(arcId, opType);
  }
  else if (OperationType == OpMAKEARC)
  {
    result = this->MakeArc(arcId, arcSource);
  }
  return result;
}

bool vtkCMBSubArcModifyClientOperator::UpdateArc(
  vtkIdType arcId, vtkSMNewWidgetRepresentationProxy* widgetProxy, vtkSMSourceProxy* arcSource)
{
  if (!arcSource || arcId == -1)
  {
    return false;
  }

  this->Info->Reset();
  //reset the createdArcs array
  if (this->CreatedArcs)
  {
    this->CreatedArcs->Delete();
    this->CreatedArcs = NULL;
  }
  this->CreatedArcs = vtkIdTypeArray::New();

  //we need to determine where the inputed end nodes now
  //are in the arc. We than need to split on all selected
  //end nodes between the original end nodes.

  //than we need to create new arcs on any end node that exists after
  //the last end node we have

  vtkContourWidget* widget = vtkContourWidget::SafeDownCast(widgetProxy->GetWidget());
  vtkSMTKArcRepresentation* widgetRep =
    vtkSMTKArcRepresentation::SafeDownCast(widget->GetRepresentation());

  bool valid = this->FindArcsInWidgetOutput(widgetRep, arcSource);
  if (valid)
  {
    //update the original arc with its new shape
    this->UpdateOperation(arcId, widgetProxy, widgetRep);

    //now that the original arcs shape has been updated split it if
    //it nows has internal points
    this->SplitOperation(arcId);
  }

  return valid;
}

bool vtkCMBSubArcModifyClientOperator::FindArcsInWidgetOutput(
  vtkSMTKArcRepresentation* widgetRep, vtkSMSourceProxy* arcSource)
{
  //go through the widget output looking for the two end nodes that came
  //from the input.
  int size = widgetRep->GetNumberOfNodes();
  if (size < 2)
  {
    // this is invalid we have to stop
    return false;
  }
  vtkNew<vtkPVArcInfo> arcInfo;
  arcInfo->SetGatherAllInfo();

  //collect the information from the server poly source.
  arcSource->GatherInformation(arcInfo.GetPointer());
  bool isWholeArc = vtkCMBArc::IsWholeArcRange(
    this->StartPointId, this->EndPointId, arcInfo->GetNumberOfPoints(), arcInfo->IsClosedLoop());

  this->Info->WidgetSize = size;
  for (int i = 1; i < size - 1; ++i)
  {
    //find the number of end nodes in the widget output
    int selected = widgetRep->GetNthNodeSelected(i);

    //get the flags for this selected node. If the flag
    //is inserted
    if (selected)
    {
      if (isWholeArc)
      {
        this->Info->SplitIndices.push_back(i);
      }
      else if (this->StartPointId >= 0 && this->EndPointId >= 0)
      {
        if (this->StartPointId < this->EndPointId ||
          (this->StartPointId > this->EndPointId && arcInfo->IsClosedLoop()))
        {
          this->Info->SplitIndices.push_back(i + this->StartPointId);
        }
        else
        {
          this->Info->SplitIndices.insert(
            this->Info->SplitIndices.begin(), this->EndPointId + (size - 1 - i));
        }
      }
    }
  }
  return true;
}

bool vtkCMBSubArcModifyClientOperator::UpdateOperation(const vtkIdType& arcId,
  vtkSMNewWidgetRepresentationProxy* widgetProxy, vtkSMTKArcRepresentation* widgetRep)
{
  //Send the info from the widget down to the update operator
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "UpdateOperator"));

  //set the arc that this is updating
  vtkSMPropertyHelper arcIdHelper(proxy, "ArcId");
  arcIdHelper.Set(arcId);
  vtkSMPropertyHelper(proxy, "StartPointId").Set(this->StartPointId);
  vtkSMPropertyHelper(proxy, "EndPointId").Set(this->EndPointId);

  //set that the default if nothing was set is to create
  vtkSMPropertyHelper behHelper(proxy, "RecreateArcBehavior");
  behHelper.Set(1); //1 == create, 0 == move

  //determine the status of the first end node
  int flags = widgetRep->GetNodeModifiedFlags(0);
  if (flags & Point_Inserted || flags & Point_Deleted)
  {
    vtkSMPropertyHelper endNodeToMoveHelper(proxy, "EndNodeToRecreate");
    endNodeToMoveHelper.Set(0);
  }
  else if (flags & Point_Moved && flags & Point_Original)
  {
    vtkSMPropertyHelper endNodeToCreateHelper(proxy, "EndNodeToMove");
    endNodeToCreateHelper.Set(0);
  }
  //push this down to the server
  proxy->UpdateVTKObjects();

  //determine the status of the second end node
  flags = widgetRep->GetNodeModifiedFlags(this->Info->WidgetSize - 1);
  if (flags & Point_Inserted || flags & Point_Deleted)
  {
    vtkSMPropertyHelper endNodeToMoveHelper(proxy, "EndNodeToRecreate");
    endNodeToMoveHelper.Set(1);
  }
  else if (flags & Point_Moved && flags & Point_Original)
  {
    vtkSMPropertyHelper endNodeToCreateHelper(proxy, "EndNodeToMove");
    endNodeToCreateHelper.Set(1);
  }
  //push this down to the server
  proxy->UpdateVTKObjects();

  bool valid = proxy->Operate(widgetProxy);
  proxy->Delete();
  return valid;
}

bool vtkCMBSubArcModifyClientOperator::SplitOperation(const vtkIdType& arcId)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

  //we need an offset as each time we split we need
  //to give the position based on the new arc indexing
  int offset = 0;
  vtkIdType id = arcId;
  bool splitValid = true;
  std::vector<int>::iterator it;
  for (it = this->Info->SplitIndices.begin(); it != this->Info->SplitIndices.end() && splitValid;
       ++it)
  {
    vtkSMArcOperatorProxy* proxy =
      vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "SplitOnIndexOperator"));

    //zero is equal to the first non end node point (See vtkCMBArcSplitOnIndex.h)
    //so it is *it-offset-1 for the correct index
    vtkSMPropertyHelper splitIndexHelper(proxy, "SplitIndex");
    splitIndexHelper.Set(*it - offset - 1);

    splitValid = proxy->Operate(id);

    if (splitValid)
    {
      offset = *it;
      //get back the new id of the arc it created
      vtkSMPropertyHelper splitIndexHelper1(proxy, "CreatedArcId");
      splitIndexHelper1.UpdateValueFromServer();
      id = splitIndexHelper1.GetAsIdType();

      //push back each arc that we create
      this->CreatedArcs->InsertNextValue(id);
    }
    proxy->Delete();
  }

  return splitValid;
}

bool vtkCMBSubArcModifyClientOperator::ModifyOperation(const vtkIdType& arcId, const int& opType)
{
  //Send the info from the widget down to the update operator
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "ArcModifyOperator"));

  //set the arc that this is updating
  vtkSMPropertyHelper(proxy, "ArcId").Set(arcId);
  vtkSMPropertyHelper(proxy, "OperationType").Set(opType);

  //push this down to the server
  proxy->UpdateVTKObjects();

  bool valid = proxy->Operate(this->StartPointId, this->EndPointId);
  proxy->Delete();
  return valid;
}

bool vtkCMBSubArcModifyClientOperator::MakeArc(vtkIdType arcId, vtkSMSourceProxy* arcSource)
{
  vtkNew<vtkPVArcInfo> arcInfo;
  arcInfo->SetGatherAllInfo();
  //collect the information from the server poly source.
  arcSource->GatherInformation(arcInfo.GetPointer());
  vtkIdType numArcPoints = arcInfo->GetNumberOfPoints();
  bool isWholeArc = vtkCMBArc::IsWholeArcRange(
    this->StartPointId, this->EndPointId, numArcPoints, arcInfo->IsClosedLoop());
  if (isWholeArc)
  {
    // nothing to do here
    return true;
  }

  this->Info->Reset();
  //reset the createdArcs array
  if (this->CreatedArcs)
  {
    this->CreatedArcs->Delete();
    this->CreatedArcs = NULL;
  }
  this->CreatedArcs = vtkIdTypeArray::New();

  // The indices should set from small to large.
  if (this->StartPointId < this->EndPointId)
  {
    // add first node if needed
    if (this->StartPointId > 0)
    {
      this->Info->SplitIndices.push_back(this->StartPointId);
    }
    // add second node if needed
    if (this->EndPointId < numArcPoints - 1 ||
      (this->EndPointId == numArcPoints - 1 && arcInfo->IsClosedLoop()))
    {
      this->Info->SplitIndices.push_back(this->EndPointId);
    }
  }
  else if (this->StartPointId > this->EndPointId)
  {
    // add first node if needed
    if (this->EndPointId > 0)
    {
      this->Info->SplitIndices.push_back(this->EndPointId);
    }
    // add second node if needed
    if (this->StartPointId < numArcPoints - 1 ||
      (this->StartPointId == numArcPoints - 1 && arcInfo->IsClosedLoop()))
    {
      this->Info->SplitIndices.push_back(this->StartPointId);
    }
  }
  return this->SplitOperation(arcId);
}

void vtkCMBSubArcModifyClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
