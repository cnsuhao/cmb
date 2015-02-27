/*=========================================================================

  Program:   ParaView
  Module:    vtkPVMeshDataInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVMeshDataInformation - Light object for holding information
// about cell types of a mesh data set
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVMeshDataInformation_h
#define __vtkPVMeshDataInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"

class vtkCellTypes;
class vtkIntArray;

class VTKCMBCLIENT_EXPORT vtkPVMeshDataInformation : public vtkPVInformation
{
public:
  static vtkPVMeshDataInformation* New();
  vtkTypeMacro(vtkPVMeshDataInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  virtual void AddInformation(vtkPVInformation* info);

  vtkGetObjectMacro(CellTypes, vtkCellTypes);
  vtkGetObjectMacro(RegionArray, vtkIntArray);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*){;}
  virtual void CopyFromStream(const vtkClientServerStream*){;}

  //BTX
protected:
  vtkPVMeshDataInformation();
  ~vtkPVMeshDataInformation();

  // Data information collected from remote processes.
  vtkCellTypes* CellTypes;
  vtkIntArray* RegionArray;

private:

  vtkPVMeshDataInformation(const vtkPVMeshDataInformation&); // Not implemented
  void operator=(const vtkPVMeshDataInformation&); // Not implemented
  //ETX
};

#endif
