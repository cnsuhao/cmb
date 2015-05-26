//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkMergeFacesFilter.h"

#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkDataSet.h"

#include "assert.h"
#include <set>
#include <vtksys/ios/sstream>

class vtkMergeGroup
{
protected:
  std::set<vtkIdType> Set;
public:
  //typedef std::set<vtkIdType>::iterator iterator;
  typedef std::set<vtkIdType>::const_iterator const_iterator;
  vtkIdType Root() const
    {
    if (this->size() > 0)
      {
      const_iterator cit = this->begin();
      return *cit;
      }
    return -1;
    }

  void clear()
    {
      this->Set.clear();
    }

  size_t size() const
    {
      return this->Set.size();
    }

  const_iterator begin() const
    {
      return this->Set.begin();
    }

  const_iterator end() const
    {
      return this->Set.end();
    }
  void insert(const_iterator it1, const_iterator it2)
    {
      this->Set.insert(it1, it2);
    }

  const_iterator find(vtkIdType id) const
    {
      return this->Set.find(id);
    }

  std::pair<const_iterator, bool> insert(const vtkIdType& id)
    {
      return this->Set.insert(id);
    }

};

struct ltstr
{
  bool operator()(const vtkMergeGroup& s1, const vtkMergeGroup& s2) const
    {
      vtkIdType r1 = s1.Root();
      vtkIdType r2 = s2.Root();
      return (r1 < r2);
    }
};

class vtkMergeTable
{
public:
  typedef std::set<vtkMergeGroup, ltstr> RowsType;
  RowsType::const_iterator TraversalIterator;
  vtkMergeGroup::const_iterator GroupIterator;
  RowsType Rows;

  void Clear()
    {
      this->Rows.clear();
    }

  bool Empty()
    {
      return (this->Rows.size() == 0);
    }

  void Add(vtkMergeGroup& group)
    {
    if (group.size() == 0)
      {
      return;
      }

    RowsType::iterator iter = this->Rows.find(group);
    if (iter != this->Rows.end())
      {
      // If a row with the same Root() as the new group exists, then the
      // contents of this group are merged into the same row.
      group.insert(iter->begin(), iter->end());
      this->Rows.erase(iter);
      this->Rows.insert(group);
      }
    else
      {
      // Try to expand the group to include all already existing rows that now
      // get clubbed together due to this new merge group.
      for (iter = this->Rows.begin(); iter != this->Rows.end(); )
        {
        if (group.find(iter->Root()) != group.end() ||
          iter->find(group.Root()) != iter->end())
          {
          RowsType::iterator cur = iter;
          group.insert(iter->begin(), iter->end());
          iter++;
          this->Rows.erase(cur);
          }
        else
          {
          iter++;
          }
        }
      this->Rows.insert(group);
      }
    }

  vtkIdType GetId(vtkIdType old)
    {
    RowsType::iterator iter = this->Rows.begin();
    for (; iter != this->Rows.end(); ++iter)
      {
      if (iter->find(old) != iter->end())
        {
        return iter->Root();
        }
      }
    return old;
    }
};

//----------------------------------------------------------------------------
class vtkMergeFacesFilter::vtkInternal
{
public:
  vtkMergeTable MergeTable;
};

vtkStandardNewMacro(vtkMergeFacesFilter);

//----------------------------------------------------------------------------
vtkMergeFacesFilter::vtkMergeFacesFilter()
{
//  this->NewIds = vtkIdList::New();
  this->Internal = new vtkInternal();

/*  this->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_CELLS,
    vtkDataSetAttributes::SCALARS);

  this->Merge(0, 1);
  this->Merge(3, 4);
  this->Merge(3, 5);
  this->Merge(5, 6);
  */
}

//----------------------------------------------------------------------------
vtkMergeFacesFilter::~vtkMergeFacesFilter()
{
//  this->NewIds->Delete();
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::Merge(vtkIdType mergeInto, vtkIdType faceid)
{
  vtkMergeGroup group;
  group.insert(mergeInto);
  group.insert(faceid);
  this->Internal->MergeTable.Add(group);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::RemoveAllMergedFaces()
{
  this->RemoveAllMergedFacesInternal();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::RemoveAllMergedFacesInternal()
{
  this->Internal->MergeTable.Clear();
}

//----------------------------------------------------------------------------
int vtkMergeFacesFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->ShallowCopy(input);

  if (this->Internal->MergeTable.Empty())
    {
    return 1;
    }

  vtkMultiBlockWrapper* mbw = vtkMultiBlockWrapper::New();
  mbw->SetMultiBlock(output);

  int toFaceId, fromFaceId;
  this->Begin();
  while(!this->IsDone() && !this->IsDoneGroup())
    {
    toFaceId = this->GetElement();
    vtkIdList* fromFaces = vtkIdList::New();
    this->NextElement();
    while(!this->IsDoneGroup())
      {
      fromFaceId = this->GetElement();
      fromFaces->InsertNextId(fromFaceId);
      this->NextElement();
      }
    mbw->MergeModelFaces(toFaceId, fromFaces);
    fromFaces->Delete();
    this->NextGroup();
    }
  this->RemoveAllMergedFacesInternal();
  mbw->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::Begin()
{
  this->Internal->MergeTable.TraversalIterator =
    this->Internal->MergeTable.Rows.begin();
  if (!this->IsDone())
    {
    this->Internal->MergeTable.GroupIterator =
      this->Internal->MergeTable.TraversalIterator->begin();
    }
}

//----------------------------------------------------------------------------
bool vtkMergeFacesFilter::IsDone()
{
  return this->Internal->MergeTable.TraversalIterator ==
    this->Internal->MergeTable.Rows.end();
}

//----------------------------------------------------------------------------
bool vtkMergeFacesFilter::IsDoneGroup()
{
  return this->IsDone() ||
    this->Internal->MergeTable.GroupIterator ==
    this->Internal->MergeTable.TraversalIterator->end();
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::NextElement()
{
  this->Internal->MergeTable.GroupIterator++;
}

//----------------------------------------------------------------------------
void vtkMergeFacesFilter::NextGroup()
{
  this->Internal->MergeTable.TraversalIterator++;
  if (!this->IsDone())
    {
    this->Internal->MergeTable.GroupIterator =
      this->Internal->MergeTable.TraversalIterator->begin();
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkMergeFacesFilter::GetElement()
{
  return *this->Internal->MergeTable.GroupIterator;
}

int vtkMergeFacesFilter::FillInputPortInformation(int /*port*/, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}
//----------------------------------------------------------------------------
void vtkMergeFacesFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
