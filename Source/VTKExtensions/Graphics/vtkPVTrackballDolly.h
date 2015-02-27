/*=========================================================================

  Program:   ParaView
  Module:    vtkPVTrackballDolly.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVTrackballDolly - Zooms camera with vertical mouse movement.
// .SECTION Description
// vtkPVTrackballDolly allows the user to interactively
// manipulate the camera, the viewpoint of the scene.
// Moving the mouse down moves the camera in. Up moves it out.

#ifndef __vtkPVTrackballDolly_h
#define __vtkPVTrackballDolly_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkCameraManipulator.h"
#include "cmbSystemConfig.h"

class VTKCMBGRAPHICS_EXPORT vtkPVTrackballDolly : public vtkCameraManipulator
{
public:
  static vtkPVTrackballDolly *New();
  vtkTypeMacro(vtkPVTrackballDolly, vtkCameraManipulator);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Event bindings controlling the effects of pressing mouse buttons
  // or moving the mouse.
  virtual void OnMouseMove(int x, int y, vtkRenderer *ren,
                           vtkRenderWindowInteractor *rwi);
  virtual void OnButtonDown(int x, int y, vtkRenderer *ren,
                            vtkRenderWindowInteractor *rwi);
  virtual void OnButtonUp(int x, int y, vtkRenderer *ren,
                          vtkRenderWindowInteractor *rwi);

protected:
  vtkPVTrackballDolly();
  ~vtkPVTrackballDolly();

  double ZoomScale;

  vtkPVTrackballDolly(const vtkPVTrackballDolly&); // Not implemented
  void operator=(const vtkPVTrackballDolly&); // Not implemented
};

#endif
