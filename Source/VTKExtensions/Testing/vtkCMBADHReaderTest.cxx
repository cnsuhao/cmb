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
