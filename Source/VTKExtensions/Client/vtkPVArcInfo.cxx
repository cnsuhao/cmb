//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVArcInfo.h"

#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcProvider.h"

vtkStandardNewMacro(vtkPVArcInfo);

vtkPVArcInfo::vtkPVArcInfo()
{
  this->ArcId = -1;
  this->GatherLoopInfoOnly = true;

  this->ClosedLoop = 0;
  this->NumberOfPoints = 0;
  this->PointLocations = NULL;
  this->PointIds = NULL;
  this->EndNodeIds = NULL;

  this->EndNodePos = new double[6];
  this->EndNodePos[0] = 0;
  this->EndNodePos[1] = 0;
  this->EndNodePos[2] = 0;
  this->EndNodePos[3] = 0;
  this->EndNodePos[4] = 0;
  this->EndNodePos[5] = 0;
}

vtkPVArcInfo::~vtkPVArcInfo()
{
  if (this->PointLocations)
  {
    this->PointLocations->Delete();
    this->PointIds->Delete();
  }
  if (this->EndNodeIds)
  {
    this->EndNodeIds->Delete();
  }
  delete[] this->EndNodePos;
}

void vtkPVArcInfo::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkPVArcInfo::CopyFromObject(vtkObject* obj)
{
  //reset member variables to defaults
  bool gatherLoopOnly = this->GatherLoopInfoOnly;
  this->GatherLoopInfoOnly = true;
  this->ClosedLoop = false;
  this->ArcId = -1;

  if (this->PointLocations)
  {
    this->PointLocations->Initialize();
    this->PointIds->Initialize();
  }
  if (this->EndNodeIds)
  {
    this->EndNodeIds->Initialize();
  }

  vtkCMBArcProvider* provider = vtkCMBArcProvider::SafeDownCast(obj);
  if (!provider)
  {
    return;
  }
  this->ArcId = provider->GetArcId();

  if (gatherLoopOnly)
  {
    //gather the loop info
    this->GatherLoopInfo();
  }
  else
  {
    //gather the rest of the info
    this->GatherDetailedInfo();
  }
}

void vtkPVArcInfo::GatherLoopInfo()
{
  vtkCMBArc* arc = vtkCMBArcManager::GetInstance()->GetArc(this->ArcId);
  if (arc)
  {
    this->ClosedLoop = arc->IsClosedArc();
  }
}

void vtkPVArcInfo::GatherDetailedInfo()
{
  this->GatherLoopInfo();

  vtkCMBArc* arc = vtkCMBArcManager::GetInstance()->GetArc(this->ArcId);

  //get the number of points on the arc
  this->NumberOfPoints = arc->GetNumberOfEndNodes() + arc->GetNumberOfInternalPoints();
  if (!this->EndNodeIds)
  {
    this->EndNodeIds = vtkIdTypeArray::New();
  }
  this->EndNodeIds->SetNumberOfValues(arc->GetNumberOfEndNodes());

  //get all the point locations and jam them into a double array
  if (!this->PointLocations)
  {
    this->PointLocations = vtkDoubleArray::New();
    this->PointIds = vtkIdTypeArray::New();
  }
  this->PointLocations->SetNumberOfComponents(3);
  this->PointLocations->SetNumberOfTuples(this->NumberOfPoints);
  this->PointIds->SetNumberOfComponents(1);
  this->PointIds->SetNumberOfTuples(this->NumberOfPoints);

  //add all the points
  vtkIdType index = 0;
  arc->GetEndNode(0)->GetPosition(this->EndNodePos);
  this->PointLocations->InsertTuple(index, this->EndNodePos);
  this->PointIds->InsertTuple1(index, arc->GetEndNode(0)->GetPointId());
  this->EndNodeIds->InsertValue(0, 0);
  index++;

  arc->InitTraversal();
  vtkCMBArc::Point point;
  //TODO Pass the id
  while (arc->GetNextPoint(point))
  {
    double pos[3] = { point[0], point[1], point[2] };
    this->PointLocations->InsertTuple(index, pos);
    this->PointIds->InsertTuple1(index, point.GetId());
    ++index;
  }

  //get the end node positions
  if (!this->ClosedLoop)
  {
    arc->GetEndNode(1)->GetPosition(&this->EndNodePos[3]);
    this->PointLocations->InsertTuple(index, &this->EndNodePos[3]);
    this->PointIds->InsertTuple1(index, arc->GetEndNode(1)->GetPointId());
    this->EndNodeIds->InsertValue(1, index);
  }
}

bool vtkPVArcInfo::GetEndNodePos(vtkIdType index, double pos[3])
{
  if (this->ArcId == -1)
  {
    return false;
  }

  vtkIdType max = this->ClosedLoop ? 0 : 1;
  if (index < 0 || index > max)
  {
    return false;
  }

  index *= 3; //get the proper offset
  pos[0] = this->EndNodePos[index + 0];
  pos[1] = this->EndNodePos[index + 1];
  pos[2] = this->EndNodePos[index + 2];

  return true;
}

bool vtkPVArcInfo::GetPointLocation(vtkIdType index, double pos[3])
{
  if (this->PointLocations && index >= 0 && index < this->PointLocations->GetNumberOfTuples())
  {
    double locations[3];
    this->PointLocations->GetTuple(index, locations);
    for (int i = 0; i < 3; i++)
    {
      pos[i] = locations[i];
    }
    return true;
  }
  return false;
}

bool vtkPVArcInfo::GetPointLocationById(vtkIdType ptID, double pos[3])
{
  //TODO make this faster as a look up table
  for (vtkIdType i = 0; i < this->PointLocations->GetNumberOfTuples(); ++i)
  {
    if (this->PointIds->GetTuple1(i) == ptID)
    {
      return this->GetPointLocation(i, pos);
    }
  }
  return false;
}

bool vtkPVArcInfo::GetPointID(vtkIdType index, vtkIdType& id)
{
  if (this->PointIds && index >= 0 && index < this->PointIds->GetNumberOfTuples())
  {
    id = this->PointIds->GetTuple1(index);
    return true;
  }
  return false;
}

void vtkPVArcInfo::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << vtkClientServerStream::End;
}

void vtkPVArcInfo::CopyFromStream(const vtkClientServerStream*)
{
}
