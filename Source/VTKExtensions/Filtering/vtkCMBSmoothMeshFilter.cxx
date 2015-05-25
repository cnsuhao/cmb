//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBSmoothMeshFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkSelectionNode.h"
#include "vtkUnstructuredGrid.h"
#include "vtkConvertSelection.h"
#include "vtkPolygon.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkSmartPointer.h"

#include <math.h>

vtkStandardNewMacro(vtkCMBSmoothMeshFilter);

//----------------------------------------------------------------------------
vtkCMBSmoothMeshFilter::vtkCMBSmoothMeshFilter()
{
  this->SetNumberOfInputPorts(3);

  this->Convergence = 0.0; //goes to number of specified iterations
  this->NumberOfIterations = 20;

  this->RelaxationFactor = .01;

  this->FeatureAngle = 45.0;
  this->EdgeAngle = 15.0;
  this->FeatureEdgeSmoothing = 0;
  this->BoundarySmoothing = 1;
  this->SmoothPolyFilter = NULL;
}

//----------------------------------------------------------------------------
vtkCMBSmoothMeshFilter::~vtkCMBSmoothMeshFilter()
{
  if(this->SmoothPolyFilter)
    {
    this->SmoothPolyFilter->Delete();
    }
}

//----------------------------------------------------------------------------
int vtkCMBSmoothMeshFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the input and output
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  // expecting the original mesh input
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkIdType numCells = input->GetNumberOfCells();
  vtkPoints *inPts=input->GetPoints();
  bool bVolume = (input->GetCell(0)->GetCellType() == VTK_TETRA) ? true : false;
  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL)
    {
    vtkWarningMacro(<<"No input data");
    return 1;
    }

  // Get the original "Cell ID" array
  vtkIdTypeArray* meshCellIdArray =vtkIdTypeArray::SafeDownCast(
    input->GetCellData()->GetArray("Mesh Cell ID"));
  if ( ! meshCellIdArray )
    {
    vtkErrorMacro(<<"The Mesh Cell ID array is missing from input.");
    return 0;
    }
  vtkIdTypeArray* meshNodeIdArray =vtkIdTypeArray::SafeDownCast(
    input->GetPointData()->GetArray("Mesh Node ID"));
  if ( ! meshNodeIdArray )
    {
    vtkErrorMacro(<<"The Mesh Node ID array is missing from input.");
    return 0;
    }

  // get the info objects
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  vtkIdType i;
  vtkSelection* selInput = NULL;
  vtkInformation *selInfo = inputVector[1]->GetInformationObject(0);
  if(selInfo)
    {
    selInput = vtkSelection::SafeDownCast(
      selInfo->Get(vtkDataObject::DATA_OBJECT()));
    }
  vtkSelectionNode* node = selInput && selInput->GetNumberOfNodes()>0 ?
    selInput->GetNode(0) : 0;
  if (!node)
    {
    vtkErrorMacro("Input selection must have a single node.");
    return 0;
    }
  vtkSelection* idxSel = NULL;
  if(node->GetContentType() != vtkSelectionNode::INDICES)
    {
    idxSel = vtkConvertSelection::ToIndexSelection(selInput, input);
    node = idxSel->GetNode(0);
    }
  vtkIdTypeArray* selArray = vtkIdTypeArray::SafeDownCast(
    node->GetSelectionList());
  if(!selArray)
    {
    if(idxSel)
      {
      idxSel->Delete();
      }
    vtkErrorMacro("No IDs found from Selection.");
    return 0;
    }

  // Extract the surface if it is a volume
  vtkSmartPointer<vtkIdList> SurfaceNodeIdList=0;
  if(bVolume)
    {
    vtkPolyData* surfaceInput = 0;
    vtkInformation *surfaceInfo = inputVector[2]->GetInformationObject(0);
    if(surfaceInfo)
      {
      surfaceInput = vtkPolyData::SafeDownCast(
        surfaceInfo->Get(vtkDataObject::DATA_OBJECT()));
      }
    if(!surfaceInput)
      {
      vtkSmartPointer<vtkDataSetSurfaceFilter> SurfaceFilter =
        vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
      SurfaceFilter->SetInputData(input);
      SurfaceFilter->Update();
      surfaceInput = SurfaceFilter->GetOutput();
      }

    vtkIdTypeArray* SurfaceNodeIdArray =vtkIdTypeArray::SafeDownCast(
      surfaceInput->GetPointData()->GetArray("Mesh Node ID"));
    if ( !SurfaceNodeIdArray )
      {
      vtkErrorMacro(<<"The Mesh Node ID array is missing from input.");
      return 0;
      }
    vtkIdType numSurfaceNodes = SurfaceNodeIdArray->GetNumberOfTuples();
    SurfaceNodeIdList = vtkSmartPointer<vtkIdList>::New();
    vtkIdType* idsP = SurfaceNodeIdList->WritePointer(0, numSurfaceNodes);
    memcpy(idsP, SurfaceNodeIdArray->GetPointer(0), numSurfaceNodes*sizeof(vtkIdType));
    }

  vtkSmartPointer<vtkIdList> outNodeIdList = vtkSmartPointer<vtkIdList>::New();
  vtkIdType estimatedSize = numPts/2;

  // For surface mesh, this array only contains the mesh cell Ids
  // that are partially in the contour.
  vtkSmartPointer<vtkIdList> outMeshCellIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> outVerts = vtkSmartPointer<vtkCellArray>::New();

  vtkSmartPointer<vtkPolyData> inSmoothPD = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> smoothPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> smoothPolys = vtkSmartPointer<vtkCellArray>::New();
  inSmoothPD->Initialize();
  // Lets allocate the arrays
  smoothPoints->SetDataTypeToDouble();
  smoothPoints->Allocate(estimatedSize);
  inSmoothPD->SetPoints(smoothPoints);
  smoothPolys->Allocate(4*numCells,numCells/2);
  inSmoothPD->SetPolys(smoothPolys);

  vtkIdType selid, currentCellId;
  vtkSmartPointer<vtkIdList> tmpIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdList> nxtPts = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdList> ptCellIds = vtkSmartPointer<vtkIdList>::New();

  for(i=0;i<selArray->GetNumberOfTuples();i++)
    {
    selid = selArray->GetValue(i);
    if(selid < numCells && node->GetFieldType() == vtkSelectionNode::CELL)
      {
      this->ExtractSurfaceCells(bVolume, tmpIds,
        nxtPts, selid, input, meshCellIdArray,
        meshNodeIdArray, smoothPoints, smoothPolys,
        outMeshCellIds, outNodeIdList, SurfaceNodeIdList);
      }
    // this will also smooth other surface points that are
    // part of the same cells as selected points, because
    // the vtkSmoothPolyDataFilter does not smooth vertex.
    else if(selid < numPts && node->GetFieldType() == vtkSelectionNode::POINT)
      {
      ptCellIds->Initialize();
      input->GetPointCells(selid, ptCellIds);
      for ( vtkIdType id=0;id<ptCellIds->GetNumberOfIds(); id++)
        {
        currentCellId=ptCellIds->GetId(id);
        this->ExtractSurfaceCells(bVolume, tmpIds,
          nxtPts, currentCellId, input, meshCellIdArray,
          meshNodeIdArray, smoothPoints, smoothPolys,
          outMeshCellIds, outNodeIdList, SurfaceNodeIdList);
        }
      }
    }
  if (!outMeshCellIds->GetNumberOfIds() || !outNodeIdList->GetNumberOfIds() )
    {
    if(idxSel)
      {
      idxSel->Delete();
      }
    vtkErrorMacro(<<"Failed to create polydata as vtkSmoothPolyDataFilter input.");
    return 0;
    }

// Now use the smooth polydata filter to create the smoothed output.
  if(!this->SmoothPolyFilter)
    {
    this->SmoothPolyFilter = vtkSmoothPolyDataFilter::New();
    }
  this->SmoothPolyFilter->SetInputData(inSmoothPD);
  this->SmoothPolyFilter->SetConvergence(this->Convergence);
  this->SmoothPolyFilter->SetNumberOfIterations(this->NumberOfIterations);
  this->SmoothPolyFilter->SetRelaxationFactor(this->RelaxationFactor);
  this->SmoothPolyFilter->SetFeatureAngle(this->FeatureAngle);
  this->SmoothPolyFilter->SetEdgeAngle(this->EdgeAngle);
  this->SmoothPolyFilter->SetFeatureEdgeSmoothing(this->FeatureEdgeSmoothing);
  this->SmoothPolyFilter->SetBoundarySmoothing(this->BoundarySmoothing);
  this->SmoothPolyFilter->Update();

  vtkPolyData* outSmoothPD = this->SmoothPolyFilter->GetOutput();
  output->Initialize();
  output->ShallowCopy(outSmoothPD);

  vtkIdTypeArray* outMeshCellArray = vtkIdTypeArray::New();
  outMeshCellArray->SetName("Mesh Cell ID");
  vtkIdTypeArray* outNodeIdArray = vtkIdTypeArray::New();
  outNodeIdArray->SetName("Mesh Node ID");
  vtkIdType numIds = outMeshCellIds->GetNumberOfIds();
  outMeshCellArray->SetNumberOfComponents(1);
  outMeshCellArray->SetNumberOfTuples(numIds);
  for(int n=0; n<numIds; n++)
    {
    outMeshCellArray->SetValue(n, outMeshCellIds->GetId(n));
    }
  numIds = outNodeIdList->GetNumberOfIds();
  outNodeIdArray->SetNumberOfComponents(1);
  outNodeIdArray->SetNumberOfTuples(numIds);
  for(int n=0; n<numIds; n++)
    {
    outNodeIdArray->SetValue(n, outNodeIdList->GetId(n));
    }
  output->GetFieldData()->AddArray(outMeshCellArray);
  output->GetPointData()->AddArray(outNodeIdArray);
  outMeshCellArray->Delete();
  outNodeIdArray->Delete();
  output->Squeeze();
  if(idxSel)
    {
    idxSel->Delete();
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBSmoothMeshFilter::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port==0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    }
  else if(port==1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  else if(port==2)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}
//----------------------------------------------------------------------------
void vtkCMBSmoothMeshFilter::ExtractSurfaceCells(bool bVolume,
   vtkIdList* tmpIds, vtkIdList* nxtPts, vtkIdType cellId,
   vtkUnstructuredGrid* input, vtkIdTypeArray* meshCellIdArray,
   vtkIdTypeArray* meshNodeIdArray, vtkPoints* smoothPoints,
   vtkCellArray* smoothPolys, vtkIdList* outMeshCellIds,
   vtkIdList* outNodeIdList, vtkIdList* surfaceNodeList)
{
  vtkIdType meshCellId, meshNodeId;
  meshCellId = meshCellIdArray->GetValue(cellId);
  if(outMeshCellIds->IsId(meshCellId)>=0)
    {
    return;
    }

  vtkIdType npts, *pts;
  double point[3], *pointPtr;
  vtkIdType nextPt;

  input->GetCellPoints(cellId, npts, pts);
  if(bVolume && surfaceNodeList) // only check the surface points
    {
    tmpIds->Initialize();
    for(vtkIdType n=0; n<npts; n++)
      {
      meshNodeId = meshNodeIdArray->GetValue(pts[n]);
      if(surfaceNodeList->IsId(meshNodeId)>=0)
        {
        tmpIds->InsertUniqueId(pts[n]);
        }
      }
    npts = tmpIds->GetNumberOfIds();
    pts = tmpIds->GetPointer(0);
    }
  if(npts == 3 || npts == 4) // only handle triangle and quad
    {
    vtkIdType pid;
    nxtPts->Initialize();
    outMeshCellIds->InsertNextId(meshCellId);
    for(vtkIdType n=0; n<npts; n++)
      {
      input->GetPoint(pts[n], point);
      meshNodeId = meshNodeIdArray->GetValue(pts[n]);
      pointPtr = point;
      pid = outNodeIdList->IsId(meshNodeId);
      if(pid<0)
        {
        outNodeIdList->InsertNextId(meshNodeId);
        nextPt = smoothPoints->InsertNextPoint(pointPtr);
        //continue;
        }
      else
        {
        nextPt = pid;
        }
      nxtPts->InsertNextId(nextPt);
      }
    smoothPolys->InsertNextCell(npts, nxtPts->GetPointer(0));
    }
}

//----------------------------------------------------------------------------
void vtkCMBSmoothMeshFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Convergence: " << this->Convergence << "\n";
  os << indent << "Number of Iterations: " << this->NumberOfIterations << "\n";
  os << indent << "Relaxation Factor: " << this->RelaxationFactor << "\n";
  os << indent << "Feature Edge Smoothing: " << (this->FeatureEdgeSmoothing ? "On\n" : "Off\n");
  os << indent << "Feature Angle: " << this->FeatureAngle << "\n";
  os << indent << "Edge Angle: " << this->EdgeAngle << "\n";
  os << indent << "Boundary Smoothing: " << (this->BoundarySmoothing ? "On\n" : "Off\n");
}
