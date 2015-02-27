/*=========================================================================

  Program:   ParaView
  Module:    vtkPVLASOutputBlockInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVLASOutputBlockInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVLASOutputBlockInformation_h
#define __vtkPVLASOutputBlockInformation_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>

class vtkTransform;

class VTKCMBCLIENT_EXPORT vtkPVLASOutputBlockInformation : public vtkPVInformation
{
public:
  static vtkPVLASOutputBlockInformation* New();
  vtkTypeMacro(vtkPVLASOutputBlockInformation, vtkPVInformation);
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

  vtkGetMacro(NumberOfPoints, vtkIdType);
  vtkGetMacro(NumberOfPointsInClassification, vtkIdType);
  vtkGetVector6Macro(Bounds, double);
  vtkGetMacro(Classification, unsigned char);
  const char *GetClassificationName() { return this->ClassificationName.c_str(); }

  //BTX
protected:
  vtkPVLASOutputBlockInformation();
  ~vtkPVLASOutputBlockInformation();

  // Data information collected from remote processes.
  vtkIdType      NumberOfPoints;
  vtkIdType      NumberOfPointsInClassification;
  std::string ClassificationName;
  unsigned char  Classification;
  double         Bounds[6];

private:

  vtkPVLASOutputBlockInformation(const vtkPVLASOutputBlockInformation&); // Not implemented
  void operator=(const vtkPVLASOutputBlockInformation&); // Not implemented
  //ETX
};

#endif
