/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
#include "vtkCMBMeshReader.h"
#include "vtkGAMBITWriter.h"
#include "vtkGAMBITReader.h"
#include "vtkGeometryFilter.h"
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
