/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
// .NAME vtkIdentifyNonManifoldPts - filter looks for pts with use by more than 2 lines
// .SECTION Description
// The filter outputs only those points (as vertices) that are used by more
// than 2 lines.  Note, the data is cleaned before analysis.

#ifndef __vtkIdentifyNonManifoldPts_h
#define __vtkIdentifyNonManifoldPts_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBFILTERING_EXPORT vtkIdentifyNonManifoldPts : public vtkPolyDataAlgorithm
{
public:
  static vtkIdentifyNonManifoldPts *New();
  vtkTypeMacro(vtkIdentifyNonManifoldPts,vtkPolyDataAlgorithm);

//BTX
protected:
  vtkIdentifyNonManifoldPts() {};
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkIdentifyNonManifoldPts(const vtkIdentifyNonManifoldPts&);  // Not implemented.
  void operator=(const vtkIdentifyNonManifoldPts&);  // Not implemented.

//ETX
};

#endif
