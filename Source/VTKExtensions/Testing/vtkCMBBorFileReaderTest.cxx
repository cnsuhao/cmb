#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCMBBorFileReader.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTesting.h"
#include "vtkSmartPointer.h"
#include "vtkNew.h"
#include <vtksys/SystemTools.hxx>

int main(int argc, char** argv)
{
  vtkNew<vtkTesting> testHelper;
  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }
  vtkNew<vtkCMBBorFileReader> reader;

  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/BoreholeGeology/Ex5-ComplexHorizons.bor";

  reader->SetFileName(filename.c_str());
  // Extract the Grid
  reader->Update();
  return vtkTesting::PASSED;
}
