/*=========================================================================

  Program:   ParaView
  Module:    vtkPVGMSRegionArrayInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVGMSRegionArrayInformation - Light object for holding information
// about a mesh cell array.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVGMSRegionArrayInformation_h
#define __vtkPVGMSRegionArrayInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>

class vtkIntArray;

class VTKCMBCLIENT_EXPORT vtkPVGMSRegionArrayInformation : public vtkPVInformation
{
public:
  static vtkPVGMSRegionArrayInformation* New();
  vtkTypeMacro(vtkPVGMSRegionArrayInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  virtual void AddInformation(vtkPVInformation* info);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  vtkGetObjectMacro(RegionArray, vtkIntArray);

  //BTX
protected:
  vtkPVGMSRegionArrayInformation();
  ~vtkPVGMSRegionArrayInformation();

  // Data information collected from remote processes.
  vtkIntArray* RegionArray;

private:

  vtkPVGMSRegionArrayInformation(const vtkPVGMSRegionArrayInformation&); // Not implemented
  void operator=(const vtkPVGMSRegionArrayInformation&); // Not implemented
  //ETX
};

#endif
