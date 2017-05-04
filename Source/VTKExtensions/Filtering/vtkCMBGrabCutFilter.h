//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkCMBGrabCutFilter_h
#define vtkCMBGrabCutFilter_h

#include "vtkCMBFilteringModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkCMBGrabCutFilter : public vtkImageAlgorithm
{
public:
  static vtkCMBGrabCutFilter* New();
  vtkTypeMacro(vtkCMBGrabCutFilter, vtkImageAlgorithm);

  vtkSetMacro(NumberOfIterations, int);
  vtkGetMacro(NumberOfIterations, int);

  vtkSetMacro(PotentialForegroundValue, int);
  vtkGetMacro(PotentialForegroundValue, int);

  vtkSetMacro(PotentialBackgroundValue, int);
  vtkGetMacro(PotentialBackgroundValue, int);

  vtkSetMacro(ForegroundValue, int);
  vtkGetMacro(ForegroundValue, int);

  vtkSetMacro(BackgroundValue, int);
  vtkGetMacro(BackgroundValue, int);

  void DoGrabCut()
  {
    RunGrabCuts = true;
    this->Modified();
  }

  ~vtkCMBGrabCutFilter() override;

protected:
  int NumberOfIterations;
  int PotentialForegroundValue;
  int PotentialBackgroundValue;
  int ForegroundValue;
  int BackgroundValue;

  vtkCMBGrabCutFilter();

  int FillOutputPortInformation(int port, vtkInformation* info) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkCMBGrabCutFilter(const vtkCMBGrabCutFilter&); // Not implemented.
  void operator=(const vtkCMBGrabCutFilter&);      // Not implemented.
  bool RunGrabCuts;
  class InternalData;
  InternalData* internal;
};

#endif
