/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


#include "smtkModel.h"

#include "smtkModelGroup.h"
#include "smtkModelItem.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkModelMaterial.h"
#include "vtkModelItemIterator.h"
#include "vtkModelUserName.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxy.h"
#include "vtkWeakPointer.h"

//----------------------------------------------------------------------------
class smtkModelInternals
{
  public:

    vtkWeakPointer<vtkDiscreteModel> DiscreteModel;
    vtkWeakPointer<vtkSMProxy> ModelWrapper;
};
//----------------------------------------------------------------------------
smtkModel::smtkModel()
{
  this->Internals = new smtkModelInternals;
}

//----------------------------------------------------------------------------
smtkModel::~smtkModel()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
smtk::model::MaskType smtkModel::convertGroupTypeToMask(
  int grouptype, int modelDimension)
{
  smtk::model::MaskType grpMask = smtk::model::Item::GROUP;
  switch (grouptype)
    {
    case vtkDiscreteModelEntityGroupType:
      if(modelDimension == 3)
        {
        grpMask |= smtk::model::Item::FACE;
        }
      else if(modelDimension == 2)
        {
        grpMask |= smtk::model::Item::EDGE;
        }
      break;
    case vtkModelMaterialType: // DomainSet, or Material Type
      if(modelDimension == 3)
        {
        grpMask |= smtk::model::Item::REGION;
        }
      else if(modelDimension == 2)
        {
        grpMask |= smtk::model::Item::FACE;
        }
      break;
    default:
      break;
    }
  return grpMask;
}
//----------------------------------------------------------------------------
bool smtkModel::convertEntityType(int modelType,
  smtk::model::Item::Type& enType)
{
  bool result = true;
  switch (modelType)
  {
    case vtkModelVertexType:
      enType = smtk::model::Item::VERTEX;
      break;
    case vtkModelEdgeType:
      enType = smtk::model::Item::EDGE;
      break;
    case vtkModelFaceType:
      enType = smtk::model::Item::FACE;
      break;
    case vtkModelRegionType:
      enType = smtk::model::Item::REGION;
      break;
    case vtkModelType:
      enType = smtk::model::Item::MODEL_DOMAIN;
      break;
    default:
      result = false;
      break;
  }
  return result;
}
//----------------------------------------------------------------------------
vtkDiscreteModel* smtkModel::discreteModel()
{
  return this->Internals->DiscreteModel;
}
//----------------------------------------------------------------------------
void smtkModel::setDiscreteModel(vtkDiscreteModel* model)
{
  if(this->Internals->DiscreteModel == model)
    {
    return;
    }
  this->Internals->DiscreteModel = model;
}
//----------------------------------------------------------------------------
vtkSMProxy* smtkModel::modelWrapper()
{
  return this->Internals->ModelWrapper;
}
//----------------------------------------------------------------------------
void smtkModel::setModelWrapper(vtkSMProxy* modelWrapper)
{
  if(this->Internals->ModelWrapper == modelWrapper)
    {
    return;
    }
  this->Internals->ModelWrapper = modelWrapper;
}

//----------------------------------------------------------------------------
void smtkModel::loadGroupItems(int grouptype)
{
  vtkDiscreteModel *cmbModel = this->Internals->DiscreteModel;
  if(!cmbModel)
    {
    return;
    }
  if(grouptype != vtkDiscreteModelEntityGroupType &&
     grouptype != vtkModelMaterialType)
    {
    vtkGenericWarningMacro("Trying to create a group from an invalid type.");
    }

  vtkModelItemIterator* iter=cmbModel->NewIterator(grouptype);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEntity* entity =
      vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    if(entity)
      {
      this->createModelGroupFromCMBGroup(entity->GetUniquePersistentId());
      }
    }
  iter->Delete();
}

//----------------------------------------------------------------------------
void smtkModel::updateGroupItems(int grouptype)
{
  // two types supported in smtk are vtkDiscreteModelEntityGroupType and vtkModelMaterialType
  // these types come from vtkDiscreteModel.
  vtkModelItemIterator* groupIter = this->discreteModel()->NewIterator(grouptype);
  for(groupIter->Begin();!groupIter->IsAtEnd();groupIter->Next())
    {
    this->deleteModelGroup(vtkModelEntity::SafeDownCast(groupIter->GetCurrentItem())->GetUniquePersistentId());
    }
  groupIter->Delete();
  this->loadGroupItems(grouptype);
}

//-----------------------------------------------------------------------------
void smtkModel::updateModelItemName(int Id, std::string& entName)
{
  smtk::model::ItemPtr item = this->getModelItem(Id);
  if(item)
    {
    item->setName(entName);
    }
}
//-----------------------------------------------------------------------------
void smtkModel::removeModelItem(int Id)
{
  this->deleteModelGroup(Id);
}

//----------------------------------------------------------------------------
smtk::model::GroupItemPtr smtkModel::createModelGroupFromCMBGroup(int groupId)
{
  std::map<int, smtk::model::ItemPtr>::iterator it = this->m_items.find(groupId);
  if (it != this->m_items.end())
    {
    return smtk::dynamic_pointer_cast<smtk::model::GroupItem>(it->second);
    }

  smtk::model::GroupItemPtr aGroup = smtk::model::GroupItemPtr();
  if(!this->Internals->DiscreteModel)
    {
    return aGroup;
    }
  vtkModelEntity* entityGroup = this->Internals->DiscreteModel->GetModelEntity(groupId);
  if(!entityGroup)
    {
    return aGroup;
    }

  int modelDimension = this->Internals->DiscreteModel->GetNumberOfModelEntities(vtkModelRegionType) != 0 ?
    3 : 2;

  smtk::model::MaskType grpMask = this->convertGroupTypeToMask(
    entityGroup->GetType(), modelDimension);
  return this->createModelGroup(vtkModelUserName::GetUserName(entityGroup),
   groupId, grpMask);
}

//----------------------------------------------------------------------------
smtk::model::ItemPtr smtkModel::createModelItem(vtkModelEntity* entity)
{
  smtk::model::ItemPtr anItem = smtk::model::ItemPtr();
  if(!entity)
    {
    return anItem;
    }
  int entType = entity->GetType();
  smtk::model::Item::Type itemType;
  if(this->convertEntityType(entType, itemType))
    {
    anItem = this->createModelItem(vtkModelUserName::GetUserName(entity),
      entity->GetUniquePersistentId(), itemType);
    }
  return anItem;
}

//----------------------------------------------------------------------------
smtk::model::ItemPtr smtkModel::createModelItem(
  const std::string &name, int id, smtk::model::Item::Type itemType)
{
  return smtk::model::ItemPtr(
    new smtkModelItem(this, id, itemType, name));
}

//----------------------------------------------------------------------------
bool smtkModel::deleteModelGroup(int id)
{
  std::map<int, smtk::model::ItemPtr>::iterator it = this->m_items.find(id);
  if (it != this->m_items.end())
    {
    it->second->detachAllAttributes();
    this->m_items.erase(it);
    return true;
    }

  return false;
}
