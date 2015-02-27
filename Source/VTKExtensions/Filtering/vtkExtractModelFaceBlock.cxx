/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractModelFaceBlock.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkExtractModelFaceBlock.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkCompositeDataIterator.h"

vtkStandardNewMacro(vtkExtractModelFaceBlock);

//----------------------------------------------------------------------------
vtkExtractModelFaceBlock::vtkExtractModelFaceBlock()
{
  this->FaceId = -1;
}

//----------------------------------------------------------------------------
int vtkExtractModelFaceBlock::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkExtractModelFaceBlock::RequestData(
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

  vtkMultiBlockWrapper* mbw = vtkMultiBlockWrapper::New();
  mbw->SetMultiBlock(input);
  int blockIndex = mbw->GetModelFaceBlockIndex(this->FaceId);
  if( blockIndex>=0 && blockIndex < mbw->GetNumberOfModelFaces())
    {
    vtkPolyData *block = mbw->GetModelFaceWithIndex(blockIndex);
    if (block)
      {
      output->ShallowCopy( block );
      mbw->Delete();
      return 1;
      }
    }

  mbw->Delete();
  vtkErrorMacro("Specified model face id is not found!");
  return 0;

}

//----------------------------------------------------------------------------
void vtkExtractModelFaceBlock::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Face Id: " << this->FaceId << "\n";
}
