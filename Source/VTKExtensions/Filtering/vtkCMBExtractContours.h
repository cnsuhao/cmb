//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBExtractContours
// .SECTION Description

#ifndef __vtkCMBExtractContours_h
#define __vtkCMBExtractContours_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
class vtkPolyData;

class VTKCMBFILTERING_EXPORT vtkCMBExtractContours : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBExtractContours* New();
  vtkTypeMacro(vtkCMBExtractContours, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(ContourIndex, vtkIdType);
  vtkGetMacro(ContourIndex, vtkIdType);

  vtkSetMacro(NumberOfContours, vtkIdType);
  vtkGetMacro(NumberOfContours, vtkIdType);

protected:
  vtkCMBExtractContours();
  ~vtkCMBExtractContours() override;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  void BuildSelectedIds(vtkPolyData* input);
  void BuildGlobalPointCollection(vtkPolyData* input);

  //this is the index of the contour we are going to read.
  vtkIdType ContourIndex;
  vtkIdType NumberOfContours;

  //Stores all the id's of selected points across all the contours
  class vtkInternalSet;
  vtkInternalSet* SelectedIds;

private:
  bool BuildGlobalPointCollectionBefore;
  vtkCMBExtractContours(const vtkCMBExtractContours&); // Not implemented.
  void operator=(const vtkCMBExtractContours&);        // Not implemented.
};

#endif
