//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkExtractMultiBlockBlock.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkExtractMultiBlockBlock);

//----------------------------------------------------------------------------
vtkExtractMultiBlockBlock::vtkExtractMultiBlockBlock()
{
  this->BlockIndex = -1;
}

//----------------------------------------------------------------------------
int vtkExtractMultiBlockBlock::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkExtractMultiBlockBlock::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMultiBlockDataSet* input = vtkMultiBlockDataSet::GetData(inputVector[0], 0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outputVector, 0);

  if (!input)
  {
    vtkErrorMacro("Input not specified!");
    return 0;
  }

  if (this->BlockIndex < 0 || this->BlockIndex >= static_cast<int>(input->GetNumberOfBlocks()))
  {
    vtkErrorMacro("Must specify a valid block index to extract!");
    return 0;
  }

  vtkMultiBlockDataSet* block =
    vtkMultiBlockDataSet::SafeDownCast(input->GetBlock(this->BlockIndex));
  if (block)
  {
    output->ShallowCopy(block);
  }
  else
  {
    vtkErrorMacro("The specified block is not a Multi-block dataset!");
    return 0;
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractMultiBlockBlock::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Block Index: " << this->BlockIndex << "\n";
}
