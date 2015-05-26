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
#include "vtkCMBArcProvider.h"
#include "vtkCMBArcSplitOnPositionOperator.h"

#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkRegressionTestImage.h"

namespace
{
void AddMapperForArc(vtkRenderer *renderer, const vtkIdType& arcId)
  {
  vtkSmartPointer<vtkCMBArcProvider> filter =
      vtkSmartPointer<vtkCMBArcProvider>::New();
  filter->SetArcId(arcId);

  vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( filter->GetOutputPort() );

  vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  renderer->AddViewProp( actor );

  }
}

int ArcVisSplitTest( int argc, char *argv[] )
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  double bl[3]={0,0,0};
  double br[3]={5,0,0};
  double tl[3]={0,5,0};
  double tr[3]={5,5,0};

  vtkCMBArc *rectangleArc = vtkCMBArc::New();
  rectangleArc->SetEndNode(0,bl);
  rectangleArc->SetEndNode(1,bl);
  rectangleArc->InsertNextPoint(br);
  rectangleArc->InsertNextPoint(tr);
  rectangleArc->InsertNextPoint(tl);

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
    AddMapperForArc(renderer,i);
    }
  iren->Initialize();
  renWin->Render();

  vtkSmartPointer<vtkCMBArcSplitOnPositionOperator> splitter =
      vtkSmartPointer<vtkCMBArcSplitOnPositionOperator>::New();
  splitter->SetSplitPosition(tr);
  splitter->Operate(rectangleArc->GetId());
  vtkIdType splitId = splitter->GetCreatedArcId();
  if (-1==splitId)
    {
    cerr << "unable to split at the top right position" << endl;
    return 1;
    }
  AddMapperForArc(renderer,splitId);

  //this one should fail
  splitter->SetSplitPosition(tr);
  bool valid = splitter->Operate(rectangleArc->GetId());
  if (valid)
    {
    cerr << "Split at an invalid location!" << endl;
    return 1;
    }

  splitter->SetSplitPosition(tl);
  splitter->Operate(splitId);
  splitId = splitter->GetCreatedArcId();
  if (-1==splitId)
    {
    cerr << "unable to split at the top left position" << endl;
    return 1;
    }
  AddMapperForArc(renderer,splitId);

  splitter->SetSplitPosition(br);
  splitter->Operate(rectangleArc->GetId());
  splitId = splitter->GetCreatedArcId();
  if (-1==splitId)
    {
    cerr << "unable to split at the top right position" << endl;
    return 1;
    }
  AddMapperForArc(renderer,splitId);

  //Confirm the connectivity
  if (rectangleArc->GetNumberOfConnectedArcs() != 2 ||
      rectangleArc->GetNumberOfConnectedArcs(0) != 1 ||
      rectangleArc->GetNumberOfConnectedArcs(1) != 1)
    {
    cerr << "Incorrect connectivy on the original arc after the splits" <<endl;
    cerr << "rectangleArc->GetNumberOfConnectedArcs() " << rectangleArc->GetNumberOfConnectedArcs() << endl;
    cerr <<  "rectangleArc->GetNumberOfConnectedArcs(0) " << rectangleArc->GetNumberOfConnectedArcs(0) << endl;
    cerr <<  "rectangleArc->GetNumberOfConnectedArcs(1) " << rectangleArc->GetNumberOfConnectedArcs(1) << endl;
    return 1;
    }

  vtkCMBArcEndNode *en = vtkCMBArcManager::GetInstance()->GetEndNodeAt(tr);
  if (vtkCMBArcManager::GetInstance()->GetConnectedArcs(en).size() != 2)
    {
    cerr << "Incorrect connectivy on the top right end node after the splits" <<endl;
    cerr << "vtkCMBArcManager::GetInstance()->GetConnectedArcs(en).size() " << vtkCMBArcManager::GetInstance()->GetConnectedArcs(en).size() << endl;
    return 1;
    }

  if (vtkCMBArcManager::GetInstance()->GetNumberOfArcs() != 4 )
    {
    cerr << "wrong number of arcs after split" << endl;
    return 1;
    }

  if (vtkCMBArcManager::GetInstance()->GetNumberOfEndNodes() != 4 )
    {
    cerr << "wrong number of end nodes after split" << endl;
    return 1;
    }

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
