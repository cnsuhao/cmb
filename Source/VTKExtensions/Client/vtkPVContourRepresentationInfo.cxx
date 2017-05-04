//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVContourRepresentationInfo.h"

#include "vtkCellArray.h"
#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"

#include <map>

#include "vtkContourRepresentation.h"
#include "vtkSceneContourSource.h"

vtkStandardNewMacro(vtkPVContourRepresentationInfo);

vtkPVContourRepresentationInfo::vtkPVContourRepresentationInfo()
{
  this->AllNodesWorldPositions = NULL;
  this->SelectedNodes = NULL;
  this->ClosedLoop = 0;
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = 0.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -1.0;
}

vtkPVContourRepresentationInfo::~vtkPVContourRepresentationInfo()
{
  if (this->AllNodesWorldPositions)
  {
    this->AllNodesWorldPositions->Delete();
  }
  if (this->SelectedNodes)
  {
    this->SelectedNodes->Delete();
  }
}

void vtkPVContourRepresentationInfo::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkPVContourRepresentationInfo::CopyFromObject(vtkObject* obj)
{
  this->ClosedLoop = false;
  vtkSceneContourSource* source = vtkSceneContourSource::SafeDownCast(obj);
  if (source)
  {
    this->CopyFromContourPolySource(source);
    return;
  }

  vtkContourRepresentation* contourRep = vtkContourRepresentation::SafeDownCast(obj);
  if (contourRep)
  {
    this->CopyFromContourRepresentation(contourRep);
    return;
  }
}

void vtkPVContourRepresentationInfo::CopyFromContourPolySource(vtkSceneContourSource* source)
{

  vtkPolyData* data = vtkPolyData::SafeDownCast(source->GetOutputDataObject(0));

  if (!this->AllNodesWorldPositions)
  {
    this->AllNodesWorldPositions = vtkDoubleArray::New();
    this->AllNodesWorldPositions->SetNumberOfComponents(3);
  }

  if (!this->SelectedNodes)
  {
    this->SelectedNodes = vtkIdTypeArray::New();
    this->SelectedNodes->SetNumberOfComponents(1);
  }

  this->AllNodesWorldPositions->Reset();
  this->SelectedNodes->Reset();

  //this is going to be generous
  this->AllNodesWorldPositions->Resize(data->GetNumberOfPoints());

  this->ClosedLoop = source->GetClosedLoop();

  //we need to build a mapping of point ids to
  //position in the line. This is because selected
  //nodes are kept in the source as point ids
  std::map<vtkIdType, vtkIdType> idToIndex;

  //we can't use points, because of it is actually a collection of points
  //instead we have to get the first cell
  vtkIdList* pointIds = vtkIdList::New();
  vtkIdType actualNumPoints = 0;
  double pos[3];

  vtkIdType pointIdx = 0;
  data->GetLines()->GetCell(0, pointIds);
  //do not add the last point if the contour is closed, as that will
  //actually cause the contour to close, and break the close contour widget function
  for (vtkIdType i = 0; i < pointIds->GetNumberOfIds() - this->ClosedLoop; ++i)
  {
    pointIdx = pointIds->GetId(i);
    data->GetPoint(pointIdx, pos);
    this->AllNodesWorldPositions->SetTuple(i, pos);
    idToIndex.insert(std::pair<vtkIdType, vtkIdType>(pointIdx, i));
    ++actualNumPoints;
  }
  pointIds->Delete();

  this->AllNodesWorldPositions->Resize(actualNumPoints);
  this->AllNodesWorldPositions->SetNumberOfTuples(actualNumPoints);

  //now create the selected nodes
  std::map<vtkIdType, vtkIdType>::iterator it;
  source->RegenerateEndNodes(); //get the latest end nodes
  vtkIdTypeArray* selected = source->GetEndNodes();
  this->SelectedNodes->SetNumberOfTuples(selected->GetNumberOfTuples());
  for (vtkIdType i = 0; i < selected->GetNumberOfTuples(); ++i)
  {
    it = idToIndex.find(selected->GetValue(i));
    if (it != idToIndex.end())
    {
      this->SelectedNodes->SetValue(i, it->second);
    }
  }
  // update bounds
  data->GetBounds(this->Bounds);
}

void vtkPVContourRepresentationInfo::CopyFromContourRepresentation(
  vtkContourRepresentation* contourRep)
{
  if (contourRep->GetNumberOfNodes())
  {

    this->ClosedLoop = contourRep->GetClosedLoop();

    if (!this->AllNodesWorldPositions)
    {
      this->AllNodesWorldPositions = vtkDoubleArray::New();
      this->AllNodesWorldPositions->SetNumberOfComponents(3);
    }
    this->AllNodesWorldPositions->Reset();
    this->AllNodesWorldPositions->SetNumberOfTuples(0);

    if (!this->SelectedNodes)
    {
      this->SelectedNodes = vtkIdTypeArray::New();
      this->SelectedNodes->SetNumberOfComponents(1);
    }
    this->SelectedNodes->Reset();
    this->SelectedNodes->SetNumberOfTuples(0);

    for (int i = 0; i < contourRep->GetNumberOfNodes(); i++)
    {
      double pos[3];
      contourRep->GetNthNodeWorldPosition(i, pos);
      this->AllNodesWorldPositions->InsertNextTypedTuple(pos);
      if (contourRep->GetNthNodeSelected(i))
      {
        vtkIdType IdTypeValue = i;
        this->SelectedNodes->InsertNextTypedTuple(&IdTypeValue);
      }
    }
    double* bounds = contourRep->GetBounds();
    for (int i = 0; i < 6; i++)
    {
      this->Bounds[i] = bounds[i];
    }
  }
}

int vtkPVContourRepresentationInfo::GetNumberOfAllNodes()
{
  return this->AllNodesWorldPositions ? this->AllNodesWorldPositions->GetNumberOfTuples() : 0;
}

int vtkPVContourRepresentationInfo::GetNumberOfSelectedNodes()
{
  return this->SelectedNodes ? this->SelectedNodes->GetNumberOfTuples() : 0;
}

void vtkPVContourRepresentationInfo::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << vtkClientServerStream::End;
}

void vtkPVContourRepresentationInfo::CopyFromStream(const vtkClientServerStream*)
{
}

void vtkPVContourRepresentationInfo::GetBounds(
  double& xMin, double& xMax, double& yMin, double& yMax, double& zMin, double& zMax) const
{
  xMin = this->Bounds[0];
  xMax = this->Bounds[1];
  yMin = this->Bounds[2];
  yMax = this->Bounds[3];
  zMin = this->Bounds[4];
  zMax = this->Bounds[5];
}
