#include "vtkPVSMTKModelInformation.h"

#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataIterator.h"

#include "vtkPVSMTKModelSource.h"

vtkStandardNewMacro(vtkPVSMTKModelInformation);

//----------------------------------------------------------------------------
vtkPVSMTKModelInformation::vtkPVSMTKModelInformation()
{
}

//----------------------------------------------------------------------------
vtkPVSMTKModelInformation::~vtkPVSMTKModelInformation()
{
  this->UUID2BlockIdMap.clear();
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::CopyFromObject(vtkObject* obj)
{
  this->UUID2BlockIdMap.clear();
  vtkPVSMTKModelSource *modelsource = vtkPVSMTKModelSource::SafeDownCast( obj );

  if (!modelsource)
    {
    vtkErrorMacro("Object is not a vtkPVSMTKModelSource!");
    return;
    }

  modelsource->GetUUID2BlockIdMap(this->UUID2BlockIdMap);
  this->BlockId2UUIDMap.clear();
  std::map<smtk::common::UUID, unsigned int>::iterator it =
    this->UUID2BlockIdMap.begin();
  for(; it != this->UUID2BlockIdMap.end(); ++it)
    {
    this->BlockId2UUIDMap[it->second] = it->first;
    }
}

//----------------------------------------------------------------------------
bool vtkPVSMTKModelInformation::GetBlockId(
  const smtk::common::UUID& uuid, unsigned int &bid)
{
  if(this->UUID2BlockIdMap.find(uuid) != this->UUID2BlockIdMap.end())
    {
    bid = this->UUID2BlockIdMap[uuid];
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
const smtk::common::UUID&  vtkPVSMTKModelInformation::GetModelEntityId(
  unsigned int bid)
{
  if(this->BlockId2UUIDMap.find(bid) != this->BlockId2UUIDMap.end())
    {
    return this->BlockId2UUIDMap[bid];
    }
  return this->m_dummyID;
}

//----------------------------------------------------------------------------
smtk::common::UUIDs vtkPVSMTKModelInformation::GetBlockUUIDs() const
{
  smtk::common::UUIDs uids;
  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    this->UUID2BlockIdMap.begin();
  for(; it != this->UUID2BlockIdMap.end(); ++it)
    {
    uids.insert(smtk::common::UUID(it->first));
    }
  return uids;
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVSMTKModelInformation *modelInfo =
    vtkPVSMTKModelInformation::SafeDownCast(info);
  if (modelInfo)
    {
    this->UUID2BlockIdMap.clear();
    this->UUID2BlockIdMap.insert(
      modelInfo->UUID2BlockIdMap.begin(), modelInfo->UUID2BlockIdMap.end());
    }
}
