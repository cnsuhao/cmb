/*=========================================================================

  Program:   CMB
  Module:    vtkCMBInteractorStyleRubberBand.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
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
