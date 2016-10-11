//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMTKModelFieldArrayFilter.h"

#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkModelManagerWrapper.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Manager.h"

vtkStandardNewMacro(vtkSMTKModelFieldArrayFilter);
vtkCxxSetObjectMacro(vtkSMTKModelFieldArrayFilter,ModelManagerWrapper,vtkModelManagerWrapper);

vtkSMTKModelFieldArrayFilter::vtkSMTKModelFieldArrayFilter()
{
  this->ModelManagerWrapper = NULL;
  this->AttributeDefinitionType = NULL;
  this->AttributeItemName = NULL;
  this->AttributeSystemContents = NULL;
  this->AddGroupArray = false;
}

vtkSMTKModelFieldArrayFilter::~vtkSMTKModelFieldArrayFilter()
{
  this->SetModelManagerWrapper(NULL);
  this->SetAttributeDefinitionType(NULL);
  this->SetAttributeItemName(NULL);
  this->SetAttributeSystemContents(NULL);
}

void vtkSMTKModelFieldArrayFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ModelManagerWrapper: " << this->ModelManagerWrapper << "\n";
  os << indent << "AttributeDefinitionType: " <<
    (this->AttributeDefinitionType ? this->AttributeDefinitionType : "null") << "\n";
  os << indent << "AttributeItemName: " <<
    (this->AttributeItemName ? this->AttributeItemName : "null") << "\n";
  os << indent << "AttributeSystemContents: " <<
    (this->AttributeSystemContents ? this->AttributeSystemContents : "null") << "\n";
}

int vtkSMTKModelFieldArrayFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

/***
* Paraview has a bug with coloring by field data array, where if the array only exists
* in some of the blocks, the color will leak to other blocks when rendering. This is
* a known bug in openGL-1 rendering pipeline for paraview, and for openGL-2, it will
* cause a crash as of June-11-2015. Therefore, we create a field-data-array for the
* block even it is empty.
***/
/// Add customized block info.
/// Mapping from UUID to block id
/// Field data arrays
static void internal_AddBlockGroupInfo(
  vtkDataObject* objBlock,
  const smtk::model::ManagerPtr& modelMan)
{
  if(!objBlock)
    return;
  vtkStringArray* uuidArray = vtkStringArray::SafeDownCast(
    objBlock->GetFieldData()->GetAbstractArray(
    vtkModelMultiBlockSource::GetEntityTagName()));
  if(!uuidArray)
    return;
  std::string entid = uuidArray->GetValue(0);
  smtk::model::EntityRef entityref(modelMan, entid);
  // Add group UUID to fieldData
  vtkStringArray* groupArray = vtkStringArray::SafeDownCast(
    objBlock->GetFieldData()->GetAbstractArray(
    vtkModelMultiBlockSource::GetGroupTagName()));
  bool newArray = false;
  if(!groupArray)
    {
    groupArray = vtkStringArray::New();
    newArray = true;
    }
  groupArray->SetNumberOfComponents(1);
  groupArray->SetNumberOfTuples(1);
  smtk::model::Groups relatedGrps = entityref.containingGroups();
  int na = static_cast<int>(relatedGrps.size());
  groupArray->SetValue(0, (na > 0) ?
                       relatedGrps[0].entity().toString() :
                       "no group");
  if(newArray)
    {
    groupArray->SetName(vtkModelMultiBlockSource::GetGroupTagName());
    objBlock->GetFieldData()->AddArray(groupArray);
    groupArray->Delete();
    }
}

void internal_addBlockAttributeFieldData(vtkDataObject* objBlock,
                           const smtk::model::ManagerPtr& vtkNotUsed(modelMan),
                           const smtk::attribute::SystemPtr& attsys,
                           const char*  attDefType,
                           const char*  attItemName)
{
  std::string arrayname = attDefType;
  if(attItemName && attItemName[0] != '\0')
    arrayname.append(" (").append(attItemName).append(")");

  vtkStringArray* fieldAttArray = vtkStringArray::SafeDownCast(
    objBlock->GetFieldData()->GetAbstractArray(arrayname.c_str()));
  std::vector<smtk::attribute::AttributePtr> atts;
  attsys->findAttributes(attDefType, atts);
  bool newArray = false;
  if(!fieldAttArray)
    {
    fieldAttArray = vtkStringArray::New();
    newArray = true;
    }
  fieldAttArray->SetNumberOfComponents(1);
  fieldAttArray->SetNumberOfTuples(1);
  vtkStringArray* uuidArray = vtkStringArray::SafeDownCast(
    objBlock->GetFieldData()->GetAbstractArray(
    vtkModelMultiBlockSource::GetEntityTagName()));

  if(atts.size() == 0 || !uuidArray)
    {
    fieldAttArray->SetValue(0, "no attribute");
    if(newArray)
      {
      fieldAttArray->SetName(arrayname.c_str());
      objBlock->GetFieldData()->AddArray(fieldAttArray);
      fieldAttArray->Delete();
      }
    return;
    }

  std::string entid = uuidArray->GetValue(0);
  std::string strValEntry;
  std::vector<smtk::attribute::AttributePtr>::const_iterator ait;
  for(ait = atts.begin(); ait != atts.end(); ++ait)
    {
    smtk::common::UUIDs associatedEntities = (*ait)->associatedModelEntityIds();
    if(std::find(associatedEntities.begin(), associatedEntities.end(),
       entid) != associatedEntities.end())
      {
      // this entity is associated with an attribute
      std::string valuestr;
      smtk::attribute::ItemPtr attitem;
      // Figure out which variant of the item to use, if it exists
      if(attItemName && attItemName[0] != '\0' && (attitem = (*ait)->find(attItemName)))
        {
        if(attitem->type() == smtk::attribute::Item::DOUBLE)
          {
          valuestr =
            smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(attitem)->valueAsString();
          }
        else if (attitem->type() == smtk::attribute::Item::INT)
          {
          valuestr =
            smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(attitem)->valueAsString();
          }    
        else if (attitem->type() == smtk::attribute::Item::STRING)
          {
          valuestr =
            smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(attitem)->valueAsString();
          }
        }
      // if find an attribute, stop
      strValEntry = !valuestr.empty() ? valuestr : (*ait)->name();
      break;
      }
    }

  // if there is no valid attribute, add "no attribute".
  fieldAttArray->SetValue(0, strValEntry.empty() ?
                      "no attribute" : strValEntry.c_str());

  if(newArray)
    {
    fieldAttArray->SetName(arrayname.c_str());
    objBlock->GetFieldData()->AddArray(fieldAttArray);
    fieldAttArray->Delete();
    }
}

int vtkSMTKModelFieldArrayFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inInfo,
  vtkInformationVector* outInfo)
{
  // get the info and input data
  vtkMultiBlockDataSet* input = vtkMultiBlockDataSet::GetData(inInfo[0], 0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!input || !output)
    {
    vtkErrorMacro("No input or output dataset");
    return 0;
    }

  if (!this->ModelManagerWrapper ||
      !this->ModelManagerWrapper->GetModelManager())
    {
    vtkErrorMacro("No input model manager!");
    return 0;
    }

  output->ShallowCopy(input);
  smtk::model::ManagerPtr modelMan = this->ModelManagerWrapper->GetModelManager();
  if(this->GetAddGroupArray())
    {
    for (unsigned int blockID = 0;
      blockID < output->GetNumberOfBlocks(); blockID++)
      {
      internal_AddBlockGroupInfo(output->GetBlock(blockID), modelMan);
      }
    }

  if(this->AttributeDefinitionType == NULL ||
     this->AttributeSystemContents == NULL)
    {
    return 1;
    }
//  smtk::attribute::System* attsys = modelMan->attributeSystem();

  smtk::io::AttributeReader attributeReader;
  smtk::io::Logger logger;
  smtk::attribute::SystemPtr attsys =
    smtk::attribute::SystemPtr(new smtk::attribute::System());
  bool hasErrors = attributeReader.readContents(*attsys,
    this->GetAttributeSystemContents(), logger);
  if(hasErrors)
    {
    std::cerr << logger.convertToString() << std::endl;
    return 0;
    }

  for (unsigned int blockId = 0;
    blockId < output->GetNumberOfBlocks(); blockId++)
    {
    internal_addBlockAttributeFieldData(
      output->GetBlock(blockId),
      modelMan,
      attsys,
      this->AttributeDefinitionType,
      this->AttributeItemName);
    }

/*
*/
  return 1;
}
