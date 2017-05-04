//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Description: A simple program for creating VTK representations of meshes
// Generate by the Delos Mesher

#include "vtkActor.h"
#include "vtkDelosMeshReader.h"
#include "vtkDoubleArray.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataNormals.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkXMLPolyDataWriter.h"
#include <string>

#include "vtkProperty.h"

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: delosMeshReader delosMeshFile vtkFileName\n";
    return -1;
  }

  vtkNew<vtkRenderWindow> renWin;
  vtkNew<vtkRenderer> renderer;

  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyleSwitch> style;
  style->SetCurrentStyleToTrackballCamera();
  iren->SetInteractorStyle(style.GetPointer());

  iren->SetRenderWindow(renWin.GetPointer());
  renWin->AddRenderer(renderer.GetPointer());

  vtkNew<vtkDelosMeshReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(reader->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Write();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(reader->GetOutputPort(0));
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper.GetPointer());
  vtkNew<vtkProperty> property;
  property->SetDiffuseColor(1.0, 0.5, 0.2);
  property->SetEdgeColor(0.2, 0.2, 1.0);
  property->EdgeVisibilityOn();
  property->SetRepresentationToSurface();
  actor->SetProperty(property.GetPointer());
  renderer->AddViewProp(actor.GetPointer());
  iren->Initialize();
  renWin->Render();
  iren->Start();

  return 0;
}
