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
#include "vtkCMBArcManager.h"
#include "vtkCMBArcSplitOnIndexOperator.h"

#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerSplitTest2(int /*argc*/, char* /*argv*/ [])
{
  double en1[3] = { -50, 0, -20 };
  double en2[3] = { 10, 10, 10 };
  double midPos[3] = { -50.0, 10.0, -20.0 };

  vtkCMBArc* arc = vtkCMBArc::New();
  arc->SetEndNode(0, vtkCMBArc::Point(en1, 0));
  arc->SetEndNode(1, vtkCMBArc::Point(en2, 1));

  //attempt to split an arc with no internal points
  vtkSmartPointer<vtkCMBArcSplitOnIndexOperator> splitIndex =
    vtkSmartPointer<vtkCMBArcSplitOnIndexOperator>::New();

  //now add multiple internal point
  arc->InsertNextPoint(2, -50, 2.5, -20);
  arc->InsertNextPoint(3, -50, 5, -20);
  arc->InsertNextPoint(4, -50, 7.5, -20);
  arc->InsertNextPoint(5, midPos); //Split Here
  arc->InsertNextPoint(6, -40, 10, -20);
  arc->InsertNextPoint(7, -30, 10, -20);
  arc->InsertNextPoint(8, -20, 10, -20);
  arc->InsertNextPoint(9, -10, 10, -20);
  arc->InsertNextPoint(10, 0, 10, 0);
  arc->InsertNextPoint(11, 10, 10, 10);

  //now lets split!
  splitIndex->SetIndex(3);
  if (!splitIndex->Operate(arc->GetId()))
  {
    cerr << "failed to split on the fourth internal point" << endl;
    return 1;
  }
  vtkIdType newArcId = splitIndex->GetCreatedArcId();
  vtkCMBArc* newArc = vtkCMBArcManager::GetInstance()->GetArc(newArcId);
  if (!newArc)
  {
    cerr << "The arc Id returned by the split on index operation is an invalid id";
    return 1;
  }

  //validate that the new and old arc have no internal points
  if (arc->GetNumberOfInternalPoints() != 3)
  {
    cerr << "After split the original arc can't have internal points" << endl;
    return 1;
  }
  if (newArc->GetNumberOfInternalPoints() != 6)
  {
    cerr << "After split the new arc can't have internal points" << endl;
    return 1;
  }

  //lets make sure the arcs are properly connected
  if (arc->GetNumberOfConnectedArcs() != 1)
  {
    cerr << "Original arc should be connected only to the new arc" << endl;
    return 1;
  }
  if (newArc->GetNumberOfConnectedArcs() != 1)
  {
    cerr << "New Arc connected to" << newArc->GetNumberOfConnectedArcs() << endl;
    cerr << "New arc should be connected only to the original arc" << endl;
    return 1;
  }

  vtkCMBArcEndNode* en = vtkCMBArcManager::GetInstance()->GetEndNodeAt(midPos);
  if (arc->GetEndNode(1) != en)
  {
    cerr << "Original Arc end node 1 is at wrong position" << endl;
    return 1;
  }
  if (newArc->GetEndNode(0) != en)
  {
    cerr << "new Arc end node 0 is at wrong position" << endl;
    return 1;
  }

  en = vtkCMBArcManager::GetInstance()->GetEndNodeAt(en2);
  if (newArc->GetEndNode(1) != en)
  {
    cerr << "new Arc end node 1 is at wrong position" << endl;
    return 1;
  }

  return 0;
}
