//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

#include "cmbSystemConfig.h"
#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPVImplicitPlaneRepresentation.h"

class VTKCMBGRAPHICS_EXPORT vtkCMBImplicitPlaneRepresentation
  : public vtkPVImplicitPlaneRepresentation
{
public:
  // Description:
  // Instantiate the class.
  static vtkCMBImplicitPlaneRepresentation* New();

  // Description:
  // Standard methods for the class.
  vtkTypeMacro(vtkCMBImplicitPlaneRepresentation, vtkPVImplicitPlaneRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Turn on/off the ability to scale the widget with the mouse.
  vtkSetMacro(NormalFixed, int);
  vtkGetMacro(NormalFixed, int);
  vtkBooleanMacro(NormalFixed, int);

  // Description:
  // Methods to interface with the vtkSliderWidget.
  int ComputeInteractionState(int X, int Y, int modify = 0) override;
  void WidgetInteraction(double newEventPos[2]) override;

protected:
  vtkCMBImplicitPlaneRepresentation();
  ~vtkCMBImplicitPlaneRepresentation() override;

  void TranslateAlongNormal(double* p1, double* p2);

  int NormalFixed;

private:
  vtkCMBImplicitPlaneRepresentation(const vtkCMBImplicitPlaneRepresentation&); //Not implemented
  void operator=(const vtkCMBImplicitPlaneRepresentation&);                    //Not implemented
};

#endif
