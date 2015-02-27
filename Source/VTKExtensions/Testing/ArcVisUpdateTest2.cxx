
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    cmbContourTest.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcProvider.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcCreateOperator.h"
#include "vtkCMBArcUpdateOperator.h"


#include "vtkActor.h"

#include "vtkCellArray.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkRegressionTestImage.h"


namespace
{
int create_default_arc()
{
  bool valid = false;
  double en1[3]={0,50,10};
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(4,50,10);
  points->InsertNextPoint( 4, 54, 10 );
  points->InsertNextPoint( 0, 54, 10 );
  points->InsertNextPoint( 0, 52, 10 );
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

  //create another arc off of arc1 on the shared end node
  //this will be the loop of the letter p
  vtkCMBArc* secondArc = vtkCMBArc::New();
  secondArc->SetEndNode(0,en1);
  secondArc->SetEndNode(1,en1);
  secondArc->InsertNextPoint(3,0,10);
  secondArc->InsertNextPoint(3,-3,10);
  secondArc->InsertNextPoint(0,-3,10);
  return 0;
}

vtkPolyData* moved_arc()
{
  //Should look like the letter: p
  //this will be the stalk
  vtkPolyData* arcP = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(0,0,10);
  points->InsertNextPoint(0,-1,10);
  points->InsertNextPoint(0,-2,10);
  points->InsertNextPoint(0,-3,10);
  points->InsertNextPoint(0,-4,10);
  points->InsertNextPoint(0,-5,10);
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

int ArcVisUpdateTest2( int argc, char *argv[] )
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(600, 600);

  //add the loop arc
  int res = create_default_arc();
  if ( res == 1)
    {
    return 1;
    }

  //create a mapper and actor for each arc
  int numArcs = vtkCMBArcManager::GetInstance()->GetNumberOfArcs();
  for (int i=0; i < numArcs; i++)
    {
    vtkSmartPointer<vtkCMBArcProvider> filter =
      vtkSmartPointer<vtkCMBArcProvider>::New();
    filter->SetArcId(i);

    vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( filter->GetOutputPort() );

    vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper );
    renderer->AddViewProp( actor );
    }
  iren->Initialize();
  renWin->Render();

  //take a loop arc, and convert it to an arc
  vtkSmartPointer<vtkCMBArcUpdateOperator> updateArc = vtkSmartPointer<vtkCMBArcUpdateOperator>::New();
  updateArc->SetArcId(0);
  updateArc->SetEndNodeToMove(0);
  updateArc->SetEndNodeToRecreate(1);

  vtkPolyData* mArc = moved_arc();
  /*bool valid = */updateArc->Operate(mArc);
  mArc->Delete();

  renderer->ResetCamera();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    return 0;
    }

  vtkCMBArc *secondArc = vtkCMBArcManager::GetInstance()->GetArc(1);
  if (!secondArc->IsClosedArc())
    {
    cerr << "second arc should be a closed loop" << endl;
    return 1;
    }
  if (secondArc->GetNumberOfConnectedArcs() != 1)
    {
    cerr << "second arc should be connected toanother loop" << endl;
    }

  return !retVal;
}
