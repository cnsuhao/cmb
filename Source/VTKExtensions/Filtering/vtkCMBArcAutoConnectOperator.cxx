//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcAutoConnectOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkIdTypeArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include <set>
#include <list>

vtkStandardNewMacro(vtkCMBArcAutoConnectOperator);

//----------------------------------------------------------------------------
vtkCMBArcAutoConnectOperator::vtkCMBArcAutoConnectOperator()
{
  this->CreatedArcId = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcAutoConnectOperator::~vtkCMBArcAutoConnectOperator()
{

}

//----------------------------------------------------------------------------
bool vtkCMBArcAutoConnectOperator::Operate(vtkIdType firstId,
                                           vtkIdType secondId)
{
  this->CreatedArcId = -1;

  if (secondId == firstId)
    {
    //can't auto connect to your self
    return false;
    }

  if (secondId < 0 || firstId < 0)
    {
    return false;
    }

  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  vtkCMBArc *first = manager->GetArc(firstId);
  vtkCMBArc *second = manager->GetArc(secondId);
  if ( second == NULL || first == NULL )
    {
    return false;
    }

  if (first->IsClosedArc() || second->IsClosedArc())
    {
    return false;
    }

  std::list<vtkCMBArcEndNode*> firstArcsEndNodes;
  std::list<vtkCMBArcEndNode*> secondArcsEndNodes;

  //check the first arc
  if (first->GetNumberOfConnectedArcs(0) == 0)
    {
    firstArcsEndNodes.push_back(first->GetEndNode(0));
    }
  if (first->GetNumberOfConnectedArcs(1) == 0)
    {
    firstArcsEndNodes.push_back(first->GetEndNode(1));
    }
  //check the second arc
  if (second->GetNumberOfConnectedArcs(0) == 0)
    {
    secondArcsEndNodes.push_back(second->GetEndNode(0));
    }
  if (second->GetNumberOfConnectedArcs(1) == 0)
    {
    secondArcsEndNodes.push_back(second->GetEndNode(1));
    }


  double startPos[3],endPos[3];
  vtkIdType startID, endID;
  if ( firstArcsEndNodes.size() == 0 || secondArcsEndNodes.size() == 0)
    {
    //unable to connect these arcs
    return false;
    }
  else if ( firstArcsEndNodes.size() == 1 && secondArcsEndNodes.size() == 1)
    {
    //simple case if both only have one free end node, than we don't
    //have to do anything
    (*firstArcsEndNodes.begin())->GetPosition(startPos);
    startID = (*firstArcsEndNodes.begin())->GetId();
    (*secondArcsEndNodes.begin())->GetPosition(endPos);
    endID = (*secondArcsEndNodes.begin())->GetId();
    }
  else
    {
    //first the two closest points by finding the distance between them all
    double tmp;
    double dist = VTK_DOUBLE_MAX;
    double point1[3], point2[3];
    vtkIdType point1ID, point2ID;
    std::list<vtkCMBArcEndNode*>::const_iterator it1,it2;
    for (it1 = firstArcsEndNodes.begin(); it1 != firstArcsEndNodes.end(); it1++)
      {
      for ( it2 = secondArcsEndNodes.begin(); it2 != secondArcsEndNodes.end(); it2++)
        {
        (*it1)->GetPosition(point1);
        (*it2)->GetPosition(point2);
        point1ID = (*it1)->GetId();
        point2ID = (*it2)->GetId();
        tmp = vtkMath::Distance2BetweenPoints(point1,point2);
        if ( tmp < dist )
          {
          dist = tmp;
          startPos[0]=point1[0];startPos[1]=point1[1];startPos[2]=point1[2];
          endPos[0]=point2[0];endPos[1]=point2[1];endPos[2]=point2[2];
          startID = point1ID;
          endID = point2ID;
          }
        }
      }
    }

  //create the auto connect arc
  vtkCMBArc *autoConnectedArc = vtkCMBArc::New();
  vtkCMBArc::Point startPoint(startPos, startID);
  autoConnectedArc->SetEndNode(0, startPoint);
  vtkCMBArc::Point endPoint(endPos, endID);
  autoConnectedArc->SetEndNode(1, endPoint);
  this->CreatedArcId = autoConnectedArc->GetId();
  return true;
  }

//----------------------------------------------------------------------------
void vtkCMBArcAutoConnectOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
