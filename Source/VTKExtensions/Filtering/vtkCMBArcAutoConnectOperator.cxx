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
    (*secondArcsEndNodes.begin())->GetPosition(endPos);
    }
  else
    {
    //first the two closest points by finding the distance between them all
    double tmp;
    double dist = VTK_DOUBLE_MAX;
    double point1[3], point2[3];
    std::list<vtkCMBArcEndNode*>::const_iterator it1,it2;
    for (it1 = firstArcsEndNodes.begin(); it1 != firstArcsEndNodes.end(); it1++)
      {
      for ( it2 = secondArcsEndNodes.begin(); it2 != secondArcsEndNodes.end(); it2++)
        {
        (*it1)->GetPosition(point1);
        (*it2)->GetPosition(point2);
        tmp = vtkMath::Distance2BetweenPoints(point1,point2);
        if ( tmp < dist )
          {
          dist = tmp;
          startPos[0]=point1[0];startPos[1]=point1[1];startPos[2]=point1[2];
          endPos[0]=point2[0];endPos[1]=point2[1];endPos[2]=point2[2];
          }
        }
      }
    }

  //create the auto connect arc
  vtkCMBArc *autoConnectedArc = vtkCMBArc::New();
  autoConnectedArc->SetEndNode(0,startPos);
  autoConnectedArc->SetEndNode(1,endPos);
  this->CreatedArcId = autoConnectedArc->GetId();
  return true;
  }

//----------------------------------------------------------------------------
void vtkCMBArcAutoConnectOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
