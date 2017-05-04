//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVSceneGenFileInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSceneGenFileInformation_h
#define __vtkPVSceneGenFileInformation_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include <string>

class vtkTransform;

class VTKCMBCLIENT_EXPORT vtkPVSceneGenFileInformation : public vtkPVInformation
{
public:
  static vtkPVSceneGenFileInformation* New();
  vtkTypeMacro(vtkPVSceneGenFileInformation, vtkPVInformation);
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

  const char* GetFileContents() { return this->FileContents.c_str(); }
  const char* GetFileName() { return this->FileName.c_str(); }

  //BTX
protected:
  vtkPVSceneGenFileInformation();
  ~vtkPVSceneGenFileInformation() override;

  // Data information collected from remote processes.
  std::string FileContents;
  std::string FileName;

private:
  vtkPVSceneGenFileInformation(const vtkPVSceneGenFileInformation&); // Not implemented
  void operator=(const vtkPVSceneGenFileInformation&);               // Not implemented
  //ETX
};

#endif
