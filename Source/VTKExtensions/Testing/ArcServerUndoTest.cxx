//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Test functionality of the vtkCMBArc Creation.
// Will test adding arcs to the manager
// Will Test undo / redoing an arc
// Will test doing a merge on an end node that is part of a arc that
// is on the undo stack

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkTesting.h"

int ArcServerUndoTest( int /*argc*/, char * /*argv*/[] )
{
  double en1[3]={0,0,0};
  double en2[3]={10,1,0};
  double en3[3]={2,2,0};


  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0,vtkCMBArc::Point(en1,0));
  arc->SetEndNode(1,vtkCMBArc::Point(en2,1));

  arc->InsertNextPoint( 2, 1, 0, 0 );
  arc->InsertNextPoint( 3, 4, 0, 0 );
  arc->InsertNextPoint( 4, 8, 0.5, 0 );
  arc->InsertNextPoint( 5, 9, 1, 0 );

  vtkCMBArc *arc2 = vtkCMBArc::New();
  arc2->SetEndNode(0, vtkCMBArc::Point(en1,0));

  arc2->InsertNextPoint( 6, -1, 0, 0 );
  arc2->InsertNextPoint( 7, -4, 0, 0 );
  arc2->InsertNextPoint( 8, -8, -0.5, 0 );
  arc2->InsertNextPoint( 9, -9, -1, 0 );

  vtkCMBArc *loopArc = vtkCMBArc::New();
  loopArc->SetEndNode(0,vtkCMBArc::Point(en3, 10));
  loopArc->SetEndNode(1,vtkCMBArc::Point(en3, 10));
  loopArc->InsertNextPoint(11, 0, 2, 0 );
  loopArc->InsertNextPoint(12, -2, 0, 0 );


  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  //make sure the manager info is all valid
  if ( 3 !=  manager->GetNumberOfArcs() )
    {
    //isn't tracking all the arcs
    return 1;
    }

  arc->MarkedForDeletion();
  if ( 2 != manager->GetNumberOfArcs() )
    {
    return 1;
    }

  //make sure the id of arc is an invalid id to get back from the manager
  if ( NULL != manager->GetArc( arc->GetId() ) )
    {
    return 1;
    }

  //undo the undo
  arc->UnMarkedForDeletion();
  if ( NULL == manager->GetArc( arc->GetId() ) )
    {
    return 1;
    }
  if ( 2 == manager->GetNumberOfArcs() )
    {
    return 1;
    }

  //next step is a byte more complicated. We remove arc1, than
  //we move arc3 endnode to be the same place as arc1 end node 2 was at.
  //when we undo the delete on arc1 we need to make sure it uses the same
  //endnode as arc3
  arc->MarkedForDeletion();
  loopArc->MoveEndNode(0, vtkCMBArc::Point(en2,2));
  arc->UnMarkedForDeletion();

  if ( arc->GetEndNode(1) != loopArc->GetEndNode(0) )
    {
    return 1;
    }
  if ( arc->GetEndNode(1) != loopArc->GetEndNode(1) )
    {
    return 1;
    }
  if ( arc->GetEndNode(0) != arc2->GetEndNode(0) )
    {
    return 1;
    }

  vtkCMBArcEndNode* en = loopArc->GetEndNode(0);
  if ( manager->GetConnectedArcs(en).size() != 2 )
    {
    return 1;
    }
  if ( manager->GetConnectedArcs( loopArc ).size() != 1 ||
       loopArc->GetNumberOfConnectedArcs() != 1)
    {
    return 1;
    }
  if ( manager->GetConnectedArcs( arc ).size() != 2 )
    {
    return 1;
    }

  en = arc->GetEndNode(0);
  if ( manager->GetConnectedArcs( en ).size() != 2 )
    {
    return 1;
    }

  en = arc->GetEndNode(1);
  if ( manager->GetConnectedArcs( en ).size() != 2 )
    {
    return 1;
    }

  loopArc->MarkedForDeletion();
  arc->MarkedForDeletion();
  arc2->MarkedForDeletion();

  return 0;
}
