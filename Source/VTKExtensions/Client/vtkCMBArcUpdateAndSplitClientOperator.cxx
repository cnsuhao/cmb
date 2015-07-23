//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcUpdateAndSplitClientOperator.h"

#include "vtkClientServerStream.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkSMProxy.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMPropertyHelper.h"

#include "vtkContourWidget.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkCMBArcWidgetRepresentation.h"
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
class vtkCMBArcUpdateAndSplitClientOperator::InternalInfo
  {
public:
  InternalInfo():
    WidgetSize(0),
    SplitIndices()
    {
    }

  ~InternalInfo(){}

  void Reset()
    {
    this->WidgetSize = 0;
    this->SplitIndices.clear();
    }
  int WidgetSize;
  std::vector<int> SplitIndices;
  };

vtkStandardNewMacro(vtkCMBArcUpdateAndSplitClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcUpdateAndSplitClientOperator::vtkCMBArcUpdateAndSplitClientOperator()
{
  this->CreatedArcs = NULL;
  this->Info = new vtkCMBArcUpdateAndSplitClientOperator::InternalInfo();
}

//---------------------------------------------------------------------------
vtkCMBArcUpdateAndSplitClientOperator::~vtkCMBArcUpdateAndSplitClientOperator()
{
  if (this->CreatedArcs)
    {
    this->CreatedArcs->Delete();
    this->CreatedArcs=NULL;
    }
  if(this->Info)
    {
    delete this->Info;
    }
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateAndSplitClientOperator::Operate(vtkIdType arcId,
                      vtkSMNewWidgetRepresentationProxy *widgetProxy)
{
  if (!widgetProxy || arcId == -1)
    {
    return false;
    }

  this->Info->Reset();
  //reset the createdArcs array
  if(this->CreatedArcs)
    {
    this->CreatedArcs->Delete();
    this->CreatedArcs=NULL;
    }
  this->CreatedArcs = vtkIdTypeArray::New();

  //we need to determine where the inputed end nodes now
  //are in the arc. We than need to split on all selected
  //end nodes between the original end nodes.

  //than we need to create new arcs on any end node that exists after
  //the last end node we have


  vtkContourWidget *widget = vtkContourWidget::SafeDownCast(
      widgetProxy->GetWidget());
  vtkCMBArcWidgetRepresentation *widgetRep =
      vtkCMBArcWidgetRepresentation::SafeDownCast(widget->GetRepresentation());

  bool valid = this->FindArcsInWidgetOutput(widgetRep);
  if ( valid )
    {
    //update the original arc with its new shape
    this->UpdateOperation(arcId,widgetProxy,widgetRep);

    //now that the original arcs shape has been updated split it if
    //it nows has internal points
    this->SplitOperation(arcId,widgetRep);
    }

  return valid;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateAndSplitClientOperator::FindArcsInWidgetOutput(
  vtkCMBArcWidgetRepresentation *widgetRep)
{
  //go through the widget output looking for the two end nodes that came
  //from the input.
  int size = widgetRep->GetNumberOfNodes();
  if (size < 2)
    {
    // this is invalid we have to stop
    return false;
    }
  this->Info->WidgetSize = size;
  for (int i=1; i < size-1; ++i)
    {
    //find the number of end nodes in the widget output
    int selected = widgetRep->GetNthNodeSelected(i);

    //get the flags for this selected node. If the flag
    //is inserted
    if(selected)
      {
      this->Info->SplitIndices.push_back(i);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateAndSplitClientOperator::UpdateOperation(
  const vtkIdType& arcId, vtkSMNewWidgetRepresentationProxy *widgetProxy,
  vtkCMBArcWidgetRepresentation *widgetRep)
{
  //Send the info from the widget down to the update operator
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
        manager->NewProxy("CmbArcGroup","UpdateOperator"));

  //set the arc that this is updating
  vtkSMPropertyHelper arcIdHelper(proxy,"ArcId");
  arcIdHelper.Set(arcId);

  //set that the default if nothing was set is to create
  vtkSMPropertyHelper behHelper(proxy,"RecreateArcBehavior");
  behHelper.Set(1); //1 == create, 0 == move

  //determine the status of the first end node
  int flags = widgetRep->GetNodeModifiedFlags(0);
  if (flags & Point_Inserted  || flags & Point_Deleted)
    {
    vtkSMPropertyHelper endNodeToMoveHelper(proxy,"EndNodeToRecreate");
    endNodeToMoveHelper.Set(0);
    }
  else if (flags & Point_Moved && flags & Point_Original)
    {
    vtkSMPropertyHelper endNodeToCreateHelper(proxy,"EndNodeToMove");
    endNodeToCreateHelper.Set(0);
    }
  //push this down to the server
  proxy->UpdateVTKObjects();

  //determine the status of the second end node
  flags = widgetRep->GetNodeModifiedFlags(this->Info->WidgetSize-1);
  if (flags & Point_Inserted  || flags & Point_Deleted)
    {
    vtkSMPropertyHelper endNodeToMoveHelper(proxy,"EndNodeToRecreate");
    endNodeToMoveHelper.Set(1);
    }
  else if (flags & Point_Moved && flags & Point_Original)
    {
    vtkSMPropertyHelper endNodeToCreateHelper(proxy,"EndNodeToMove");
    endNodeToCreateHelper.Set(1);
    }
  //push this down to the server
  proxy->UpdateVTKObjects();

  bool valid = proxy->Operate(widgetProxy);
  proxy->Delete();
  return valid;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateAndSplitClientOperator::SplitOperation(
  const vtkIdType& arcId,vtkCMBArcWidgetRepresentation * /*widgetRep*/)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

  //we need an offset as each time we split we need
  //to give the position based on the new arc indexing
  int offset = 0;
  vtkIdType id = arcId;
  bool splitValid = true;
  std::vector<int>::iterator it;
  for (it=this->Info->SplitIndices.begin();
       it!=this->Info->SplitIndices.end() && splitValid;
       ++it)
    {
    vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
          manager->NewProxy("CmbArcGroup","SplitOnIndexOperator"));

    //zero is equal to the first non end node point (See vtkCMBArcSplitOnIndex.h)
    //so it is *it-offset-1 for the correct index
    vtkSMPropertyHelper splitIndexHelper(proxy,"SplitIndex");
    splitIndexHelper.Set(*it-offset-1);

    splitValid = proxy->Operate(id);

    if (splitValid)
      {
      offset = *it;
      //get back the new id of the arc it created
      vtkSMPropertyHelper splitIndexHelper1(proxy,"CreatedArcId");
      splitIndexHelper1.UpdateValueFromServer();
      id = splitIndexHelper1.GetAsIdType();

      //push back each arc that we create
      this->CreatedArcs->InsertNextValue(id);
      }
    proxy->Delete();
    }

  return splitValid;
}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateAndSplitClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
