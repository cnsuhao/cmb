//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSMTKMeshInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"

#include "vtkPVSMTKMeshSource.h"

vtkStandardNewMacro(vtkPVSMTKMeshInformation);

//----------------------------------------------------------------------------
vtkPVSMTKMeshInformation::vtkPVSMTKMeshInformation()
{
}

//----------------------------------------------------------------------------
vtkPVSMTKMeshInformation::~vtkPVSMTKMeshInformation()
{
  this->Mesh2BlockIdMap.clear();
}

//----------------------------------------------------------------------------
void vtkPVSMTKMeshInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVSMTKMeshInformation::CopyFromObject(vtkObject* obj)
{
  this->Mesh2BlockIdMap.clear();
  vtkPVSMTKMeshSource* meshsource = vtkPVSMTKMeshSource::SafeDownCast(obj);

  if (!meshsource)
  {
    vtkErrorMacro("Object is not a vtkPVSMTKMeshSource!");
    return;
  }

  meshsource->GetMeshSet2BlockIdMap(this->Mesh2BlockIdMap);
  this->BlockId2MeshMap.clear();
  std::map<smtk::mesh::MeshSet, unsigned int>::iterator it = this->Mesh2BlockIdMap.begin();
  for (; it != this->Mesh2BlockIdMap.end(); ++it)
  {
    this->BlockId2MeshMap[it->second] = it->first;
  }
  this->m_ModelUUID = smtk::common::UUID(meshsource->GetModelEntityID());
  this->m_MeshCollectionId = smtk::common::UUID(meshsource->GetMeshCollectionID());
}

//----------------------------------------------------------------------------
bool vtkPVSMTKMeshInformation::GetBlockId(const smtk::mesh::MeshSet& mesh, unsigned int& bid)
{
  if (this->Mesh2BlockIdMap.find(mesh) != this->Mesh2BlockIdMap.end())
  {
    bid = this->Mesh2BlockIdMap[mesh];
    return true;
  }
  return false;
}
//----------------------------------------------------------------------------
const smtk::common::UUID& vtkPVSMTKMeshInformation::GetModelUUID()
{
  return this->m_ModelUUID;
}
//----------------------------------------------------------------------------
const smtk::common::UUID& vtkPVSMTKMeshInformation::GetMeshCollectionID()
{
  return this->m_MeshCollectionId;
}

//----------------------------------------------------------------------------
const smtk::mesh::MeshSet& vtkPVSMTKMeshInformation::GetMeshSet(unsigned int bid)
{
  return this->BlockId2MeshMap[bid];
}
//----------------------------------------------------------------------------
void vtkPVSMTKMeshInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVSMTKMeshInformation* modelInfo = vtkPVSMTKMeshInformation::SafeDownCast(info);
  if (modelInfo)
  {
    this->Mesh2BlockIdMap.clear();
    this->Mesh2BlockIdMap.insert(
      modelInfo->Mesh2BlockIdMap.begin(), modelInfo->Mesh2BlockIdMap.end());
    this->BlockId2MeshMap.clear();
    this->BlockId2MeshMap.insert(
      modelInfo->BlockId2MeshMap.begin(), modelInfo->BlockId2MeshMap.end());
    this->m_ModelUUID = modelInfo->m_ModelUUID;
    this->m_MeshCollectionId = modelInfo->m_MeshCollectionId;
  }
}
