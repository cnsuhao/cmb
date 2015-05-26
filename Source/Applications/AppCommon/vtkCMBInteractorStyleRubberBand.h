//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBInteractorStyleRubberBand - a CMB rubber band interactor style
// .SECTION Description
// This class overrides some methods in
// vtkInteractorStyleRubberBandPick so that selection mode can also
// be controlled programatically.
// .SECTION See Also
// vtkInteractorStyleRubberBandPick

#ifndef __vtkCMBInteractorStyleRubberBand_h
#define __vtkCMBInteractorStyleRubberBand_h

#include "cmbAppCommonExport.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "cmbSystemConfig.h"

class CMBAPPCOMMON_EXPORT vtkCMBInteractorStyleRubberBand : public vtkInteractorStyleRubberBandPick
{
public:
  static vtkCMBInteractorStyleRubberBand *New();
  vtkTypeMacro(vtkCMBInteractorStyleRubberBand, vtkInteractorStyleRubberBandPick);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Event bindings
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();

protected:
  vtkCMBInteractorStyleRubberBand();
  ~vtkCMBInteractorStyleRubberBand();

private:
  vtkCMBInteractorStyleRubberBand(const vtkCMBInteractorStyleRubberBand&);  // Not implemented
  void operator=(const vtkCMBInteractorStyleRubberBand&);  // Not implemented
};

#endif
