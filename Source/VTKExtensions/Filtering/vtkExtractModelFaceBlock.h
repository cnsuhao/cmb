//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkExtractModelFaceBlock - extracts a single leaf block from a multiblock dataset.

// .SECTION See Also
// vtkExtractBlock

#ifndef __vtkExtractModelFaceBlock_h
#define __vtkExtractModelFaceBlock_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkExtractModelFaceBlock : public vtkPolyDataAlgorithm
{
public:
  static vtkExtractModelFaceBlock* New();
  vtkTypeMacro(vtkExtractModelFaceBlock, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Select the face id to be extracted.  The filter will iterate through
  // the leves of the dataset until it reaches the indicated leaf block.
  vtkSetMacro(FaceId, int);
  vtkGetMacro(FaceId, int);

  //BTX
protected:
  vtkExtractModelFaceBlock();
  ~vtkExtractModelFaceBlock() override{};

  int FillInputPortInformation(int port, vtkInformation* info) override;

  /// Implementation of the algorithm.
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkExtractModelFaceBlock(const vtkExtractModelFaceBlock&); // Not implemented.
  void operator=(const vtkExtractModelFaceBlock&);           // Not implemented.

  int FaceId;
  //ETX
};

#endif
