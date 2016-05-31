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
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkPolyDataWriter.h"
#include "vtkTesting.h"
#include "smtk/extension/vtk/reader/vtkCMBMapReader.h"

int main(int argc, char** argv)
{
  vtkNew<vtkTesting> testHelper;

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }
  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/bay.map";

  vtkNew<vtkCMBMapReader> reader;
  reader->SetFileName(filename.c_str());
  reader->Update();

  vtkNew<vtkPolyDataMapper> cubeMapper;
  cubeMapper->SetInputConnection(reader->GetOutputPort());
  cubeMapper->SetScalarRange(0,3);
  vtkNew<vtkActor> cubeActor;
  cubeActor->SetMapper(cubeMapper.GetPointer());

  // The usual rendering stuff.
  vtkNew<vtkCamera> camera;
  camera->SetPosition(1,1,1);
  camera->SetFocalPoint(1973616, 523763,0);
  camera->Azimuth(-90);


  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renderer->AddActor(cubeActor.GetPointer());
  renderer->SetActiveCamera(camera.GetPointer());
  renderer->ResetCamera();
  renderer->SetBackground(.1,.1,.1);

  renWin->SetSize(600,600);
  iren->Initialize();
  renWin->Render();

  int retVal = vtkTesting::FAILED;
  if (testHelper->IsFlagSpecified("-V"))
    {
    testHelper->SetRenderWindow(renWin.GetPointer());
    retVal = testHelper->RegressionTest(10);
    }

  if (testHelper->IsInteractiveModeSpecified())
    {
    iren->Start();
    }

  return (retVal == vtkTesting::PASSED) ? 0 : 1;
}
