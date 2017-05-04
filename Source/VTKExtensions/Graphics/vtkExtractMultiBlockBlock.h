//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkExtractMultiBlockBlock - extracts a multi-block block from a multiblock dataset.
// .SECTION Description
// vtkExtractMultiBlockBlock is simialr to vtkExtractBlock, except that only the multi-block
// blocks are returned.
// .SECTION See Also
// vtkExtractBlock, vtkExtractLeafBlock

#ifndef __vtkExtractMultiBlockBlock_h
#define __vtkExtractMultiBlockBlock_h

#include "cmbSystemConfig.h"
#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"

class VTKCMBGRAPHICS_EXPORT vtkExtractMultiBlockBlock : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkExtractMultiBlockBlock* New();
  vtkTypeMacro(vtkExtractMultiBlockBlock, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the block index to be extracted.
  vtkSetMacro(BlockIndex, int);
  vtkGetMacro(BlockIndex, int);

  //BTX
protected:
  vtkExtractMultiBlockBlock();
  ~vtkExtractMultiBlockBlock() override{};

  int FillInputPortInformation(int port, vtkInformation* info) override;

  /// Implementation of the algorithm.
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkExtractMultiBlockBlock(const vtkExtractMultiBlockBlock&); // Not implemented.
  void operator=(const vtkExtractMultiBlockBlock&);            // Not implemented.

  int BlockIndex;
  //ETX
};

#endif
