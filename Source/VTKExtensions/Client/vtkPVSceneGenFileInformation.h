/*=========================================================================

  Program:   ParaView
  Module:    vtkPVSceneGenFileInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVSceneGenFileInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSceneGenFileInformation_h
#define __vtkPVSceneGenFileInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>

class vtkTransform;

class VTKCMBCLIENT_EXPORT vtkPVSceneGenFileInformation : public vtkPVInformation
{
public:
  static vtkPVSceneGenFileInformation* New();
  vtkTypeMacro(vtkPVSceneGenFileInformation, vtkPVInformation);
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

  const char *GetFileContents() { return this->FileContents.c_str(); }
  const char *GetFileName() { return this->FileName.c_str(); }

  //BTX
protected:
  vtkPVSceneGenFileInformation();
  ~vtkPVSceneGenFileInformation();

  // Data information collected from remote processes.
  std::string FileContents;
  std::string FileName;

private:

  vtkPVSceneGenFileInformation(const vtkPVSceneGenFileInformation&); // Not implemented
  void operator=(const vtkPVSceneGenFileInformation&); // Not implemented
  //ETX
};

#endif
