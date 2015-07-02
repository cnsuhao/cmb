//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkSMTKModelFieldArrayFilter_h
#define __vtkSMTKModelFieldArrayFilter_h

#include "ModelBridgeClientModule.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <string>

class vtkModelManagerWrapper;

/**\brief A VTK filter for adding field data to smtk model source
  *
  * This filter will add field data arrays based on an attribute or its item.
  */
class MODELBRIDGECLIENT_EXPORT vtkSMTKModelFieldArrayFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSMTKModelFieldArrayFilter* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSMTKModelFieldArrayFilter,vtkMultiBlockDataSetAlgorithm);

  // Description:
  // Set model manager wrapper
  void SetModelManagerWrapper(vtkModelManagerWrapper *modelManager);
  vtkGetObjectMacro(ModelManagerWrapper, vtkModelManagerWrapper);

  // Description:
  // Get/set the AttributeDefinitionType.
  vtkSetStringMacro(AttributeDefinitionType);
  vtkGetStringMacro(AttributeDefinitionType);

  // Get/set the AttributeItemName
  vtkSetStringMacro(AttributeItemName);
  vtkGetStringMacro(AttributeItemName);

  // Description:
  // Set/Get the AttributeSystemContents.
  vtkSetStringMacro(AttributeSystemContents);
  vtkGetStringMacro(AttributeSystemContents);

  // Description:
  // If true, the 'Group' field data will be recreated.
  vtkBooleanMacro(AddGroupArray, bool);
  vtkSetMacro(AddGroupArray, bool);
  vtkGetMacro(AddGroupArray, bool);

protected:
  vtkSMTKModelFieldArrayFilter();
  virtual ~vtkSMTKModelFieldArrayFilter();

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  // Reference model Manager wrapper:
  vtkModelManagerWrapper* ModelManagerWrapper;
  char *AttributeDefinitionType;
  char *AttributeItemName;
  char *AttributeSystemContents;
  bool AddGroupArray;
private:

  vtkSMTKModelFieldArrayFilter(const vtkSMTKModelFieldArrayFilter&); // Not implemented.
  void operator = (const vtkSMTKModelFieldArrayFilter&); // Not implemented.
};

#endif // __vtkSMTKModelFieldArrayFilter_h
