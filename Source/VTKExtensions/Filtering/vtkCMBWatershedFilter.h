//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __vtkCMBWatershedFilter_h
#define __vtkCMBWatershedFilter_h

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

  virtual int FillOutputPortInformation(int port, vtkInformation *info);

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:
  vtkCMBWatershedFilter(const vtkCMBWatershedFilter&);  // Not implemented.
  void operator=(const vtkCMBWatershedFilter&);  // Not implemented.
};

#endif