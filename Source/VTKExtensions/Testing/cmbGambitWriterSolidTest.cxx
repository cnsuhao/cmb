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
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkDoubleArray.h"
#include <string>
#include <math.h>
#include "vtkTesting.h"
#include "vtkGAMBITWriter.h"
#include "vtkGAMBITReader.h"
#include "vtkGeometryFilter.h"
#include "smtk/extension/vtk/reader/vtkCMBMeshReader.h"

//----------------------------------------------------------------------------


int main(int argc, char *argv[])
{
    vtkSmartPointer<vtkTesting> testHelper =
        vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-T"))
    {
    std::cerr << "Error: -T /path/to/test dir was not specified.";
    return 1;
    }

  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data dir was not specified.";
    return 1;
    }

  std::string tempRoot = testHelper->GetTempDirectory();
  std::string dataRoot = testHelper->GetDataRoot();
  std::string ifilename = dataRoot + "/HardpointsTest/mesh_hardpoints_test.3dm";
  std::string tfilename = tempRoot + "/gambitVolumeMesh.neu";
  vtkSmartPointer<vtkRenderWindow> renWin =
      vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkInteractorStyleSwitch> style =
      vtkSmartPointer<vtkInteractorStyleSwitch>::New();
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle( style);

  iren->SetRenderWindow(renWin);
  renWin->AddRenderer( renderer);

  vtkSmartPointer<vtkCMBMeshReader> meshReader =
      vtkSmartPointer<vtkCMBMeshReader>::New();
  meshReader->SetFileName(ifilename.c_str());
  meshReader->Update();

  // Lets convert the sphere into a GAMBIT surface mesh
  vtkSmartPointer<vtkGAMBITWriter> writer =
      vtkSmartPointer<vtkGAMBITWriter>::New();
  writer->SetFileName(tfilename.c_str());
  writer->SetInputConnection(meshReader->GetOutputPort());
  writer->Write();

  // Lets read in the result
  vtkSmartPointer<vtkGAMBITReader> reader =
      vtkSmartPointer<vtkGAMBITReader>::New();
  reader->SetFileName(tfilename.c_str());

  vtkSmartPointer<vtkGeometryFilter> filter =
      vtkSmartPointer<vtkGeometryFilter>::New();
  filter->SetInputConnection(reader->GetOutputPort());


  vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( filter->GetOutputPort() );
  vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper);
  renderer->AddViewProp( actor);

  iren->Initialize();
  renWin->Render();

  testHelper->SetRenderWindow(renWin);
  int retVal = testHelper->RegressionTest(75);

  if (testHelper->IsInteractiveModeSpecified())
    {
    iren->Start();
    }

  return (retVal == vtkTesting::PASSED) ? 0 : 1;
}
