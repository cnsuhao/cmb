//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcSplitOnIndexOperator.h"

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkObjectFactory.h"

#include <list>

vtkStandardNewMacro(vtkCMBArcSplitOnIndexOperator);

namespace
{
struct iPoint
{
public:
  double x, y, z;
  vtkIdType ptId;
  iPoint(double xin, double yin, double zin, vtkIdType id)
    : x(xin)
    , y(yin)
    , z(zin)
    , ptId(id)
  {
  }
};
}

//----------------------------------------------------------------------------
vtkCMBArcSplitOnIndexOperator::vtkCMBArcSplitOnIndexOperator()
{
  this->Index = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcSplitOnIndexOperator::~vtkCMBArcSplitOnIndexOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcSplitOnIndexOperator::Operate(vtkIdType arcId)
{
  //we have to reset the CreatedArcId everytime we split
  //so that we can use the same operator for multiple split operations
  this->CreatedArcId = -1;
  vtkCMBArc* arc = vtkCMBArcManager::GetInstance()->GetArc(arcId);
  if (!arc)
  {
    return false;
  }

  if (this->Index < 0)
  {
    return false;
  }

  //copy of all points we iterate before the split position
  std::list<vtkCMBArc::Point> originalPoints;

  //go through the points of the arc and split it on the index
  int i = 0;
  vtkCMBArc::Point point;
  bool foundSplitPoint = false;
  arc->InitTraversal();
  while (arc->GetNextPoint(point))
  {
    if (i == this->Index)
    {
      foundSplitPoint = true;
      break;
    }
    else
    {
      originalPoints.push_back(point);
    }
    ++i;
  }

  if (!foundSplitPoint)
  {
    //we can't split this arc at all
    return false;
  }

  //we found the valid index to split on.
  vtkCMBArc* createdArc = vtkCMBArc::New();

  //record the id of the arc we created with the split
  this->CreatedArcId = createdArc->GetId();

  //setup the new end nodes for both arcs
  createdArc->SetEndNode(0, point);
  vtkCMBArc::Point point2;
  arc->GetEndNode(1)->GetPosition(point2);
  createdArc->SetEndNode(1, point2);
  arc->SetEndNode(1, point);

  //now move all the interal end points after the split
  //point to the createdArc
  while (arc->GetNextPoint(point))
  {
    createdArc->InsertNextPoint(point);
  }

  //now clear the original arc internal points
  //and reset its internal points to all the points before the split
  arc->ClearPoints();

  std::list<vtkCMBArc::Point>::iterator it;
  for (it = originalPoints.begin(); it != originalPoints.end(); it++)
  {
    arc->InsertNextPoint(*it);
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcSplitOnIndexOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
