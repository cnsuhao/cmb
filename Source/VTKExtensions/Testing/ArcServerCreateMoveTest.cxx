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
// Test functionality of the vtkCMBArc Creation.
// Will test adding arcs to the manager
// Will test an arc with one end node,two end nodes and a closed loop
// Test moving a shared end node
// Test delete of arcs

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkTesting.h"

int ArcServerCreateMoveTest( int /*argc*/, char * /*argv*/[] )
{
  double en1[3]={0,0,0};
  double en2[3]={10,1,0};
  double en3[3]={2,2,0};


  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0,en1);
  arc->SetEndNode(1,en2);

  arc->InsertNextPoint( 1, 0, 0 );
  arc->InsertNextPoint( 4, 0, 0 );
  arc->InsertNextPoint( 8, 0.5, 0 );
  arc->InsertNextPoint( 9, 1, 0 );

  vtkCMBArc *arc2 = vtkCMBArc::New();
  arc2->SetEndNode(0,en1);

  arc2->InsertNextPoint( -1, 0, 0 );
  arc2->InsertNextPoint( -4, 0, 0 );
  arc2->InsertNextPoint( -8, -0.5, 0 );
  arc2->InsertNextPoint( -9, -1, 0 );

  vtkCMBArc *loopArc = vtkCMBArc::New();
  loopArc->SetEndNode(0,en3);
  loopArc->SetEndNode(1,en3);
  loopArc->InsertNextPoint( 0, 2, 0 );
  loopArc->InsertNextPoint( -2, 0, 0 );

  //don't delete the arcs as that will remove them from the manager
  arc = NULL;
  arc2 = NULL;
  loopArc = NULL;


  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();

  //get the number of arc from the manager
  int numArcs = manager->GetNumberOfArcs();
  //make sure the manager info is all valid
  if ( 3 != numArcs )
    {
    //isn't tracking all the arcs
    return 1;
    }

  double pos[3] = {10,1,0};
  vtkCMBArcEndNode *en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(0)->GetEndNode(1) )
    {
    //locator failed
    return 1;
    }

  pos[0] = 0;
  pos[1] = 0;
  pos[2] = 0;
  en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(0)->GetEndNode(0) )
    {
    //locator failed
    return 1;
    }
  if ( en != manager->GetArc(1)->GetEndNode(0))
    {
    //locator failed to find a shared end point
    return 1;
    }

  pos[0] = 2;
  pos[1] = 2;
  pos[2] = 0;
  en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(2)->GetEndNode(0) )
    {
    //locator failed on loop end node
    return 1;
    }
  if ( en != manager->GetArc(2)->GetEndNode(1) )
    {
    //locator failed on loop end node
    return 1;
    }

  //move the shared end node and make sure both arcs move!
  pos[0] = 1;
  pos[1] = 0;
  pos[2] = 0;
  arc = manager->GetArc(1);
  arc->MoveEndNode(0,pos);


  en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(0)->GetEndNode(0) )
    {
    //locator failed
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( en != manager->GetArc(1)->GetEndNode(0))
    {
    //locator failed to find a shared end point
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  //move the loop to be merged on top of the other two arcs
  arc = manager->GetArc(2);
  arc->MoveEndNode(1,pos);

  en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(0)->GetEndNode(0) )
    {
    //locator failed
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( en != manager->GetArc(1)->GetEndNode(0))
    {
    //locator failed to find a shared end point
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
if ( en != manager->GetArc(2)->GetEndNode(0) )
    {
    //locator failed on loop end node
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( en != manager->GetArc(2)->GetEndNode(1) )
    {
    //locator failed on loop end node
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  if ( 3 != manager->GetNumberOfArcs(en) )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  if ( manager->GetConnectedArcs(manager->GetEndNodeAt(en2)).size() != 1 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  if ( manager->GetConnectedArcs( manager->GetArc(0) ).size() != 2 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( manager->GetConnectedArcs( manager->GetArc(1) ).size() != 2 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( manager->GetConnectedArcs( manager->GetArc(2) ).size() != 2 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  //now lets test out the delete arc methods
  arc = manager->GetArc(0);
  arc->Delete();

  numArcs = manager->GetNumberOfArcs();
  if ( numArcs != 2 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  if (manager->GetConnectedArcs( manager->GetEndNodeAt( pos ) ).size() != 2)
    {
    cerr << "Found " <<
            manager->GetConnectedArcs( manager->GetEndNodeAt( pos ) ).size() <<
            " Arcs connected to the end node." << endl;
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if (manager->GetConnectedArcs( manager->GetEndNodeAt( en1 ) ).size() != 0)
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if (manager->GetConnectedArcs( manager->GetEndNodeAt( en2 ) ).size() != 0)
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if (manager->GetConnectedArcs( manager->GetEndNodeAt( en3 ) ).size() != 0)
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  cout << "test Passed" << endl;
  return 0;
}
