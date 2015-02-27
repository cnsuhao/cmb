
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

#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkRegressionTestImage.h"

int ArcVisMoveTest( int argc, char *argv[] )
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  double bl[3]={0,0,0};
  double br[3]={5,0,0};
  double tl[3]={0,5,0};
  double tr[3]={5,5,0};

  vtkCMBArc *bottomArc = vtkCMBArc::New();
  bottomArc->SetEndNode(0,bl);
  bottomArc->SetEndNode(1,br);

  vtkCMBArc *topArc = vtkCMBArc::New();
  topArc->SetEndNode(0,tr);
  topArc->InsertNextPoint(4,5,0);
  topArc->InsertNextPoint(3,5,0);
  topArc->InsertNextPoint(2,5,0);
  topArc->InsertNextPoint(1,5,0);
  topArc->SetEndNode(1,tl);

  double tlhalf[3]={0,2.5,0};
  vtkCMBArc *leftArc1 = vtkCMBArc::New();
  leftArc1->SetEndNode(0,bl);
  leftArc1->SetEndNode(1,tlhalf);

  vtkCMBArc *leftArc2 = vtkCMBArc::New();
  leftArc2->SetEndNode(0,tlhalf);
  leftArc2->InsertNextPoint(0,4,0);
  leftArc2->SetEndNode(1,tl);

  double trhalf[3]={5,2.5,0};
  vtkCMBArc *rightArc1 = vtkCMBArc::New();
  rightArc1->SetEndNode(0,br);
  rightArc1->SetEndNode(1,trhalf);

  vtkCMBArc *rightArc2 = vtkCMBArc::New();
  rightArc2->SetEndNode(0,trhalf);
  rightArc2->InsertNextPoint(5,3,0);
  rightArc2->InsertNextPoint(5,3.5,0);
  rightArc2->InsertNextPoint(5,4,0);
  rightArc2->InsertNextPoint(5,4.5,0);
  rightArc2->SetEndNode(1,tr);

  //render the box
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(600, 600);

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

  //move the end nodes and make sure all the arcs move
  //correctly
  double bl2[3]={0,0,10};
  double br2[3]={5,0,10};
  bottomArc->MoveEndNode(0,bl2);
  bottomArc->MoveEndNode(1,br2);
  renderer->ResetCamera();
  renWin->Render();

  //move the side mid points out
  double trhalf2[3]={7,2.5,0};
  double tlhalf2[3]={-2,2.5,0};
  leftArc2->MoveEndNode(0,tlhalf2);
  rightArc2->MoveEndNode(0,trhalf2);
  renderer->ResetCamera();
  renWin->Render();

  //Merge two end nodes together and make sure everything refreshes
  //correctly
  double middlePoint[3] = {2.5,2.5,5};
  leftArc1->MoveEndNode(1,middlePoint);
  rightArc1->MoveEndNode(1,middlePoint);
  renderer->ResetCamera();
  renWin->Render();

  //Move the merged end node once again
  double middlePoint2[3] = {2.5,12.5,5};
  rightArc1->MoveEndNode(1,middlePoint2);
  renderer->ResetCamera();
  renWin->Render();

  //Remove all the internal points
  bottomArc->ClearPoints();
  topArc->ClearPoints();
  rightArc1->ClearPoints();
  rightArc2->ClearPoints();
  leftArc1->ClearPoints();
  leftArc2->ClearPoints();

  renderer->ResetCamera();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    return 0;
    }

  return !retVal;
}
