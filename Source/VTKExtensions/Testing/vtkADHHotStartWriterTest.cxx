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
#include "vtkADHHotStartWriter.h"
#include "vtkCMBADHReader.h"
#include "vtkCMBGeometryReader.h"
#include "vtkDataSetMapper.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
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

  vtkNew<vtkCMBGeometryReader> reader;
  std::string filename;
  std::string datafilename;
  std::string dataroot = testHelper->GetDataRoot();
  filename = dataroot + "/HotStart/angle_sup.3dm";
  datafilename = dataroot + "/HotStart/angle_sup.hot";

  reader->SetFileName(filename.c_str());
  reader->Update();

  vtkNew<vtkCMBADHReader> adh;
  adh->SetFileName(datafilename.c_str());
  adh->SetInputConnection(reader->GetOutputPort());
  adh->Update();

  vtkPointSet* ptset = adh->GetOutput(0);
  vtkPolyData* poly = vtkPolyData::SafeDownCast(ptset);

  vtkSmartPointer<vtkPointData> pd = poly->GetPointData();
  if (!pd)
    {
    std::cerr<<"GetPointData Failed";
    return 1;
    }
  std::string iohName = "ioh";
  std::string iovName = "iov";
  vtkNew<vtkADHHotStartWriter> writer;
  bool foundArrays = false;
  for (int i = 0; i < pd->GetNumberOfArrays(); i++)
    {
    if (pd->GetArrayName(i) == iohName ||
      pd->GetArrayName(i) == iovName)
      {
      writer->AddInputPointArrayToProcess(pd->GetArrayName(i));
      foundArrays = true;
      }
    }

  if (!foundArrays)
    {
    std::cerr<<"This Test requires input point array to be set";
    return 1;
    }

  std::string tmproot = testHelper->GetTempDirectory();

  std::string outfile = tmproot + "/test_hotstart_out.hot";
  writer->SetFileName(outfile.c_str());
  writer->SetInputData(poly);
  writer->Write();

  std::string basehotfile=dataroot + "/HotStart/base_angle_sup.hot";;
  if(vtksys::SystemTools::FilesDiffer(outfile.c_str(), basehotfile.c_str()))
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
