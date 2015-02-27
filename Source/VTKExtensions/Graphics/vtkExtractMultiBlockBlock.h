/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractMultiBlockBlock.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkExtractMultiBlockBlock - extracts a multi-block block from a multiblock dataset.
// .SECTION Description
// vtkExtractMultiBlockBlock is simialr to vtkExtractBlock, except that only the multi-block
// blocks are returned.
// .SECTION See Also
// vtkExtractBlock, vtkExtractLeafBlock

#ifndef __vtkExtractMultiBlockBlock_h
#define __vtkExtractMultiBlockBlock_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBGRAPHICS_EXPORT vtkExtractMultiBlockBlock : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkExtractMultiBlockBlock* New();
  vtkTypeMacro(vtkExtractMultiBlockBlock, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the block index to be extracted.
  vtkSetMacro(BlockIndex, int);
  vtkGetMacro(BlockIndex, int);

//BTX
protected:
  vtkExtractMultiBlockBlock();
  ~vtkExtractMultiBlockBlock() {};

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  /// Implementation of the algorithm.
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);


private:
  vtkExtractMultiBlockBlock(const vtkExtractMultiBlockBlock&); // Not implemented.
  void operator=(const vtkExtractMultiBlockBlock&); // Not implemented.

  int BlockIndex;
//ETX
};

#endif


