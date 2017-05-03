//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkIdentifyNonManifoldPts - filter looks for pts with use by more than 2 lines
// .SECTION Description
// The filter outputs only those points (as vertices) that are used by more
// than 2 lines.  Note, the data is cleaned before analysis.

#ifndef __vtkIdentifyNonManifoldPts_h
#define __vtkIdentifyNonManifoldPts_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkIdentifyNonManifoldPts : public vtkPolyDataAlgorithm
{
public:
  static vtkIdentifyNonManifoldPts* New();
  vtkTypeMacro(vtkIdentifyNonManifoldPts, vtkPolyDataAlgorithm);

  //BTX
protected:
  vtkIdentifyNonManifoldPts(){};
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkIdentifyNonManifoldPts(const vtkIdentifyNonManifoldPts&); // Not implemented.
  void operator=(const vtkIdentifyNonManifoldPts&);            // Not implemented.

  //ETX
};

#endif
