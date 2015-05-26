//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBSubArcModifyOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkVector.h"

vtkStandardNewMacro(vtkCMBSubArcModifyOperator);

//----------------------------------------------------------------------------
vtkCMBSubArcModifyOperator::vtkCMBSubArcModifyOperator()
{
  this->Reset();
}

//----------------------------------------------------------------------------
vtkCMBSubArcModifyOperator::~vtkCMBSubArcModifyOperator()
{
}

//----------------------------------------------------------------------------
void vtkCMBSubArcModifyOperator::Reset()
{
  this->ArcId = -1;
  this->OperationType = OpNONE;
}

//----------------------------------------------------------------------------
bool vtkCMBSubArcModifyOperator::Operate(
  vtkIdType startPointId, vtkIdType endPointId)
{
  if (this->OperationType == OpNONE || this->ArcId < 0)
    {
    this->Reset();
    return false;
    }

  vtkCMBArc* updatedArc = vtkCMBArcManager::GetInstance()->GetArc(this->ArcId);
  if(!updatedArc)
    {
    this->Reset();
    return false;
    }
  bool opWholeArc = vtkCMBArc::IsWholeArcRange(startPointId, endPointId,
    updatedArc->GetNumberOfArcPoints(), updatedArc->IsClosedArc());
  // we can not collapse a whole arc
  if(opWholeArc && this->OperationType == OpCOLLAPSE)
    {
    this->Reset();
    return false;
    }
  bool result = false;
  if(this->OperationType == OpSTRAIGHTEN)
    {
    if(opWholeArc)
      {
      double newEndNode[3];
      if(updatedArc->IsClosedArc())
        {
        if(updatedArc->GetArcInternalPoint(
          updatedArc->GetNumberOfInternalPoints() - 1, newEndNode))
          {
          updatedArc->ClearPoints();
          updatedArc->SetEndNode(1, newEndNode);
          result = true;
          }
        }
      else
        {
         // if it is not a sub-arc, go on with whole arc
        updatedArc->ClearPoints();
        result = true;
        }
      }
    else
      {
      result = this->StraightenSubArc(startPointId, endPointId, updatedArc);
      }
    }
  else // collapse, collapse the endPoint to startPoint
    {
    result = this->CollapseSubArc(startPointId, endPointId, updatedArc);
    }

  this->Reset();
  return result;
}

//----------------------------------------------------------------------------
bool vtkCMBSubArcModifyOperator::StraightenSubArc(
  vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc)
{
  if(startPointId < 0 || endPointId < 0 ||
    startPointId == endPointId)
    {
    return false;
    }

  std::list<vtkVector3d> newPoints;
  bool result = updatedArc->ReplacePoints(
    startPointId, endPointId, newPoints, false);

  return result;
}
//----------------------------------------------------------------------------
bool vtkCMBSubArcModifyOperator::CollapseSubArc(
  vtkIdType startPointId, vtkIdType endPointId, vtkCMBArc* updatedArc)
{
  if(startPointId < 0 || endPointId < 0 ||
    startPointId == endPointId)
    {
    return false;
    }

  std::list<vtkVector3d> newPoints;
  // based on where startPoint and EndPoint is, we may need to update
  // the end node. We always merge endPoint to startPoint
  double pos[3];
  int nodeIndex = -1;
  vtkCMBArcEndNode *endNode = NULL;
  if(endPointId == 0)
    {
    endNode = updatedArc->GetEndNode(0);
    nodeIndex = 0;
    // cache the startPoint position, this is where the endNode will be.
    updatedArc->GetArcInternalPoint(startPointId-1, pos);
    }
  else if(endPointId == updatedArc->GetNumberOfArcPoints() - 1)
    {
    if(updatedArc->IsClosedArc())
      {
      // we need to create a new end node
      updatedArc->GetArcInternalPoint(endPointId-1, pos);
      newPoints.push_back(vtkVector3d(pos));
     }
    else
      {
      endNode = updatedArc->GetEndNode(1);
      nodeIndex = 1;
      // cache the startPoint position, this is where the endNode will be.
      updatedArc->GetArcInternalPoint(startPointId-1, pos);
      }
    }
  else
    {
    updatedArc->GetArcInternalPoint(startPointId-1, pos);
    newPoints.push_back(vtkVector3d(pos));
    }
  bool result = updatedArc->ReplacePoints(
    startPointId, endPointId, newPoints, true);
  if(result && endNode && nodeIndex >=0)
    {
    updatedArc->MoveEndNode(nodeIndex, pos);
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkCMBSubArcModifyOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
