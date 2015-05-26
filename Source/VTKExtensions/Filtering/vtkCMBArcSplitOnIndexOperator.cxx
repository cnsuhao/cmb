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

#include "vtkCMBArcManager.h"
#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"

#include "vtkObjectFactory.h"

#include <list>

vtkStandardNewMacro(vtkCMBArcSplitOnIndexOperator);

namespace
{
struct iPoint
  {
  public:
  double x,y,z;
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
  vtkCMBArc *arc = vtkCMBArcManager::GetInstance()->GetArc(arcId);
  if (!arc)
    {
    return false;
    }

  if ( this->Index < 0)
    {
    return false;
    }

  //copy of all points we iterate before the split position
  std::list<iPoint> originalPoints;

  //go through the points of the arc and split it on the index
  int i=0;
  double position[3];
  bool foundSplitPoint = false;
  arc->InitTraversal();
  while (arc->GetNextPoint(position))
    {
    if (i == this->Index)
      {
      foundSplitPoint = true;
      break;
      }
    else
      {
      iPoint p={position[0],position[1],position[2]};
      originalPoints.push_back(p);
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
  createdArc->SetEndNode(0,position);
  double position2[3];
  arc->GetEndNode(1)->GetPosition(position2);
  createdArc->SetEndNode(1,position2);
  arc->SetEndNode(1,position);

  //now move all the interal end points after the split
  //point to the createdArc
  while (arc->GetNextPoint(position))
    {
    createdArc->InsertNextPoint(position);
    }

  //now clear the original arc internal points
  //and reset its internal points to all the points before the split
  arc->ClearPoints();

  std::list<iPoint>::iterator it;
  for (it = originalPoints.begin(); it != originalPoints.end(); it++)
    {
    arc->InsertNextPoint(it->x,it->y,it->z);
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcSplitOnIndexOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
