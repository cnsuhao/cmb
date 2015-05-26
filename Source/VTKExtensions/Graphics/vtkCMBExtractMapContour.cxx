//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBExtractMapContour.h"

#include "vtkObjectFactory.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkIdList.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkErrorCode.h"
#include "vtkAppendPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkCMBPrepareForTriangleMesher.h"
#include "assert.h"
#include <vtksys/SystemTools.hxx>



vtkStandardNewMacro(vtkCMBExtractMapContour);

vtkCMBExtractMapContour::vtkCMBExtractMapContour()
{
}
//-----------------------------------------------------------------------------
vtkCMBExtractMapContour::~vtkCMBExtractMapContour()
{
}
//-----------------------------------------------------------------------------
int vtkCMBExtractMapContour::RequestData(vtkInformation */*request*/,
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  //Have to clear field data because it is copied from
  //input otherwise
  vtkFieldData* emptyFD = vtkFieldData::New();
  output->SetFieldData(emptyFD);
  emptyFD->FastDelete();

  vtkCMBPrepareForTriangleMesher* mapInterface = vtkCMBPrepareForTriangleMesher::New();
  mapInterface->SetPolyData(input);
  mapInterface->GetArc(contoursToExtract[0],output);
  mapInterface->Delete();

  return 1;
  /*
  //Old way TODO: Remove when old map files are phased out
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]);
  if (input == 0)
    {
    vtkErrorMacro("Must set Input!");
    return 0;
    }

  vtkCellArray* inputLines = input->GetLines();
  vtkCellData* inputCellData = input->GetCellData();
  vtkPoints* inputPoints = input->GetPoints();

  //setup variables for output
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lineSegments = vtkCellArray::New();

  vtkIntArray *cellDataScalars = vtkIntArray::New();
  cellDataScalars->Initialize();
  int numScalars = inputCellData->GetScalars()->GetNumberOfComponents();
  cellDataScalars->SetNumberOfComponents(numScalars);

  int numArrays = input->GetCellData()->GetNumberOfArrays();

  vtkIntArray** cellDataArrays = new vtkIntArray*[numArrays];

  for(int curr_arr = 0; curr_arr < numArrays; ++curr_arr)
    {
    cellDataArrays[curr_arr] = vtkIntArray::New();
    cellDataArrays[curr_arr]->Initialize();
    cellDataArrays[curr_arr]->SetNumberOfComponents(input->GetCellData()->GetArray(curr_arr)->GetNumberOfComponents());
    cellDataArrays[curr_arr]->SetName(input->GetCellData()->GetArray(curr_arr)->GetName());
    }

  map<vector<double>, vtkIdType> point2Id; //use this to make sure there are no duplicate points

  for(int i = 0; i < input->GetNumberOfCells(); i++)
    {
    vtkCell* cell = input->GetCell(i);
    if ( cell->GetCellType() == VTK_LINE)
      {
      vector<int>::iterator iter;
      int arcId = inputCellData->GetArray("ModelEdgeIds")->GetTuple1(i);
      double *nTuple = new double[numScalars];
      inputCellData->GetScalars()->GetTuple(i,nTuple);

      for(iter = contoursToExtract.begin(); iter != contoursToExtract.end(); ++iter)
        {
        int arcIdToExtract = (*iter);
        if (arcIdToExtract == arcId)
          {
          vtkIdList* idLst = cell->GetPointIds();
          assert(cell->GetNumberOfPoints() == 2);
          vtkIdType arcVerts[2];
          for(int j = 0; j < 2; ++j)
            {
            double xyz[3];
            inputPoints->GetPoint(idLst->GetId(j),xyz);
            vector<double> xyz_triple;
            xyz_triple.push_back(xyz[0]);
            xyz_triple.push_back(xyz[1]);
            xyz_triple.push_back(xyz[2]);
            vtkIdType currentPointId;
            map<vector<double>,vtkIdType>::iterator it = point2Id.find(xyz_triple);
            if(it == point2Id.end())
              {
              currentPointId = points->InsertNextPoint(xyz);
              point2Id[xyz_triple] = currentPointId;
              }
            else
              {
              currentPointId = (*it).second;
              }
            arcVerts[j] = currentPointId;
            }

          lineSegments->InsertNextCell(2,arcVerts);

          cellDataScalars->InsertNextTuple(nTuple);
          for(int curr_arr = 0; curr_arr < numArrays; ++curr_arr)
            {
            double *tuple = inputCellData->GetArray(curr_arr)->GetTuple(i);
            cellDataArrays[curr_arr]->InsertNextTuple(tuple);
            }
          }
        }
      delete[] nTuple;
      }
    }
  output->SetPoints(points);
  points->Delete();
  output->SetLines(lineSegments);
  lineSegments->Delete();
  cellDataScalars->SetName(inputCellData->GetScalars()->GetName());
  output->GetCellData()->SetScalars(cellDataScalars);
  cellDataScalars->Delete();
  for(int curr_arr = 0; curr_arr < numArrays; ++curr_arr)
    {
    output->GetCellData()->AddArray(cellDataArrays[curr_arr]);
    cellDataArrays[curr_arr]->Delete();
    }
  delete[] cellDataArrays;
  return 1;
  */
}
//-----------------------------------------------------------------------------
void vtkCMBExtractMapContour::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << " \n";
}
//----------------------------------------------------------------------------
void vtkCMBExtractMapContour::AddContourToExtract(int index)
{
  contoursToExtract.push_back(index);
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkCMBExtractMapContour::ExtractSingleContour(int index)
{
  contoursToExtract.clear();
  contoursToExtract.push_back(index);
  this->Modified();
}
