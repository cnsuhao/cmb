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

#include "vtkCMBArcUpdateOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkVector.h"

vtkStandardNewMacro(vtkCMBArcUpdateOperator);

//----------------------------------------------------------------------------
vtkCMBArcUpdateOperator::vtkCMBArcUpdateOperator()
{
  this->Reset();
}

//----------------------------------------------------------------------------
vtkCMBArcUpdateOperator::~vtkCMBArcUpdateOperator()
{

}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateOperator::Reset()
{
  this->ArcId = -1;
  this->StartPointId = -1;
  this->EndPointId = -1;
  this->RecreateArcBehavior = 0;
  this->EndNodeFlags[0] = this->EndNodeDefault;
  this->EndNodeFlags[1] = this->EndNodeDefault;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateOperator::SetEndNodeToMove(int endNode)
{
  if ( endNode < 0 || endNode > 1)
    {
    return false;
    }
  this->EndNodeFlags[endNode] = this->EndNodeMoved;
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateOperator::SetEndNodeToRecreate(int endNode)
{
  if ( endNode < 0 || endNode > 1)
    {
    return false;
    }
  this->EndNodeFlags[endNode] = this->EndNodeRecreated;
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateOperator::SetArcId(vtkIdType arcId)
{
  if (this->ArcId != arcId)
    {
    //we need to keep a reference to the arc so that
    //we can properly repor the correct MTime if any of the end nodes change
    this->ArcId = arcId;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateOperator::Operate(vtkPolyData *source)
{
  //create the new arc we are going to add all the info too
  vtkCMBArc* updatedArc = vtkCMBArcManager::GetInstance()->GetArc(this->ArcId);
  if(!vtkCMBArc::IsWholeArcRange(this->StartPointId, this->EndPointId,
    updatedArc->GetNumberOfArcPoints(), updatedArc->IsClosedArc()))
    {
    return this->UpdateSubArc(source, updatedArc);
    }

  if (source == NULL)
    {
    this->Reset();
    return false;
    }

  //we need at least two point for this to be able to update the arc
  if(source->GetNumberOfPoints() < 2 ||
     source->GetNumberOfLines() == 0)
    {
    this->Reset();
    return false;
    }
  // if it is not a sub-arc, go on with whole arc
  updatedArc->ClearPoints();

  //variables for polydata to arc conversion
  double pos[3];
  vtkCellArray* lines = source->GetLines();
  vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();

  vtkIdType currentIndex = 0;
  vtkIdType numberOfCells = lines->GetNumberOfCells();
  vtkIdType numberOfInternalEndNodes = lines->GetNumberOfConnectivityEntries() - numberOfCells;

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      if (currentIndex != 0 && currentIndex != numberOfInternalEndNodes-1)
        {
        updatedArc->InsertNextPoint(pos);
        }
      else if (currentIndex == 0)
        {
        this->UpdateEndNode(updatedArc,0,pos);
        }
      else
        {
        this->UpdateEndNode(updatedArc,1,pos);
        }
      ++currentIndex;
      }
    //clear the id structures
    ids->Reset();
    }

  this->Reset();
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBArcUpdateOperator::UpdateSubArc(
  vtkPolyData *source, vtkCMBArc* updatedArc)
{
  if(this->StartPointId < 0 || this->EndPointId < 0 ||
    this->StartPointId == this->EndPointId)
    {
    return false;
    }

  vtkIdType numArcPoints = updatedArc->GetNumberOfArcPoints();
  //variables for polydata to arc conversion
  double pos[3];
  vtkCellArray* lines = source->GetLines();
  vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();

  vtkIdType currentIndex = 0;
  vtkIdType numberOfCells = lines->GetNumberOfCells();
  vtkIdType numberOfInternalEndNodes = lines->GetNumberOfConnectivityEntries() - numberOfCells;
  std::list<vtkVector3d> newPoints;

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      // if there is an end node involved
      if(currentIndex == 0 && (this->StartPointId==0 ||
        this->StartPointId == numArcPoints - 1))
        {
        this->UpdateEndNode(updatedArc, this->StartPointId==0 ? 0 : 1 ,pos);
        }
      else if(currentIndex == numberOfInternalEndNodes-1 &&
        (this->EndPointId==0 || this->EndPointId == numArcPoints - 1))
        {
        this->UpdateEndNode(updatedArc, this->EndPointId==0 ? 0 : 1 ,pos);
        }
      else
        {
        newPoints.push_back(vtkVector3d(pos));
        }
      ++currentIndex;
      }
    //clear the id structures
    ids->Reset();
    }
  bool result = updatedArc->ReplacePoints(
    this->StartPointId, this->EndPointId, newPoints, true);

  this->Reset();
  return result;
}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateOperator::UpdateEndNode(vtkCMBArc *arc, const int &endNodePos, double pos[3])
{
  if (this->EndNodeFlags[endNodePos] == this->EndNodeDefault)
    {
    this->EndNodeFlags[endNodePos] = (this->RecreateArcBehavior==1) ?
          this->EndNodeRecreated : this->EndNodeMoved;
    }

  //we don't have information so we are removing the old ed
  if (this->EndNodeFlags[endNodePos]==this->EndNodeMoved)
    {
    arc->MoveEndNode(endNodePos,pos);
    }
  else if(this->EndNodeFlags[endNodePos]==this->EndNodeRecreated)
    {
    arc->SetEndNode(endNodePos,pos);
    }
}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
