//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcEndNode
// .SECTION Description
// Light weight class that holds all the info
// needed for an arc end node
#include "vtkCMBArcEndNode.h"

//----------------------------------------------------------------------------
vtkIdType vtkCMBArcEndNode::NextId = 0;

//----------------------------------------------------------------------------
vtkCMBArcEndNode::vtkCMBArcEndNode(double position[3], unsigned int pid)
: PointId(pid), Id(NextId++)
{
  this->Position[0] = position[0];
  this->Position[1] = position[1];
  this->Position[2] = position[2];
}
//----------------------------------------------------------------------------
vtkCMBArcEndNode::~vtkCMBArcEndNode()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcEndNode::operator<(const vtkCMBArcEndNode &p) const
{
   return (this->Id < p.Id);
}

//----------------------------------------------------------------------------
vtkIdType vtkCMBArcEndNode::GetId() const
{
  return this->Id;
}

//----------------------------------------------------------------------------
void vtkCMBArcEndNode::GetPosition(double pos[3]) const
{
  pos[0] = this->Position[0];
  pos[1] = this->Position[1];
  pos[2] = this->Position[2];
}

void vtkCMBArcEndNode::GetPosition(vtkCMBArc::Point& pt) const
{
  pt = vtkCMBArc::Point(this->Position, this->PointId);
}

//----------------------------------------------------------------------------
const double* vtkCMBArcEndNode::GetPosition( ) const
{
  return this->Position;
}

//----------------------------------------------------------------------------
void vtkCMBArcEndNode::SetPosition(double* position)
{
  this->Position[0] = position[0];
  this->Position[1] = position[1];
  this->Position[2] = position[2];
}

void vtkCMBArcEndNode::SetPosition(vtkCMBArc::Point& pt)
{
  this->Position[0] = pt[0];
  this->Position[1] = pt[1];
  this->Position[2] = pt[2];
}

//----------------------------------------------------------------------------
unsigned int vtkCMBArcEndNode::GetPointId() const
{
  return this->PointId;
}
