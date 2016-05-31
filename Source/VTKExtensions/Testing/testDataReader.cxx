//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPolyDataNormals.h"
#include "vtkOBJReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include <vtkNew.h>
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkGlyph3D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkOSDLReader.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkExtractLeafBlock.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkTransform.h"
#include <string>

#include "vtkElevationFilter.h"
#include "vtkProperty.h"
#include "smtk/extension/vtk/reader/vtkCUBITReader.h"
#include "smtk/extension/vtk/reader/vtkLASReader.h"
#include "smtk/extension/vtk/reader/vtkLIDARReader.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
    {
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

  std::string inFName = argv[1];
  std::string spts = ".pts";
  std::string sobj = ".obj";
  std::string sosd = ".osd.txt";
  std::string slas = ".las";
  std::string sfac = ".fac";

  if (inFName.find(spts) != std::string::npos)
    {
    vtkNew<vtkLIDARReader> reader;
    reader->SetFileName( argv[1] );
    reader->ConvertFromLatLongToXYZOn();
    reader->Update();

    double *bounds = reader->GetDataBounds();
    vtkNew<vtkElevationFilter> filter;
    filter->SetInputConnection( reader->GetOutputPort() );
    double minPt[3] = {0, 0, bounds[4]}, maxPt[3] = {0, 0, bounds[5]};
    filter->SetLowPoint( minPt );
    filter->SetHighPoint( maxPt );

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection( filter->GetOutputPort(0) );
    vtkNew<vtkActor> actor;
    actor->SetMapper( mapper.GetPointer() );

    renderer->AddViewProp( actor.GetPointer() );
    }
  else if (inFName.find(slas) != std::string::npos)
    {
    double color[4][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
    vtkNew<vtkLASReader> reader;
    reader->SetFileName( argv[1] );
    //reader->ConvertFromLatLongToXYZOn();
    reader->Update();

    vtkMultiBlockDataSet *output = reader->GetOutput();
    for (unsigned int i = 0; i < output->GetNumberOfBlocks(); i++)
      {
      vtkNew<vtkPolyDataMapper> mapper;
      mapper->SetInputData( vtkPolyData::SafeDownCast(output->GetBlock(i)) );
      vtkNew<vtkActor> actor;
      actor->SetMapper( mapper.GetPointer() );
      actor->GetProperty()->SetColor( color[i%4] );
      renderer->AddViewProp( actor.GetPointer() );
      }
    }
  else if (inFName.find(sfac) != std::string::npos)
    {
    vtkNew<vtkCUBITReader> reader;
    reader->SetFileName( argv[1] );
    reader->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection( reader->GetOutputPort(0) );
    vtkNew<vtkActor> actor;
    actor->SetMapper( mapper.GetPointer() );

    vtkNew<vtkProperty> backface;
    backface->SetColor(1,0,0);
    actor->SetBackfaceProperty(backface.GetPointer());

    renderer->AddViewProp( actor.GetPointer() );
    }
  else if (inFName.find(sobj) != std::string::npos)
    {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName( argv[1] );

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection( reader->GetOutputPort(0) );
    vtkNew<vtkActor> actor;
    actor->SetMapper( mapper.GetPointer() );

    renderer->AddViewProp( actor.GetPointer() );
    }
  else if (inFName.find(sosd) != std::string::npos)
    {
    vtkNew<vtkOSDLReader> reader;
    reader->SetFileName( argv[1] );
    reader->Update();

    int numBlocks = reader->GetOutput()->GetNumberOfBlocks();

    // get the number of blocks in the dataset
    for(int i = 0; i < numBlocks; i++)
      {
      vtkNew<vtkExtractLeafBlock> extract;
      extract->SetInputConnection( reader->GetOutputPort() );
      extract->SetBlockIndex(i);
      extract->Update();
      vtkPolyData *pD = extract->GetOutput();
      vtkDoubleArray *transformation = vtkDoubleArray::SafeDownCast(
        pD->GetFieldData()->GetArray( "Transformation" ) );

      double elements[16];
      transformation->GetTuple(0, elements);
      vtkNew<vtkTransform> transform;
      transform->SetMatrix( elements );

      vtkNew<vtkPolyDataMapper> mapper;
      mapper->SetInputConnection( extract->GetOutputPort(0) );
      vtkNew<vtkActor> actor;
      actor->SetMapper( mapper.GetPointer() );
      actor->SetUserTransform( transform.GetPointer() );

      renderer->AddViewProp( actor.GetPointer() );
      }
    }
  else
    {
    cerr << "Unrecognized extension\n";
    return -1;
    }


  iren->Initialize();
  renWin->Render();
  iren->Start();

  return 0;
}
