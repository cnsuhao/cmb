//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMeshReader.h"
#include "vtkCMBADHReader.h"
#include "vtkGeometryFilter.h"
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
#include "vtkPointSet.h"
#include "vtkTesting.h"
#include "vtkStreamingDemandDrivenPipeline.h"

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
  std::string datafilename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/simple.3dm";
  datafilename = dataroot + "/simpleadh.dat";

  vtkNew<vtkCMBMeshReader> reader;
  reader->SetFileName(filename.c_str());

  vtkNew<vtkCMBADHReader> adh;
  adh->SetFileName(datafilename.c_str());
  adh->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkGeometryFilter> geomfilter;
  geomfilter->SetInputConnection(adh->GetOutputPort());

  vtkNew<vtkPolyDataMapper> adhMapper;
      adhMapper->SetScalarModeToUsePointFieldData();
      adhMapper->SelectColorArray("TestData");
      adhMapper->SetScalarRange(1,5);
      adhMapper->SetInputConnection(geomfilter->GetOutputPort());
      adhMapper->Update();

  vtkNew<vtkActor> act;
      act->SetMapper(adhMapper.GetPointer());

  // The usual rendering stuff.
  vtkNew<vtkCamera> camera;
      camera->SetPosition(1,1,1);
      camera->SetFocalPoint(8, 6,0);
      camera->Azimuth(-90);


  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renderer->AddActor(act.GetPointer());
  renderer->SetActiveCamera(camera.GetPointer());
  renderer->ResetCamera();
  renderer->SetBackground(.1,.1,.1);

  renWin->SetSize(600,600);
  iren->Initialize();
  renWin->Render();

  vtkSmartPointer<vtkStreamingDemandDrivenPipeline> sdd =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(adh->GetExecutive());
  static double time = 1;
  sdd->SetUpdateTimeStep(0, time);
  adhMapper->Modified();
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
