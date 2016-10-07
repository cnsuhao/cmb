//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkHydroModelCreator.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkCell.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkCellData.h"
#include "vtkDataObject.h"
#include <vector>
#include <map>

vtkStandardNewMacro(vtkHydroModelCreator);

//-----------------------------------------------------------------------------
vtkHydroModelCreator::vtkHydroModelCreator()
{
}

//-----------------------------------------------------------------------------
vtkHydroModelCreator::~vtkHydroModelCreator()
{
}

//-----------------------------------------------------------------------------
int vtkHydroModelCreator::FillInputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkPolyData");
  return 1;
}


//-----------------------------------------------------------------------------
int vtkHydroModelCreator::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!input)
    {
    vtkWarningMacro("The input did not pass in a vtkPolyData object");
    return 0;
    }

  vtkIdType i, numcells = input->GetNumberOfCells();
  vtkCellData* celldata = input->GetCellData();
  vtkIntArray* intmodelfacedata = vtkIntArray::SafeDownCast(
    celldata->GetArray(vtkMultiBlockWrapper::GetModelFaceTagName()) );

  std::map<vtkIdType, vtkIdList*> modelFaces;
  vtkMultiBlockWrapper* mbw = vtkMultiBlockWrapper::New();
  vtkPolyData* newpoly = vtkPolyData::New();
  vtkMultiBlockDataSet* mfMDS = vtkMultiBlockDataSet::New();
  vtkMultiBlockDataSet* ngMDS = vtkMultiBlockDataSet::New();
  newpoly->ShallowCopy(input);
  output->SetBlock(0, newpoly); // Master polydata block
  output->SetBlock(1, mfMDS); // Model Faces Root block
  output->SetBlock(2, ngMDS); // Nodal Groups Root block.
  mfMDS->Delete();
  ngMDS->Delete();

  mbw->SetMultiBlock(output);
  for(i=0;i<numcells;i++)
    {
    vtkIdType value = intmodelfacedata->GetValue(i);
    std::map<vtkIdType, vtkIdList*>::iterator it =
      modelFaces.find(value);
    if(it == modelFaces.end())
      {
      modelFaces[value] = vtkIdList::New();
      it = modelFaces.find(value);
      }
    it->second->InsertNextId(i);
    }
  newpoly->Delete();

  vtkIntArray* intshelldata = vtkIntArray::SafeDownCast(
    celldata->GetArray(vtkMultiBlockWrapper::GetShellTagName()) );
  vtkIntArray* intmaterialdata = vtkIntArray::SafeDownCast(
    celldata->GetArray(vtkMultiBlockWrapper::GetMaterialTagName()) );
  std::map<vtkIdType, vtkIdList*>::iterator it;
  float rgba[4] = {-1, -1, -1, -1};
  // first create maps for shells and materials
  //map is current id to an id between 0 and numids-1
  std::map<int, int> shellMap;
  std::map<int, int> materialMap;
  // storage for the shells' materials
  std::vector<int> shellMaterialIds;
  for(it=modelFaces.begin();
      it!=modelFaces.end();it++)
    {
    int cellid = it->second->GetId(0);
    if(shellMap.find(intshelldata->GetValue(cellid)) == shellMap.end())
      {
      unsigned int sNum = static_cast<unsigned int>( shellMap.size() );
      shellMap[intshelldata->GetValue(cellid)] = static_cast<int>(sNum);
      shellMaterialIds.push_back(intmaterialdata->GetValue(cellid));
      }
    if(materialMap.find(intmaterialdata->GetValue(cellid))
       == materialMap.end())
      {
      unsigned int mNum = static_cast<unsigned int>( materialMap.size() );
      materialMap[intmaterialdata->GetValue(cellid)] = static_cast<int>(mNum);
      }
    }
  std::map<int,int>::size_type sz;
  char name[20];

  for(sz=0;sz<materialMap.size();sz++)
    {
    sprintf(name,"%s %d","material", static_cast<int>(sz));
    mbw->AddMaterial(-1, name, rgba);
    }
  for(sz=0;sz<shellMap.size();sz++)
    {
    sprintf(name,"%s %d","shell", static_cast<int>(sz));
    mbw->AddShell(name, rgba, shellMaterialIds[sz]-1, 0); // decrement material id to start from 0
    }
  for(it=modelFaces.begin();
      it!=modelFaces.end();it++)
    {
    int cellid = it->second->GetId(0);
    mbw->CreateModelFace(shellMap[intshelldata->GetValue(cellid)],
                         materialMap[intmaterialdata->GetValue(cellid)],
                         rgba, it->second, 0, -1);
    it->second->Delete();
    }

  mbw->Delete();

  return 1;
}

//-----------------------------------------------------------------------------
void vtkHydroModelCreator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
