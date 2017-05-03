//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkFacesConnectivityFilter.h"

#include "vtkCellData.h"
#include "vtkConnectivityFilter.h"
#include "vtkDataSet.h"
#include "vtkExtractModelFaceBlock.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"

//#include "vtkExtractSelection.h"
//#include "vtkExtractSelectedPolyDataIds.h"
//#include "vtkSelectionSource.h"
//#include "vtkSelection.h"
//#include "vtkConvertSelection.h"

#include "vtkCompositeDataPipeline.h"
#include "vtkInformationVector.h"

#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"

#include "vtkPolyDataNormals.h"

#include "assert.h"
#include <sstream>

//----------------------------------------------------------------------------
class vtkFacesConnectivityFilter::vtkInternal
{
public:
  vtkPolyDataNormals* pdNormals;
  //  vtkExtractSelection* ExtractFaces;
  //  vtkExtractSelectedPolyDataIds* ExtractIds;
  vtkExtractModelFaceBlock* extractLeaf;
  vtkConnectivityFilter* PVConnectivity;
};

vtkStandardNewMacro(vtkFacesConnectivityFilter);

//----------------------------------------------------------------------------
vtkFacesConnectivityFilter::vtkFacesConnectivityFilter()
{
  //this->NewBlockIndices = vtkIdList::New();
  this->FeatureAngle = 70.0;
  this->FaceID = -1;
  this->SetNumberOfInputPorts(1);

  this->Internal = new vtkInternal();
  this->Internal->PVConnectivity = vtkConnectivityFilter::New();
  this->Internal->PVConnectivity->SetExtractionModeToAllRegions();
  this->Internal->PVConnectivity->SetColorRegions(1);

  //this->Internal->ExtractFaces = vtkExtractSelection::New();
  //this->Internal->ExtractIds = vtkExtractSelectedPolyDataIds::New();

  this->Internal->pdNormals = vtkPolyDataNormals::New();
  this->Internal->extractLeaf = vtkExtractModelFaceBlock::New();
}

//----------------------------------------------------------------------------
vtkFacesConnectivityFilter::~vtkFacesConnectivityFilter()
{
  //this->NewBlockIndices->Delete();

  this->Internal->pdNormals->Delete();
  this->Internal->PVConnectivity->Delete();
  this->Internal->extractLeaf->Delete();
  //this->Internal->ExtractFaces->Delete();
  //this->Internal->ExtractIds->Delete();

  delete this->Internal;
}

//----------------------------------------------------------------------------
int vtkFacesConnectivityFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkMultiBlockDataSet* input =
    vtkMultiBlockDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet* output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkExtractModelFaceBlock* extract = this->Internal->extractLeaf;
  extract->SetInputData(0, input);
  extract->SetFaceId(this->FaceID);
  extract->Update();

  vtkDataSet* inputCopy = vtkDataSet::SafeDownCast(extract->GetOutputDataObject(0))->NewInstance();
  inputCopy->ShallowCopy(extract->GetOutputDataObject(0));
  vtkIntArray* cellIdArray = vtkIntArray::SafeDownCast(
    inputCopy->GetCellData()->GetArray(vtkMultiBlockWrapper::GetReverseClassificationTagName()));

  vtkDataSet* inputToPDNormals = inputCopy;
  this->Internal->pdNormals->SetFeatureAngle(this->GetFeatureAngle());
  this->Internal->pdNormals->SetInputData(0, inputToPDNormals);

  this->Internal->pdNormals->Update();

  this->Internal->PVConnectivity->SetInputConnection(0, this->Internal->pdNormals->GetOutputPort());

  this->Internal->PVConnectivity->Update();
  //  int numExtFaces = this->Internal->PVConnectivity->GetNumberOfExtractedRegions();

  vtkDataSet* ecOutput =
    vtkDataSet::SafeDownCast(this->Internal->PVConnectivity->GetOutputDataObject(0));

  vtkIdTypeArray* newFaceIds =
    vtkIdTypeArray::SafeDownCast(ecOutput->GetCellData()->GetArray("RegionId"));

  output->ShallowCopy(input);
  vtkMultiBlockWrapper* mbw = vtkMultiBlockWrapper::New();
  mbw->SetMultiBlock(output);

  vtkPolyData* rootPoly = mbw->GetMasterPolyData();
  vtkIntArray* classificationInfo = vtkIntArray::SafeDownCast(
    rootPoly->GetCellData()->GetArray(vtkMultiBlockWrapper::GetModelFaceTagName()));

  double range[2];
  classificationInfo->GetRange(range);
  int maxFaceId = static_cast<int>((range[1] > range[0]) ? range[1] : range[0]);

  std::map<vtkIdType, vtkIdList*> faceList;
  // Update the root polydata to reflect the new faces
  this->UpdateFaceIDArray(maxFaceId, newFaceIds, cellIdArray, faceList);

  vtkIntArray* newfaces = vtkIntArray::New();
  for (std::map<vtkIdType, vtkIdList*>::iterator it = faceList.begin(); it != faceList.end(); it++)
  {
    int mfId = mbw->CreateModelFace(it->second);
    newfaces->InsertNextValue(mfId);
    it->second->Delete();
  }
  // clean out the modified model faces that had cells deleted from them
  mbw->RemoveDeletedCellsFromModelFaces();

  mbw->SetSplitModelFaces(this->GetFaceID(), newfaces);
  newfaces->Delete();
  mbw->Delete();

  //make sure everything is deallocated
  inputCopy->Delete();
  ecOutput->Initialize();
  this->Internal->PVConnectivity->SetInputData(0, static_cast<vtkDataSet*>(NULL));
  return 1;
}

//----------------------------------------------------------------------------
void vtkFacesConnectivityFilter::UpdateFaceIDArray(int maxFaceId, vtkIdTypeArray* newRegionIds,
  vtkIntArray* selCellIndices, std::map<vtkIdType, vtkIdList*>& faceList)
{

  if (newRegionIds && newRegionIds->GetNumberOfTuples() > 0)
  {
    int newFid, faceId, cellId;
    for (int i = 0; i < selCellIndices->GetNumberOfTuples(); i++)
    {
      faceId = newRegionIds->GetValue(i);
      cellId = selCellIndices->GetValue(i);
      if (faceId > 0)
      {
        newFid = maxFaceId + faceId;
        if (faceList.find(newFid) == faceList.end())
        {
          faceList[newFid] = vtkIdList::New();
        }
        faceList[newFid]->InsertNextId(cellId);
        //outFaceIds->SetValue(cellId, newFid);
      }
      //*numNewFaces = faceId>*numNewFaces ? faceId : *numNewFaces;
      //newFaces->InsertNextValue(maxFaceId+newFid);
    }
  }
}

//----------------------------------------------------------------------------
int vtkFacesConnectivityFilter::FillInputPortInformation(int /*port*/, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  /*
  if (port==0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    }
  else
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
*/
  return 1;
}

//----------------------------------------------------------------------------
void vtkFacesConnectivityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
