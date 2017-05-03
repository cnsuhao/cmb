//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkIdentifyNonManifoldPts.h"

#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStripper.h"

vtkStandardNewMacro(vtkIdentifyNonManifoldPts);

//----------------------------------------------------------------------------
int vtkIdentifyNonManifoldPts::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkPoints> outPts = vtkSmartPointer<vtkPoints>::New();
  output->SetPoints(outPts);
  vtkSmartPointer<vtkCellArray> outVerts = vtkSmartPointer<vtkCellArray>::New();
  output->SetVerts(outVerts);

  // get rid of any duplicated points
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->SetInputData(input);
  cleaner->Update();

  vtkSmartPointer<vtkPolyData> tmpPD = vtkSmartPointer<vtkPolyData>::New();
  tmpPD->SetLines(cleaner->GetOutput()->GetLines());
  tmpPD->SetPoints(cleaner->GetOutput()->GetPoints());
  tmpPD->BuildLinks();
  vtkPoints* pts = tmpPD->GetPoints();
  unsigned short ncells;
  vtkIdType* cells;
  for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); i++)
  {
    tmpPD->GetPointCells(i, ncells, cells);
    if (ncells > 2)
    {
      vtkIdType nextPt = outPts->InsertNextPoint(pts->GetPoint(i));
      outVerts->InsertNextCell(1, &nextPt);
    }
  }
  return 1;
}
