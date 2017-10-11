//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkMultiBlockWrapper - Wrapper class for CMB multiblocks
// .SECTION Description
// Wrapper class for using vtkMultiBlockDataSet for CMB.  The "system" id
// is created by this wrapper and the "user" id and name is just stored
// in this data structure for GUI reference.  All system ids will just
// be referred to as id and user ids and names will be referred to as
// UserId and UserName, respectively.

#ifndef __MultiBlockWrapper_h
#define __MultiBlockWrapper_h

#include "cmbSystemConfig.h"
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkObject.h"
#include "vtkSetGet.h"

class vtkMultiBlockDataSet;
class vtkPolyData;
class vtkIdList;
class vtkIntArray;
class vtkDataArray;
class vtkFieldData;
class vtkDataObject;
class vtkAbstractArray;
class vtkFloatArray;
class vtkInformationIntegerKey;

class VTKCMBGENERAL_EXPORT vtkMultiBlockWrapper : public vtkObject
{
public:
  static vtkMultiBlockWrapper* New();
  vtkTypeMacro(vtkMultiBlockWrapper, vtkObject);

  // Description:
  // Functions get string names used to store cell/field data.
  static const char* GetModelFaceTagName();
  static const char* GetShellTagName();
  static const char* GetMaterialTagName();
  static const char* GetReverseClassificationTagName();
  static const char* GetSplitFacesTagName();
  static const char* GetModelFaceDataName();
  static const char* GetModelFaceConvenientArrayName();
  static const char* GetMaterialUserNamesString();
  static const char* GetShellUserNamesString();
  static const char* GetShellTranslationPointString();
  static const char* GetModelFaceUserNamesString();
  static const char* GetBCSUserNamesString();
  static const char* GetShellColorsString();
  static const char* GetMaterialColorsString();
  static const char* GetBCSColorsString();
  static const char* GetModelFaceColorsString();
  static const char* GetModelFaceUse1String();

  // Description:
  // Key used to put block visibility in the meta-data associated with a block.
  static vtkInformationIntegerKey* BlockVisibilityMetaKey();

  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMultiBlockWrapper();
  ~vtkMultiBlockWrapper() override;

  vtkMultiBlockDataSet* mb;
  bool needToRemoveDeletedCells;

private:
  vtkMultiBlockWrapper(const vtkMultiBlockWrapper&); // Not implemented.
  void operator=(const vtkMultiBlockWrapper&);       // Not implemented.
};

#endif
