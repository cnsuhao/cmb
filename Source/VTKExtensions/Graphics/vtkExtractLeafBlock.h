/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractLeafBlock.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkExtractLeafBlock - extracts a single leaf block from a multiblock dataset.
// .SECTION Description
// vtkExtractLeafBlock is simialr to vtkExtractBlock, except that only leaf
// blocks are returned and then only if vtkPolyData or easily converted to
// vtkPolyData (for now, only vtkUnstructuredGrid are converted).  Another
// difference is that only a single block can be extracted.

// .SECTION See Also
// vtkExtractBlock

#ifndef __vtkExtractLeafBlock_h
#define __vtkExtractLeafBlock_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBGRAPHICS_EXPORT vtkExtractLeafBlock : public vtkPolyDataAlgorithm
{
public:
  static vtkExtractLeafBlock* New();
  vtkTypeMacro(vtkExtractLeafBlock, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Select the block index to be extracted.  The filter will iterate through
  // the leves of the dataset until it reaches the indicated leaf block.
  vtkSetMacro(BlockIndex, int);
  vtkGetMacro(BlockIndex, int);

//BTX
protected:
  vtkExtractLeafBlock();
  ~vtkExtractLeafBlock() {};

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  /// Implementation of the algorithm.
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);


private:
  vtkExtractLeafBlock(const vtkExtractLeafBlock&); // Not implemented.
  void operator=(const vtkExtractLeafBlock&); // Not implemented.

  int BlockIndex;
//ETX
};

#endif


