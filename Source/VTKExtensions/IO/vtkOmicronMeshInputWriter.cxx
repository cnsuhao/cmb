//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkOmicronMeshInputWriter.h"

#include "vtkCompositeDataIterator.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#define SEPARATOR "    "

vtkStandardNewMacro(vtkOmicronMeshInputWriter);

vtkOmicronMeshInputWriter::vtkOmicronMeshInputWriter()
{
  this->FileName = 0;
  this->GeometryFileName = 0;
  this->VolumeConstraint = 0.001;
}

vtkOmicronMeshInputWriter::~vtkOmicronMeshInputWriter()
{
  this->SetFileName(0);
  this->SetGeometryFileName(0);
}

void vtkOmicronMeshInputWriter::SetInputData(vtkMultiBlockDataSet* dataSet)
{
  this->Superclass::SetInputData(dataSet);
}

void vtkOmicronMeshInputWriter::WriteData()
{
  if (!this->FileName)
  {
    vtkErrorMacro("Must specify FileName. Write Failed!\n");
    return;
  }

  if (!this->GeometryFileName)
  {
    vtkErrorMacro("Must specify GeometryFileName. Write Failed!\n");
    return;
  }

  vtkMultiBlockDataSet* input = vtkMultiBlockDataSet::SafeDownCast(this->GetInput());
  if (!input)
  {
    vtkErrorMacro("Must specify Input. Write Failed!\n");
    return;
  }

  ofstream fout(this->FileName);
  if (!fout)
  {
    vtkErrorMacro("Could not open file " << this->FileName);
    return;
  }

  fout << this->GeometryFileName << "\n";
  fout << "volume_constraint: " << this->VolumeConstraint << "\n";
  fout << "number_of_regions: " << input->GetNumberOfBlocks() << "\n";

  vtkCompositeDataIterator* iter = input->NewIterator();
  iter->InitTraversal();
  fout.precision(16);
  while (!iter->IsDoneWithTraversal())
  {
    vtkDataObject* block = iter->GetCurrentDataObject();
    vtkDoubleArray* pointInsideFD =
      vtkDoubleArray::SafeDownCast(block->GetFieldData()->GetArray("PointInside"));
    if (pointInsideFD)
    {
      vtkStringArray* fileName =
        vtkStringArray::SafeDownCast(block->GetFieldData()->GetAbstractArray("FileName"));
      vtkIntArray* materialID =
        vtkIntArray::SafeDownCast(block->GetFieldData()->GetArray("MaterialID"));
      vtkStringArray* identifier =
        vtkStringArray::SafeDownCast(block->GetFieldData()->GetAbstractArray("Identifier"));

      fout << "(Object filename, Material ID): "
           << (identifier ? identifier->GetValue(0) : fileName->GetValue(0)) << " "
           << materialID->GetValue(0) << "\n";
      double* pointInside = pointInsideFD->GetTuple3(0);
      fout << "(point inside object): " << pointInside[0] << " " << pointInside[1] << " "
           << pointInside[2] << "\n";
    }
    iter->GoToNextItem();
  }
  iter->Delete();

  fout.close();
}

int vtkOmicronMeshInputWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

void vtkOmicronMeshInputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
