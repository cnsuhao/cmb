//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMeshContourSelector.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkConvertSelection.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImplicitSelectionLoop.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMergePoints.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkStandardNewMacro(vtkCMBMeshContourSelector);
vtkCxxSetObjectMacro(vtkCMBMeshContourSelector, Contour, vtkImplicitSelectionLoop);

vtkCMBMeshContourSelector::vtkCMBMeshContourSelector()
{
  this->SetNumberOfInputPorts(3);

  this->Contour = NULL;
  this->IsProcessing = false;
  this->GenerateSelectedOutput = 0;
  this->SelectionFieldType = CELL;
  this->SelectCellThrough = 0;
  this->InsideOut = 0;
  this->SelectContourType = ALL_IN;
  this->SetNumberOfOutputPorts(2);
  this->SelectionPolyData = NULL;
  this->IsSelectionEmpty = 1;
  this->OrientationOfSelectedNodes[0] = this->OrientationOfSelectedNodes[1] =
    this->OrientationOfSelectedNodes[2] = 0.0;
}

vtkCMBMeshContourSelector::~vtkCMBMeshContourSelector()
{
  this->SetContour(NULL);
  if (this->SelectionPolyData)
  {
    this->SelectionPolyData->Delete();
  }
}

// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
vtkMTimeType vtkCMBMeshContourSelector::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  vtkMTimeType time;

  if (this->Contour != NULL)
  {
    time = this->Contour->GetMTime();
    mTime = (time > mTime ? time : mTime);
  }

  return mTime;
}

int vtkCMBMeshContourSelector::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkSelection");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  return 0;
}

vtkPolyData* vtkCMBMeshContourSelector::GetSelectionPolyData()
{
  //return this->SelectionPolyData;
  return vtkPolyData::SafeDownCast(this->GetOutputDataObject(1));
}

int vtkCMBMeshContourSelector::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  if (this->IsProcessing)
  {
    vtkErrorMacro(
      << "vtkCMBMeshContourSelector::RequestData is called again while it is still processing");
    return 0;
  }
  this->IsProcessing = true;

  // get the input and output
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  // expecting the original mesh input
  vtkUnstructuredGrid* input =
    vtkUnstructuredGrid::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkIdType numPts = input->GetNumberOfPoints();
  vtkIdType numCells = input->GetNumberOfCells();
  vtkPoints* inPts = input->GetPoints();
  bool bVolume = (input->GetCell(0)->GetCellType() == VTK_TETRA) ? true : false;
  // Initialize self; create output objects
  //
  if (numPts < 1 || inPts == NULL)
  {
    vtkWarningMacro(<< "No input data");
    this->IsProcessing = false;
    return 1;
  }
  // The contour could be null for surface selection.
  // If this is just a rubber band surface selection,
  // we will just grab all the surface nodes of this cell selection
  if (this->Contour == NULL && this->SelectCellThrough)
  {
    vtkWarningMacro(<< "Contour is not set for selecting cell through");
    this->IsProcessing = false;
    return 0;
  }
  // Get the original "Cell ID" array
  vtkIdTypeArray* meshCellIdArray =
    vtkIdTypeArray::SafeDownCast(input->GetCellData()->GetArray("Mesh Cell ID"));
  if (!meshCellIdArray)
  {
    vtkErrorMacro(<< "The Mesh Cell ID array is missing from input.");
    this->IsProcessing = false;
    return 0;
  }
  vtkIdTypeArray* meshNodeIdArray =
    vtkIdTypeArray::SafeDownCast(input->GetPointData()->GetArray("Mesh Node ID"));
  if (!meshNodeIdArray)
  {
    vtkErrorMacro(<< "The Mesh Node ID array is missing from input.");
    this->IsProcessing = false;
    return 0;
  }

  // get the info objects
  vtkSelection* output = vtkSelection::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  //  vtkSelection* output = vtkSelection::GetData(outputVector);
  vtkSelectionNode* selNode = vtkSelectionNode::New();
  vtkInformation* oProperties = selNode->GetProperties();
  oProperties->Set(vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::INDICES);
  oProperties->Set(vtkSelectionNode::FIELD_TYPE(), this->SelectionFieldType);
  output->AddNode(selNode);
  this->IsSelectionEmpty = 1;
  // Create the selection list
  vtkIdTypeArray* outSelectionList = vtkIdTypeArray::New();
  outSelectionList->SetNumberOfComponents(1);
  vtkIdType numSelIds = 0;
  vtkIdType i;
  if (this->SelectCellThrough && this->Contour)
  {
    vtkIdType npts, *pts;
    for (i = 0; i < numCells; i++)
    {
      input->GetCellPoints(i, npts, pts);
      if (this->DoCellContourCheck(npts, pts, input))
      {
        outSelectionList->InsertNextValue(i);
      }
    }
  }
  // if we are NOT doing select through, then we need a selection that
  // contains all the surface cells/points selected based on the contour's boundary or a rubber band
  // This selection of cell/point Id is based on the input mesh. This filter
  // will then do the contour function check on those cells/points, and if this is a volume
  // mesh, a surface mesh will be extracted to identify surface nodes of the volume mesh.
  else
  {
    vtkSelection* selInput = NULL;
    vtkInformation* selInfo = inputVector[1]->GetInformationObject(0);
    if (selInfo)
    {
      selInput = vtkSelection::SafeDownCast(selInfo->Get(vtkDataObject::DATA_OBJECT()));
    }
    vtkSelectionNode* node =
      selInput && selInput->GetNumberOfNodes() > 0 ? selInput->GetNode(0) : 0;
    if (!node)
    {
      vtkErrorMacro("Input selection must have a single node.");
      this->IsProcessing = false;
      return 0;
    }
    vtkSelection* idxSel = NULL;
    if (node->GetContentType() != vtkSelectionNode::INDICES)
    {
      idxSel = vtkConvertSelection::ToIndexSelection(selInput, input);
      node = idxSel->GetNode(0);
    }
    vtkIdTypeArray* selArray = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
    if (!selArray)
    {
      vtkErrorMacro("No IDs found from Selection.");
      this->IsProcessing = false;
      return 0;
    }

    // Extract the surface if it is a volume and we are not doing select through
    vtkSmartPointer<vtkIdList> SurfaceNodeIdList = 0;
    if (bVolume)
    {
      vtkPolyData* surfaceInput = 0;
      vtkInformation* surfaceInfo = inputVector[2]->GetInformationObject(0);
      if (surfaceInfo)
      {
        surfaceInput = vtkPolyData::SafeDownCast(surfaceInfo->Get(vtkDataObject::DATA_OBJECT()));
      }
      if (!surfaceInput)
      {
        vtkNew<vtkDataSetSurfaceFilter> SurfaceFilter;
        SurfaceFilter->SetInputData(input);
        SurfaceFilter->Update();
        surfaceInput = SurfaceFilter->GetOutput();
      }

      vtkIdTypeArray* SurfaceNodeIdArray =
        vtkIdTypeArray::SafeDownCast(surfaceInput->GetPointData()->GetArray("Mesh Node ID"));
      if (!SurfaceNodeIdArray)
      {
        vtkErrorMacro(<< "The Mesh Node ID array is missing from input.");
        this->IsProcessing = false;
        return 0;
      }
      vtkIdType numSurfaceNodes = SurfaceNodeIdArray->GetNumberOfTuples();
      SurfaceNodeIdList = vtkSmartPointer<vtkIdList>::New();
      vtkIdType* idsP = SurfaceNodeIdList->WritePointer(0, numSurfaceNodes);
      memcpy(idsP, SurfaceNodeIdArray->GetPointer(0), numSurfaceNodes * sizeof(vtkIdType));
    }

    vtkNew<vtkIdList> outNodeIdList;
    vtkIdType estimatedSize = numPts / 2;
    vtkInformation* outPolyInfo = outputVector->GetInformationObject(1);
    vtkPolyData* outSelectedPD =
      vtkPolyData::SafeDownCast(outPolyInfo->Get(vtkDataObject::DATA_OBJECT()));

    // For surface mesh, this array only contains the mesh cell Ids
    // that are partially in the contour.
    vtkNew<vtkIdList> outMeshCellIds;
    vtkNew<vtkPoints> newPoints;
    vtkNew<vtkCellArray> outVerts;
    if (this->GenerateSelectedOutput && outSelectedPD)
    {
      outSelectedPD->Initialize();
      // Lets allocate the arrays
      newPoints->SetDataTypeToDouble();
      newPoints->Allocate(estimatedSize);
      outSelectedPD->SetPoints(newPoints.GetPointer());
      outSelectedPD->SetVerts(outVerts.GetPointer());
    }

    vtkIdType selid;
    double totNormal[3] = { 0.0, 0.0, 0.0 };
    int totNumNormals = 0;
    vtkNew<vtkIdList> tmpIds;
    numSelIds = selArray->GetNumberOfTuples();
    for (i = 0; i < numSelIds; i++)
    {
      selid = selArray->GetValue(i);
      this->DoSurfaceSelectionCheck(node->GetFieldType(), tmpIds.GetPointer(), bVolume, selid,
        input, meshCellIdArray, meshNodeIdArray, newPoints.GetPointer(), outVerts.GetPointer(),
        outMeshCellIds.GetPointer(), outNodeIdList.GetPointer(), SurfaceNodeIdList,
        outSelectionList, totNormal, totNumNormals);
    }

    if (totNumNormals > 0)
    {
      bool zeroNormal = totNormal[0] == 0.0 && totNormal[1] == 0.0 && totNormal[2] == 0.0;
      double contourNormal[3];
      if (zeroNormal && this->Contour)
      {
        vtkPolygon::ComputeNormal(this->Contour->GetLoop(), contourNormal);
      }
      for (int n = 0; n < 3; ++n)
      {
        this->OrientationOfSelectedNodes[n] =
          zeroNormal ? contourNormal[n] : (totNormal[n] / static_cast<double>(totNumNormals));
      }
    }
    if (this->GenerateSelectedOutput && outSelectedPD)
    {
      vtkIdTypeArray* outMeshCellArray = vtkIdTypeArray::New();
      outMeshCellArray->SetName("Mesh Cell ID");
      vtkIdTypeArray* outNodeIdArray = vtkIdTypeArray::New();
      outNodeIdArray->SetName("Mesh Node ID");
      vtkIdType numIds = outMeshCellIds->GetNumberOfIds();
      outMeshCellArray->SetNumberOfComponents(1);
      outMeshCellArray->SetNumberOfTuples(numIds);
      for (int n = 0; n < numIds; n++)
      {
        outMeshCellArray->SetValue(n, outMeshCellIds->GetId(n));
      }
      numIds = outNodeIdList->GetNumberOfIds();
      outNodeIdArray->SetNumberOfComponents(1);
      outNodeIdArray->SetNumberOfTuples(numIds);
      for (int n = 0; n < numIds; n++)
      {
        outNodeIdArray->SetValue(n, outNodeIdList->GetId(n));
      }
      outSelectedPD->GetFieldData()->AddArray(outMeshCellArray);
      outSelectedPD->GetPointData()->AddArray(outNodeIdArray);
      outMeshCellArray->Delete();
      outNodeIdArray->Delete();
      outSelectedPD->Squeeze();
    }
  }

  if (outSelectionList->GetNumberOfTuples() > 0)
  {
    selNode->SetSelectionList(outSelectionList);
    oProperties->Set(vtkSelectionNode::CONTAINING_CELLS(), 0);
    oProperties->Set(vtkSelectionNode::INVERSE(), this->InsideOut);
    if (selNode->GetSelectionList())
    {
      selNode->GetSelectionList()->SetName("IDs");
    }
  }

  numSelIds = this->Contour ? outSelectionList->GetNumberOfTuples() : numSelIds;
  vtkIdType numIds = this->SelectionFieldType == vtkSelectionNode::CELL ? numCells : numPts;
  if ((!this->InsideOut && numSelIds > 0) || (this->InsideOut && numSelIds < numIds))
  {
    this->IsSelectionEmpty = 0;
  }
  outSelectionList->Delete();
  selNode->Delete();
  this->IsProcessing = false;

  return 1;
}

int vtkCMBMeshContourSelector::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  }
  else if (port == 2)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  }
  return 1;
}

void vtkCMBMeshContourSelector::DoSurfaceSelectionCheck(int selType, vtkIdList* tmpIds,
  bool bVolume, vtkIdType selId, vtkUnstructuredGrid* input, vtkIdTypeArray* meshCellIdArray,
  vtkIdTypeArray* meshNodeIdArray, vtkPoints* newPoints, vtkCellArray* outVerts,
  vtkIdList* outMeshCellIds, vtkIdList* outNodeIdList, vtkIdList* surfaceNodeList,
  vtkIdTypeArray* outSelectionList, double* totNormal, int& totNumNormals)
{
  vtkIdType npts, *pts;
  double point[3], *pointPtr;
  bool bKeep = false;
  vtkIdType meshCellId, meshNodeId;
  vtkIdType nextPt;
  double mNormal[3] = { 0.0, 0.0, 0.0 };
  double tmpPts[4][3];
  double aNormal[3] = { 0.0, 0.0, 0.0 };

  if (selType == vtkSelectionNode::CELL)
  {
    meshCellId = meshCellIdArray->GetValue(selId);
    if (outMeshCellIds->IsId(meshCellId) >= 0) // already done
    {
      return;
    }
    input->GetCellPoints(selId, npts, pts);
    if (bVolume && surfaceNodeList) // only check the surface points
    {
      tmpIds->Initialize();
      for (vtkIdType n = 0; n < npts; n++)
      {
        meshNodeId = meshNodeIdArray->GetValue(pts[n]);
        if (surfaceNodeList->IsId(meshNodeId) >= 0)
        {
          tmpIds->InsertUniqueId(pts[n]);
        }
      }
      npts = tmpIds->GetNumberOfIds();
      pts = tmpIds->GetPointer(0);
    }
    if (this->Contour) // check against contour
    {
      if (this->DoCellContourCheck(npts, pts, input))
      {
        outSelectionList->InsertNextValue(selId);
        bKeep = true;
      }
    }
    else
    // This is a rubber band selection, just grab the points,
    // and no need to create a new selection list
    {
      bKeep = true;
    }
    if (bKeep && this->GenerateSelectedOutput)
    {
      outMeshCellIds->InsertUniqueId(meshCellId);
      for (vtkIdType n = 0; n < npts; n++)
      {
        meshNodeId = meshNodeIdArray->GetValue(pts[n]);
        if (outNodeIdList->IsId(meshNodeId) >= 0)
        {
          continue;
        }
        outNodeIdList->InsertUniqueId(meshNodeId);
        input->GetPoint(pts[n], tmpPts[n]);
        pointPtr = tmpPts[n];
        nextPt = newPoints->InsertNextPoint(pointPtr);
        outVerts->InsertNextCell(1, &nextPt);
      }
      // compute the average normal for the point
      if (npts == 3 || npts == 4)
      {
        vtkTriangle::ComputeNormal(tmpPts[0], tmpPts[1], tmpPts[2], mNormal);
        if (npts == 4 && mNormal[0] == 0.0 && mNormal[1] == 0.0 && mNormal[2] == 0.0)
        {
          vtkTriangle::ComputeNormal(tmpPts[1], tmpPts[2], tmpPts[3], mNormal);
        }
        totNormal[0] += mNormal[0];
        totNormal[1] += mNormal[1];
        totNormal[2] += mNormal[2];
        totNumNormals++;
      }
    }
  }
  else if (selType == vtkSelectionNode::POINT)
  {
    meshNodeId = meshNodeIdArray->GetValue(selId);
    if (outNodeIdList->IsId(meshNodeId) >= 0) // already done
    {
      return;
    }
    input->GetPoint(selId, point);
    pointPtr = point;
    if (this->Contour)
    {
      double s = this->Contour->FunctionValue(pointPtr);
      if (s < 0) // point is inside contour
      {
        bKeep = true;
        outSelectionList->InsertNextValue(selId);
      }
    }
    else //Just a rubber band selection of surface points
    {
      bKeep = true;
    }
    if (bKeep && this->GenerateSelectedOutput)
    {
      tmpIds->Initialize();
      outNodeIdList->InsertUniqueId(meshNodeId);
      nextPt = newPoints->InsertNextPoint(pointPtr);
      outVerts->InsertNextCell(1, &nextPt);
      // ideally, for surface mesh, we should only check the cells
      // that are only partially inside
      input->GetPointCells(selId, tmpIds);
      vtkIdType currentCellId;
      vtkIdType numCells = tmpIds->GetNumberOfIds();
      int numUsedCells = 0;
      for (vtkIdType i = 0; i < numCells; i++)
      {
        currentCellId = tmpIds->GetId(i);
        meshCellId = meshCellIdArray->GetValue(currentCellId);
        if (outMeshCellIds->IsId(meshCellId) >= 0) // done
        {
          continue;
        }
        input->GetCellPoints(currentCellId, npts, pts);
        vtkSmartPointer<vtkIdList> ptsIds = vtkSmartPointer<vtkIdList>::New();
        if (bVolume && surfaceNodeList)
        {
          vtkIdType ptMeshId;
          for (vtkIdType n = 0; n < npts; n++)
          {
            ptMeshId = meshNodeIdArray->GetValue(pts[n]);
            if (surfaceNodeList->IsId(ptMeshId) >= 0)
            {
              ptsIds->InsertNextId(pts[n]);
            }
          }
          npts = ptsIds->GetNumberOfIds();
          pts = ptsIds->GetPointer(0);
        }

        // if this is surface, we only add in the cells that
        // are partially in, which are the only cells that need to validate
        // for moving mesh nodes.
        if (npts == 3 || npts == 4)
        // The following logic needs to be investigated more
        {
          // compute the average normal for the point
          for (vtkIdType n = 0; n < npts; n++)
          {
            input->GetPoint(pts[n], tmpPts[n]);
            /* // Let's keep everything for now
            // if any point is outside, keep it
            if(this->Contour->FunctionValue(tmpPts[n])>0)
              {
              keep = true;
              // do not break, in order to calculate average normal
              }
            */
          }
          vtkTriangle::ComputeNormal(tmpPts[0], tmpPts[1], tmpPts[2], mNormal);
          if (npts == 4 && mNormal[0] == 0.0 && mNormal[1] == 0.0 && mNormal[2] == 0.0)
          {
            vtkTriangle::ComputeNormal(tmpPts[1], tmpPts[2], tmpPts[3], mNormal);
          }
          aNormal[0] += mNormal[0];
          aNormal[1] += mNormal[1];
          aNormal[2] += mNormal[2];
          numUsedCells++;
        }
        outMeshCellIds->InsertUniqueId(meshCellId);
      }
      if (numUsedCells > 0)
      {
        aNormal[0] /= numUsedCells;
        aNormal[1] /= numUsedCells;
        aNormal[2] /= numUsedCells;
        totNormal[0] += aNormal[0];
        totNormal[1] += aNormal[1];
        totNormal[2] += aNormal[2];
        totNumNormals++;
      }
    }
  }
}

inline bool IsAllIn(vtkIdType npts, vtkIdType* pts, double* point, vtkImplicitSelectionLoop* cFunc,
  vtkUnstructuredGrid* input)
{
  double s;
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
  {
    input->GetPoint(pts[ptIndex], point);
    s = cFunc->FunctionValue(point);
    if (s >= 0)
    {
      return false;
    }
  }
  return true;
}
inline bool IsAllOut(vtkIdType npts, vtkIdType* pts, double* point, vtkImplicitSelectionLoop* cFunc,
  vtkUnstructuredGrid* input)
{
  double s;
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
  {
    input->GetPoint(pts[ptIndex], point);
    s = cFunc->FunctionValue(point);
    if (s < 0)
    {
      return false;
    }
  }
  return true;
}
inline bool IsIntersecting(vtkIdType npts, vtkIdType* pts, double* point,
  vtkImplicitSelectionLoop* cFunc, vtkUnstructuredGrid* input)
{
  double s;
  bool partIn = false;
  bool partOut = false;
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
  {
    input->GetPoint(pts[ptIndex], point);
    s = cFunc->FunctionValue(point);
    if (s < 0)
    {
      partIn = true;
    }
    else
    {
      partOut = true;
    }
  }
  return partIn && partOut;
}

bool vtkCMBMeshContourSelector::DoCellContourCheck(
  vtkIdType npts, vtkIdType* pts, vtkUnstructuredGrid* input)
{
  double point[3];
  bool bKeep = false;
  if (npts <= 0 || !pts || !input)
  {
    return bKeep;
  }
  switch (this->SelectContourType)
  {
    case ALL_IN:
      bKeep = IsAllIn(npts, pts, point, this->Contour, input);
      break;
    case INTERSECT_ONLY:
      bKeep = IsIntersecting(npts, pts, point, this->Contour, input);
      break;
    case PARTIAL_OR_ALL_IN:
      bKeep = !IsAllOut(npts, pts, point, this->Contour, input);
      break;
  }
  return bKeep;
}

void vtkCMBMeshContourSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SelectCellThrough: " << this->SelectCellThrough << "\n";
  os << indent << "SelectContourType: " << this->SelectContourType << "\n";
  os << indent << "SelectionFieldType: " << this->SelectionFieldType << "\n";
  os << indent << "InsideOut: " << this->InsideOut << "\n";
  os << indent << "IsSelectionEmpty: " << this->IsSelectionEmpty << "\n";
  os << indent << "GenerateSelectedOutput: " << this->GenerateSelectedOutput << "\n";
  for (int i = 0; i < 3; i++)
  {
    os << indent << "OrientationOfSelectedNodes[" << i
       << "]: " << this->OrientationOfSelectedNodes[i] << "\n";
  }

  if (this->Contour)
  {
    os << indent << "Contour: " << this->Contour << "\n";
  }
  else
  {
    os << indent << "Contour: (none)\n";
  }
  if (this->SelectionPolyData)
  {
    os << indent << "SelectionPolyData: " << this->SelectionPolyData << "\n";
  }
  else
  {
    os << indent << "SelectionPolyData: (none)\n";
  }
}
