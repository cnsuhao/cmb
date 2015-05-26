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
#include "vtkCMBArcMergeArcsOperator.h"

#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerMergeTest( int argc, char *argv[] )
{
  double en1[3]={0,0,2};
  double en2[3]={-10,-1,3};
  double en3[3]={2,2,0};


  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0,en1);
  arc->SetEndNode(1,en1);

  arc->InsertNextPoint( 1, 0, 2 );
  arc->InsertNextPoint( 4, 0, 2 );

  vtkCMBArc *arc2 = vtkCMBArc::New();
  arc2->SetEndNode(0,en1);
  arc2->SetEndNode(1,en2);

  arc2->InsertNextPoint( -1, 0, 4 );
  arc2->InsertNextPoint( -4, 0, 4 );
  arc2->InsertNextPoint( -8, -0.5, 4 );
  arc2->InsertNextPoint( -9, -1, 4 );

  vtkCMBArc *arc3 = vtkCMBArc::New();
  arc3->SetEndNode(0,en3);
  arc3->SetEndNode(1,en3);
  arc3->InsertNextPoint( 0, 2, 0 );
  arc3->InsertNextPoint( -2, 0, 0 );

  vtkSmartPointer<vtkCMBArcMergeArcsOperator> merge =
      vtkSmartPointer<vtkCMBArcMergeArcsOperator>::New();

  bool valid = merge->Operate(-1,300);
  if (valid)
    {
    cerr << "Passed invalid arc ids merge can't pass" << endl;
    return 1;
    }

  valid = merge->Operate(arc->GetId(),300);
  if (valid)
    {
    cerr << "Passed closed arc, no way to merge" << endl;
    return 1;
    }

  valid = merge->Operate(arc->GetId(),arc3->GetId());
  if (valid)
    {
    cerr << "Passed two closed arcs, no way to merge" << endl;
    return 1;
    }

  valid = merge->Operate(arc->GetId(),arc2->GetId());
  if (valid)
    {
    cerr << "Passed a closed arc and a single arc no way to merge" << endl;
    return 1;
    }

  vtkCMBArc *arc4 = vtkCMBArc::New();
  arc4->SetEndNode(0,en1);
  arc4->SetEndNode(1,en2);

  arc4->InsertNextPoint( 1, 0, 4 );
  arc4->InsertNextPoint( 4, 0, 4 );
  arc4->InsertNextPoint( 8, -0.5, 4 );
  arc4->InsertNextPoint( 9, -1, 4 );

  valid = merge->Operate(arc4->GetId(),arc2->GetId());
  if (!valid)
    {
    cerr << "Passed a two arcs that make a loop this should pass" << endl;
    }
  vtkIdType createdId = merge->GetCreatedArcId();


  vtkCMBArc *createdArc = vtkCMBArcManager::GetInstance()->GetArc(createdId);
  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  manager->GetArc(merge->GetArcIdToDelete())->Delete();
  if (!createdArc->IsClosedArc())
    {
    cerr << "The arc needs to be closed" << endl;
    return 1;
    }
  if (manager->GetNumberOfArcs() != 3)
    {
    cerr << "Incorrect cleanup of arcs after merge orperator" << endl;
    return 1;
    }

  return 0;
}
