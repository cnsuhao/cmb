//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMSceneContourSourceProxy - "data-centric" proxy for VTK source on a server
// .SECTION Description
// vtkSMSceneContourSourceProxy adds a CopyData method to the vtkSMSourceProxy API
// to give a "data-centric" behaviour; the output data of the input
// vtkSMSourceProxy (to CopyData) is copied by the VTK object managed
// by the vtkSMSceneContourSourceProxy.
// .SECTION See Also
// vtkSMSourceProxy vtkSMNewWidgetRepresentationProxy


#ifndef __vtkSMSceneContourSourceProxy_h
#define __vtkSMSceneContourSourceProxy_h

#include "vtkSMSourceProxy.h"
#include "vtkCMBClientModule.h" // For export macro
#include "cmbSystemConfig.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkSMSceneContourSourceProxy : public vtkSMSourceProxy
{
public:
  static vtkSMSceneContourSourceProxy* New();
  vtkTypeMacro(vtkSMSceneContourSourceProxy, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Copies data from a widget proxy to object represented by this
  // source proxy object.
  void CopyData(vtkSMNewWidgetRepresentationProxy *widgetProxy);

  // Description:
  // Copies the data this proxy output to the input of the widget
  void EditData(vtkSMNewWidgetRepresentationProxy *widgetProxy, bool &closed);

  // Dexcription:
  //copies the data from a source proxy using the GetOutput method
  void ExtractContour(vtkSMSourceProxy *sourceProxy);

//BTX
protected:
  vtkSMSceneContourSourceProxy();
  ~vtkSMSceneContourSourceProxy();

private:
  vtkSMSceneContourSourceProxy(const vtkSMSceneContourSourceProxy&); // Not implemented
  void operator=(const vtkSMSceneContourSourceProxy&); // Not implemented
//ETX
};

#endif

