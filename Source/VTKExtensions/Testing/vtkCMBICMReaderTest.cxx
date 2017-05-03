//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBICMReader.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkDataSetMapper.h"
#include "vtkGeometryFilter.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTesting.h"
#include "vtkUnstructuredGrid.h"

int main(int argc, char** argv)
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc, const_cast<const char**>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
  {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
  }

  vtkNew<vtkCMBICMReader> reader;
  reader->SetDataIsLatLong(true);
  reader->SetDataIsPositiveEast(false);

  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/allboxcorner_56920.dat";

  reader->SetFileName(filename.c_str());
  reader->Update();

  vtkNew<vtkGeometryFilter> geoFilter;
  geoFilter->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkPolyDataMapper> pdm;
  pdm->SetInputConnection(geoFilter->GetOutputPort());
  pdm->SetScalarRange(0, 23);
  vtkNew<vtkActor> act;
  act->SetMapper(pdm.GetPointer());
  act->GetProperty()->SetColor(1, 0, 0);
  act->SetScale(1, 1, .001);

  // Create the usual rendering stuff
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renderer->AddActor(act.GetPointer());
  renderer->SetBackground(.1, .1, .1);
  renWin->SetSize(600, 600);
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(-180);
  renWin->Render();

  vtkSmartPointer<vtkStreamingDemandDrivenPipeline> sdd =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(geoFilter->GetExecutive());
  double time = 190;
  reader->SetDataFileName("allboxcorner_56920-timedata");
  sdd->SetUpdateTimeStep(0, time);
  pdm->Modified();
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
