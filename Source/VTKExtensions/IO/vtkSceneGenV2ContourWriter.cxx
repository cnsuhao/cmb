//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSceneGenV2ContourWriter.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataWriter.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkSceneGenV2ContourWriter);

//-----------------------------------------------------------------------------
vtkSceneGenV2ContourWriter::vtkSceneGenV2ContourWriter()
{
  this->SetNumberOfInputPorts(1);
  this->FileName = NULL;
}

//-----------------------------------------------------------------------------
vtkSceneGenV2ContourWriter::~vtkSceneGenV2ContourWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
int vtkSceneGenV2ContourWriter::FillInputPortInformation(int /*port*/, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkSceneGenV2ContourWriter::RequestData(vtkInformation* /*request*/,
  vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/)
{
  int numOfContours = this->GetNumberOfInputConnections(0);

  vtkAppendPolyData* append = vtkAppendPolyData::New();
  for (int i = 0; i < numOfContours; ++i)
  {
    append->AddInputConnection(this->GetInputConnection(0, i));
  }

  //now that we have a single dataset, lets clean it to remove unused points
  vtkCleanPolyData* clean = vtkCleanPolyData::New();
  clean->SetInputConnection(append->GetOutputPort());
  clean->PointMergingOff();
  clean->ConvertLinesToPointsOff();
  clean->ConvertPolysToLinesOff();
  clean->ConvertStripsToPolysOff();

  //now write out the file
  vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
  writer->SetInputConnection(clean->GetOutputPort());
  writer->SetFileName(this->FileName);
  writer->SetDataModeToBinary();
  writer->Update();

  writer->Delete();
  clean->Delete();
  append->Delete();
  return 1;
}
//----------------------------------------------------------------------------
void vtkSceneGenV2ContourWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}
