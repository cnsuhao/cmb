//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcSplitOnPositionOperator.h"

#include "vtkCMBArc.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcSplitOnIndexOperator.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBArcSplitOnPositionOperator);

namespace
{

bool pointsEqual(double pos1[3], vtkCMBArc::Point pos2, double tol)
{
  return ((pos1[0] <= pos2[0] + tol) && (pos1[0] >= pos2[0] - tol)) &&
    ((pos1[1] <= pos2[1] + tol) && (pos1[1] >= pos2[1] - tol)) &&
    ((pos1[2] <= pos2[2] + tol) && (pos1[2] >= pos2[2] - tol));
}
}

vtkCMBArcSplitOnPositionOperator::vtkCMBArcSplitOnPositionOperator()
{
  this->PositionTolerance = 1e-05;
  this->SplitPosition[0] = -1;
  this->SplitPosition[1] = -1;
  this->SplitPosition[2] = -1;
  this->ValidPosition = false;
  this->CreatedArcId = -1;
}

vtkCMBArcSplitOnPositionOperator::~vtkCMBArcSplitOnPositionOperator()
{
}

void vtkCMBArcSplitOnPositionOperator::SetSplitPosition(double x, double y, double z)
{
  this->ValidPosition = true;
  this->SplitPosition[0] = x;
  this->SplitPosition[1] = y;
  this->SplitPosition[2] = z;
  this->Modified();
}

bool vtkCMBArcSplitOnPositionOperator::Operate(vtkIdType arcId)
{
  //we have to reset the CreatedArcId everytime we split
  //so that we can use the same operator for multiple split operations
  this->CreatedArcId = -1;
  vtkCMBArc* arc = vtkCMBArcManager::GetInstance()->GetArc(arcId);
  if (!arc)
  {
    return false;
  }

  if (!this->ValidPosition)
  {
    return false;
  }
  //go through the points of the arc and split it on the position
  bool foundSplitPoint = false;
  int index = 0;
  vtkCMBArc::Point point;
  arc->InitTraversal();
  while (arc->GetNextPoint(point))
  {
    if (pointsEqual(this->SplitPosition, point, this->PositionTolerance))
    {
      foundSplitPoint = true;
      break;
    }
    index++;
  }

  if (!foundSplitPoint)
  {
    return false;
  }

  //we found the valid point time to split on this point.
  //we defer the split operation to vtkCMBArcSplitOnIndexOperator
  vtkCMBArcSplitOnIndexOperator* split = vtkCMBArcSplitOnIndexOperator::New();
  split->SetIndex(index);
  bool retCode = split->Operate(arcId);
  this->CreatedArcId = split->GetCreatedArcId();
  split->Delete();
  return retCode;
}

void vtkCMBArcSplitOnPositionOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
