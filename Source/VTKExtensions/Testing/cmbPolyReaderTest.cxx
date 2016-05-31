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
#include <string>
#include "vtkTesting.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  std::string polyFilename[3];
  std::string dataRoot = testHelper->GetDataRoot();
  polyFilename[0] = dataRoot + "/cylinder_3d_circular_nohp.poly";
  polyFilename[1] = dataRoot + "/cylinder_3d_circular.poly";
  polyFilename[2] = dataRoot + "/cyl_frame.poly";

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

  vtkSmartPointer<vtkDiscreteLookupTable> lut =
    vtkSmartPointer<vtkDiscreteLookupTable>::New();
  lut->SetNumberOfValues(16);
  lut->Build();

  for (int i = 0; i < 3; i++)
    {
    vtkSmartPointer<vtkCMBGeometryReader> reader =
      vtkSmartPointer<vtkCMBGeometryReader>::New();
    reader->SetFileName(polyFilename[i].c_str());
    reader->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( reader->GetOutputPort() );
    mapper->SetLookupTable(lut);

    vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper );

    if (i == 0)
      {
      reader->GetOutput()->GetCellData()->SetActiveScalars("BoundaryMarkers");
      }
    else if (i == 1)
      {
      actor->RotateY(-25);
      actor->SetPosition(0, 1.5, -1);
      reader->GetOutput()->GetCellData()->SetActiveScalars(
        vtkMultiBlockWrapper::GetModelFaceTagName() );
      }
    else if (i == 2)
      {
      actor->RotateX(45);
      actor->SetPosition(0, -1.0, 1);
      actor->GetProperty()->SetColor(0.0, 0.0, 1.0);
      }

    renderer->AddViewProp( actor );
    }

  renderer->ResetCamera();
  double *position = renderer->GetActiveCamera()->GetPosition();
  renderer->GetActiveCamera()->SetPosition(position[0] + 5.0,
    position[1] + 5.0, position[2]);
  iren->Initialize();
  renWin->Render();

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
