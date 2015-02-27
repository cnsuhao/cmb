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
// Description: A simple program for creating VTK representations of meshes
// Generate by the Delos Mesher

#include "vtkNew.h"
#include "vtkDelosMeshReader.h"
#include "vtkPolyDataNormals.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkDoubleArray.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkProperty.h"
#include <string>

#include "vtkProperty.h"

int main(int argc, char *argv[])
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
  iren->SetInteractorStyle( style.GetPointer() );

  iren->SetRenderWindow(renWin.GetPointer());
  renWin->AddRenderer( renderer.GetPointer() );

  vtkNew<vtkDelosMeshReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(reader->GetOutput());
  writer->SetFileName(argv[2]);
  writer->Write();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection( reader->GetOutputPort(0) );
  vtkNew<vtkActor> actor;
  actor->SetMapper( mapper.GetPointer() );
  vtkNew<vtkProperty> property;
  property->SetDiffuseColor(1.0, 0.5, 0.2);
  property->SetEdgeColor(0.2, 0.2, 1.0);
  property->EdgeVisibilityOn();
  property->SetRepresentationToSurface();
  actor->SetProperty(property.GetPointer());
  renderer->AddViewProp( actor.GetPointer() );
  iren->Initialize();
  renWin->Render();
  iren->Start();

  return 0;
}
