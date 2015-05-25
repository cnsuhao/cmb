//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBLargeTextureSurfaceRepresentation - representation that can be used to
// show a CmbSurface in a render view.
// .SECTION Description
// The main purpose for the subclass is to override AddToView() to
// add the CmbSurface actor

#ifndef __vtkCMBLargeTextureSurfaceRepresentation_h
#define __vtkCMBLargeTextureSurfaceRepresentation_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkGeometryRepresentationWithFaces.h"
#include "cmbSystemConfig.h"

class vtkPolyDataMapper;
class vtkPVLODActor;
class vtkImageTextureCrop;
class vtkInformationRequestKey;
class vtkInformation;

class VTKCMBGRAPHICS_EXPORT vtkCMBLargeTextureSurfaceRepresentation :
  public vtkGeometryRepresentationWithFaces
{
public:
  static vtkCMBLargeTextureSurfaceRepresentation* New();
  vtkTypeMacro(vtkCMBLargeTextureSurfaceRepresentation, vtkGeometryRepresentationWithFaces);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
  // typically called by the vtkView to request meta-data from the
  // representations or ask them to perform certain tasks e.g.
  // PrepareForRendering.
  virtual int ProcessViewRequest(vtkInformationRequestKey* request_type,
    vtkInformation* inInfo, vtkInformation* outInfo);

  void RemoveLargeTextureInput();

//BTX
protected:
  vtkCMBLargeTextureSurfaceRepresentation();
  ~vtkCMBLargeTextureSurfaceRepresentation();

  virtual bool AddToView(vtkView* view);

  // Description:
  // Fill input port information.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  // Overriding to connect in the vtkImageTextureCrop filter
  virtual int RequestData(vtkInformation*,
    vtkInformationVector**, vtkInformationVector*);

  vtkImageTextureCrop *LODTextureCrop;
  vtkImageTextureCrop *TextureCrop;
  vtkTexture *LargeTexture;

private:
  vtkCMBLargeTextureSurfaceRepresentation(const vtkCMBLargeTextureSurfaceRepresentation&); // Not implemented
  void operator=(const vtkCMBLargeTextureSurfaceRepresentation&); // Not implemented
//ETX
};

#endif

