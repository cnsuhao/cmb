/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCMBGlyphPointSourceProxy.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMCMBGlyphPointSourceProxy - "data-centric" proxy for VTK source on a server
// .SECTION Description
// vtkSMCMBGlyphPointSourceProxy adds a CopyData method to the vtkSMSourceProxy API
// to give a "data-centric" behaviour; the output data of the input
// vtkSMSourceProxy (to CopyData) is copied by the VTK object managed
// by the vtkSMCMBGlyphPointSourceProxy.
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkSMCMBGlyphPointSourceProxy_h
#define __vtkSMCMBGlyphPointSourceProxy_h

#include "vtkSMSourceProxy.h"
#include "vtkCMBClientModule.h" // For export macro
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkSMCMBGlyphPointSourceProxy : public vtkSMSourceProxy
{
public:
  static vtkSMCMBGlyphPointSourceProxy* New();
  vtkTypeMacro(vtkSMCMBGlyphPointSourceProxy, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Insert the next point into the object
  vtkIdType InsertNextPoint(double *p);

  // Description:
  // Insert the next point and its properties into the object
  vtkIdType InsertNextPoint(double *point, double *color, double *scale,
                            double *orientation, int visibility);

  void SetScale(vtkIdType index, double *scale);
  void SetOrientation(vtkIdType index, double *orientation);
  void SetVisibility(vtkIdType index, int flag);
  void SetColor(vtkIdType index, double *color);
  void SetDefaultColor(double *color);
  void UnsetColor(vtkIdType index);
  void ResetColorsToDefault();
  void SetPoint(vtkIdType index, double *point);
  void GetPoint(vtkIdType index, double *p);
  void GetScale(vtkIdType index, double *s);
  void GetOrientation(vtkIdType index, double *o);
  int GetVisibility(vtkIdType index);
  void GetColor(vtkIdType index, double *color);
  void GetDefaultColor(double *color);
  vtkIdType GetNumberOfPoints();
  void SetGlyphSourceBounds(double bounds[6]);
  void ApplyTransform(double *orinetationDelta, double *positionDelta,
                      double *scaleDelta);
  void ApplyTransform(vtkIdType index, double *orinetationDelta,
                      double *positionDelta, double *scaleDelta);
  void GetBounds(vtkIdType index, double *bounds);
  // Load the point information from a file
  void ReadFromFile(const char *);

  // Write the point information to a file
  void WriteToFile(const char *);

//BTX
protected:
  vtkSMCMBGlyphPointSourceProxy();
  ~vtkSMCMBGlyphPointSourceProxy();
  void SendString(const char *func,
                  const char *data);
  void SendDouble3Vector(const char *func,
                        vtkIdType index, double *data);
  void ReceiveDouble3Vector(const char *func,
                        vtkIdType index, double *data);

private:
  vtkSMCMBGlyphPointSourceProxy(const vtkSMCMBGlyphPointSourceProxy&); // Not implemented
  void operator=(const vtkSMCMBGlyphPointSourceProxy&); // Not implemented
//ETX
};

#endif

