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
#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkCMBGlyphPointSource.h"
#include "vtkGlyph3DMapper.h"
#include <string>
#include "vtkTesting.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"

//----------------------------------------------------------------------------
// Tests Glyph Point Source's individual property settings
int main(int argc, char *argv[])
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  std::string polyFilename;
  std::string dataRoot = testHelper->GetDataRoot();
  polyFilename = dataRoot + "/small_creosote.obj";

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleSwitch> style =
    vtkSmartPointer<vtkInteractorStyleSwitch>::New();
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle( style );

  iren->SetRenderWindow(renWin);
  renWin->AddRenderer( renderer );

  vtkSmartPointer<vtkCMBGlyphPointSource> points =
    vtkSmartPointer<vtkCMBGlyphPointSource>::New();
  // Lets create some points
  int i, j, n=50, vis;
  double x, y, r, g, b, a, s, zrot, delta = 1.0, nscale = 1.0 / static_cast<double>(n-1);
  double start = -0.5 * delta * n;
  double rad, RadLimit = 0.5 *n;
  vtkIdType id;
  for (j = 0, y = start; j < n; j++, y+=delta)
    {
    for (i = 0, x = start; i < n; i++, x+=delta)
      {
      if (x < 0.0)
        {
        g = 0.0;
        }
      else
        {
        g = 1.0;
        }
      r = static_cast<double>(j) * nscale;
      b = static_cast<double>(i) * nscale;
      if ((!i) && (!j))
        {
        r = b = g = 1.0;
        }
      rad = sqrt((x*x) + (y*y));
      a = 1.0 - (rad / RadLimit);
      if (a < 0.0)
        {
        vis = 0;
        }
      else
        {
        vis = 1;
        }
      if (a < 0.25)
        {
        a = 0.25;
        }
      zrot = b * 360.0;
      s = 4.0 * a;
//       points->InsertNextPoint(x, y, 0.0, r, g, b, a, s, s, s, 0.0, 0.0, zrot,
//                               vis);
      id = points->InsertNextPoint(x, y, 0.0);
      points->SetColor(id, r, g, b, 1.0);
      points->SetVisibility(id, vis);
      points->SetScale(id, s, s, s);
//       cout << "p: (" << x << "," << y <<") c: (" << r << "," << g << "," << b << "," << a
//            << ") vis: " << vis << "\n";
      }
    }
  vtkSmartPointer<vtkDiscreteLookupTable> lut =
    vtkSmartPointer<vtkDiscreteLookupTable>::New();
  lut->SetNumberOfValues(16);
  lut->Build();

  vtkSmartPointer<vtkCMBGeometryReader> reader =
    vtkSmartPointer<vtkCMBGeometryReader>::New();
  reader->SetFileName(polyFilename.c_str());
  reader->Update();

  vtkSmartPointer<vtkGlyph3DMapper> mapper =
    vtkSmartPointer<vtkGlyph3DMapper>::New();
  mapper->SetSourceConnection(reader->GetOutputPort() );
  mapper->SetInputConnection(points->GetOutputPort());
  mapper->SetMaskArray("Visibility");
  mapper->SetOrientationArray("Orientation");
  mapper->SetScaleArray("Scaling");
  mapper->SetMasking(true);
  mapper->SetOrientationModeToRotation();
  mapper->SetScaleModeToScaleByVectorComponents();

  //mapper->SetLookupTable(lut);

  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  renderer->AddViewProp( actor );

  renderer->ResetCamera();
//   double *position = renderer->GetActiveCamera()->GetPosition();
//   renderer->GetActiveCamera()->SetPosition(position[0] + 5.0,
//     position[1] + 5.0, position[2]);
  iren->Initialize();
  renWin->Render();
  cout << "Rendered first frame\n";

  int retVal = vtkTesting::FAILED;
  if (testHelper->IsFlagSpecified("-V"))
    {
    testHelper->SetRenderWindow(renWin);
    retVal = testHelper->RegressionTest(10);
    }

  if (testHelper->IsInteractiveModeSpecified())
    {
    iren->Start();
    }

  return (retVal == vtkTesting::PASSED) ? 0 : 1;
}
