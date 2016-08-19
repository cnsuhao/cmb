//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSmartPointer.h"
#include "vtkCMBGrabCutFilter.h"
#include "vtkTesting.h"
#include "vtkNew.h"
#include "vtkPNGReader.h"
#include "vtkGDALRasterReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPNGWriter.h"

int main(int argc, char** argv)
{
  vtkSmartPointer<vtkPNGReader> mask_reader = vtkSmartPointer<vtkPNGReader>::New();
  mask_reader->SetFileName("/Users/jacobbecker/Downloads/World_Imagery_mask.png");
  vtkSmartPointer<vtkGDALRasterReader> gdal_reader = vtkSmartPointer<vtkGDALRasterReader>::New();
  gdal_reader->SetFileName("/Users/jacobbecker/Downloads/World Imagery.tif");
  vtkSmartPointer<vtkCMBGrabCutFilter> grabCut = vtkSmartPointer<vtkCMBGrabCutFilter>::New();
  grabCut->SetInputConnection(0, gdal_reader->GetOutputPort());
  grabCut->SetInputConnection(1, mask_reader->GetOutputPort());
  grabCut->SetNumberOfIterations(24);
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName("/Users/jacobbecker/Downloads/testWorld.vtp");
  writer->SetInputConnection(grabCut->GetOutputPort(2));
  vtkSmartPointer<vtkPNGWriter> writer_label = vtkSmartPointer<vtkPNGWriter>::New();
  writer_label->SetFileName("/Users/jacobbecker/Downloads/testWorldLabel.png");
  writer_label->SetInputConnection(grabCut->GetOutputPort(0));
  writer_label->Write();
  vtkSmartPointer<vtkPNGWriter> writer_next = vtkSmartPointer<vtkPNGWriter>::New();
  writer_next->SetFileName("/Users/jacobbecker/Downloads/testWorldNext.png");
  writer_next->SetInputConnection(grabCut->GetOutputPort(1));
  writer_next->Write();
  writer->Update();

  return 0;
}
