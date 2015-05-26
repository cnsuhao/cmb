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
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBPolygonFromArcsOperator.h"
#include "vtkCMBArcPolygonProvider.h"

#include "vtkIdTypeArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

#include "vtkRegressionTestImage.h"

int ArcVisMeshTest1( int argc, char *argv[] )
{
  double en1[3]={0,0,0};
  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0,en1);
  arc->SetEndNode(1,en1);

  arc->InsertNextPoint( 10, 0, 0 );
  arc->InsertNextPoint( 10, 10, 0 );
  arc->InsertNextPoint( 0, 10, 0 );


  vtkSmartPointer<vtkCMBPolygonFromArcsOperator> pfa =
      vtkSmartPointer<vtkCMBPolygonFromArcsOperator>::New();

  pfa->AddArcId(0);

  bool valid = pfa->Operate();
  if (!valid)
    {
    cerr << "unable to mesh a basic loop" << endl;
    return 1;
    }

  vtkSmartPointer<vtkCMBArcPolygonProvider> polygon =
      vtkSmartPointer<vtkCMBArcPolygonProvider>::New();
  polygon->SetOuterLoopArcIds(pfa->GetOuterLoop());


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

  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  renderer->AddViewProp( actor );


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
