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
#include "vtkCMBArcGrowOperator.h"

#include "vtkIdTypeArray.h"
#include "vtkSmartPointer.h"
#include "vtkTesting.h"

int ArcServerGrowTest( int /*argc*/, char * /*argv*/[] )
{
  //what happens when given two outer loops that
  //share an edge
  double en[5][3]={{0,0,0},
                  {-5,5,0},
                  {10,0,0},
                  {5,5,0},
                  {0,0,0}};

  for ( int i = 0; i < 4; i++)
    {
    vtkCMBArc *arc = vtkCMBArc::New();
    arc->SetEndNode(0, vtkCMBArc::Point(en[i], i));
    arc->SetEndNode(1, vtkCMBArc::Point(en[i+1], i+1));
    }

  //make the loop invalid by splitting it down the middle
  vtkCMBArc *arc = vtkCMBArc::New();
  arc->SetEndNode(0, vtkCMBArc::Point(en[2], 2));
  arc->SetEndNode(1, vtkCMBArc::Point(en[0], 0));

  vtkSmartPointer<vtkCMBArcGrowOperator> grow =
      vtkSmartPointer<vtkCMBArcGrowOperator>::New();
  grow->AddArc(0);
  bool valid = grow->Operate();
  if (!valid)
    {
    return 1;
    }

  //now confirm the grow contains 0,1,3,4
  vtkIdTypeArray *result = grow->GetGrownArcSetIds();
  if (result->GetNumberOfTuples() != 4)
    {
    return 1;
    }
  int sum = 0;
  for (vtkIdType i=0; i <4; i++)
    {
    sum += result->GetValue(i);
    if (result->GetValue(i) == 2)
      {
      return 1;
      }
    }
  if ( sum != 8)
    {
    //we didn't select the correct ids
    return 1;
    }


  return 0;
}
