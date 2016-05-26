//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <rtvl/rtvl_refine.hxx>
#include <rtvl/rtvl_tokens.hxx>
#include <rgtl/rgtl_serialize_ostream.hxx>

#include <vcl_exception.h>
#include <vcl_fstream.h>
#include <vcl_memory.h>
#include <vcl_string.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkXMLPolyDataWriter.h>
#if defined(_WIN32)
# include <vtkWin32ProcessOutputWindow.h>
#endif

#include <vtkLIDARReader.h>

class TokenRefine
{
public:
  TokenRefine(const char* fname, const char* outname);
  void Analyze();
private:
  void Visualize(unsigned int level);
  vcl_auto_ptr< rtvl_refine<3> > Refine;
  rtvl_tokens<3> Tokens;

  vcl_string OutName;
  vtkSmartPointer<vtkTimerLog> Timer;

  vtkSmartPointer<vtkPoints> OutPoints;
  vtkSmartPointer<vtkPolyData> OutPD;
  vtkSmartPointer<vtkDoubleArray> OutNormals;
  vtkSmartPointer<vtkDoubleArray> OutBiNormals;
  vtkSmartPointer<vtkDoubleArray> OutSurfaceness;
  vtkSmartPointer<vtkDoubleArray> OutCurves;
  vtkSmartPointer<vtkDoubleArray> OutCurveness;
  vtkSmartPointer<vtkDoubleArray> OutBalls;
  vtkSmartPointer<vtkDoubleArray> OutBallness;
  vtkSmartPointer<vtkDoubleArray> OutLambda1;
  vtkSmartPointer<vtkDoubleArray> OutLambda2;
  vtkSmartPointer<vtkDoubleArray> OutLambda3;
};

TokenRefine::TokenRefine(const char* fname, const char* outname)
{
  this->OutName = outname;
  this->Timer = vtkSmartPointer<vtkTimerLog>::New();
  vtkPoints* inPoints;
  vtkAlgorithm *reader;
  std::string fileNameStr = fname;
  if (fileNameStr.find(".pts") != std::string::npos ||
    fileNameStr.find(".bin") != std::string::npos)
    {
    reader = vtkLIDARReader::New();
    vtkLIDARReader::SafeDownCast(reader)->SetFileName(fname);
    reader->Update();
    inPoints = vtkLIDARReader::SafeDownCast(reader)->GetOutput()->GetPoints();
    }
  else
    {
    reader = vtkXMLPolyDataReader::New();
    vtkXMLPolyDataReader::SafeDownCast(reader)->SetFileName(fname);
    reader->Update();
    inPoints = vtkXMLPolyDataReader::SafeDownCast(reader)->GetOutput()->GetPoints();
    }

  vcl_cout << "loaded " << inPoints->GetNumberOfPoints() << " points"
           << vcl_endl;

  vtkIdType n = inPoints->GetNumberOfPoints();
  vcl_vector<double> points(n*3);
  for(vtkIdType i=0; i < n; ++i)
    {
    inPoints->GetPoint(i, &points[i*3]);
    }
  reader->Delete();
  this->Timer->StartTimer();
  this->Refine.reset(new rtvl_refine<3>(n, &points[0]));
  this->Timer->StopTimer();

  this->OutPoints = vtkSmartPointer<vtkPoints>::New();
  this->OutNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutNormals->SetNumberOfComponents(3);
  this->OutNormals->SetName("TVSurface");
  this->OutBiNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutBiNormals->SetNumberOfComponents(3);
  this->OutBiNormals->SetName("TVBinormal");
  this->OutSurfaceness = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutSurfaceness->SetName("TVSurfaceness");
  this->OutCurves = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutCurves->SetNumberOfComponents(3);
  this->OutCurves->SetName("TVCurve");
  this->OutCurveness = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutCurveness->SetName("TVCurveness");
  this->OutBallness = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutBallness->SetName("TVBallness");
  this->OutLambda1 = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutLambda1->SetName("Lambda 1");
  this->OutLambda2 = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutLambda2->SetName("Lambda 2");
  this->OutLambda3 = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutLambda3->SetName("Lambda 3");

  this->OutPD = vtkSmartPointer<vtkPolyData>::New();
  this->OutPD->SetPoints(this->OutPoints);
  this->OutPD->GetPointData()->SetScalars(this->OutSurfaceness);
  this->OutPD->GetPointData()->SetNormals(this->OutNormals);
  this->OutPD->GetPointData()->AddArray(this->OutBiNormals);
  this->OutPD->GetPointData()->AddArray(this->OutCurves);
  this->OutPD->GetPointData()->AddArray(this->OutCurveness);
  this->OutPD->GetPointData()->AddArray(this->OutBallness);
  this->OutPD->GetPointData()->AddArray(this->OutLambda1);
  this->OutPD->GetPointData()->AddArray(this->OutLambda2);
  this->OutPD->GetPointData()->AddArray(this->OutLambda3);
}

void TokenRefine::Analyze()
{
  bool have_level = true;
  for(unsigned int level = 0; have_level; ++level)
    {
    this->Refine->get_tokens(this->Tokens);

    // Write the output file.
    vcl_string fname = this->OutName;
    {
    char buf[64];
    sprintf(buf, "_level_%02u.tvl", level);
    fname += buf;
    vcl_ofstream fout(fname.c_str(), vcl_ios::out | vcl_ios::binary);
    rgtl_serialize_ostream saver(fout);
    saver << this->Tokens;
    }
    vcl_cout << "saved scale " << this->Tokens.scale
             << " to " << fname << vcl_endl;
    vcl_cout << "  refinement time = "
             << this->Timer->GetElapsedTime() << vcl_endl;
    vcl_cout << "  output tokens   = "
             << this->Tokens.tokens.size() << vcl_endl;
    vcl_cout << "  votes cast      = "
             << this->Refine->get_vote_count() << vcl_endl;

    this->Visualize(level);

    this->Timer->StartTimer();
    have_level = this->Refine->next_scale();
    this->Timer->StopTimer();
    }
}

void TokenRefine::Visualize(unsigned int level)
{
  vtkIdType n = this->Tokens.points.get_number_of_points();
  vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();

  this->OutPoints->SetNumberOfPoints(n);
  this->OutNormals->SetNumberOfTuples(n);
  this->OutBiNormals->SetNumberOfTuples(n);
  this->OutCurves->SetNumberOfTuples(n);
  this->OutSurfaceness->SetNumberOfTuples(n);
  this->OutCurveness->SetNumberOfTuples(n);
  this->OutBallness->SetNumberOfTuples(n);
  this->OutLambda1->SetNumberOfTuples(n);
  this->OutLambda2->SetNumberOfTuples(n);
  this->OutLambda3->SetNumberOfTuples(n);

  for(vtkIdType i=0; i < n; ++i)
    {
    double p[3];
    this->Tokens.points.get_point(i, p);
    this->OutPoints->SetPoint(i, p);
    verts->InsertNextCell(1, &i);
    rtvl_tensor<3> const& tensor = this->Tokens.tokens[i];
    double surfaceness = tensor.saliency(0);
    double curveness = tensor.saliency(1);
    double ballness = tensor.saliency(2);
    double lambda1 = tensor.lambda(0);
    double lambda2 = tensor.lambda(1);
    double lambda3 = tensor.lambda(2);
    double normal[3];
    double binormal[3];
    double tangent[3];
    tensor.basis(0).copy_out(normal);
    tensor.basis(1).copy_out(binormal);
    tensor.basis(2).copy_out(tangent);

    this->OutNormals->SetTypedTuple(i, normal);
    this->OutBiNormals->SetTypedTuple(i, binormal);
    this->OutCurves->SetTypedTuple(i, tangent);
    this->OutSurfaceness->SetTypedTuple(i, &surfaceness);
    this->OutCurveness->SetTypedTuple(i, &curveness);
    this->OutBallness->SetTypedTuple(i, &ballness);
    this->OutLambda1->SetTypedTuple(i, &lambda1);
    this->OutLambda2->SetTypedTuple(i, &lambda2);
    this->OutLambda3->SetTypedTuple(i, &lambda3);
    }
  this->OutPD->SetVerts(verts);

  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  vcl_string fname = this->OutName;
  char buf[64];
  sprintf(buf, "_level_%02u.vtp", level);
  fname += buf;
  writer->SetFileName(fname.c_str());
  writer->SetInput(this->OutPD);
  writer->Write();
}

int main(int argc, const char* argv[])
{
#if defined(USE_WIN32_OUTPUT_WINDOW)
  {
  vtkSmartPointer<vtkWin32ProcessOutputWindow> ow =
    vtkSmartPointer<vtkWin32ProcessOutputWindow>::New();
  ow->SetInstance(ow);
  }
#endif
  if(argc < 3)
    {
    fprintf(stderr, "Specify input.vtp outname\n");
    return 1;
    }
  try
    {
    TokenRefine t(argv[1], argv[2]);
    t.Analyze();
    }
  catch(vcl_exception& e)
    {
    vcl_cerr << "caught exception: " << e.what() << vcl_endl;
    return 1;
    }
  catch(...)
    {
    vcl_cerr << "caught unknown exception!" << vcl_endl;
    return 1;
    }
  return 0;
}
