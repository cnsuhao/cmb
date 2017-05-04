//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBExtractContours.h"

#include "vtkCellArray.h"
#include "vtkContourPointCollection.h"
#include "vtkDataArray.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataReader.h"

#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBExtractContours);

namespace
{
typedef std::set<vtkIdType> vtkInternalSetBase;
}
class vtkCMBExtractContours::vtkInternalSet : public vtkInternalSetBase
{
};

//-----------------------------------------------------------------------------
vtkCMBExtractContours::vtkCMBExtractContours()
{
  this->SetNumberOfInputPorts(1);
  this->ContourIndex = 0;
  this->BuildGlobalPointCollectionBefore = false;
  this->NumberOfContours = 0;
  this->SelectedIds = new vtkCMBExtractContours::vtkInternalSet();
}

//-----------------------------------------------------------------------------
vtkCMBExtractContours::~vtkCMBExtractContours()
{
  if (this->SelectedIds)
  {
    delete this->SelectedIds;
  }
}

//-----------------------------------------------------------------------------
int vtkCMBExtractContours::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input)
  {
    vtkErrorMacro("Unable able to reader the Contour file");
    return 0;
  }
  this->SetNumberOfContours(input->GetNumberOfLines());

  //now reparse the verts as a set of id's
  this->BuildSelectedIds(input);

  //setup the global contour points in the singleton
  this->BuildGlobalPointCollection(input);

  if (this->ContourIndex < 0 || this->ContourIndex >= this->NumberOfContours)
  {
    vtkErrorMacro("Invalid Contour Index");
    return 0;
  }

  vtkIdList* origPointIds = vtkIdList::New();
  vtkCellArray* origCell = input->GetLines();
  origCell->InitTraversal();
  int counter = this->ContourIndex;
  while (counter-- >= 0)
  {
    origPointIds->Reset();
    origCell->GetNextCell(origPointIds);
  }

  vtkIdType origNumPoints = origPointIds->GetNumberOfIds();
  if (origNumPoints == 0)
  {
    vtkErrorMacro("Unable to find a Line at index" << this->ContourIndex);
    return 0;
  }

  //now that we the point id's construct our output polydata
  vtkPoints* points = vtkPoints::New();
  vtkDataArray* pointIds = vtkIntArray::New();
  vtkCellArray* lines = vtkCellArray::New();
  vtkCellArray* verts = vtkCellArray::New();
  vtkDataArray* oIds = input->GetPointData()->GetScalars();

  vtkIdType selectedCount = 0;
  points->SetNumberOfPoints(origNumPoints);
  pointIds->SetNumberOfTuples(origNumPoints);
  lines->InsertNextCell(origNumPoints);
  verts->InsertNextCell(origNumPoints);
  for (vtkIdType i = 0; i < origNumPoints; ++i)
  {
    vtkIdType pointId = origPointIds->GetId(i);
    if (this->SelectedIds->find(pointId) != this->SelectedIds->end())
    {
      ++selectedCount;
      verts->InsertCellPoint(i);
    }
    double temp[3];
    input->GetPoint(pointId, temp);
    points->InsertPoint(i, input->GetPoint(pointId));
    if (oIds)
    {
      unsigned int id = oIds->GetTuple1(pointId);
      pointIds->SetTuple1(i, id);
    }
    else
    {
      pointIds->SetTuple1(i, i);
    }
    lines->InsertCellPoint(i);
  }
  verts->UpdateCellCount(selectedCount);

  output->SetPoints(points);
  output->GetPointData()->SetScalars(pointIds);
  output->SetVerts(verts);
  output->SetLines(lines);

  origPointIds->Delete();
  points->FastDelete();
  pointIds->FastDelete();
  lines->FastDelete();
  verts->FastDelete();

  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBExtractContours::BuildSelectedIds(vtkPolyData* input)
{
  vtkIdList* ids = vtkIdList::New();
  vtkCellArray* verts = input->GetVerts();
  verts->InitTraversal();
  while (verts->GetNextCell(ids))
  {
    //for each cell we convert the ids from the old set, to the new id's
    for (vtkIdType i = 0; i < ids->GetNumberOfIds(); ++i)
    {
      this->SelectedIds->insert(ids->GetId(i));
    }
  }
  ids->Delete();
}

//-----------------------------------------------------------------------------
void vtkCMBExtractContours::BuildGlobalPointCollection(vtkPolyData* input)
{
  if (this->BuildGlobalPointCollectionBefore == false)
  {
    //okay we can only create the point collection once and here is why.
    //if we call this a second time we will have a two contours each pointing
    //at the same contour point collection, BUT the first contour will have its
    //internal source points pointing to a null, since we deleted it!

    this->BuildGlobalPointCollectionBefore = true;
    vtkPoints* points = vtkPoints::New();
    points->DeepCopy(input->GetPoints());
    vtkContourPointCollection* coll = vtkContourPointCollection::GetInstance();
    coll->SetPoints(points);
    coll->InitLocator();
    points->FastDelete();
  }
}

//----------------------------------------------------------------------------
void vtkCMBExtractContours::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
