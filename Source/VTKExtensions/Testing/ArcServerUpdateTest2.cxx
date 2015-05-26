//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Test functionality of the vtkCMBArc Update Operator
// Create a basic loop arc, than update it to be a straight arc
// this will test to make sure we can convert an arc from having shared end nodes
// to having different end nodes

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcCreateOperator.h"
#include "vtkCMBArcUpdateOperator.h"

#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

namespace
{
int create_default_arc()
{
  bool valid = false;
  double en1[3]={0,0,10};
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(4,0,10);
  points->InsertNextPoint( 4, 4, 10 );
  points->InsertNextPoint( 0, 4, 10 );
  points->InsertNextPoint( 0, 2, 10 );
  points->InsertNextPoint(en1);
  arcP->SetPoints(points);
  points->FastDelete();

  vtkCellArray *lines = vtkCellArray::New();
  for (vtkIdType i=0; i < 5; ++i)
    {
    lines->InsertNextCell(2);
    lines->InsertCellPoint(i);
    lines->InsertCellPoint(i+1);
    }
  arcP->SetLines(lines);
  lines->FastDelete();

  vtkSmartPointer<vtkCMBArcCreateOperator> createArc = vtkSmartPointer<vtkCMBArcCreateOperator>::New();
  valid = createArc->Operate(arcP);
  if (!valid)
    {
    cerr << "Unable to create the 1st arc" << endl;
    return 1;
    }
  return 0;
}

vtkPolyData* moved_arc()
{
  vtkPolyData* arcP = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(0,0,9);
  points->InsertNextPoint(4,0,10);
  points->InsertNextPoint(4,4,11);
  points->InsertNextPoint(0,4,12);
  points->InsertNextPoint(0,2,13);
  points->InsertNextPoint(0,0,14);
  arcP->SetPoints(points);
  points->FastDelete();

  vtkCellArray *lines = vtkCellArray::New();
  for (vtkIdType i=0; i < 5; ++i)
    {
    lines->InsertNextCell(2);
    lines->InsertCellPoint(i);
    lines->InsertCellPoint(i+1);
    }
  arcP->SetLines(lines);
  lines->FastDelete();

  return arcP;
}
}

int ArcServerUpdateTest2( int /*argc*/, char * /*argv*/[] )
{
  bool valid = false;
  int res = create_default_arc();
  if ( res == 1)
    {
    return 1;
    }

  //take a loop arc, and convert it to an arc
  vtkSmartPointer<vtkCMBArcUpdateOperator> updateArc = vtkSmartPointer<vtkCMBArcUpdateOperator>::New();
  updateArc->SetArcId(0);
  updateArc->SetRecreateArcBehavior(1); //set the default to be recreate
  updateArc->SetEndNodeToMove(1);  //override default on end node 1

  vtkPolyData* mArc = moved_arc();
  valid = updateArc->Operate(mArc);
  mArc->Delete();

  if(valid==false)
    {
    cerr << "No reason it can't update a loop" << endl;
    return 1;
    }

  vtkCMBArc *arc = vtkCMBArcManager::GetInstance()->GetArc(0);
  if (!arc)
    {
    cerr << "Unable to get arc 0" << endl;
    return 1;
    }

  if (arc->IsClosedArc())
    {
    cerr << "Arc is closed after the update operator" <<endl;
    return 1;
    }

  double pos[3];
  vtkCMBArcEndNode *en = arc->GetEndNode(0);
  en->GetPosition(pos);
  if ( pos[2] != 9)
    {
    cerr << "end node 1 failed to move properly" << endl;
    return 1;
    }

  en = arc->GetEndNode(1);
  en->GetPosition(pos);
  if ( pos[2] != 14)
    {
    cerr << "end node 2 failed to move properly" << endl;
    return 1;
    }


  return 0;
}
