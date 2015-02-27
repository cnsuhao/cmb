#include "vtkKMLExporter.h"

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTesting.h"

int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();

  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-T"))
    {
    std::cerr << "Error: -T /path/to/temporary was not specified.";
    return 1;
    }

  std::string tempRoot = testHelper->GetTempDirectory();
  std::string oFileName = tempRoot + "/testKMLExporterGeometry.kml";

  vtkSmartPointer<vtkSphereSource> sps (vtkSmartPointer<vtkSphereSource>::New());
  sps->SetRadius(1.0);

  vtkSmartPointer<vtkPolyDataMapper> mpr (vtkSmartPointer<vtkPolyDataMapper>::New());
  mpr->SetInputConnection(sps->GetOutputPort());

  vtkSmartPointer<vtkActor> actr (vtkSmartPointer<vtkActor>::New());
  actr->SetMapper(mpr);

  vtkSmartPointer<vtkRenderer> ren (vtkSmartPointer<vtkRenderer>::New());
  ren->AddActor(actr);

  vtkSmartPointer<vtkRenderWindow> renWin (vtkSmartPointer<vtkRenderWindow>::New());
  renWin->AddRenderer(ren);
  renWin->SetAlphaBitPlanes(1);
  renWin->SetSize(256, 256);
  renWin->Render();

  vtkSmartPointer<vtkKMLExporter> kmlExptr (vtkSmartPointer<vtkKMLExporter>::New());
  kmlExptr->SetRenderWindow(renWin);
  kmlExptr->SetRenderScene(false);
  kmlExptr->SetRenderPolyDataAsImage(false);
  kmlExptr->SetCellCountThreshold(1000000);
  kmlExptr->SetFileName(oFileName.c_str());
  kmlExptr->Write();

  return 0;
}
