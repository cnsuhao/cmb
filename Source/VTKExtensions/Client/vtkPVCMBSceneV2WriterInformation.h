/*=========================================================================

  Program:   ParaView
  Module:    vtkPVCMBSceneV2WriterInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVCMBSceneV2WriterInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVCMBSceneV2WriterInformation_h
#define __vtkPVCMBSceneV2WriterInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
class vtkStringList;

class VTKCMBCLIENT_EXPORT vtkPVCMBSceneV2WriterInformation : public vtkPVInformation
{
public:
  static vtkPVCMBSceneV2WriterInformation* New();
  vtkTypeMacro(vtkPVCMBSceneV2WriterInformation, vtkPVInformation);
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


  const char *GetObjectFileName( int index );
  int GetNumberOfObjectFileNames();

  //BTX
protected:
  vtkPVCMBSceneV2WriterInformation();
  ~vtkPVCMBSceneV2WriterInformation();

  vtkStringList *ObjectFileNames;

private:

  vtkPVCMBSceneV2WriterInformation(const vtkPVCMBSceneV2WriterInformation&); // Not implemented
  void operator=(const vtkPVCMBSceneV2WriterInformation&); // Not implemented
  //ETX
};

#endif
