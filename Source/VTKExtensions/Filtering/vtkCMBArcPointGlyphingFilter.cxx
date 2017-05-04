//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcPointGlyphingFilter.h"

#include <vtkBitArray.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

vtkStandardNewMacro(vtkCMBArcPointGlyphingFilter);

void vtkCMBArcPointGlyphingFilter::PrintSelf(ostream& /*os*/, vtkIndent /*indent*/)
{
  //TODO
}

void vtkCMBArcPointGlyphingFilter::clearVisible()
{
  this->visible.clear();
  this->Modified();
}

void vtkCMBArcPointGlyphingFilter::setVisible(int index)
{
  if (index < 0)
    return;
  this->visible.insert(index);
  this->Modified();
}

void vtkCMBArcPointGlyphingFilter::setScale(double s)
{
  this->scale = s;
  this->Modified();
}

vtkCMBArcPointGlyphingFilter::vtkCMBArcPointGlyphingFilter()
  : scale(5.0)
{
  GetInputPortInformation(0)->Set(INPUT_IS_OPTIONAL(), 1);
}

vtkCMBArcPointGlyphingFilter::~vtkCMBArcPointGlyphingFilter()
{
}

int vtkCMBArcPointGlyphingFilter::RequestData(
  vtkInformation* /*info*/, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  if (inInfo == NULL)
    return 1; //no input
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts = input->GetNumberOfPoints();
  vtkPoints* inPts = input->GetPoints();

  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
  newPoints->Allocate(numPts, numPts / 2);

  vtkSmartPointer<vtkCellArray> cellIds = vtkSmartPointer<vtkCellArray>::New();
  cellIds->Allocate(numPts, numPts / 2);
  cellIds->InsertNextCell(0);
  output->SetVerts(cellIds);

  // Add color information
  vtkSmartPointer<vtkUnsignedCharArray> color = vtkSmartPointer<vtkUnsignedCharArray>::New();
  color->Allocate(numPts, numPts / 2);
  color->SetName("Color");
  color->SetNumberOfComponents(4);

  // Add Scaling information
  vtkSmartPointer<vtkDoubleArray> scaling = vtkSmartPointer<vtkDoubleArray>::New();
  scaling->Allocate(numPts, numPts / 2);
  scaling->SetName("Scaling");
  scaling->SetNumberOfComponents(3);

  // Add Visibility information
  vtkSmartPointer<vtkBitArray> visibility = vtkSmartPointer<vtkBitArray>::New();
  visibility->Allocate(numPts, numPts / 2);
  visibility->SetName("Visibility");
  visibility->SetNumberOfComponents(1);

  double point[3];

  for (int i = 0; i < numPts; i++)
  {
    if (visible.find(i) != visible.end())
    {
      inPts->GetPoint(i, point);
      vtkIdType id = newPoints->InsertNextPoint(point);
      color->InsertNextTuple4(225, 0, 0, 255);
      scaling->InsertNextTuple3(this->scale, this->scale, this->scale);
      visibility->InsertNextValue(1);
      cellIds->InsertCellPoint(id);
      cellIds->UpdateCellCount(id + 1);
    }
  }

  output->SetPoints(newPoints);
  vtkPointData* pdata = output->GetPointData();
  pdata->AddArray(color);
  pdata->AddArray(scaling);
  pdata->AddArray(visibility);
  return 1;
}
