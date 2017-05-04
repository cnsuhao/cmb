//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVCMBSceneV2WriterInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVCMBSceneV2WriterInformation_h
#define __vtkPVCMBSceneV2WriterInformation_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
class vtkStringList;

class VTKCMBCLIENT_EXPORT vtkPVCMBSceneV2WriterInformation : public vtkPVInformation
{
public:
  static vtkPVCMBSceneV2WriterInformation* New();
  vtkTypeMacro(vtkPVCMBSceneV2WriterInformation, vtkPVInformation);
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

  const char* GetObjectFileName(int index);
  int GetNumberOfObjectFileNames();

  //BTX
protected:
  vtkPVCMBSceneV2WriterInformation();
  ~vtkPVCMBSceneV2WriterInformation() override;

  vtkStringList* ObjectFileNames;

private:
  vtkPVCMBSceneV2WriterInformation(const vtkPVCMBSceneV2WriterInformation&); // Not implemented
  void operator=(const vtkPVCMBSceneV2WriterInformation&);                   // Not implemented
  //ETX
};

#endif
