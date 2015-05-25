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
//Plan is to create two arcs that are connect too each other on both ends
//we will move one end node and see both arcs move, while on the end node
//we will break the connectivity
//this test will also use decimal numbers for all positions

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcCreateOperator.h"
#include "vtkCMBArcUpdateOperator.h"
#include "vtkCMBArcDeleteOperator.h"

#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerUpdateTest1( int /*argc*/, char * /*argv*/[] )
{
  bool valid = false;
  double en1[3]={0.4,0.0,0.0};
  double en2[3]={0.5,1.0,0.0};
  double en1Moved[3]={0.5,0.0,0.0};
  double en2Created[3]={0.2,0.4,0.0};

  //create arc 1
  {
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(0.8,0.0,0.0);
  points->InsertNextPoint(0.8,1.0,0);
  points->InsertNextPoint(en2);
  arcP->SetPoints(points);
  points->FastDelete();

  vtkCellArray *lines = vtkCellArray::New();
  for (vtkIdType i=0; i < 3; ++i)
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
  }

  //create arc 2
  {
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(0.0,0.0,0.0);
  points->InsertNextPoint(0.0,1.0,0);
  points->InsertNextPoint(en2);
  arcP->SetPoints(points);
  points->FastDelete();

  vtkCellArray *lines = vtkCellArray::New();
  for (vtkIdType i=0; i < 3; ++i)
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
  }


  //now create the updated arc representation
  vtkPolyData* updatedArcP = vtkPolyData::New();
  {
    vtkPoints *points = vtkPoints::New();
    points->InsertNextPoint(en1Moved);
    points->InsertNextPoint(0.5,0.4,0.0);
    points->InsertNextPoint(0.2,0.4,0.0);
    points->InsertNextPoint(0.2,0.2,0.0);
    points->InsertNextPoint(en2Created);
    updatedArcP->SetPoints(points);
    points->FastDelete();

    vtkCellArray *lines = vtkCellArray::New();
    for (vtkIdType i=0; i < 4; ++i)
      {
      lines->InsertNextCell(2);
      lines->InsertCellPoint(i);
      lines->InsertCellPoint(i+1);
      }
    updatedArcP->SetLines(lines);
    lines->FastDelete();
  }

  vtkSmartPointer<vtkCMBArcUpdateOperator> updateArc = vtkSmartPointer<vtkCMBArcUpdateOperator>::New();
  updateArc->SetArcId(0);
  updateArc->SetEndNodeToMove(0);  //override default on end node 1
  updateArc->SetEndNodeToRecreate(1);  //override default on end node 1
  valid = updateArc->Operate(updatedArcP);
  if (!valid)
    {
    cerr << "Operatoer failed for no good reason!" << endl;
    return 1;
    }
  updatedArcP->Delete();

  vtkCMBArc *arc = vtkCMBArcManager::GetInstance()->GetArc(0);
  if (arc->GetNumberOfConnectedArcs(0) != 1)
    {
    cerr << "end node 1 of arc 0 isn't connected anymore" << endl;
    return 1;
    }

  if (arc->GetNumberOfConnectedArcs(1) != 0)
    {
    cerr << "end node 2 of arc 0 IS connected!" << endl;
    return 1;
    }

  arc = vtkCMBArcManager::GetInstance()->GetArc(1);
  if (arc->GetNumberOfConnectedArcs() != 1)
    {
    cerr << "Connectivity on Arc 1 is wrong after update" << endl;
    return 1;
    }

  double pos[3];
  vtkCMBArcEndNode *en = arc->GetEndNode(0);
  en->GetPosition(pos);
  if ( pos[0] != 0.5)
    {
    cerr << "end node 1 failed to move properly" << endl;
    return 1;
    }

  en = arc->GetEndNode(1);
  en->GetPosition(pos);
  if ( pos[1] != 1.0 || pos[0] != 0.5)
    {
    cerr << "end node 2 failed to move properly" << endl;
    return 1;
    }




  return 0;
}
