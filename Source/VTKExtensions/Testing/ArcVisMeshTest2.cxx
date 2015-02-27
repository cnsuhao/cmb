
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
#include "vtkCMBArcManager.h"
#include "vtkCMBPolygonFromArcsOperator.h"
#include "vtkCMBArcPolygonProvider.h"

#include "vtkIdTypeArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

#include "vtkRegressionTestImage.h"

int ArcVisMeshTest2( int argc, char *argv[] )
{
  //what happens when given two outer loops that
  //share an edge
  double en[5][3]={{0,0,0},
                  {-5,5,0},
                  {0,10,0},
                  {5,5,0},
                  {0,0,0}};

  for ( int i = 0; i < 4; i++)
    {
    vtkCMBArc *arc = vtkCMBArc::New();
    arc->SetEndNode(0,en[i]);
    arc->SetEndNode(1,en[i+1]);
    }

  //make the loop invalid by splitting it down the middle
  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0,en[2]);
  arc->SetEndNode(1,en[0]);

  //now add a outer loop around the entire scene
  vtkCMBArc *outerArc = vtkCMBArc::New();
  double oen[3] = {-5,10,0};
  outerArc->SetEndNode(0,oen);
  outerArc->SetEndNode(1,oen);
  outerArc->InsertNextPoint(5,10,0);
  outerArc->InsertNextPoint(5,0,0);
  outerArc->InsertNextPoint(-5,0,0);

  vtkSmartPointer<vtkCMBPolygonFromArcsOperator> pfa =
      vtkSmartPointer<vtkCMBPolygonFromArcsOperator>::New();

  for ( int i = 0; i < 6; i++)
    {
    pfa->AddArcId(i);
    }

  bool valid = pfa->Operate();
  if(!valid)
    {
    cerr << "failed to find loops" << endl;
    return 1;
    }

  vtkSmartPointer<vtkCMBArcPolygonProvider> polygon =
      vtkSmartPointer<vtkCMBArcPolygonProvider>::New();
  polygon->SetOuterLoopArcIds(pfa->GetOuterLoop());


  //ToDo we can't mesh two holes that share an edge
  vtkIdType size = pfa->GetNumberOfInnerLoops();
  for(vtkIdType i=0;i<size-1;++i)
    {
    polygon->AddInnerLoopArcIds(pfa->GetInnerLoop(i));
    }


  // Create the RenderWindow, Renderer and both Actors
  //
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
      vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(600, 600);
  iren->Initialize();

  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( polygon->GetOutputPort() );

  vtkSmartPointer<vtkPolyDataMapper> mapper2 =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper2->SetInputConnection( polygon->GetOutputPort() );


  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  actor->GetProperty()->SetRepresentationToWireframe();
  renderer->AddViewProp( actor );

  vtkSmartPointer<vtkActor> actor2 =
    vtkSmartPointer<vtkActor>::New();
  actor2->SetMapper( mapper2 );
  actor2->SetPosition(10,0,0);
  renderer->AddViewProp( actor2 );


  renderer->ResetCamera();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    return 0;
    }

  return !retVal;


  return 0;
}
