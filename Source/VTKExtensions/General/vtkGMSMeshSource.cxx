//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkGMSMeshSource.h"

#include "vtkUnstructuredGrid.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkAbstractTransform.h"
#include "vtkIdTypeArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkGMSMeshSource);

//-----------------------------------------------------------------------------
vtkGMSMeshSource::vtkGMSMeshSource()
{
  this->Source = vtkUnstructuredGrid::New();
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkGMSMeshSource::~vtkGMSMeshSource()
{
  this->Source->Delete();
}

//-----------------------------------------------------------------------------
void vtkGMSMeshSource::CopyData(vtkUnstructuredGrid *source)
{
  this->Source->ShallowCopy( source );
  this->Modified();
}
//-----------------------------------------------------------------------------
bool vtkGMSMeshSource::MovePoints(vtkPolyData *movedPoly)
{
  return MoveTransformPoints(movedPoly,NULL);
}
//-----------------------------------------------------------------------------
bool vtkGMSMeshSource::MoveTransformPoints(vtkPolyData *movedPoly,
  vtkAbstractTransform* transform)
{
  if(!(this->Source && movedPoly))
    {
    return false;
    }
  bool moved = false;

  vtkIdTypeArray* meshNodeIdArray = vtkIdTypeArray::SafeDownCast(
    movedPoly->GetPointData()->GetArray("Mesh Node ID"));

  vtkSmartPointer<vtkPoints> transformedPts;
  if(transform)
    {
    transformedPts = vtkSmartPointer<vtkPoints>::New();
    transformedPts->Allocate(movedPoly->GetNumberOfPoints());
    transform->TransformPoints(movedPoly->GetPoints(),transformedPts);
    }
  else
    {
    transformedPts = movedPoly->GetPoints();
    }
  // Using a vtkIdList to make search easier
  vtkIdList* meshNodeIds = vtkIdList::New();
  vtkIdType numIds = meshNodeIdArray->GetNumberOfTuples();
  meshNodeIds->SetNumberOfIds(numIds);
  meshNodeIds->Allocate(numIds);
  vtkIdType* idsP = meshNodeIds->WritePointer(0, numIds);
  memcpy(idsP,meshNodeIdArray->GetPointer(0), numIds*sizeof(vtkIdType));

  // vtkWarningMacro("WARNING: The mesh points are moved without mesh validation check!");
  moved = this->MoveMeshPoints(meshNodeIds, transformedPts);

  meshNodeIds->Delete();
  if(moved)
    {
    this->Modified();
    return true;
    }
  return false;
}
//-----------------------------------------------------------------------------
bool vtkGMSMeshSource::MoveSurfacePoints(vtkIdTypeArray* meshCellArray,
  vtkIdList* meshIdList, vtkPoints* transformedPts)
{
  vtkUnstructuredGrid* meshData = this->GetSource();
  vtkIdType numCells = meshCellArray->GetNumberOfTuples();
  vtkIdType cellId;
  bool movable = true;
  vtkIdType npts, *pts, pidx;
  double mNormal[3], tNormal[3];
  double tmpPts[4][3];
  double dotprod;
  for(vtkIdType i=0; i<numCells; i++)
    {
    cellId = meshCellArray->GetValue(i);
    meshData->GetCellPoints(cellId, npts, pts);
    if (npts != 3 && npts != 4)// only handle VTK_TRIANGLE and VTK_QUAD
      {
      continue;
      }
    for(vtkIdType n=0; n<npts; n++)
      {
      meshData->GetPoint(pts[n], tmpPts[n]);
      }

    vtkTriangle::ComputeNormal(tmpPts[0], tmpPts[1], tmpPts[2], mNormal);
    if(npts==4 && mNormal[0]==0.0 && mNormal[1]==0.0 && mNormal[2]==0.0)
      {
      vtkTriangle::ComputeNormal (tmpPts[1], tmpPts[2], tmpPts[3], mNormal);
      }

    for(vtkIdType idx=0; idx<npts; idx++)
      {
      pidx = meshIdList->IsId(pts[idx]);
      if(pidx>=0)// get the transformed point
        {
        transformedPts->GetPoint(pidx, tmpPts[idx]);
        }
      }
    vtkTriangle::ComputeNormal(tmpPts[0], tmpPts[1], tmpPts[2], tNormal);
    if(npts==4 && tNormal[0]==0.0 && tNormal[1]==0.0 && tNormal[2]==0.0)
      {
      vtkTriangle::ComputeNormal (tmpPts[1], tmpPts[2], tmpPts[3], tNormal);
      }
    // compare the original normal with the new normal after transform
    dotprod = vtkMath::Dot(mNormal, tNormal);
    if(dotprod<=0) // normal reversed
      {
      movable = false;
      vtkWarningMacro("WARNING: Some mesh points can't be moved because cell normal is reversed!");
      break;
      }
    }
  if(movable)
    {
    return this->MoveMeshPoints(meshIdList, transformedPts);
    }
  return movable;
}
//-----------------------------------------------------------------------------
bool vtkGMSMeshSource::MoveVolumePoints(vtkIdTypeArray* meshCellArray,
  vtkIdList* meshIdList, vtkPoints* transformedPts)
{
  vtkUnstructuredGrid* meshData = this->GetSource();
  vtkIdType numCells = meshCellArray->GetNumberOfTuples();
  vtkIdType cellId;
  bool movable = true;
  vtkIdType npts, *pts, pidx;
  double mVolume, tVolume;
  double tmpPts[4][3];
  double timespro;
  for(vtkIdType i=0; i<numCells; i++)
    {
    cellId = meshCellArray->GetValue(i);
    meshData->GetCellPoints(cellId, npts, pts);
    if (npts != 4)
      {
      continue;
      }
    meshData->GetPoint(pts[0], tmpPts[0]);
    meshData->GetPoint(pts[1], tmpPts[1]);
    meshData->GetPoint(pts[2], tmpPts[2]);
    meshData->GetPoint(pts[3], tmpPts[3]);
    mVolume = vtkTetra::ComputeVolume(tmpPts[0], tmpPts[1], tmpPts[2],  tmpPts[3]);

    for(int idx=0; idx<npts; idx++)
      {
      pidx = meshIdList->IsId(pts[idx]);
      if(pidx>=0)// get the transformed point
        {
        transformedPts->GetPoint(pidx, tmpPts[idx]);
        }
      }
    tVolume = vtkTetra::ComputeVolume(tmpPts[0], tmpPts[1], tmpPts[2],  tmpPts[3]);
    // compare the original volume with the new volume after transform
    timespro = mVolume*tVolume;
    if(timespro<=0) // volume reversed
      {
      movable = false;
      vtkWarningMacro("WARNING: Some mesh points can't be moved because cell volume is reversed!");
      break;
      }
    }
  if(movable)
    {
    return this->MoveMeshPoints(meshIdList, transformedPts);
    }
  return movable;
}
//-----------------------------------------------------------------------------
bool vtkGMSMeshSource::MoveMeshPoints(
  vtkIdList* meshIdList, vtkPoints* transformedPts)
{
  vtkIdType numIds = meshIdList->GetNumberOfIds();
  vtkIdType meshPid;
  double tPt[3];
  vtkPoints* meshPts = this->GetSource()->GetPoints();
  for(vtkIdType i=0; i<numIds; i++)
    {
    meshPid = meshIdList->GetId(i);
    transformedPts->GetPoint(i, tPt);
    meshPts->SetPoint(meshPid,tPt);
    }
  return true;
}
//-----------------------------------------------------------------------------
int vtkGMSMeshSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  // now move the input through to the output
  output->ShallowCopy( this->Source );

  // First, copy the input to the output as a starting point
  //output->CopyStructure( this->Source );
  //output->GetPointData()->PassData(this->Source->GetPointData());
  //output->GetCellData()->PassData(this->Source->GetCellData());

  return 1;
}

//-----------------------------------------------------------------------------
void vtkGMSMeshSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
}
