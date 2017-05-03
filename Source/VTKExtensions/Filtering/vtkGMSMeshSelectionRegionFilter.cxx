//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkGMSMeshSelectionRegionFilter.h"

#include "vtkCellData.h"
#include "vtkConvertSelection.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkGMSMeshSelectionRegionFilter);

//----------------------------------------------------------------------------
vtkGMSMeshSelectionRegionFilter::vtkGMSMeshSelectionRegionFilter()
{
  this->SelectionRegionId = -1;
  this->IsNewRegionIdSet = 0;
  this->SetNumberOfInputPorts(3);
}

//----------------------------------------------------------------------------
vtkGMSMeshSelectionRegionFilter::~vtkGMSMeshSelectionRegionFilter()
{
}
//----------------------------------------------------------------------------
void vtkGMSMeshSelectionRegionFilter::SetSelectionRegionId(int val)
{
  if (this->SelectionRegionId == val)
  {
    return;
  }

  this->SelectionRegionId = val;
  this->IsNewRegionIdSet = 1;
  this->Modified();
}
//----------------------------------------------------------------------------
int vtkGMSMeshSelectionRegionFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    // Can work with composite datasets.
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  }
  else
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkGMSMeshSelectionRegionFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* selInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* meshInfo = inputVector[2]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // verify the input, selection and output
  vtkUnstructuredGrid* input = vtkUnstructuredGrid::GetData(inInfo);
  if (!input)
  {
    vtkErrorMacro(<< "Currently, only expecting vtkUnstructuredGrid input.");
    return 0;
  }
  vtkUnstructuredGrid* meshinput = vtkUnstructuredGrid::GetData(meshInfo);
  if (!meshinput)
  {
    vtkErrorMacro(<< "Need a mesh input.");
    return 0;
  }

  vtkUnstructuredGrid* output =
    vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!selInfo || !this->IsNewRegionIdSet)
  {
    output->ShallowCopy(meshinput);
    //When not given a selection, quietly select nothing.
    return 1;
  }

  vtkIntArray* materialArray =
    vtkIntArray::SafeDownCast(this->GetInputArrayToProcess(0, meshinput));
  if (!materialArray)
  {
    vtkErrorMacro("Failed to locate material array from input.");
    return 0;
  }

  vtkSelection* sel = vtkSelection::SafeDownCast(selInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSelectionNode* node = 0;
  if (sel->GetNumberOfNodes() > 0)
  {
    node = sel->GetNode(0);
  }
  if (!node)
  {
    vtkErrorMacro("Selection must have a single node.");
    return 0;
  }

  vtkSelection* idxSel = NULL;
  if (node->GetContentType() != vtkSelectionNode::INDICES)
  {
    idxSel = vtkConvertSelection::ToIndexSelection(sel, input);
    node = idxSel->GetNode(0);
  }

  vtkIntArray* materialIds = materialArray->NewInstance();
  materialIds->DeepCopy(materialArray);
  materialIds->SetName(materialArray->GetName());
  // Get the original "Cell ID" array
  vtkIdTypeArray* cellIdArray =
    vtkIdTypeArray::SafeDownCast(input->GetCellData()->GetArray("Mesh Cell ID"));

  int res = this->ModifySelectedCellRegions(node, materialIds, cellIdArray);
  if (res)
  {
    output->ShallowCopy(meshinput);
    output->GetCellData()->RemoveArray(materialArray->GetName());
    output->GetCellData()->AddArray(materialIds);
  }

  materialIds->Delete();
  if (idxSel)
  {
    idxSel->Delete();
  }
  this->IsNewRegionIdSet = 0;

  return res;
}

//-----------------------------------------------------------------------------
int vtkGMSMeshSelectionRegionFilter::ModifySelectedCellRegions(
  vtkSelectionNode* selNode, vtkIntArray* outArray, vtkIdTypeArray* cellIDArray)
{
  if (!outArray || !cellIDArray || !selNode)
  {
    return 0;
  }
  int seltype = selNode->GetContentType();
  vtkIdType selId;
  switch (seltype)
  {
    //    case vtkSelectionNode::GLOBALIDS:
    //    case vtkSelectionNode::PEDIGREEIDS:
    //    case vtkSelectionNode::VALUES:
    //    case vtkSelectionNode::FRUSTUM:
    case vtkSelectionNode::INDICES:
    {
      vtkIdTypeArray* selArray = vtkIdTypeArray::SafeDownCast(selNode->GetSelectionList());
      if (!selArray)
      {
        vtkErrorMacro("No IDs found from Selection.");
        return 0;
      }
      vtkIdType numCells = cellIDArray->GetNumberOfTuples();
      vtkIdType cellId;
      vtkIdType numSelCells = selArray->GetNumberOfTuples();
      vtkInformation* oProperties = selNode->GetProperties();
      if (oProperties->Get(vtkSelectionNode::INVERSE()))
      {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        vtkIdType* idsP = ids->WritePointer(0, numSelCells);
        memcpy(idsP, selArray->GetPointer(0), numSelCells * sizeof(vtkIdType));
        for (vtkIdType i = 0; i < numCells; i++)
        {
          cellId = cellIDArray->GetValue(i);
          if (ids->IsId(cellId) < 0)
          {
            // we need to map this selId back to the cellId
            outArray->SetValue(cellId, this->SelectionRegionId);
          }
        }
      }
      else
      {
        for (vtkIdType i = 0; i < numSelCells; i++)
        {
          selId = selArray->GetValue(i);
          // we need to map this selId back to the cellId
          cellId = cellIDArray->GetValue(selId);
          outArray->SetValue(cellId, this->SelectionRegionId);
        }
      }
    }
    break;
    default:
      vtkErrorMacro(
        "Wrong type of selection from input; only GlobalIDs type selection in supported.");
      return 0;
      break;
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkGMSMeshSelectionRegionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "IsNewRegionIdSet: " << this->IsNewRegionIdSet << endl;
  os << indent << "SelectionRegionId: " << this->SelectionRegionId << endl;
}
