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

#include "smtk/PublicPointerDefs.h"
#include "vtkMultiBlockDataSetAlgorithm.h"

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
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkSMTKModelFieldArrayFilter, vtkMultiBlockDataSetAlgorithm);

  // Description:
  // Set model manager wrapper
  void SetModelManagerWrapper(vtkModelManagerWrapper* modelManager);
  vtkGetObjectMacro(ModelManagerWrapper, vtkModelManagerWrapper);

  // Description:
  // Get/set the AttributeDefinitionType.
  vtkSetStringMacro(AttributeDefinitionType);
  vtkGetStringMacro(AttributeDefinitionType);

  // Get/set the AttributeItemName
  vtkSetStringMacro(AttributeItemName);
  vtkGetStringMacro(AttributeItemName);

  // Description:
  // Set/Get the AttributeCollectionContents.
  vtkSetStringMacro(AttributeCollectionContents);
  vtkGetStringMacro(AttributeCollectionContents);

  // Description:
  // If true, the 'Group' field data will be recreated.
  vtkBooleanMacro(AddGroupArray, bool);
  vtkSetMacro(AddGroupArray, bool);
  vtkGetMacro(AddGroupArray, bool);

protected:
  vtkSMTKModelFieldArrayFilter();
  ~vtkSMTKModelFieldArrayFilter() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;
  int FillInputPortInformation(int port, vtkInformation* info) override;

  // Reference model Manager wrapper:
  vtkModelManagerWrapper* ModelManagerWrapper;
  char* AttributeDefinitionType;
  char* AttributeItemName;
  char* AttributeCollectionContents;
  bool AddGroupArray;

private:
  vtkSMTKModelFieldArrayFilter(const vtkSMTKModelFieldArrayFilter&); // Not implemented.
  void operator=(const vtkSMTKModelFieldArrayFilter&);               // Not implemented.
};

#endif // __vtkSMTKModelFieldArrayFilter_h
