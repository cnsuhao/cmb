//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVModelVertexObjectInformation - Light object for holding information
// about a model vertex object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVModelVertexObjectInformation_h
#define __vtkPVModelVertexObjectInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>

class VTKCMBCLIENT_EXPORT vtkPVModelVertexObjectInformation : public vtkPVInformation
{
public:
  static vtkPVModelVertexObjectInformation* New();
  vtkTypeMacro(vtkPVModelVertexObjectInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  void AddInformation(vtkPVInformation* info) override;

  // Description:
  // Manage a serialized version of the information.
  void CopyToStream(vtkClientServerStream*) override;
  void CopyFromStream(const vtkClientServerStream*) override;

  vtkGetVector3Macro(Location, double);
  vtkGetMacro(IsInfoValid, int);
  vtkGetMacro(PointId, vtkIdType);
  //BTX
protected:
  vtkPVModelVertexObjectInformation();
  ~vtkPVModelVertexObjectInformation() override;

  // Data information collected from remote processes.
  double         Location[3];
  int IsInfoValid;
  vtkIdType PointId;

private:
  vtkPVModelVertexObjectInformation(const vtkPVModelVertexObjectInformation&); // Not implemented
  void operator=(const vtkPVModelVertexObjectInformation&); // Not implemented
  //ETX
};

#endif
