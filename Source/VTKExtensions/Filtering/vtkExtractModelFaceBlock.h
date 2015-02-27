/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractModelFaceBlock.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkExtractModelFaceBlock - extracts a single leaf block from a multiblock dataset.

// .SECTION See Also
// vtkExtractBlock

#ifndef __vtkExtractModelFaceBlock_h
#define __vtkExtractModelFaceBlock_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBFILTERING_EXPORT vtkExtractModelFaceBlock : public vtkPolyDataAlgorithm
{
public:
  static vtkExtractModelFaceBlock* New();
  vtkTypeMacro(vtkExtractModelFaceBlock, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Select the face id to be extracted.  The filter will iterate through
  // the leves of the dataset until it reaches the indicated leaf block.
  vtkSetMacro(FaceId, int);
  vtkGetMacro(FaceId, int);

//BTX
protected:
  vtkExtractModelFaceBlock();
  ~vtkExtractModelFaceBlock() {};

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  /// Implementation of the algorithm.
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);


private:
  vtkExtractModelFaceBlock(const vtkExtractModelFaceBlock&); // Not implemented.
  void operator=(const vtkExtractModelFaceBlock&); // Not implemented.

  int FaceId;
//ETX
};

#endif
