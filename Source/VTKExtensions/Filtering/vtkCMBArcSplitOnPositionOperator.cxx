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

#include "vtkCMBArcSplitOnPositionOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArc.h"
#include "vtkCMBArcSplitOnIndexOperator.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBArcSplitOnPositionOperator);

namespace
{

bool pointsEqual(double pos1[3], double pos2[3], double tol)
  {
  return  (( pos1[0] <= pos2[0] + tol) && ( pos1[0] >= pos2[0] - tol)) &&
          (( pos1[1] <= pos2[1] + tol) && ( pos1[1] >= pos2[1] - tol)) &&
          (( pos1[2] <= pos2[2] + tol) && ( pos1[2] >= pos2[2] - tol));
  }

}

//----------------------------------------------------------------------------
vtkCMBArcSplitOnPositionOperator::vtkCMBArcSplitOnPositionOperator()
{
  this->PositionTolerance = 1e-05;
  this->SplitPosition[0] = -1;
  this->SplitPosition[1] = -1;
  this->SplitPosition[2] = -1;
  this->ValidPosition = false;
  this->CreatedArcId = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcSplitOnPositionOperator::~vtkCMBArcSplitOnPositionOperator()
{

}

//----------------------------------------------------------------------------
void vtkCMBArcSplitOnPositionOperator::SetSplitPosition(double x, double y, double z)
{
  this->ValidPosition = true;
  this->SplitPosition[0] = x;
  this->SplitPosition[1] = y;
  this->SplitPosition[2] = z;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkCMBArcSplitOnPositionOperator::Operate(vtkIdType arcId)
{
  //we have to reset the CreatedArcId everytime we split
  //so that we can use the same operator for multiple split operations
  this->CreatedArcId = -1;
  vtkCMBArc *arc = vtkCMBArcManager::GetInstance()->GetArc(arcId);
  if (!arc)
    {
    return false;
    }

  if ( !this->ValidPosition )
    {
    return false;
    }
  //go through the points of the arc and split it on the position
  bool foundSplitPoint = false;
  int index = 0;
  double point[3];
  arc->InitTraversal();
  while (arc->GetNextPoint(point))
    {
    if (pointsEqual(this->SplitPosition, point, this->PositionTolerance) )
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
  vtkCMBArcSplitOnIndexOperator *split = vtkCMBArcSplitOnIndexOperator::New();
  split->SetIndex(index);
  bool retCode = split->Operate(arcId);
  this->CreatedArcId = split->GetCreatedArcId();
  split->Delete();
  return retCode;
}

//----------------------------------------------------------------------------
void vtkCMBArcSplitOnPositionOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
