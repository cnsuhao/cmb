//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVLASOutputBlockInformation - Light object for holding information
// about a SceneGen object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVLASOutputBlockInformation_h
#define __vtkPVLASOutputBlockInformation_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include <string>

class vtkTransform;

class VTKCMBCLIENT_EXPORT vtkPVLASOutputBlockInformation : public vtkPVInformation
{
public:
  static vtkPVLASOutputBlockInformation* New();
  vtkTypeMacro(vtkPVLASOutputBlockInformation, vtkPVInformation);
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

  vtkGetMacro(NumberOfPoints, vtkIdType);
  vtkGetMacro(NumberOfPointsInClassification, vtkIdType);
  vtkGetVector6Macro(Bounds, double);
  vtkGetMacro(Classification, unsigned char);
  const char* GetClassificationName() { return this->ClassificationName.c_str(); }

  //BTX
protected:
  vtkPVLASOutputBlockInformation();
  ~vtkPVLASOutputBlockInformation() override;

  // Data information collected from remote processes.
  vtkIdType NumberOfPoints;
  vtkIdType NumberOfPointsInClassification;
  std::string ClassificationName;
  unsigned char Classification;
  double Bounds[6];

private:
  vtkPVLASOutputBlockInformation(const vtkPVLASOutputBlockInformation&); // Not implemented
  void operator=(const vtkPVLASOutputBlockInformation&);                 // Not implemented
  //ETX
};

#endif
