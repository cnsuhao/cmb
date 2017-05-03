//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkActor.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkGlyph3D.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkUnsignedCharArray.h"
#include <string>

//----------------------------------------------------------------------------
void CalcRGB(double llimit, double ulimit, double& r, double& g, double& b)
{
  double l;
  while (1)
  {
    r = vtkMath::Random(0.0, 1.0);
    g = vtkMath::Random(0.0, 1.0);
    b = vtkMath::Random(0.0, 1.0);

    l = (0.11 * b) + (0.3 * r) + (0.59 * g);
    if ((l >= llimit) && (l <= ulimit))
    {
      return;
    }
  }
}
//----------------------------------------------------------------------------
vtkIdType GetNextIndex(vtkIdType i, vtkUnsignedCharArray* avail)
{
  if (!avail->GetValue(i))
  {
    vtkIdType orig = i;
    vtkIdType n = avail->GetNumberOfTuples();
    cout << "N = " << n << "\n";
    for (; i < n; i++)
    {
      if (avail->GetValue(i))
      {
        break;
      }
    }
    if (i == n)
    {
      for (i = 0; i < orig; i++)
      {
        if (avail->GetValue(i))
        {
          break;
        }
      }
      if (!avail->GetValue(i))
      {
        return -1;
      }
    }
  }
  avail->SetValue(i, 0);
  return i;
}
//----------------------------------------------------------------------------
void CreateLookupTable(vtkLookupTable* lut, double llimit, double ulimit, int urange, double rgbD2)
{
  vtkIdType n = lut->GetNumberOfTableValues();
  vtkIdType i;
  double r, g, b;
  bool found;
  int trys;
  double currentD2 = -1.0;
  ;
  for (i = 0; i < n; i++)
  {
    found = false;
    trys = 0;
    currentD2 = rgbD2;
    while (!found)
    {
      ++trys;
      if (trys % 20)
      {
        currentD2 *= 0.5;
      }
      CalcRGB(llimit, ulimit, r, g, b);
      found = true;
      if (!urange)
      {
        break;
      }
      int j = 0;
      int start = 0;
      double d2, *color, a;
      if (i >= urange)
      {
        j = i - urange;
      }
      for (; j < i; j++)
      {
        d2 = 0.0;
        color = lut->GetTableValue(j);
        a = color[0] - r;
        d2 += a * a;
        a = color[1] - g;
        d2 += a * a;
        a = color[2] - b;
        d2 += a * a;

        if (d2 < currentD2)
        {
          found = false;
          break;
        }
      }
    }
    cout << "Color: " << i << " : ( " << r << ", " << g << ", " << b
         << "), l = " << ((0.11 * b) + (0.3 * r) + (0.59 * g)) << " RGB Distance = " << currentD2
         << "\n";
    lut->SetTableValue(i, r, g, b);
  }
}
//----------------------------------------------------------------------------
void CreatePolyDataGrid(vtkPolyData* poly, const int numColors)
{
  // we hard code how many steps to display
  double n = numColors;
  int numRow, numCol;
  numRow = numCol = vtkMath::Floor(sqrt(n));
  int leftover = numColors % numRow;

  //for points

  numRow++;
  numCol++;

  int numPts = numRow * numCol;
  numPts = leftover ? numPts + leftover + 1 : numPts;

  vtkPoints* pts = vtkPoints::New();
  pts->SetNumberOfPoints(numPts);

  int i, j, id;
  for (i = 0; i < numRow; i++)
  {
    id = i * numCol;
    for (j = 0; j < numCol; j++)
    {
      pts->InsertPoint(id + j, i * 0.3, j * 0.3, 0);
    }
  }

  int tmpNum = numRow * numCol;
  for (i = tmpNum; i < numPts; i++)
  {
    pts->InsertPoint(i, (i - tmpNum) * 0.3, numCol * 0.3, 0);
  }
  poly->SetPoints(pts);
  pts->Delete();

  // for cells

  const int numCells = numColors;
  poly->Allocate(numCells);
  int id1;
  for (i = 0; i < numRow - 1; i++)
  {
    id = i * numCol;
    id1 = (i + 1) * numCol;
    for (j = 0; j < numCol - 1; j++)
    {
      vtkIdType nodes[4];
      nodes[0] = id + j;
      nodes[1] = id + j + 1;
      nodes[2] = id1 + j + 1;
      nodes[3] = id1 + j;
      poly->InsertNextCell(VTK_QUAD, 4, nodes);
    }
  }
  tmpNum = (numRow - 1) * (numCol - 1);
  id = numRow * numCol;
  id1 = numPts - 1 - leftover;
  for (j = 0; j < leftover; j++)
  {
    vtkIdType nodes[4];
    nodes[0] = id + j;
    nodes[1] = id + j + 1;
    nodes[2] = id1 + j + 1;
    nodes[3] = id1 + j;
    poly->InsertNextCell(VTK_QUAD, 4, nodes);
  }

  vtkIntArray* shellData = vtkIntArray::New();
  shellData->SetNumberOfComponents(1);
  shellData->SetNumberOfTuples(numCells);

  for (i = 0; i < numCells; i++)
  {
    shellData->SetValue(i, i);
  }
  shellData->SetName("Colors");
  poly->GetCellData()->SetScalars(shellData);
  shellData->Delete();
}

int main(int argc, char* argv[])
{
  int numColors = 64;
  double llimit = 0.0;
  double ulimit = 1.0;
  double rgbD2 = 0.0;
  int urange = 0;
  if (argc > 1)
  {
    numColors = atoi(argv[1]);
    if (argc > 2)
    {
      llimit = strtod(argv[2], NULL);
      if (argc > 3)
      {
        ulimit = strtod(argv[3], NULL);
        if (argc > 4)
        {
          rgbD2 = strtod(argv[4], NULL);
          if (argc > 5)
          {
            urange = atoi(argv[5]);
          }
        }
      }
    }
  }

  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleSwitch> style =
    vtkSmartPointer<vtkInteractorStyleSwitch>::New();
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle(style);

  iren->SetRenderWindow(renWin);
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(numColors);
  lut->SetRange(0, numColors - 1);
  lut->Build();
  CreateLookupTable(lut, llimit, ulimit, urange, rgbD2);
  vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
  poly->Initialize();
  CreatePolyDataGrid(poly, numColors);

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInput(poly);
  mapper->SetLookupTable(lut);
  mapper->SetScalarRange(0, numColors - 1);
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  renderer->AddViewProp(actor);

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return 0;
}
