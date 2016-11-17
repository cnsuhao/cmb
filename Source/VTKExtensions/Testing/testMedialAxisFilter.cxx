//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMedialAxisFilter.h"
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkSmartPointer.h>

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    return -1;
  }
  std::cout << argv[1] << std::endl;
  std::cout << argv[2] << std::endl;
  vtkSmartPointer<vtkXMLPolyDataReader> polyReader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  vtkSmartPointer<vtkXMLImageDataReader> imgReader = vtkSmartPointer<vtkXMLImageDataReader>::New();
  vtkSmartPointer<vtkCMBMedialAxisFilter> medialFilter = vtkSmartPointer<vtkCMBMedialAxisFilter>::New();
  polyReader->SetFileName(argv[1]);
  polyReader->Update();
  imgReader->SetFileName(argv[2]);
  imgReader->Update();
  medialFilter->SetInputData(0, polyReader->GetOutput());
  medialFilter->SetInputData(1, imgReader->GetOutput());
  medialFilter->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> output = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  output->SetFileName(argv[3]);
  output->SetInputData(medialFilter->GetOutput());
  output->Update();
  
  return 0;
}
