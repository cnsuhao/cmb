#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkThreshold.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCMBMeshReader.h"
#include "vtkGeometryFilter.h"
#include "vtkCMBConeCellClassifier.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTesting.h"
#include "vtkSmartPointer.h"
#include "vtkCMBConeSource.h"
#include "vtkNew.h"

int main(int argc, char** argv)
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  vtkNew<vtkCMBMeshReader> reader;

  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/ConeTest/volumeMesh.3dm";

  reader->SetFileName(filename.c_str());
  reader->CreateMeshMaterialIdArrayOn();

  double baseCenter[3], axisDir[3], trans[3], ori[3], scale[3];
  baseCenter[0] = 0.0;
  baseCenter[1] = 0.0;
  baseCenter[2] = 0.0;
  axisDir[0] = 0.0;
  axisDir[1] = 0.0;
  axisDir[2] = -1.0;
  trans[0] = 12.2;
  trans[1] = 3.4;
  trans[2] = 9.8;
  ori[0] = 14.8;
  ori[1] = -4.64;
  ori[2] = 26.8;
  scale[0] = 0.86;
  scale[1] = 1.15;
  scale[2] = 0.871;

  vtkNew<vtkCMBConeCellClassifier> filter;
  filter->SetInputArrayToProcess(0, 0, 0,
                                vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                "Region");
  filter->SetBaseCenter(baseCenter);
  filter->SetHeight(1.0);
  filter->SetAxisDirection(axisDir);
  filter->SetTopRadius(0.25);
  filter->SetBaseRadius(0.5);
  filter->SetTranslation(trans);
  filter->SetOrientation(ori);
  filter->SetScaling(scale);
  filter->SetOriginalCellValue(1);
  filter->SetNewCellValue(10);
  filter->SetClassificationMode(1);
  filter->SetInputConnection(reader->GetOutputPort());
  vtkNew<vtkThreshold> tfilter;
  tfilter->ThresholdByUpper(9.5);
  tfilter->SetInputConnection(filter->GetOutputPort());
  tfilter->SetInputArrayToProcess(0, 0, 0,
                                vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                "NewIds");
  vtkNew<vtkGeometryFilter> geoFilter;
  geoFilter->SetInputConnection(tfilter->GetOutputPort());
  vtkNew<vtkPolyDataMapper> pdm;
  pdm->SetInputConnection(geoFilter->GetOutputPort());
  vtkNew<vtkActor> actor;
  actor->SetMapper(pdm.GetPointer());
  actor->GetProperty()->EdgeVisibilityOn();

  // Create the cone as a reference
  vtkNew<vtkCMBConeSource> cone;
  cone->SetHeight(1.0);
  cone->SetDirection(axisDir);
  cone->SetResolution(20);
  cone->SetBaseCenter(baseCenter);
  cone->SetTopRadius(0.25);
  cone->SetBaseRadius(0.5);
  vtkNew<vtkPolyDataMapper> pdm1;
  pdm1->SetInputConnection(cone->GetOutputPort());
  vtkNew<vtkActor> actor1;
  actor1->SetMapper(pdm1.GetPointer());
  actor1->GetProperty()->SetRepresentationToWireframe();
  actor1->GetProperty()->SetLighting(false);
  actor1->GetProperty()->SetColor(1, 1, 1);
  actor1->SetPosition(trans);
  actor1->SetOrientation(ori);
  actor1->SetScale(scale);
  // The usual rendering stuff.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  renderer->AddActor(actor.GetPointer());
  renderer->AddActor(actor1.GetPointer());
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
