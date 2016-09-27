//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBDEMExportDataExtractor - Wraps reading of dem images for pointsbuilder
// .SECTION Description

#ifndef __vtkCMBDEMExportDataExtractor_h
#define __vtkCMBDEMExportDataExtractor_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

#include <string>

class VTKCMBGENERAL_EXPORT vtkCMBDEMExportDataExtractor : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBDEMExportDataExtractor *New();
  vtkTypeMacro(vtkCMBDEMExportDataExtractor, vtkPolyDataAlgorithm);

  vtkCMBDEMExportDataExtractor();
  ~vtkCMBDEMExportDataExtractor() override;

  // Description:
  // Return proj4 spatial reference.
  const char*  GetProjectionString() const
  {
    return NULL;
  }

  vtkGetVector2Macro(Min, double);
  vtkGetVector2Macro(Max, double);
  vtkGetVector2Macro(Spacing, double);

  vtkGetMacro(Zone, int);
  vtkGetMacro(IsNorth, bool);

  vtkGetMacro(Scale, double);

protected:

  int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  int FillInputPortInformation(int port, vtkInformation *info) override;

protected:

  int Zone;
  bool IsNorth;

  double Min[2];
  double Max[2];
  double Spacing[2];
  double Scale;

private:
  vtkCMBDEMExportDataExtractor(const vtkCMBDEMExportDataExtractor&); // Not implemented.
  vtkCMBDEMExportDataExtractor& operator=(const vtkCMBDEMExportDataExtractor&); // Not implemented.
};
#endif
