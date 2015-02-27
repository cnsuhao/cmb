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

