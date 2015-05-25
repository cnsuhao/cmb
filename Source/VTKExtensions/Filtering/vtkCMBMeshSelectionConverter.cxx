//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMeshSelectionConverter.h"

#include "vtkCellData.h"
#include "vtkConvertSelection.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPointData.h"
#include "vtkIdList.h"

vtkStandardNewMacro(vtkCMBMeshSelectionConverter);

//----------------------------------------------------------------------------
vtkCMBMeshSelectionConverter::vtkCMBMeshSelectionConverter()
{
  this->SetNumberOfInputPorts(3);
}

//----------------------------------------------------------------------------
vtkCMBMeshSelectionConverter::~vtkCMBMeshSelectionConverter()
{
}

//----------------------------------------------------------------------------
int vtkCMBMeshSelectionConverter::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port==0)
    {
    // Can work with composite datasets.
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    }
  else if(port==1)
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
int vtkCMBMeshSelectionConverter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *selInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *meshInfo = inputVector[2]->GetInformationObject(0);

  vtkSelection* output = vtkSelection::GetData(outputVector);
  vtkSelectionNode* selNode = vtkSelectionNode::New();
  vtkInformation* oProperties = selNode->GetProperties();
  oProperties->Set(vtkSelectionNode::CONTENT_TYPE(),
    vtkSelectionNode::INDICES);
  //oProperties->Set(vtkSelectionNode::FIELD_TYPE(), vtkSelectionNode::CELL);
  output->AddNode(selNode);
  selNode->Delete();

  // verify the input, selection and output
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::GetData(inInfo);
  if ( ! input )
    {
    vtkErrorMacro(<<"Currently, only expecting vtkUnstructuredGrid input.");
    return 0;
    }
  vtkUnstructuredGrid *meshinput = vtkUnstructuredGrid::GetData(meshInfo);
  if ( ! meshinput )
    {
    vtkErrorMacro(<<"Need a mesh input.");
    return 0;
    }

  vtkSelection *sel = vtkSelection::SafeDownCast(
    selInfo->Get(vtkDataObject::DATA_OBJECT()));
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
  if(node->GetContentType() != vtkSelectionNode::INDICES)
    {
    idxSel = vtkConvertSelection::ToIndexSelection(sel, input);
    node = idxSel->GetNode(0);
    }

  vtkInformation* iProperties = node->GetProperties();
  int fieldType = iProperties->Get(vtkSelectionNode::FIELD_TYPE());
  oProperties->Set(vtkSelectionNode::FIELD_TYPE(), fieldType);

  // Create the selection list
  vtkIdTypeArray* outSelectionList = vtkIdTypeArray::New();
  outSelectionList->SetNumberOfComponents(1);

  int res = this->CreateSelectionLists(node, outSelectionList, fieldType,input);
  if(res)
    {
    selNode->SetSelectionList(outSelectionList);
    oProperties->Set(vtkSelectionNode::CONTAINING_CELLS(),1);
    oProperties->Set(vtkSelectionNode::INVERSE(),0);
    if (selNode->GetSelectionList())
      {
      selNode->GetSelectionList()->SetName("IDs");
      }
    }

  outSelectionList->Delete();
  if(idxSel)
    {
    idxSel->Delete();
    }

  return res;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshSelectionConverter::CreateSelectionLists(
  vtkSelectionNode* selNode, vtkIdTypeArray* outSelectionList,
  int fieldType, vtkUnstructuredGrid* meshInput)
{
  if(!outSelectionList || !meshInput || !selNode)
    {
    return 0;
    }

  // Get the original "Cell ID" array
  vtkIdTypeArray* origIdArray = NULL;
  if(fieldType == vtkSelectionNode::CELL)
    {
    origIdArray =vtkIdTypeArray::SafeDownCast(
      meshInput->GetCellData()->GetArray("Mesh Cell ID"));
    }
  else if(fieldType == vtkSelectionNode::POINT)
    {
    origIdArray =vtkIdTypeArray::SafeDownCast(
      meshInput->GetPointData()->GetArray("Mesh Node ID"));
    }
  if ( ! origIdArray )
    {
    vtkDebugMacro(<<"Using the celll or point index for selection.");
//    return 0;
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
      vtkIdTypeArray* selArray = vtkIdTypeArray::SafeDownCast(
        selNode->GetSelectionList());
      if(!selArray)
        {
        vtkErrorMacro("No IDs found from Selection.");
        return 0;
        }
      vtkIdType origId;
      vtkIdType numCells = origIdArray->GetNumberOfTuples();
      vtkIdType numSelCells = selArray->GetNumberOfTuples();
      vtkIdType cellId;
      vtkInformation* oProperties = selNode->GetProperties();
      if(oProperties->Get(vtkSelectionNode::INVERSE()))
        {
        vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
        vtkIdType* idsP = ids->WritePointer(0, numSelCells);
        memcpy(idsP, selArray->GetPointer(0), numSelCells*sizeof(vtkIdType));
        for(vtkIdType i=0;i<numCells;i++)
          {
          if(ids->IsId(i)<0)
            {
            cellId = origIdArray ? origIdArray->GetValue(i) : i;
            outSelectionList->InsertNextValue(cellId);
            }
          }
        }
      else
        {
        for(vtkIdType i=0;i<numSelCells;i++)
          {
          selId = selArray->GetValue(i);
          // we need to map this selId back to the origId
          // vtk id is zero-based
          origId = origIdArray ? origIdArray->GetValue(selId) : selId;
          outSelectionList->InsertNextValue(origId);
          }
        }
      }
      break;
    default:
      vtkErrorMacro("Wrong type of selection from input; only GlobalIDs type selection in supported.");
      return 0;
      break;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBMeshSelectionConverter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
