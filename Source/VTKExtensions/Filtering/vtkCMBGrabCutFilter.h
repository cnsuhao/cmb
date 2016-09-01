//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __vtkCMBGrabCutFilter_h
#define __vtkCMBGrabCutFilter_h

#include "vtkCMBFilteringModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkCMBGrabCutFilter : public vtkImageAlgorithm
{
public:
  static vtkCMBGrabCutFilter *New();
  vtkTypeMacro(vtkCMBGrabCutFilter,vtkImageAlgorithm);

  vtkSetMacro(NumberOfIterations,int);
  vtkGetMacro(NumberOfIterations,int);

  vtkSetMacro(PotentialForegroundValue,int);
  vtkGetMacro(PotentialForegroundValue,int);

  vtkSetMacro(PotentialBackgroundValue,int);
  vtkGetMacro(PotentialBackgroundValue,int);

  vtkSetMacro(ForegroundValue,int);
  vtkGetMacro(ForegroundValue,int);

  vtkSetMacro(BackgroundValue,int);
  vtkGetMacro(BackgroundValue,int);

  void DoGrabCut()
  {
    RunGrabCuts = true;
    this->Modified();
  }

  virtual ~vtkCMBGrabCutFilter();

protected:

  int NumberOfIterations;
  int PotentialForegroundValue;
  int PotentialBackgroundValue;
  int ForegroundValue;
  int BackgroundValue;

  vtkCMBGrabCutFilter();

  virtual int FillOutputPortInformation(int port, vtkInformation *info);

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:
  vtkCMBGrabCutFilter(const vtkCMBGrabCutFilter&);  // Not implemented.
  void operator=(const vtkCMBGrabCutFilter&);  // Not implemented.
  bool RunGrabCuts;
  class InternalData;
  InternalData * internal;
};

#endif