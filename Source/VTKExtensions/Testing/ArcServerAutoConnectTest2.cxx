//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArc.h"
#include "vtkCMBArcAutoConnectOperator.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerAutoConnectTest2(int /*argc*/, char* /*argv*/ [])
{
  //first two arcs are in a straight line
  //third arc closest point on arc 1 can't be used, so test that
  //fourth arc is a loop and can't connect to anything
  double en[8][3] = { { 0, 0, 0 }, { 5, 0, 0 }, { 5, 0, 0 }, { 6, 0, 0 }, { 0, 5, 0 }, { 5, 1, 0 },
    { 6, -1, 0 }, { 6, -1, 0 } };

  for (int i = 0; i < 8; i += 2)
  {
    vtkCMBArc* arc = vtkCMBArc::New();
    vtkCMBArc::Point pt1(en[i], i);
    vtkCMBArc::Point pt2(en[i + 1], i + 1);
    arc->SetEndNode(0, pt1);
    arc->SetEndNode(1, pt2);
  }

  vtkSmartPointer<vtkCMBArcAutoConnectOperator> connect =
    vtkSmartPointer<vtkCMBArcAutoConnectOperator>::New();

  bool valid = connect->Operate(0, 2);
  vtkIdType createdId = connect->GetCreatedArcId();
  if (!valid)
  {
    //this should pass
    return 1;
  }
  //confirm that the arc that is created connected between
  //0,0,0 and 0,5,0
  vtkCMBArc* createdArc = vtkCMBArcManager::GetInstance()->GetArc(createdId);
  double pos[3];
  createdArc->GetEndNode(0)->GetPosition(pos);
  if (pos[0] != 0 || pos[1] != 0 || pos[2] != 0)
  {
    return 1;
  }

  createdArc->GetEndNode(1)->GetPosition(pos);
  if (pos[0] != 0 || pos[1] != 5 || pos[2] != 0)
  {
    return 1;
  }

  valid = connect->Operate(1, 3);
  if (valid)
  {
    //you can't connect to a loop arc
    return 1;
  }

  return 0;
}
