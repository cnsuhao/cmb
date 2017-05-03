//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMMeshSourceProxy - "data-centric" proxy for mesh source on a server
// .SECTION Description
// vtkSMMeshSourceProxy adds a MovePoints method to the vtkSMSourceProxy API
// to give a "data-centric" behavior; the output data of the input
// vtkSMSourceProxy is copied by the VTK object managed
// by the vtkSMMeshSourceProxy, and then if MovePoints is called with a moved proxy,
// the Source will be tranformed if the transform does not violate
// connectivity of mesh cells.
// .SECTION See Also
// vtkSMSourceProxy

#ifndef __vtkSMMeshSourceProxy_h
#define __vtkSMMeshSourceProxy_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMDataSourceProxy.h"

class VTKCMBCLIENT_EXPORT vtkSMMeshSourceProxy : public vtkSMDataSourceProxy
{
public:
  static vtkSMMeshSourceProxy* New();
  vtkTypeMacro(vtkSMMeshSourceProxy, vtkSMDataSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the mesh according to the transformed moved input proxy
  // Return true on success; false on failure.
  bool MovePoints(vtkSMProxy* movedProxy, vtkSMProxy* transformProxy);

  //BTX
protected:
  vtkSMMeshSourceProxy();
  ~vtkSMMeshSourceProxy() override;

private:
  vtkSMMeshSourceProxy(const vtkSMMeshSourceProxy&); // Not implemented
  void operator=(const vtkSMMeshSourceProxy&);       // Not implemented
  //ETX
};

#endif
