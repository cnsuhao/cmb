//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVTrackballDolly.h"

#include "vtkCamera.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

vtkStandardNewMacro(vtkPVTrackballDolly);

//-------------------------------------------------------------------------
vtkPVTrackballDolly::vtkPVTrackballDolly()
{
  this->ZoomScale = 0.0;
}

//-------------------------------------------------------------------------
vtkPVTrackballDolly::~vtkPVTrackballDolly()
{
}

//-------------------------------------------------------------------------
void vtkPVTrackballDolly::OnButtonDown(int, int, vtkRenderer *ren,
                                      vtkRenderWindowInteractor *)
{
  int *size = ren->GetSize();
  vtkCamera *camera = ren->GetActiveCamera();
  double *range = camera->GetClippingRange();
  this->ZoomScale = 1.5 * range[1] / static_cast<double>(size[1]);
}


//-------------------------------------------------------------------------
void vtkPVTrackballDolly::OnButtonUp(int, int, vtkRenderer *,
                                    vtkRenderWindowInteractor *)
{
}

//-------------------------------------------------------------------------
void vtkPVTrackballDolly::OnMouseMove(int vtkNotUsed(x), int y,
                                     vtkRenderer *ren,
                                     vtkRenderWindowInteractor *rwi)
{
  double dy = rwi->GetLastEventPosition()[1] - y;
  vtkCamera *camera = ren->GetActiveCamera();
  double pos[3], fp[3], *norm, k, tmp;

  camera->GetPosition(pos);
  camera->GetFocalPoint(fp);
  norm = camera->GetDirectionOfProjection();
  double cameraDistance = camera->GetDistance();
  // Reversed the direction so that it is consistent with zoom in/out.
  k = (-dy) * this->ZoomScale;

  // in general we expect / pln to only use this to move along an axis
  // (in 2D mode), but go ahead and cacculate in gereral sense
  if (cameraDistance < k * 4.0)
    {
    k = cameraDistance / 4.0;
    if (k < 1e-10)
      {
      k = 0;
      }
    }

  tmp = k * norm[0];
  pos[0] += tmp;

  tmp = k * norm[1];
  pos[1] += tmp;

  tmp = k * norm[2];
  pos[2] += tmp;

  camera->SetPosition(pos);
  ren->ResetCameraClippingRange();

  rwi->Render();
}

//-------------------------------------------------------------------------
void vtkPVTrackballDolly::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ZoomScale: {" << this->ZoomScale << endl;
}






