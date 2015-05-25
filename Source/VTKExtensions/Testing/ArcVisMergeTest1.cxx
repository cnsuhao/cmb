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
#include "vtkCMBArcMergeArcsOperator.h"

#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkRegressionTestImage.h"

#include <map>
namespace
{
std::map<vtkIdType,vtkCMBArcProvider*> providerMap;
std::map<vtkIdType,vtkActor*> actorMap;

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

  providerMap.insert( std::pair<vtkIdType,vtkCMBArcProvider*> (arcId,filter));
  actorMap.insert( std::pair<vtkIdType,vtkActor*> (arcId,actor));
  }

void RemoveArc(vtkRenderer *renderer, const vtkIdType& arcId)
  {
  vtkActor *actor = actorMap.find(arcId)->second;
  renderer->RemoveViewProp(actor);
  vtkCMBArcManager::GetInstance()->GetArc(arcId)->Delete();
  }
}
int ArcVisMergeTest1( int argc, char *argv[] )
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();
  //render the box
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(600, 600);

  double en[6][3]={{0,0,0},
                  {10,0,0},
                  {10,5,0},
                  {5,2.5,0},
                  {0,5,0},
                  {0,0,0}};

  for ( int i = 0; i < 5; i++)
    {
    vtkCMBArc *arc = vtkCMBArc::New();
    if ( i % 2 == 0)
      {
      arc->SetEndNode(0,en[i]);
      arc->SetEndNode(1,en[i+1]);
      }
    else
      {
      arc->SetEndNode(0,en[i+1]);
      arc->SetEndNode(1,en[i]);
      }
    AddMapperForArc(renderer,arc->GetId());
    }

  bool valid;
  vtkSmartPointer<vtkCMBArcMergeArcsOperator> merge =
      vtkSmartPointer<vtkCMBArcMergeArcsOperator>::New();

  valid = merge->Operate(0,1);
  if (!valid)
    {
    cerr << "merge 1 should pass" << endl;
    return 1;
    }
  vtkIdType createdId1 = merge->GetCreatedArcId();
  RemoveArc(renderer,merge->GetArcIdToDelete());

  valid = merge->Operate(createdId1,4);
  if (!valid)
    {
    cerr << "merge 2 should pass" << endl;
    return 1;
    }
  createdId1 = merge->GetCreatedArcId();
  RemoveArc(renderer,merge->GetArcIdToDelete());

  valid = merge->Operate(3,2);
  if (!valid)
    {
    cerr << "merge 3 should pass" << endl;
    return 1;
    }
  vtkIdType createdId2 = merge->GetCreatedArcId();
  RemoveArc(renderer,merge->GetArcIdToDelete());

  valid = merge->Operate(createdId2,createdId1);
  if(!valid)
    {
    cerr << "merge 4 should pass" << endl;
    return 1;
    }
  RemoveArc(renderer,merge->GetArcIdToDelete());

  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  if ( manager->GetNumberOfArcs() != 1 )
    {
    cout << manager->GetNumberOfArcs() << " Number of Arcs" << endl;
    return 1;
    }

  if ( manager->GetNumberOfEndNodes() != 1 )
    {
    cout << manager->GetNumberOfArcs() << " Number of End Nodes" << endl;
    return 1;
    }

  vtkCMBArc *lastArc = manager->GetArc(merge->GetCreatedArcId());
  lastArc->PrintSelf(cout,vtkIndent());

  if ( lastArc->GetNumberOfConnectedArcs() != 0 ||
      lastArc->GetNumberOfConnectedArcs(0) != 0 ||
      lastArc->GetNumberOfConnectedArcs(1) != 0)
    {
    cout << "incorrect connection" << endl;
    return 1;
    }

  if ( lastArc->IsClosedArc() == false )
    {
    cout << "last arc should be closed" << endl;
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
