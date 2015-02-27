/*=========================================================================

  Program:   CMBSuite
  Module:    vtkCMBPolylineRepresentation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBPolylineRepresentation - representation that can be used to
// show a CmbPolyline in a render view.
// .SECTION Description
// The main purpose for the subclass is to override AddToView() to add the polyline actor
// to 2D overlay render so that the polyline will "always on top" when rendering.

#ifndef __vtkCMBPolylineRepresentation_h
#define __vtkCMBPolylineRepresentation_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkGeometryRepresentation.h"
#include "cmbSystemConfig.h"

class vtkPolyDataMapper;
class vtkCMBPolylineActor;
class vtkPVLODActor;
class vtkInformationRequestKey;
class vtkInformation;

class VTKCMBGRAPHICS_EXPORT vtkCMBPolylineRepresentation :
  public vtkGeometryRepresentation
{
public:
  static vtkCMBPolylineRepresentation* New();
  vtkTypeMacro(vtkCMBPolylineRepresentation, vtkGeometryRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
  // typically called by the vtkView to request meta-data from the
  // representations or ask them to perform certain tasks e.g.
  // PrepareForRendering.
  virtual int ProcessViewRequest(vtkInformationRequestKey* request_type,
    vtkInformation* inInfo, vtkInformation* outInfo);

  // Description:
  // Get/Set the visibility for this representation. When the visibility of
  // representation of false, all view passes are ignored.
  virtual void SetVisibility(bool val);

  //**************************************************************************
  // Forwarded to vtkCMBPolylineActor
  virtual void SetOrientation(double, double, double);
  virtual void SetOrigin(double, double, double);
  virtual void SetPickable(int val);
  virtual void SetPosition(double, double, double);
  virtual void SetScale(double, double, double);
  virtual void SetTexture(vtkTexture*);

//BTX
protected:
  vtkCMBPolylineRepresentation();
  ~vtkCMBPolylineRepresentation();

  // Description:
  // Adds the representation to the view.  This is called from
  // vtkView::AddRepresentation().  Subclasses should override this method.
  // Returns true if the addition succeeds.
  virtual bool AddToView(vtkView* view);

  // Description:
  // Removes the representation to the view.  This is called from
  // vtkView::RemoveRepresentation().  Subclasses should override this method.
  // Returns true if the removal succeeds.
  virtual bool RemoveFromView(vtkView* view);

  // Description:
  // Used in ConvertSelection to locate the prop used for actual rendering.
  virtual vtkPVLODActor* GetRenderedProp();

  // Description:
  // Passes on parameters to vtkProperty and vtkMapper
  virtual void UpdateColoringParameters();

  vtkCMBPolylineActor* PolylineActor;

private:
  vtkCMBPolylineRepresentation(const vtkCMBPolylineRepresentation&); // Not implemented
  void operator=(const vtkCMBPolylineRepresentation&); // Not implemented
//ETX
};

#endif

