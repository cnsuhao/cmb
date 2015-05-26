//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVContourGroupInformation - Light object for holding information
// about contours in a contour group defined from Points Builder
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVContourGroupInformation_h
#define __vtkPVContourGroupInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"

class vtkDoubleArray;
class vtkIntArray;

class VTKCMBCLIENT_EXPORT vtkPVContourGroupInformation : public vtkPVInformation
{
public:
  static vtkPVContourGroupInformation* New();
  vtkTypeMacro(vtkPVContourGroupInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  virtual void AddInformation(vtkPVInformation* info);

  vtkGetObjectMacro(ProjectionPositionArray, vtkDoubleArray);
  vtkGetObjectMacro(ProjectionPlaneArray, vtkIntArray);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*){;}
  virtual void CopyFromStream(const vtkClientServerStream*){;}

  //BTX
protected:
  vtkPVContourGroupInformation();
  ~vtkPVContourGroupInformation();

  // Data information collected from remote processes.
  vtkDoubleArray* ProjectionPositionArray;
  vtkIntArray* ProjectionPlaneArray;

private:

  vtkPVContourGroupInformation(const vtkPVContourGroupInformation&); // Not implemented
  void operator=(const vtkPVContourGroupInformation&); // Not implemented
  //ETX
};

#endif
