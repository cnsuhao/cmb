//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBExtractCellFromDataSet.h"

#include "vtkCellArray.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBExtractCellFromDataSet);

vtkCMBExtractCellFromDataSet::vtkCMBExtractCellFromDataSet()
{
  this->CellIndex = -1;
}

int vtkCMBExtractCellFromDataSet::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  if (!input)
  {
    vtkErrorMacro("Input not specified!");
    return 0;
  }

  if (this->CellIndex == -1)
  {
    vtkErrorMacro("Must specify cell index");
    return 0;
  }

  if (this->CellIndex >= input->GetNumberOfCells())
  {
    vtkErrorMacro("Cell Index is greater than the number of cells");
    return 0;
  }

  vtkCellArray* cells = vtkCellArray::New();
  vtkPoints* points = vtkPoints::New();
  vtkIdList* pointIds = vtkIdList::New();

  //get the old cell
  vtkCell* oldCell = input->GetCell(this->CellIndex);

  //construct the new cell with the old point positions
  //but with new ids and a new point object
  vtkIdType size = oldCell->GetNumberOfPoints();
  points->SetNumberOfPoints(size);
  double p[3];
  for (vtkIdType i = 0; i < size; ++i)
  {
    //note: vtkPolyData::GetCell will fill the PointIds array with
    //the indexs the points are in the original polydata. While the Points
    //will be index based starting at 0.
    oldCell->Points->GetPoint(i, p);
    points->SetPoint(i, p);
    pointIds->InsertId(i, i);
  }
  cells->InsertNextCell(pointIds);
  pointIds->Delete();

  output->SetPoints(points);
  points->FastDelete();

  int type = input->GetCellType(this->CellIndex);
  switch (type)
  {
    case VTK_VERTEX:
    case VTK_POLY_VERTEX:
      output->SetVerts(cells);
      break;
    case VTK_LINE:
    case VTK_POLY_LINE:
      output->SetLines(cells);
      break;
    case VTK_TRIANGLE:
    case VTK_QUAD:
    case VTK_POLYGON:
      output->SetPolys(cells);
      break;
    case VTK_TRIANGLE_STRIP:
      output->SetStrips(cells);
      break;
  }
  cells->FastDelete();

  return 1;
}

void vtkCMBExtractCellFromDataSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CellIndex: " << this->CellIndex << "\n";
}
