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
// .NAME vtkCMBImplicitPlaneRepresentation - a class defining the cmb-specific
// representation for a vtkImplicitPlaneWidget2.
// .SECTION Description
// This class is derived from vtkImplicitPlaneRepresentation, and the difference is
// that this cmb-specific representation will limit the movement to only plane
// movement following the normal direction

// .SECTION See Also
// vtkImplicitPlaneWidget2 vtkImplicitPlaneRepresentation


#ifndef __vtkCMBImplicitPlaneRepresentation_h
#define __vtkCMBImplicitPlaneRepresentation_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPVImplicitPlaneRepresentation.h"
#include "cmbSystemConfig.h"

class VTKCMBGRAPHICS_EXPORT vtkCMBImplicitPlaneRepresentation : public vtkPVImplicitPlaneRepresentation
{
public:
  // Description:
  // Instantiate the class.
  static vtkCMBImplicitPlaneRepresentation *New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkCMBImplicitPlaneRepresentation,vtkPVImplicitPlaneRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Turn on/off the ability to scale the widget with the mouse.
  vtkSetMacro(NormalFixed,int);
  vtkGetMacro(NormalFixed,int);
  vtkBooleanMacro(NormalFixed,int);

  // Description:
  // Methods to interface with the vtkSliderWidget.
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void WidgetInteraction(double newEventPos[2]);

protected:
  vtkCMBImplicitPlaneRepresentation();
  ~vtkCMBImplicitPlaneRepresentation();

  void TranslateAlongNormal(double *p1, double *p2);

  int NormalFixed;
private:
  vtkCMBImplicitPlaneRepresentation(const vtkCMBImplicitPlaneRepresentation&);  //Not implemented
  void operator=(const vtkCMBImplicitPlaneRepresentation&);  //Not implemented
};

#endif
