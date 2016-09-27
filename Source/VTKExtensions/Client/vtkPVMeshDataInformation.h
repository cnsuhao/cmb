//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  void AddInformation(vtkPVInformation* info) override;

  vtkGetObjectMacro(CellTypes, vtkCellTypes);
  vtkGetObjectMacro(RegionArray, vtkIntArray);

  // Description:
  // Manage a serialized version of the information.
  void CopyToStream(vtkClientServerStream*) override{;}
  void CopyFromStream(const vtkClientServerStream*) override{;}

  //BTX
protected:
  vtkPVMeshDataInformation();
  ~vtkPVMeshDataInformation() override;

  // Data information collected from remote processes.
  vtkCellTypes* CellTypes;
  vtkIntArray* RegionArray;

private:

  vtkPVMeshDataInformation(const vtkPVMeshDataInformation&); // Not implemented
  void operator=(const vtkPVMeshDataInformation&); // Not implemented
  //ETX
};

#endif
