/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkExtractLeafBlock.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkDataSetMapper.h"
#include "vtkNew.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCMBPt123Reader.h"
#include "vtkGeometryFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTesting.h"
#include "vtkSmartPointer.h"

int main(int argc, char** argv)
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  vtkNew<vtkCMBPt123Reader> reader;

  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/TestPt123/pt_2d_s_rotation.sup";

  reader->SetFileName(filename.c_str());
  vtkNew<vtkExtractLeafBlock> exGrid, exPoints, exStreams;
  // Extract the Grid
  reader->Update();
  vtkSmartPointer<vtkUnstructuredGrid> grid =
      vtkUnstructuredGrid::SafeDownCast(reader->GetOutput()->GetBlock(0));
  vtkNew<vtkGeometryFilter> geoFilter;
  geoFilter->SetInputData(grid);
  vtkNew<vtkPolyDataMapper> pdm;
  pdm->SetInputConnection(geoFilter->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(pdm.GetPointer());

  // Extract Points
  exPoints->SetInputConnection(reader->GetOutputPort());
  exPoints->SetBlockIndex(1);
  vtkNew<vtkPolyDataMapper> pdm2;
  pdm2->SetInputConnection(exPoints->GetOutputPort());
  vtkNew<vtkActor> actor2;
  actor2->SetMapper(pdm2.GetPointer());

  // Extract Streams
  exStreams->SetInputConnection(reader->GetOutputPort());
  exStreams->SetBlockIndex(2);
  vtkNew<vtkPolyDataMapper> pdm3;
  pdm3->SetInputConnection(exStreams->GetOutputPort());
  vtkNew<vtkActor> actor3;
  actor3->SetMapper(pdm3.GetPointer());

  // The usual rendering stuff.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renderer->AddActor(actor.GetPointer());
  renderer->AddActor(actor2.GetPointer());
  renderer->AddActor(actor3.GetPointer());
  renderer->ResetCamera();
  renderer->SetBackground(.1,.1,.1);

  renWin->SetSize(600,600);
  iren->Initialize();
  renWin->Render();

  vtkSmartPointer<vtkStreamingDemandDrivenPipeline> sdd =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(reader->GetExecutive());
  static double time = 3;
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
