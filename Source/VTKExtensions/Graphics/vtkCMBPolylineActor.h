//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPolylineActor - an actor that supports Scene Polyline
// .SECTION Description
// vtkCMBPolylineActor  is a very simple version of vtkPVLODActor.
// It supports the ability to switch on/off the GL_DEPTH_TEST.
// .SECTION see also
// vtkActor vtkPVLODActor vtkRenderer vtkLODProp3D vtkLODActor

#ifndef __vtkCMBPolylineActor_h
#define __vtkCMBPolylineActor_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPVLODActor.h"
#include "cmbSystemConfig.h"

class vtkViewport;
class vtkWindow;

class VTKCMBGRAPHICS_EXPORT vtkCMBPolylineActor : public vtkPVLODActor
{
public:
  vtkTypeMacro(vtkCMBPolylineActor,vtkPVLODActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkCMBPolylineActor *New();

  // Description:
  // This causes the actor to be rendered. It, in turn, will render the actor's
  // property and then mapper.
  // virtual void Render(vtkRenderer *, vtkMapper *);

  // Description:
  // Support the standard render methods.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderOverlay(vtkViewport *viewport);

  // Description:
  // Shallow copy of an LOD actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // When set, LODMapper, if present it used, otherwise the regular mapper is
  // used.
  vtkSetMacro(DisableGLDEPTH, int);
  vtkGetMacro(DisableGLDEPTH, int);

protected:
  vtkCMBPolylineActor();
  ~vtkCMBPolylineActor();
  int DisableGLDEPTH;

private:
  vtkCMBPolylineActor(const vtkCMBPolylineActor&); // Not implemented.
  void operator=(const vtkCMBPolylineActor&); // Not implemented.
};

#endif


