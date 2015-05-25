//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkGlyph3D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkTransform.h"
#include "vtkUnsignedCharArray.h"
#include <string>

//----------------------------------------------------------------------------
void CreatePolyDataGrid(vtkPolyData* poly, const int numColors)
{
    // we hard code how many steps to display
   double n = numColors;
   int numRow, numCol;
   numRow = numCol = vtkMath::Floor(sqrt(n));
   int leftover = numColors%numRow;

   //for points

   numRow++;
   numCol++;

   int numPts = numRow*numCol;
   numPts = leftover ? numPts+leftover+1 : numPts;

  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(numPts);

  int i, j, id;
  for(i=0;i<numRow;i++)
    {
    id = i*numCol;
    for(j=0; j<numCol; j++)
      {
      pts->InsertPoint(id+j, i*0.3, j*0.3, 0);
      }
    }

  int tmpNum = numRow*numCol;
  for(i=tmpNum; i<numPts; i++)
    {
    pts->InsertPoint(i, (i-tmpNum)*0.3, numCol*0.3, 0);
    }
  poly->SetPoints(pts);
  pts->Delete();

  // for cells

  const int numCells = numColors;
  poly->Allocate(numCells);
  int id1;
  for(i=0;i<numRow-1;i++)
    {
    id = i*numCol;
    id1 = (i+1)*numCol;
    for(j=0; j<numCol-1; j++)
      {
      vtkIdType nodes[4];
      nodes[0] = id+j;
      nodes[1] = id+j+1;
      nodes[2] = id1+j+1;
      nodes[3] = id1+j;
      poly->InsertNextCell(VTK_QUAD, 4, nodes);
      }
    }
  tmpNum = (numRow-1)*(numCol-1);
  id = numRow*numCol;
  id1 = numPts - 1 -leftover;
  for(j=0; j<leftover; j++)
    {
      vtkIdType nodes[4];
      nodes[0] = id+j;
      nodes[1] = id+j+1;
      nodes[2] = id1+j+1;
      nodes[3] = id1+j;
      poly->InsertNextCell(VTK_QUAD, 4, nodes);
    }

  vtkIntArray *shellData = vtkIntArray::New();
  shellData->SetNumberOfComponents(1);
  shellData->SetNumberOfTuples(numCells);

  for(i=0;i<numCells;i++)
    {
    shellData->SetValue(i, i);
    }
  shellData->SetName("Colors");
  poly->GetCellData()->SetScalars(shellData);
  shellData->Delete();

}


int main(int /*argc*/, char * /*argv*/[])
{
  vtkNew<vtkRenderWindow> renWin;
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyleSwitch> style;
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle( style.GetPointer() );

  iren->SetRenderWindow(renWin.GetPointer());
  renWin->AddRenderer( renderer.GetPointer() );

  vtkNew<vtkDiscreteLookupTable> lut;
  lut->Build();

  int numColors = lut->GetNumberOfValues();
  vtkNew<vtkPolyData> poly;
  poly->Initialize();
  CreatePolyDataGrid(poly.GetPointer(), numColors);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData( poly.GetPointer() );
  mapper->SetLookupTable(lut.GetPointer());
  vtkNew<vtkActor> actor;
  actor->SetMapper( mapper.GetPointer() );

  renderer->AddViewProp( actor.GetPointer() );

  iren->Initialize();
  renWin->Render();
  //iren->Start();

  return 0;
}
