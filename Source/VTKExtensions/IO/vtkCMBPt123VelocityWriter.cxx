//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPt123VelocityWriter.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"

#include <vtksys/SystemTools.hxx>

#define SEPARATOR "  "

vtkStandardNewMacro(vtkCMBPt123VelocityWriter);

vtkCMBPt123VelocityWriter::vtkCMBPt123VelocityWriter()
{
  this->FileName = 0;
  this->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::VECTORS);
  this->MyData = 0;
  this->UseScientificNotation = true;
  this->FloatPrecision = 6;

  this->WriteCellBased = true;
  this->SpatialDimension = 3;
  this->MyDataSet = 0;
}

vtkCMBPt123VelocityWriter::~vtkCMBPt123VelocityWriter()
{
  this->SetFileName(0);
}

void vtkCMBPt123VelocityWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

ostream* vtkCMBPt123VelocityWriter::OpenFile()
{
  if (!this->FileName || !this->FileName[0])
  {
    vtkErrorMacro("FileName has to be specified.");
    return 0;
  }

  ostream* fp = new ofstream(this->FileName, ios::out);
  if (fp->fail())
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    delete fp;
    return 0;
  }
  return fp;
}

void vtkCMBPt123VelocityWriter::CloseFile(ostream* fp)
{
  if (fp)
  {
    delete fp;
    fp = 0;
  }
}

void vtkCMBPt123VelocityWriter::WriteData()
{
  vtkDataObject* input = this->GetInput();
  this->MyDataSet = NULL;
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(input);
  if (mds)
  {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
    {
      this->MyDataSet = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (this->MyDataSet)
      {
        break;
      }
    }
    iter->Delete();
  }
  else
  {
    this->MyDataSet = vtkDataSet::SafeDownCast(input);
  }

  if (!this->MyDataSet)
  {
    vtkErrorMacro("vtkDataSet input is required.");
    return;
  }

  //Get the data array to be processed
  this->MyData = this->GetInputArrayToProcess(0, this->MyDataSet);
  if (!this->MyData)
  {
    vtkErrorMacro("Could not find Velocity Array to be processed.");
    return;
  }
  ostream* file = this->OpenFile();
  if (!file || !this->WriteHeader(*file) || !this->WriteTimeStep(*file, 0.0) ||
    !this->WriteFooter(*file))
  {
    vtkErrorMacro("Write failed");
  }
  this->CloseFile(file);
  this->MyData = 0;
  this->MyDataSet = 0;
}

bool vtkCMBPt123VelocityWriter::WriteHeader(ostream& fp)
{
  if (WriteCellBased)
  {
    fp << this->MyDataSet->GetNumberOfCells() << "  " << this->SpatialDimension
       << " 1  No. Elements, Space Dimension, No. of Time Steps" << endl;
  }
  else
  {
    fp << this->MyData->GetNumberOfTuples() << " " << this->SpatialDimension
       << " 1 No. of Global Nodes, "
       << "No. of Dimensions, No of Time Steps" << endl;
  }
  return true;
}

bool vtkCMBPt123VelocityWriter::WriteTimeStep(ostream& fp, double t)
{
  vtkIdType i, n = this->MyData->GetNumberOfTuples();
  double v[3];
  fp << "TS" << setw(20) << setprecision(10) << t << endl;
  fp.precision(this->FloatPrecision);
  fp.setf(ios::showpoint);
  fp.setf(UseScientificNotation ? ios::scientific : ios::fixed, ios::floatfield);
  if (this->WriteCellBased)
  {
    vtkIdType j, k, nPerCell, ncells = this->MyDataSet->GetNumberOfCells();
    if (this->SpatialDimension > this->MyData->GetNumberOfComponents())
    {
      vtkErrorMacro("SpatialDimension must be <= NumberOfComponents");
      return false;
    }
    vtkNew<vtkIdList> pcell;
    for (i = 0; i < ncells; i++)
    {
      this->MyDataSet->GetCellPoints(i, pcell.GetPointer());
      nPerCell = pcell->GetNumberOfIds();
      for (j = 0; j < nPerCell; j++)
      {
        this->MyData->GetTuple(pcell->GetId(j), v);
        for (k = 0; k < this->SpatialDimension; k++)
          fp << v[k] << SEPARATOR;
      }
      fp << endl;
    }
  }
  else
  {
    for (i = 0; i < n; i++)
    {
      this->MyData->GetTuple(i, v);
      fp << v[0] << SEPARATOR << v[1] << SEPARATOR << v[2] << endl;
    }
  }
  return true;
}

bool vtkCMBPt123VelocityWriter::WriteFooter(ostream& fp)
{
  fp << "ENDR" << endl;
  return true;
}

int vtkCMBPt123VelocityWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

void vtkCMBPt123VelocityWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "FileName = " << this->FileName << endl;
  os << indent << "FloatPrecision = " << this->FloatPrecision << endl;
  os << indent << "UseScientificNotation = " << this->UseScientificNotation << endl;
  os << indent << "WriteCellBased = " << this->WriteCellBased << endl;
  os << indent << "SpatialDimension = " << this->SpatialDimension << endl;

  this->Superclass::PrintSelf(os, indent);
}
