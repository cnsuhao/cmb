//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
#include "vtkPointData.h"

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
  vtkDataArray * vda = source->GetPointData()->GetScalars();

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      unsigned int pointID = vda->GetTuple1(ids->GetId(i));
      if (currentIndex != 0 && currentIndex != numberOfInternalEndNodes-1)
        {
        updatedArc->InsertNextPoint(pointID, pos);
        }
      else if (currentIndex == 0)
        {
        this->UpdateEndNode(updatedArc,0, pointID, pos);
        }
      else
        {
        this->UpdateEndNode(updatedArc,1, pointID, pos);
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
  std::list<vtkCMBArc::Point> newPoints;
  vtkDataArray * vda = source->GetPointData()->GetScalars();

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      unsigned int pointID = vda->GetTuple1(ids->GetId(i));
      // if there is an end node involved
      if(currentIndex == 0 && (this->StartPointId==0 ||
        this->StartPointId == numArcPoints - 1))
        {
        this->UpdateEndNode(updatedArc, this->StartPointId==0 ? 0 : 1 ,pointID,pos);
        }
      else if(currentIndex == numberOfInternalEndNodes-1 &&
        (this->EndPointId==0 || this->EndPointId == numArcPoints - 1))
        {
        this->UpdateEndNode(updatedArc, this->EndPointId==0 ? 0 : 1 ,pointID,pos);
        }
      else
        {
        newPoints.push_back(vtkCMBArc::Point(pos, pointID));
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
void vtkCMBArcUpdateOperator::UpdateEndNode(vtkCMBArc *arc, const int &endNodePos, unsigned int ptId, double pos[3])
{
  vtkCMBArc::Point point(pos, ptId);
  if (this->EndNodeFlags[endNodePos] == this->EndNodeDefault)
    {
    this->EndNodeFlags[endNodePos] = (this->RecreateArcBehavior==1) ?
          this->EndNodeRecreated : this->EndNodeMoved;
    }

  //we don't have information so we are removing the old ed
  if (this->EndNodeFlags[endNodePos]==this->EndNodeMoved)
    {
    arc->MoveEndNode(endNodePos, point);
    }
  else if(this->EndNodeFlags[endNodePos]==this->EndNodeRecreated)
    {
    arc->SetEndNode(endNodePos, point);
    }
}

//----------------------------------------------------------------------------
void vtkCMBArcUpdateOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}
