//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBImplicitPlaneRepresentation.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkBox.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkConeSource.h"
#include "vtkCutter.h"
#include "vtkFeatureEdges.h"
#include "vtkImageData.h"
#include "vtkInteractorObserver.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineFilter.h"
#include "vtkPlane.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTubeFilter.h"

vtkStandardNewMacro(vtkCMBImplicitPlaneRepresentation);

//----------------------------------------------------------------------------
vtkCMBImplicitPlaneRepresentation::vtkCMBImplicitPlaneRepresentation()
{
  this->NormalFixed = 1;
  this->ScaleEnabled = 0;
  this->Tubing = 0;
  // Hide the edges.
  this->EdgesMapper->SetInputData(0);
  this->EdgesActor->SetMapper(0);
  this->EdgesActor->SetVisibility(0);
}

//----------------------------------------------------------------------------
vtkCMBImplicitPlaneRepresentation::~vtkCMBImplicitPlaneRepresentation()
{
}

//----------------------------------------------------------------------------
int vtkCMBImplicitPlaneRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
  if (!this->NormalFixed)
  {
    return this->Superclass::ComputeInteractionState(X, Y, modify);
  }

  // See if anything has been selected
  vtkAssemblyPath* path;
  this->Picker->Pick(X, Y, 0.0, this->Renderer);
  path = this->Picker->GetPath();

  if (path == NULL) //not picking this widget
  {
    this->SetRepresentationState(vtkCMBImplicitPlaneRepresentation::Outside);
    this->InteractionState = vtkCMBImplicitPlaneRepresentation::Outside;
    return this->InteractionState;
  }

  // Something picked, continue
  this->ValidPick = 1;

  // Depending on the interaction state (set by the widget) we modify
  // this state based on what is picked.
  if (this->InteractionState == vtkCMBImplicitPlaneRepresentation::Moving)
  {
    this->InteractionState = vtkCMBImplicitPlaneRepresentation::Pushing;
    this->SetRepresentationState(vtkCMBImplicitPlaneRepresentation::Pushing);
  }
  else
  {
    this->InteractionState = vtkCMBImplicitPlaneRepresentation::Outside;
  }

  return this->InteractionState;
}

//----------------------------------------------------------------------------
void vtkCMBImplicitPlaneRepresentation::WidgetInteraction(double e[2])
{
  if (!this->NormalFixed)
  {
    return this->Superclass::WidgetInteraction(e);
  }
  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z;

  vtkCamera* camera = this->Renderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }

  // Compute the two points defining the motion vector
  double pos[3];
  this->Picker->GetPickPosition(pos);
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer, pos[0], pos[1], pos[2], focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(
    this->Renderer, this->LastEventPosition[0], this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if (this->InteractionState == vtkImplicitPlaneRepresentation::Pushing)
  {
    this->TranslateAlongNormal(prevPickPoint, pickPoint);
  }

  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkCMBImplicitPlaneRepresentation::TranslateAlongNormal(double* p1, double* p2)
{
  //Get the motion vector
  double tmpV[3];
  tmpV[0] = p2[0] - p1[0];
  tmpV[1] = p2[1] - p1[1];
  tmpV[2] = p2[2] - p1[2];

  double normal[3];
  this->GetNormal(normal);

  double v[3];
  if (!vtkMath::ProjectVector(tmpV, normal, v))
  {
    return;
  }
  //Translate the bounding box
  double* origin = this->Box->GetOrigin();
  double oNew[3];
  oNew[0] = origin[0] + v[0];
  oNew[1] = origin[1] + v[1];
  oNew[2] = origin[2] + v[2];
  this->Box->SetOrigin(oNew);

  //Translate the plane
  origin = this->Plane->GetOrigin();
  oNew[0] = origin[0] + v[0];
  oNew[1] = origin[1] + v[1];
  oNew[2] = origin[2] + v[2];
  this->Plane->SetOrigin(oNew);

  this->BuildRepresentation();
}

//----------------------------------------------------------------------------
void vtkCMBImplicitPlaneRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NormalFixed: " << this->NormalFixed << "\n";
}
