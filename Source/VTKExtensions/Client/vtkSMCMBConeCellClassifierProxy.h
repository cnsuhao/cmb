//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMCMBConeCellClassifierProxy - proxy for VTK cone classifier filter
// .SECTION Description
// vtkSMCMBConeCellClassifierProxy - needed for the insert cone methods.
// .SECTION See Also

#ifndef __vtkSMCMBConeCellClassifierProxy_h
#define __vtkSMCMBConeCellClassifierProxy_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"

class vtkSMNewWidgetRepresentationProxy;

class VTKCMBCLIENT_EXPORT vtkSMCMBConeCellClassifierProxy : public vtkSMSourceProxy
{
public:
  static vtkSMCMBConeCellClassifierProxy* New();
  vtkTypeMacro(vtkSMCMBConeCellClassifierProxy, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Insert a cone classifier to the list of cones to be used.  The cone
  // can be truncated (with r0 being the radius at the base  and r1 being the
  // radius at the apex.  When the filter is executed cells with a value of
  // oldVal that are "within" the cone will be replaced by newVal
  void InsertNextCone(const double baseCenter[3], const double axisDir[3], double height, double r0,
    double r1, int oldVal, int newVal);

  //Description:
  // Insert a cone classifier to the list of cones to be used.  The cone
  // can be truncated (with r0 being the radius at the base and r1 being the
  // radius at p1.  When the filter is executed cells with a value of
  // oldVal that are "within" the cone will be replaced by newVal.  This method
  // allows the cone to translated, rotated, and scaled
  void InsertNextCone(const double baseCenter[3], const double axisDir[3], double height, double r0,
    double r1, const double translate[3], const double orientation[3], const double scaling[3],
    int oldVal, int newVal);

  //BTX
protected:
  vtkSMCMBConeCellClassifierProxy();
  ~vtkSMCMBConeCellClassifierProxy() override;

private:
  vtkSMCMBConeCellClassifierProxy(const vtkSMCMBConeCellClassifierProxy&); // Not implemented
  void operator=(const vtkSMCMBConeCellClassifierProxy&);                  // Not implemented
  //ETX
};

#endif
