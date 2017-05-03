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

int ArcServerSplitTest1(int /*argc*/, char* /*argv*/ [])
{
  double en1[3] = { 10, 100, 200 };
  double en2[3] = { 100, 0, -200 };
  double mpos[3] = { 50, 50, 100 };

  vtkCMBArc* arc = vtkCMBArc::New();
  arc->SetEndNode(0, vtkCMBArc::Point(en1, 0));
  arc->SetEndNode(1, vtkCMBArc::Point(en2, 1));

  //attempt to split an arc with no internal points
  vtkSmartPointer<vtkCMBArcSplitOnIndexOperator> splitIndex =
    vtkSmartPointer<vtkCMBArcSplitOnIndexOperator>::New();

  splitIndex->SetIndex(-1);
  if (splitIndex->Operate(arc->GetId()))
  {
    cerr << "Was able to split on index -1" << endl;
    return 1;
  }

  splitIndex->SetIndex(10);
  if (splitIndex->Operate(arc->GetId()))
  {
    cerr << "Was able to split on index 10" << endl;
    return 1;
  }

  //now add a single internal point
  arc->InsertNextPoint(2, mpos);
  splitIndex->SetIndex(-1);
  if (splitIndex->Operate(arc->GetId()))
  {
    cerr << "Was able to split on index -1" << endl;
    return 1;
  }

  splitIndex->SetIndex(10);
  if (splitIndex->Operate(arc->GetId()))
  {
    cerr << "Was able to split on index 10" << endl;
    return 1;
  }

  splitIndex->SetIndex(1);
  if (splitIndex->Operate(arc->GetId()))
  {
    cerr << "Was able to split on index 1" << endl;
    return 1;
  }

  //now lets actually split!
  //the result will be two arcs, each with no internal end nodes
  //with a shared end node
  splitIndex->SetIndex(0);
  if (!splitIndex->Operate(arc->GetId()))
  {
    cerr << "failed to split on the first internal point" << endl;
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
  if (arc->GetNumberOfInternalPoints() != 0)
  {
    cerr << "After split the original arc can't have internal points" << endl;
    return 1;
  }
  if (newArc->GetNumberOfInternalPoints() != 0)
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
    cerr << "New arc should be connected only to the original arc" << endl;
    return 1;
  }

  vtkCMBArcEndNode* en = vtkCMBArcManager::GetInstance()->GetEndNodeAt(mpos);
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
