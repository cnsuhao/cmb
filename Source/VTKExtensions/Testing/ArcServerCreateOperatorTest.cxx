//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Test functionality of the vtkCMBArc Operator
// Same basic test as ArcServerCreateMoveTest but create
// arcs using the operator rather than by hand

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcCreateOperator.h"
#include "vtkCMBArcDeleteOperator.h"

#include "vtkCellArray.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerCreateOperatorTest( int /*argc*/, char * /*argv*/[] )
{
  bool valid = false;
  double en1[3]={0,0,0};
  double en2[3]={10,1,0};
  double en3[3]={2,2,0};

  {
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(1,0,0);
  points->InsertNextPoint( 4, 0, 0 );
  points->InsertNextPoint( 8, 0.5, 0 );
  points->InsertNextPoint( 9, 1, 0 );
  points->InsertNextPoint(en2);
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
  }

  {
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en1);
  points->InsertNextPoint(-1,0,0);
  points->InsertNextPoint( -4, 0, 0 );
  points->InsertNextPoint( -8, -0.5, 0 );
  points->InsertNextPoint( -9, -1, 0 );
  arcP->SetPoints(points);
  points->FastDelete();
  vtkCellArray *lines = vtkCellArray::New();
  for (vtkIdType i=0; i < 4; ++i)
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
    cerr << "Unable to create the 2nd arc" << endl;
    return 1;
    }
  }

  {
  vtkSmartPointer<vtkPolyData> arcP = vtkSmartPointer<vtkPolyData>::New();
  vtkPoints *points = vtkPoints::New();
  points->InsertNextPoint(en3);
  points->InsertNextPoint(0,2,0);
  points->InsertNextPoint(-2, 0, 0);
  arcP->SetPoints(points);
  points->FastDelete();
  vtkCellArray *lines = vtkCellArray::New();
  lines->InsertNextCell(4);
  lines->InsertCellPoint(0);
  lines->InsertCellPoint(1);
  lines->InsertCellPoint(2);
  lines->InsertCellPoint(0);

  arcP->SetLines(lines);
  lines->FastDelete();

  vtkSmartPointer<vtkCMBArcCreateOperator> createArc = vtkSmartPointer<vtkCMBArcCreateOperator>::New();
  valid = createArc->Operate(arcP);
  if (!valid)
    {
    cerr << "Unable to create the 3rd arc" << endl;
    return 1;
    }
  }


  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();

  //get the number of arc from the manager
  int numArcs = manager->GetNumberOfArcs();

  //dump out all the arc info
  for ( int i=0; i < numArcs; i++)
    {
    vtkCMBArc* tmp = manager->GetArc(i);
    tmp->PrintSelf(cout,vtkIndent());
    }

  //make sure the manager info is all valid
  if ( 3 != numArcs )
    {
    //isn't tracking all the arcs
    cerr << "number of arcs is incorrect" << endl;
    return 1;
    }

  //verify the number of internal nodes in each arc
  if ( manager->GetArc(0)->GetNumberOfInternalPoints() != 4 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( manager->GetArc(1)->GetNumberOfInternalPoints() != 3 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( manager->GetArc(2)->GetNumberOfInternalPoints() != 2 )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  double pos[3] = {10,1,0};
  vtkCMBArcEndNode *en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(0)->GetEndNode(1) )
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  pos[0] = 0;
  pos[1] = 0;
  pos[2] = 0;
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

  pos[0] = 2;
  pos[1] = 2;
  pos[2] = 0;
  en = manager->GetEndNodeAt(pos);
  if ( en != manager->GetArc(2)->GetEndNode(0) )
    {
    //locator failed on loop end node
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }
  if ( en != manager->GetArc(2)->GetEndNode(1) )
    {
    bool isclosed = manager->GetArc(2)->IsClosedArc();
    double posEnd[3];
    manager->GetArc(2)->GetEndNode(1)->GetPosition(posEnd);
    cerr << "Second End Node Pos " << posEnd[0] << "," << posEnd[1] << "," << posEnd[2]<<endl;
    //locator failed on loop end node
    cerr << "isClosed" << isclosed << endl;
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  //move the shared end node and make sure both arcs move!
  pos[0] = 1;
  pos[1] = 0;
  pos[2] = 0;
  vtkCMBArc *arc = manager->GetArc(1);
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

  //now lets test out the delete arc operator
  vtkNew<vtkCMBArcDeleteOperator> delOp;

  valid = delOp->Operate(-1);
  if (valid)
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  valid = delOp->Operate(0);
  if (!valid)
    {
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

  valid = delOp->Operate(0);
  if (valid)
    {
    //can't delete an arc that already has been deleted
    cerr << "Failed on Line: " << __LINE__ <<endl;
    return 1;
    }

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
