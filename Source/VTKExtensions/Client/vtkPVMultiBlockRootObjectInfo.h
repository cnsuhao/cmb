//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVMultiBlockRootObjectInfo - Light object for holding information
// about the root block of a multi-block dataset.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVMultiBlockRootObjectInfo_h
#define __vtkPVMultiBlockRootObjectInfo_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include <string>

class vtkStringArray;
class vtkFloatArray;
class vtkDoubleArray;
class vtkIdList;

class VTKCMBCLIENT_EXPORT vtkPVMultiBlockRootObjectInfo : public vtkPVInformation
{
public:
  static vtkPVMultiBlockRootObjectInfo* New();
  vtkTypeMacro(vtkPVMultiBlockRootObjectInfo, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Manage a serialized version of the information.
  void CopyToStream(vtkClientServerStream*) override;
  void CopyFromStream(const vtkClientServerStream*) override;

  vtkGetObjectMacro(MaterialNames, vtkStringArray);
  vtkGetObjectMacro(ShellNames, vtkStringArray);
  vtkGetObjectMacro(BCNames, vtkStringArray);

  virtual const char* GetMaterialNameWithId(int);
  virtual const char* GetShellNameWithId(int);
  virtual const char* GetFaceNameWithId(int);
  virtual const char* GetBCNameWithId(int);

  void GetMaterialColorWithId(int, float* rgba);
  void GetShellColorWithId(int, float* rgba);
  void GetModelFaceColorWithId(int, float* rgba);
  void GetBCColorWithId(int, float* rgba);
  void GetModelFaceIds(vtkIdList*);

  int IsShellTranslationPointsLoaded();

  //BTX
protected:
  vtkPVMultiBlockRootObjectInfo();
  ~vtkPVMultiBlockRootObjectInfo() override;

  // Data information collected from remote processes.

  vtkStringArray* MaterialNames;
  vtkStringArray* ShellNames;
  vtkStringArray* FaceNames;
  vtkStringArray* BCNames;

  vtkFloatArray* ShellColors;
  vtkFloatArray* MaterialColors;
  vtkFloatArray* ModelFaceColors;
  vtkFloatArray* BCColors;
  vtkDoubleArray* ShellTranslationPoints;

  virtual const char* GetNameArrayValue(vtkStringArray* nameArray, int id);
  virtual void GetColorArrayValue(vtkFloatArray* colorArray, int id, float* rgba);

private:
  vtkPVMultiBlockRootObjectInfo(const vtkPVMultiBlockRootObjectInfo&); // Not implemented
  void operator=(const vtkPVMultiBlockRootObjectInfo&);                // Not implemented
  //ETX
};

#endif
