//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkExtractLeafBlock.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkCompositeDataIterator.h"

vtkStandardNewMacro(vtkExtractLeafBlock);

//----------------------------------------------------------------------------
vtkExtractLeafBlock::vtkExtractLeafBlock()
{
  this->BlockIndex = -1;
}


//----------------------------------------------------------------------------
int vtkExtractLeafBlock::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkExtractLeafBlock::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::GetData(inputVector[0], 0);
  vtkPolyData *output = vtkPolyData::GetData(outputVector, 0);

  if (!input)
    {
    vtkErrorMacro("Input not specified!");
    return 0;
    }

  if (this->BlockIndex == -1)
    {
    vtkErrorMacro("Must specify block index to extract!");
    return 0;
    }

  // Add a call to multiblockdataset at some point, but for now, just do here
  vtkCompositeDataIterator* iter = input->NewIterator();
  // maybe we should skip empty leaves?
  //iter->SkipEmptyNodesOn();

  int numberOfLeaves = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    numberOfLeaves++;
    }

  if (numberOfLeaves <= this->BlockIndex)
    {
    iter->Delete();
    vtkErrorMacro("Specified Index is too large!");
    return 0;
    }

  // Copy selected block over to the output.
  int index = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), index++)
    {
    if (index == this->BlockIndex)
      {
      vtkPolyData *block = vtkPolyData::SafeDownCast( iter->GetCurrentDataObject() );
      if (block)
        {
        output->ShallowCopy( block );
        }
      break;
      }
    }
  iter->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractLeafBlock::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Block Index: " << this->BlockIndex << "\n";
}

