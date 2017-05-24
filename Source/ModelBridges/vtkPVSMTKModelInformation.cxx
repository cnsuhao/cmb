//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSMTKModelInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "vtkPVSMTKModelSource.h"

vtkStandardNewMacro(vtkPVSMTKModelInformation);

vtkPVSMTKModelInformation::vtkPVSMTKModelInformation()
{
}

vtkPVSMTKModelInformation::~vtkPVSMTKModelInformation()
{
  this->UUID2BlockIdMap.clear();
}

void vtkPVSMTKModelInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkPVSMTKModelInformation::CopyFromObject(vtkObject* obj)
{
  this->UUID2BlockIdMap.clear();
  vtkPVSMTKModelSource* modelsource = vtkPVSMTKModelSource::SafeDownCast(obj);

  if (!modelsource)
  {
    vtkErrorMacro("Object is not a vtkPVSMTKModelSource!");
    return;
  }

  modelsource->GetUUID2BlockIdMap(this->UUID2BlockIdMap);
  this->BlockId2UUIDMap.clear();
  std::map<smtk::common::UUID, vtkIdType>::iterator it = this->UUID2BlockIdMap.begin();
  for (; it != this->UUID2BlockIdMap.end(); ++it)
  {
    this->BlockId2UUIDMap[it->second] = it->first;
  }
  this->m_ModelUUID = smtk::common::UUID(modelsource->GetModelEntityID());
}

bool vtkPVSMTKModelInformation::GetBlockId(const smtk::common::UUID& uuid, unsigned int& bid)
{
  if (this->UUID2BlockIdMap.find(uuid) != this->UUID2BlockIdMap.end())
  {
    bid = this->UUID2BlockIdMap[uuid];
    return true;
  }
  return false;
}

const smtk::common::UUID& vtkPVSMTKModelInformation::GetModelUUID()
{
  return this->m_ModelUUID;
  /*
  if(this->BlockId2UUIDMap.find(bid) != this->BlockId2UUIDMap.end())
    {
    return this->BlockId2UUIDMap[bid];
    }
  return this->m_dummyID;
*/
}

const smtk::common::UUID& vtkPVSMTKModelInformation::GetModelEntityId(unsigned int bid)
{
  return this->BlockId2UUIDMap[bid];
  /*
  if(this->BlockId2UUIDMap.find(bid) != this->BlockId2UUIDMap.end())
    {
    return this->BlockId2UUIDMap[bid];
    }
  return this->m_dummyID;
*/
}
/*

smtk::common::UUIDs vtkPVSMTKModelInformation::GetBlockUUIDs() const
{
  smtk::common::UUIDs uids;
  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    this->UUID2BlockIdMap.begin();
  for(; it != this->UUID2BlockIdMap.end(); ++it)
    {
    uids.insert(it->first);
    }
  return uids;
}
*/

void vtkPVSMTKModelInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVSMTKModelInformation* modelInfo = vtkPVSMTKModelInformation::SafeDownCast(info);
  if (modelInfo)
  {
    this->UUID2BlockIdMap.clear();
    this->UUID2BlockIdMap.insert(
      modelInfo->UUID2BlockIdMap.begin(), modelInfo->UUID2BlockIdMap.end());
    this->BlockId2UUIDMap.clear();
    this->BlockId2UUIDMap.insert(
      modelInfo->BlockId2UUIDMap.begin(), modelInfo->BlockId2UUIDMap.end());
    this->m_ModelUUID = modelInfo->m_ModelUUID;
  }
}
