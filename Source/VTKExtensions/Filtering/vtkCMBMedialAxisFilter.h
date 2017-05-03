//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBMedialAxisFilter
// .SECTION Description

#ifndef vtkCMBMedialAxisFilter_h
#define vtkCMBMedialAxisFilter_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkDataObjectAlgorithm.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"

class VTKCMBFILTERING_EXPORT vtkCMBMedialAxisFilter : public vtkDataObjectAlgorithm
{
public:
  static vtkCMBMedialAxisFilter* New();
  vtkTypeMacro(vtkCMBMedialAxisFilter, vtkDataObjectAlgorithm);

  // Description:
  // Get/Set ScaleFactor, controls the branchyness of the output
  vtkSetMacro(ScaleFactor, double);

  //BTX

protected:
  vtkCMBMedialAxisFilter();
  ~vtkCMBMedialAxisFilter() override;

  double ScaleFactor;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int FillInputPortInformation(int, vtkInformation*) override;
  int FillOutputPortInformation(int port, vtkInformation* info) override;

private:
  vtkCMBMedialAxisFilter(const vtkCMBMedialAxisFilter&); // Not implemented.
  void operator=(const vtkCMBMedialAxisFilter&);         // Not implemented.

  //ETX
};

#endif
