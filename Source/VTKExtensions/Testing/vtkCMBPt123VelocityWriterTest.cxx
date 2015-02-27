#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkExtractLeafBlock.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCMBPt123Reader.h"
#include "vtkCMBPt123VelocityWriter.h"
#include "vtkGeometryFilter.h"
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

  vtkNew<vtkCMBPt123Reader> reader;

  std::string filename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/TestPt123/pt_2d_s_rotation.sup";

  reader->SetFileName(filename.c_str());
  // Extract the Grid
  reader->Update();

  vtkSmartPointer<vtkStreamingDemandDrivenPipeline> sdd =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(reader->GetExecutive());
  static double time = 0;
  sdd->SetUpdateTimeStep(0, time);
  sdd->Update();

  // Test that the velocity is there
  vtkSmartPointer<vtkUnstructuredGrid> grid =
    vtkUnstructuredGrid::SafeDownCast(reader->GetOutput()->GetBlock(0));
  vtkSmartPointer<vtkPointData> pd = grid->GetPointData();
  std::string velocityName = "Velocity";
  if (!pd)
    {
    std::cerr<<"GetPointData Failed";
    return 1;
    }
  bool foundVelocity = false;
  for (int i = 0; i < pd->GetNumberOfArrays(); i++)
    {
    if (pd->GetArrayName(i) == velocityName)
      {
      foundVelocity = true;
      pd->SetActiveVectors(velocityName.c_str());
      }
    }

  if (!foundVelocity)
    {
    std::cerr<<"Velocity Test requires a velocity in PT123 data set";
    return 1;
    }

  std::string tmproot = testHelper->GetTempDirectory();

  std::string vnoutfile = tmproot + "/test_s_rotation_out.vn";
  vtkNew<vtkCMBPt123VelocityWriter> writer;
  writer->SetFileName(vnoutfile.c_str());
  writer->SetSpatialDimension(2);
  writer->SetWriteCellBased(false);
  writer->SetInputData(grid);
  writer->Write();

  std::string veoutfile = tmproot + "/test_s_rotation_out.ve";
  writer->SetFileName(veoutfile.c_str());
  writer->SetSpatialDimension(2);
  writer->SetWriteCellBased(true);
  writer->SetInputData(grid);
  writer->Write();

  // Read "output" velocity field back in using Pt123 reader
  // Current reading only supports nodal velocities

  // Copy files over from data directory. Would this be better done in cmake script?
  std::vector<std::string> sourceNames, destNames;

  sourceNames.push_back(dataroot + "/TestPt123/test_s_rotation.2dm");
  destNames.push_back(tmproot + "/test_s_rotation.2dm");

  sourceNames.push_back(dataroot + "/TestPt123/test_s_rotation.pt2");
  destNames.push_back(tmproot + "/test_s_rotation.pt2");

  sourceNames.push_back(dataroot + "/TestPt123/test_s_rotation.ob2");
  destNames.push_back(tmproot + "/test_s_rotation.ob2");

  sourceNames.push_back(dataroot + "/TestPt123/test_s_rotation.nemc2");
  destNames.push_back(tmproot + "/test_s_rotation.nemc2");

  sourceNames.push_back(dataroot + "/TestPt123/pt.outbinary");
  destNames.push_back(tmproot + "/pt.outbinary");

  if (sourceNames.size() != destNames.size())
    {
    std::cerr<<"Problem with source and destination file lists"<<std::endl;
    return 1;
    }
  for (size_t i = 0; i < sourceNames.size(); i++)
    {
      if(!vtksys::SystemTools::CopyFileIfDifferent(sourceNames[i].c_str(),
                                                   destNames[i].c_str()))
        {
        std::cerr<<"Copy of "<<sourceNames[i]<<" to "<<destNames[i]<<" failed "<<std::endl;
        return 1;
        }
    }
  std::string filename2 = tmproot + "/pt_2d_s_rotation_out.sup";

  ofstream supfile(filename2.c_str());
  supfile <<"2    "<<std::endl;
  supfile <<"0 0  "<<std::endl;
  supfile <<"1.0e-5 "<<std::endl;
  supfile <<"0 "<<std::endl;
  supfile <<"GEOM  test_s_rotation.2dm"<<std::endl;
  supfile <<"PTS2  test_s_rotation.pt2"<<std::endl;
  supfile <<"OBND  test_s_rotation.ob2"<<std::endl;
  supfile <<"VNAS  test_s_rotation_out.vn"<<std::endl;
  supfile <<"VNBF   forward_s_rotation.vn2"<<std::endl;
  supfile<<"VNBB   backward_s_rotation.vn2"<<std::endl;
  supfile<<"NEMA   test_s_rotation.nemc2"<<std::endl;
  supfile<<"NEMF   forward_s_rotation.nemc2"<<std::endl;
  supfile<<"NEMB   backward_s_rotation.nemc2"<<std::endl;
  supfile<<"SAPT   nebe_45a_n.out2"<<std::endl;
  supfile<<"SBPT   pt.outbinary"<<std::endl;
  supfile<<"ENDR   TEST.END"<<std::endl;
  supfile.close();

  vtkNew<vtkCMBPt123Reader> reader2;

  reader2->SetFileName(filename2.c_str());
  reader2->Update();

  vtkSmartPointer<vtkStreamingDemandDrivenPipeline> sdd2 =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(reader2->GetExecutive());
  sdd2->SetUpdateTimeStep(0, time);
  sdd2->Update();

  // Get Velocity and compare L2 norm versus original
  vtkSmartPointer<vtkUnstructuredGrid> grid2 =
    vtkUnstructuredGrid::SafeDownCast(reader2->GetOutput()->GetBlock(0));
  vtkSmartPointer<vtkPointData> pd2 = grid2->GetPointData();

  if (!pd2)
    {
    std::cerr<<"GetPointData for reread velocity Failed";
    return 1;
    }

  foundVelocity = false;
  for (int i = 0; i < pd2->GetNumberOfArrays(); i++)
    {
    if (pd2->GetArrayName(i) == velocityName)
      {
      foundVelocity = true;
      pd2->SetActiveVectors(velocityName.c_str());
      }
    }

  if (!foundVelocity)
    {
    std::cerr<<"Reread velocity not found";
    return 1;
    }

  double tol = 1.0e-4;
  int retVal = testHelper->CompareAverageOfL2Norm(pd->GetVectors(),pd2->GetVectors(),tol);

  return (retVal == vtkTesting::PASSED) ? 0 : 1;

}
