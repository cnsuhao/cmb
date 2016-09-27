//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkCMBWatershedFilter_h
#define vtkCMBWatershedFilter_h

#include "vtkCMBFilteringModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkCMBWatershedFilter : public vtkImageAlgorithm
{
public:
  static vtkCMBWatershedFilter *New();
  vtkTypeMacro(vtkCMBWatershedFilter,vtkImageAlgorithm);

  vtkSetMacro(ForegroundValue,int);
  vtkGetMacro(ForegroundValue,int);

  vtkSetMacro(BackgroundValue,int);
  vtkGetMacro(BackgroundValue,int);

  vtkSetMacro(UnlabeledValue,int);
  vtkGetMacro(UnlabeledValue,int);

protected:

  int ForegroundValue;
  int BackgroundValue;
  int UnlabeledValue;

  vtkCMBWatershedFilter();

  int FillOutputPortInformation(int port, vtkInformation *info) override;

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

private:
  vtkCMBWatershedFilter(const vtkCMBWatershedFilter&);  // Not implemented.
  void operator=(const vtkCMBWatershedFilter&);  // Not implemented.
};

#endif