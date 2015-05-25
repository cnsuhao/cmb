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
#include "vtkCMBGeometryReader.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkTransform.h"
#include "vtkUnsignedCharArray.h"
#include "vtkTIFFReader.h"
#include "vtkTexture.h"
#include "vtkRegisterPlanarTextureMap.h"
#include "vtkImageData.h"
#include <string>
#include <math.h>
#include "vtkTesting.h"

//----------------------------------------------------------------------------

int FindPowerOf2(int x)
{
  double a = x;
  double b = log(a) / log(2.0);
  return pow(2.0, floor(b));
}

int main(int argc, char *argv[])
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  std::string dataRoot = testHelper->GetDataRoot();
  std::string ifilename = dataRoot + "/TextureExample1/peeledix.tif";
  std::string tfilename = dataRoot +
    "/TextureExample1/bottom_layer_4.tin";
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

  int iExtents[6];

  vtkSmartPointer<vtkTIFFReader> ireader =
    vtkSmartPointer<vtkTIFFReader>::New();
  ireader->SetFileName(ifilename.c_str());
  ireader->Update();
  ireader->GetOutput()->GetExtent(iExtents);

  std::cout << "Image extents: "
            << iExtents[0] << ", " << iExtents[1] << ", "
            << iExtents[2] << ", " << iExtents[3] << ", "
            << iExtents[4] << ", " << iExtents[5] << "\n";

  vtkSmartPointer<vtkCMBGeometryReader> reader =
    vtkSmartPointer<vtkCMBGeometryReader>::New();
  reader->SetFileName(tfilename.c_str());

  vtkSmartPointer<vtkRegisterPlanarTextureMap> filter =
    vtkSmartPointer<vtkRegisterPlanarTextureMap>::New();
  filter->SetInputConnection(reader->GetOutputPort());
  filter->SetSRange(iExtents[0], iExtents[1]);
  filter->SetTRange(iExtents[2], iExtents[3]);

  double xys[3][2], sts[3][2];
  xys[0][0] = 750000.000000; xys[0][1] = 650000.000000;
  xys[1][0] = 750000.000000; xys[1][1] = 630000.000000;
  xys[2][0] = 770000.000000; xys[2][1] = 630000.000000;

  sts[0][0] = 322; sts[0][1] = 691;
  sts[1][0] = 322; sts[1][1] = 66;
  sts[2][0] = 954; sts[2][1] = 66;

  filter->SetThreePointRegistration(xys[0], sts[0],
                                xys[1], sts[1],
                                xys[2], sts[2]);
  filter->Print(std::cout);

  vtkSmartPointer<vtkTexture> texture =
    vtkSmartPointer<vtkTexture>::New();
  texture->RestrictPowerOf2ImageSmallerOn();
  texture->SetInputConnection(ireader->GetOutputPort());

  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( filter->GetOutputPort() );
  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  actor->SetTexture(texture);
  renderer->AddViewProp( actor );

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
