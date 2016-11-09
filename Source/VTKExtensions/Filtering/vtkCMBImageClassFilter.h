//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkCMBImageClassFilter_h
#define vtkCMBImageClassFilter_h

#include "vtkCMBFilteringModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkCMBImageClassFilter : public vtkImageAlgorithm
{
public:
  static vtkCMBImageClassFilter *New();
  vtkTypeMacro(vtkCMBImageClassFilter,vtkImageAlgorithm);

  vtkSetMacro(ForegroundValue,int);
  vtkGetMacro(ForegroundValue,int);

  vtkSetMacro(BackgroundValue,int);
  vtkGetMacro(BackgroundValue,int);

  vtkSetMacro(MinFGSize,double);
  vtkGetMacro(MinFGSize,double);

  vtkSetMacro(MinBGSize,double);
  vtkGetMacro(MinBGSize,double);

  ~vtkCMBImageClassFilter() override;

protected:

  int ForegroundValue;
  int BackgroundValue;

  double MinFGSize;
  double MinBGSize;

  vtkCMBImageClassFilter();

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

private:
  vtkCMBImageClassFilter(const vtkCMBImageClassFilter&);  // Not implemented.
  void operator=(const vtkCMBImageClassFilter&);  // Not implemented.
};

#endif
