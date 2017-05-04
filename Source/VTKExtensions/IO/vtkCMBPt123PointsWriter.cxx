//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPt123PointsWriter.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataSet.h"
#include "vtkDataSetAttributes.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

#define SEPARATOR "  "

vtkStandardNewMacro(vtkCMBPt123PointsWriter);

vtkCMBPt123PointsWriter::vtkCMBPt123PointsWriter()
{
  this->FileName = 0;
  this->Header = 0;
  this->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  this->MyData = 0;
  this->MyGeom = 0;
  this->UseScientificNotation = true;
  this->FloatPrecision = 6;
}

vtkCMBPt123PointsWriter::~vtkCMBPt123PointsWriter()
{
  this->SetFileName(0);
  this->SetHeader(0);
}

void vtkCMBPt123PointsWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

ostream* vtkCMBPt123PointsWriter::OpenFile()
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

void vtkCMBPt123PointsWriter::CloseFile(ostream* fp)
{
  if (fp)
  {
    delete fp;
    fp = 0;
  }
}

void vtkCMBPt123PointsWriter::WriteData()
{
  vtkDataObject* input = this->GetInput();
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(input);
  if (mds)
  {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
    {
      this->MyGeom = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (this->MyGeom)
      {
        break;
      }
    }
    iter->Delete();
  }
  else
  {
    this->MyGeom = vtkDataSet::SafeDownCast(input);
  }

  if (!this->MyGeom)
  {
    vtkErrorMacro("vtkDataSet input is required.");
    return;
  }

  //Get the data array to be processed
  this->MyData = vtkIdTypeArray::SafeDownCast(this->GetInputArrayToProcess(0, this->MyGeom));
  if (!this->MyData)
  {
    vtkErrorMacro("Could not find Classification Array to be processed.");
    return;
  }
  ostream* file = this->OpenFile();
  if (!file || !this->WriteHeader(*file) || !this->WritePoints(*file) || !this->WriteFooter(*file))
  {
    vtkErrorMacro("Write failed");
  }
  this->CloseFile(file);
  this->MyData = 0;
  this->MyGeom = 0;
}

bool vtkCMBPt123PointsWriter::WriteHeader(ostream& fp)
{
  if (this->Header && (this->Header[0] != '\0'))
  {
    fp << this->Header << endl;
  }
  fp << this->MyGeom->GetNumberOfPoints() << " ! NPT" << endl;
  return true;
}

bool vtkCMBPt123PointsWriter::WritePoints(ostream& fp)
{
  vtkIdType i, n = this->MyData->GetNumberOfTuples();
  double v[3];
  fp.precision(this->FloatPrecision);
  fp.setf(ios::showpoint);
  fp.setf(UseScientificNotation ? ios::scientific : ios::fixed, ios::floatfield);

  for (i = 0; i < n; i++)
  {
    this->MyGeom->GetPoint(i, v);
    //Convert the node ID to one-based
    fp << this->MyData->GetValue(i) + 1 << " " << v[0] << " " << v[1] << " " << v[2] << endl;
  }
  return true;
}

bool vtkCMBPt123PointsWriter::WriteFooter(ostream& fp)
{
  fp << "ENDR" << endl;
  return true;
}

int vtkCMBPt123PointsWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

void vtkCMBPt123PointsWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "FileName = " << this->FileName << endl;
  os << indent << "FloatPrecision = " << this->FloatPrecision << endl;
  os << indent << "UseScientificNotation = " << this->UseScientificNotation << endl;
  this->Superclass::PrintSelf(os, indent);
}
