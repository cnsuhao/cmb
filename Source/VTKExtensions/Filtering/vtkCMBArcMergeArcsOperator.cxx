/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkCMBArcMergeArcsOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

#include <set>
#include <list>

vtkStandardNewMacro(vtkCMBArcMergeArcsOperator);

//----------------------------------------------------------------------------
vtkCMBArcMergeArcsOperator::vtkCMBArcMergeArcsOperator()
{
  this->CreatedArcId = -1;
  this->ArcIdToDelete = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcMergeArcsOperator::~vtkCMBArcMergeArcsOperator()
{

}

//----------------------------------------------------------------------------
bool vtkCMBArcMergeArcsOperator::Operate(vtkIdType firstId, vtkIdType secondId)
{
  this->CreatedArcId = -1;
  this->ArcIdToDelete = -1;

  if (secondId == firstId)
    {
    //can't merge into your self
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
    //can't merge if one or both the arcs are closed
    return false;
    }

  //now find the common end node(s) between the two arcs
  std::set<vtkCMBArc*> firstConnections = manager->GetConnectedArcs(first);
  if (firstConnections.find(second) == firstConnections.end())
    {
    //first isn't connected to secondination
    return false;
    }

  double point[3];
  //these two variable help determine the order the internal points
  //need to be merged in
  bool reversePoints = false;
  bool firstBeforeSecond = false;
  if(second->GetEndNode(0) == first->GetEndNode(1))
    {
    reversePoints = false;
    firstBeforeSecond = true;
    }
  else if(second->GetEndNode(1) == first->GetEndNode(1))
    {
    reversePoints = true;
    firstBeforeSecond = true;
    }
  else if(second->GetEndNode(1) == first->GetEndNode(0))
    {
    reversePoints = false;
    firstBeforeSecond = false;
    }
   else if(second->GetEndNode(0) == first->GetEndNode(0))
    {
    //append all second points to the start of first
    //than set first zero node to the end of second
    //this is the exception case

    //the end node now becomes an interior point
    first->GetEndNode(0)->GetPosition(point);
    first->InsertPointAtFront(point);

    second->InitTraversal(false);
    while(second->GetNextPoint(point))
      {
      first->InsertPointAtFront(point);
      }

    //move firsts first end node to the end of second.
    second->GetEndNode(1)->GetPosition(point);
    first->SetEndNode(0,point);


    //make sure we mark the correct arc to delete
    this->ArcIdToDelete = second->GetId();
    this->CreatedArcId = first->GetId();
    return true;
    }

  int firstEndNode = (reversePoints)? 1 : 0;
  int secondEndNode = (reversePoints)? 0 : 1;
  if (!firstBeforeSecond)
    {
    vtkCMBArc *tmp = first;
    first = second;
    second = tmp;
    }

  //add in the first end node of second as a internal point
  second->GetEndNode(firstEndNode)->GetPosition(point);
  first->InsertNextPoint(point);

  second->InitTraversal(!reversePoints);
  while(second->GetNextPoint(point))
    {
    first->InsertNextPoint(point);
    }

  //now remove the old end node from second and make it
  //point to the end of first
  second->GetEndNode(secondEndNode)->GetPosition(point);
  first->SetEndNode(1,point);

  this->ArcIdToDelete = second->GetId();
  this->CreatedArcId = first->GetId();
  return true;
  }

//----------------------------------------------------------------------------
void vtkCMBArcMergeArcsOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
