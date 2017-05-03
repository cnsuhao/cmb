//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkMultiLayerTINStitcher.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkTINStitcher.h"
#include "vtkUnstructuredGrid.h"

#include <map>

vtkStandardNewMacro(vtkMultiLayerTINStitcher);

vtkMultiLayerTINStitcher::vtkMultiLayerTINStitcher()
{
  this->UseQuads = true;
  this->MinimumAngle = 25;
  this->AllowInteriorPointInsertion = false;
  this->Tolerance = 1e-6;
  this->UserSpecifiedTINType = 0; // auto-detect
}

vtkMultiLayerTINStitcher::~vtkMultiLayerTINStitcher()
{
}

void vtkMultiLayerTINStitcher::AddInputData(vtkPolyData* input)
{
  if (input)
  {
    this->AddInputDataInternal(0, input);
  }
}

void vtkMultiLayerTINStitcher::AddInputData(vtkUnstructuredGrid* input)
{
  if (input)
  {
    this->AddInputDataInternal(0, input);
  }
}

int vtkMultiLayerTINStitcher::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/)
{
  int numberOfInputs = this->GetNumberOfInputConnections(0);

  if (numberOfInputs < 2)
  {
    vtkErrorMacro("Two (or more) inputs are required!");
    return 0;
  }

  // make sure vtkPolyData or vtkUnstructuredGrid inputs, and also
  // sort by maximum Z value
  std::map<double, vtkPointSet*> sortedLayerMap;
  for (int i = 0; i < numberOfInputs; i++)
  {
    vtkPointSet* input = vtkPointSet::SafeDownCast(this->GetInputDataObject(0, i));
    if (!vtkPolyData::SafeDownCast(input) && !vtkUnstructuredGrid::SafeDownCast(input))
    {
      vtkErrorMacro("Inputs must be vtkPolyData or vtkUnstructuredGrid!");
      return 0;
    }

    sortedLayerMap[input->GetBounds()[5]] = input;
  }

  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(this->GetOutputDataObject(0));

  vtkNew<vtkTINStitcher> stitcher;
  stitcher->SetMinimumAngle(this->MinimumAngle);
  stitcher->SetUseQuads(this->UseQuads);
  stitcher->SetAllowInteriorPointInsertion(this->AllowInteriorPointInsertion);
  stitcher->SetTolerance(this->Tolerance);
  stitcher->SetUserSpecifiedTINType(this->UserSpecifiedTINType);

  // send each consecutive pair of layers to the TINStitcher
  std::map<double, vtkPointSet*>::iterator sortedLayerIter;
  sortedLayerIter = sortedLayerMap.begin();
  vtkPointSet* prevLayer = sortedLayerIter->second;
  for (sortedLayerIter++; sortedLayerIter != sortedLayerMap.end(); sortedLayerIter++)
  {
    stitcher->SetInputData(prevLayer);
    if (vtkUnstructuredGrid::SafeDownCast(sortedLayerIter->second))
    {
      stitcher->Set2ndInputData(vtkUnstructuredGrid::SafeDownCast(sortedLayerIter->second));
    }
    else
    {
      stitcher->Set2ndInputData(vtkPolyData::SafeDownCast(sortedLayerIter->second));
    }

    // update and set block in multiblock output
    stitcher->Update();
    vtkPolyData* tempOutput = vtkPolyData::New();
    tempOutput->ShallowCopy(stitcher->GetOutput());
    output->SetBlock(output->GetNumberOfBlocks(), tempOutput);
    tempOutput->Delete();

    prevLayer = sortedLayerIter->second;
  }

  return 1;
}

int vtkMultiLayerTINStitcher::FillInputPortInformation(int port, vtkInformation* info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
  {
    return 0;
  }
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  return 1;
}

void vtkMultiLayerTINStitcher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Use Quads: " << (this->UseQuads ? "On\n" : "Off\n");
  os << indent << "Minimum Angle: " << this->MinimumAngle;
  os << indent << "Allow Interior Point Insertion: "
     << (this->AllowInteriorPointInsertion ? "True\n" : "False\n");
  os << indent << "Tolerance: " << this->Tolerance;
  os << indent
     << "User Specified TIN Type: " << (this->UserSpecifiedTINType == 0 ? "Auto-Detect" : "Type I");
}
