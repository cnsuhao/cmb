//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkModelUserName - Helper class for setting/getting a user name
// .SECTION Description
// Helper class with functions for setting and getting a user name
// for a vtkModelEntity.

#ifndef __vtkModelUserName_h
#define __vtkModelUserName_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkInformationStringKey;
class vtkModelEntity;

class VTKDISCRETEMODEL_EXPORT vtkModelUserName : public vtkObject
{
public:
  static vtkModelUserName *New();
  vtkTypeMacro(vtkModelUserName,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  static void SetUserName(vtkModelEntity* entity, const char* userName);

  static const char* GetUserName(vtkModelEntity* entity);

  static vtkInformationStringKey* USERNAME();

protected:
  vtkModelUserName() {};
  ~vtkModelUserName() {};

private:
  vtkModelUserName(const vtkModelUserName&);  // Not implemented.
  void operator=(const vtkModelUserName&);  // Not implemented.
};
#endif

