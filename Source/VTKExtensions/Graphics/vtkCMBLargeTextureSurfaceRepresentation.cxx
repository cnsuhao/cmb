//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBLargeTextureSurfaceRepresentation.h"

#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkImageData.h"
#include "vtkImageTextureCrop.h"
#include "vtkInformation.h"
#include "vtkInformationRequestKey.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPVCacheKeeper.h"
#include "vtkPVLODActor.h"
#include "vtkPVRenderView.h"
#include "vtkPolyDataMapper.h"
#include "vtkQuadricClustering.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkCMBLargeTextureSurfaceRepresentation);
//----------------------------------------------------------------------------
vtkCMBLargeTextureSurfaceRepresentation::vtkCMBLargeTextureSurfaceRepresentation()
{
  this->SetNumberOfInputPorts(2); // 2nd input port for image for large texture

  this->LODTextureCrop = vtkImageTextureCrop::New();
  this->LODTextureCrop->SetInputConnection(this->Decimator->GetOutputPort());
  this->TextureCrop = vtkImageTextureCrop::New();
  this->TextureCrop->SetInputConnection(this->CacheKeeper->GetOutputPort());
  this->LargeTexture = 0;
}

//----------------------------------------------------------------------------
vtkCMBLargeTextureSurfaceRepresentation::~vtkCMBLargeTextureSurfaceRepresentation()
{
  this->LODTextureCrop->Delete();
  this->TextureCrop->Delete();
  if (this->LargeTexture)
  {
    this->LargeTexture->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkCMBLargeTextureSurfaceRepresentation::RemoveLargeTextureInput()
{
  this->SetInputConnection(1, 0);
}

//----------------------------------------------------------------------------
void vtkCMBLargeTextureSurfaceRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkCMBLargeTextureSurfaceRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rView = vtkPVRenderView::SafeDownCast(view);
  if (rView)
  {
    this->TextureCrop->SetRenderer(rView->GetRenderer());
  }

  return this->Superclass::AddToView(view);
}

//----------------------------------------------------------------------------
int vtkCMBLargeTextureSurfaceRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    return 0;
  }

  if (this->LargeTexture)
  {
    vtkImageData* textureInput = this->LargeTexture->GetInput();
    if (request_type == vtkPVView::REQUEST_UPDATE())
    {
      vtkPVRenderView::SetPiece(inInfo, this, this->TextureCrop->GetOutputDataObject(0));
      this->TextureCrop->Modified();
      this->TextureCrop->Update();
      textureInput->ShallowCopy(this->TextureCrop->GetOutputDataObject(1));
      textureInput->Modified();
    }
    else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
    {
      vtkPVRenderView::SetPieceLOD(inInfo, this, this->LODTextureCrop->GetOutputDataObject(0));
      this->LODTextureCrop->Update(); // should only do somethign the first time
      textureInput->ShallowCopy(this->LODTextureCrop->GetOutputDataObject(1));
      textureInput->Modified();
    }
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBLargeTextureSurfaceRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMath::UninitializeBounds(this->DataBounds);

  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects() == 1)
  {
    this->GeometryFilter->SetInputConnection(this->GetInternalOutputPort());
    this->CacheKeeper->Update();

    if (inputVector[1]->GetNumberOfInformationObjects() == 1)
    {
      this->TextureCrop->SetImageData(vtkDataSet::SafeDownCast(this->GetInput(1)));
      this->LODTextureCrop->SetImageData(vtkDataSet::SafeDownCast(this->GetInput(1)));
      if (!this->LargeTexture)
      {
        this->LargeTexture = vtkTexture::New();
        vtkImageData* textureInput = vtkImageData::New();
        this->LargeTexture->SetInputData(textureInput);
        textureInput->FastDelete();
      }
      this->Actor->SetTexture(this->LargeTexture);
    }
    else
    {
      if (this->LargeTexture)
      {
        this->LargeTexture->Delete();
        this->LargeTexture = 0;
        this->Actor->SetTexture(0);
      }
    }
  }

  // Determine data bounds.
  vtkCompositeDataSet* cd =
    vtkCompositeDataSet::SafeDownCast(this->CacheKeeper->GetOutputDataObject(0));
  if (cd)
  {
    vtkBoundingBox bbox;
    vtkCompositeDataIterator* iter = cd->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
      vtkDataSet* ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (ds)
      {
        bbox.AddBounds(ds->GetBounds());
      }
    }
    iter->Delete();
    if (bbox.IsValid())
    {
      bbox.GetBounds(this->DataBounds);
    }
  }

  return this->vtkPVDataRepresentation::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkCMBLargeTextureSurfaceRepresentation::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 0)
  {
    return this->Superclass::FillInputPortInformation(port, info);
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
  }

  return 0;
}
