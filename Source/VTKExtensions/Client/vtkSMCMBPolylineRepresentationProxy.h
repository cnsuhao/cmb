//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMCMBPolylineRepresentationProxy - representation that can be used to
// show a CmbPolyline in a render view.
// .SECTION Description
// The main purpose for the subclass is to override AddToView() to add the polyline actor
// to 2D overlay render so that the polyline will "always on top" when rendering.

#ifndef __vtkSMCMBPolylineRepresentationProxy_h
#define __vtkSMCMBPolylineRepresentationProxy_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMRepresentationProxy.h"
#include "cmbSystemConfig.h"

class VTKCMBCLIENT_EXPORT vtkSMCMBPolylineRepresentationProxy :
  public vtkSMRepresentationProxy
{
public:
  static vtkSMCMBPolylineRepresentationProxy* New();
  vtkTypeMacro(vtkSMCMBPolylineRepresentationProxy, vtkSMRepresentationProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the scalar coloring mode
  void SetColorAttributeType(int type);

  // Description:
  // Set the scalar color array name. If array name is 0 or "" then scalar
  // coloring is disabled.
  void SetColorArrayName(const char* name);

  // Description:
  // Check if this representation has the prop by checking its vtkClientServerID
  virtual bool HasVisibleProp3D(vtkProp3D* prop);

  // Description:
  // Views typically support a mechanism to create a selection in the view
  // itself, e.g. by click-and-dragging over a region in the view. The view
  // passes this selection to each of the representations and asks them to
  // convert it to a proxy for a selection which can be set on the view.
  // Its representation does not support selection creation, it should simply
  // return NULL. This method returns a new vtkSMProxy instance which the
  // caller must free after use.
  // This implementation converts a prop selection to a selection source.
  virtual vtkSMProxy* ConvertSelection(vtkSelection* input);

  // Description:
  // Get the bounds and transform according to rotation, translation, and scaling.
  // Returns true if the bounds are "valid" (and false otherwise)
//  virtual bool GetBounds(double bounds[6]);

//BTX
protected:
  vtkSMCMBPolylineRepresentationProxy();
  ~vtkSMCMBPolylineRepresentationProxy();

  // Description:
  // This representation needs a surface compositing strategy.
  // Overridden to request the correct type of strategy from the view.
  virtual bool InitializeStrategy(vtkSMViewProxy* view);

    // Description:
  // This method is called at the beginning of CreateVTKObjects().
  // This gives the subclasses an opportunity to set the servers flags
  // on the subproxies.
  // If this method returns false, CreateVTKObjects() is aborted.
  virtual bool BeginCreateVTKObjects();

  // Description:
  // This method is called after CreateVTKObjects().
  // This gives subclasses an opportunity to do some post-creation
  // initialization.
  virtual bool EndCreateVTKObjects();

  // Description:
  // Called when a representation is added to a view.
  // Returns true on success.
  // Added to call InitializeStrategy() to give subclassess the opportunity to
  // set up pipelines involving compositing strategy it they support it.
  virtual bool AddToView(vtkSMViewProxy* view);

  // Description:
  // Called to remove a representation from a view.
  // Returns true on success.
  // Currently a representation can be added to only one view.
  virtual bool RemoveFromView(vtkSMViewProxy* view);

  vtkSMProxy* Mapper;
  vtkSMProxy* Prop3D;
  vtkSMProxy* Property;

private:
  vtkSMCMBPolylineRepresentationProxy(const vtkSMCMBPolylineRepresentationProxy&); // Not implemented
  void operator=(const vtkSMCMBPolylineRepresentationProxy&); // Not implemented
//ETX
};

#endif

