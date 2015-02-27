/*=========================================================================

  Program:   CMBSuite
  Module:    vtkCMBPolylineRepresentation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBPolylineRepresentation.h"

#include "vtkCMBPolylineActor.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkInformation.h"
#include "vtkInformationRequestKey.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper.h"
#include "vtkPVRenderView.h"
#include "vtkRenderer.h"
#include "vtkShadowMapBakerPass.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkCMBPolylineRepresentation);
//----------------------------------------------------------------------------
vtkCMBPolylineRepresentation::vtkCMBPolylineRepresentation()
{
  this->PolylineActor = vtkCMBPolylineActor::New();

  this->PolylineActor->SetMapper(this->Mapper);
  this->PolylineActor->SetLODMapper(this->LODMapper);
  this->PolylineActor->SetProperty(this->Property);

  ///**** Copied from vtkCMBPolylineRepresentation ****///
  // Not insanely thrilled about this API on vtkProp about properties, but oh
  // well. We have to live with it.
  vtkInformation* keys = vtkInformation::New();
  this->PolylineActor->SetPropertyKeys(keys);
  keys->Delete();
}

//----------------------------------------------------------------------------
vtkCMBPolylineRepresentation::~vtkCMBPolylineRepresentation()
{
  this->PolylineActor->Delete();
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
//----------------------------------------------------------------------------
bool vtkCMBPolylineRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->AddActor(this->PolylineActor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkCMBPolylineRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->RemoveActor(this->PolylineActor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->PolylineActor->SetVisibility(val?  1 : 0);
}
//----------------------------------------------------------------------------
vtkPVLODActor* vtkCMBPolylineRepresentation::GetRenderedProp()
{
  return this->PolylineActor;
}
//----------------------------------------------------------------------------
int vtkCMBPolylineRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if(!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
    return 0;
    }
  // TODO cjh
  if (request_type == vtkPVView::REQUEST_UPDATE())
    {
    bool lod = this->SuppressLOD? false :
      (inInfo->Has(vtkPVRenderView::USE_LOD()) == 1);
    this->PolylineActor->SetEnableLOD(lod? 1 : 0);
    if (this->PolylineActor->GetProperty()->GetOpacity() < 1.0)
      {
      outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);
      }
    }
  else if (request_type == vtkPVView::REQUEST_RENDER())
    {
    this->PolylineActor->GetMapper()->Update();
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::UpdateColoringParameters()
{
  this->Superclass::UpdateColoringParameters();
  // Update shadow map properties, in case we are using shadow maps.
  if (this->Representation == SURFACE ||
    this->Representation == SURFACE_WITH_EDGES)
    {
    // just add these keys, their values don't matter.
    this->PolylineActor->GetPropertyKeys()->Set(vtkShadowMapBakerPass::OCCLUDER(), 0);
    this->PolylineActor->GetPropertyKeys()->Set(vtkShadowMapBakerPass::RECEIVER(), 0);
    }
  else
    {
    this->PolylineActor->GetPropertyKeys()->Set(vtkShadowMapBakerPass::OCCLUDER(), 0);
    this->PolylineActor->GetPropertyKeys()->Remove(vtkShadowMapBakerPass::RECEIVER());
    }

}
//**************************************************************************
// Forwarded to vtkCMBPolylineActor
//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetOrientation(double x, double y, double z)
{
  this->PolylineActor->SetOrientation(x, y, z);
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetOrigin(double x, double y, double z)
{
  this->PolylineActor->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetPickable(int val)
{
  this->PolylineActor->SetPickable(val);
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetPosition(double x, double y, double z)
{
  this->PolylineActor->SetPosition(x, y, z);
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetScale(double x, double y, double z)
{
  this->PolylineActor->SetScale(x, y, z);
}

//----------------------------------------------------------------------------
void vtkCMBPolylineRepresentation::SetTexture(vtkTexture* val)
{
  this->PolylineActor->SetTexture(val);
}
