//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPt123VelocityConversionWriter.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"

#include <vtksys/SystemTools.hxx>

#define SEPARATOR "  "

vtkStandardNewMacro(vtkCMBPt123VelocityConversionWriter);

vtkCMBPt123VelocityConversionWriter::vtkCMBPt123VelocityConversionWriter()
{
  this->FileName = 0;
  this->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  this->MyData = 0;
  this->UseScientificNotation = true;
  this->FloatPrecision = 6;
}

vtkCMBPt123VelocityConversionWriter::~vtkCMBPt123VelocityConversionWriter()
{
  this->SetFileName(0);
}

void vtkCMBPt123VelocityConversionWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

ostream* vtkCMBPt123VelocityConversionWriter::OpenFile()
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

void vtkCMBPt123VelocityConversionWriter::CloseFile(ostream* fp)
{
  if (fp)
  {
    delete fp;
    fp = 0;
  }
}

void vtkCMBPt123VelocityConversionWriter::WriteData()
{
  vtkDataObject* input = this->GetInput();
  vtkDataSet* dataSet = NULL;
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(input);
  if (mds)
  {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
    {
      dataSet = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (dataSet)
      {
        break;
      }
    }
    iter->Delete();
  }
  else
  {
    dataSet = vtkDataSet::SafeDownCast(input);
  }

  if (!dataSet)
  {
    vtkErrorMacro("vtkDataSet input is required.");
    return;
  }

  //Get the data array to be processed
  this->MyData = this->GetInputArrayToProcess(0, dataSet);
  if (!this->MyData)
  {
    vtkErrorMacro("Could not find Velocity Conversion Array to be processed.");
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
}

bool vtkCMBPt123VelocityConversionWriter::WriteHeader(ostream& fp)
{
  fp << this->MyData->GetNumberOfTuples() << " 1       NNP, NTSTEP" << endl;
  return true;
}

bool vtkCMBPt123VelocityConversionWriter::WriteTimeStep(ostream& fp, double t)
{
  vtkIdType i, n = this->MyData->GetNumberOfTuples();
  double v;
  fp << "TS" << setw(20) << setprecision(10) << t << endl;
  fp.precision(this->FloatPrecision);
  fp.setf(ios::showpoint);
  fp.setf(UseScientificNotation ? ios::scientific : ios::fixed, ios::floatfield);

  for (i = 0; i < n; i++)
  {
    this->MyData->GetTuple(i, &v);
    fp << v << endl;
  }
  return true;
}

bool vtkCMBPt123VelocityConversionWriter::WriteFooter(ostream& fp)
{
  fp << "ENDR" << endl;
  return true;
}

int vtkCMBPt123VelocityConversionWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

void vtkCMBPt123VelocityConversionWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "FileName = " << this->FileName << endl;
  os << indent << "FloatPrecision = " << this->FloatPrecision << endl;
  os << indent << "UseScientificNotation = " << this->UseScientificNotation << endl;
  this->Superclass::PrintSelf(os, indent);
}
