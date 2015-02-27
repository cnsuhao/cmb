/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "vtkOmicronMeshInputWriter.h"

#include "vtkCompositeDataIterator.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"

#define SEPARATOR "    "

vtkStandardNewMacro(vtkOmicronMeshInputWriter);
//----------------------------------------------------------------------------
vtkOmicronMeshInputWriter::vtkOmicronMeshInputWriter()
{
  this->FileName = 0;
  this->GeometryFileName = 0;
  this->VolumeConstraint = 0.001;
}

//----------------------------------------------------------------------------
vtkOmicronMeshInputWriter::~vtkOmicronMeshInputWriter()
{
  this->SetFileName(0);
  this->SetGeometryFileName(0);
}

//----------------------------------------------------------------------------
void vtkOmicronMeshInputWriter::SetInputData(vtkMultiBlockDataSet* dataSet)
{
  this->Superclass::SetInputData(dataSet);
}


//----------------------------------------------------------------------------
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

  vtkMultiBlockDataSet* input =
    vtkMultiBlockDataSet::SafeDownCast( this->GetInput() );
  if (!input)
    {
    vtkErrorMacro("Must specify Input. Write Failed!\n");
    return;
    }

  ofstream fout(this->FileName);
  if(!fout)
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
    vtkDataObject *block = iter->GetCurrentDataObject();
    vtkDoubleArray *pointInsideFD = vtkDoubleArray::SafeDownCast(
      block->GetFieldData()->GetArray("PointInside") );
    if (pointInsideFD)
      {
      vtkStringArray *fileName = vtkStringArray::SafeDownCast(
        block->GetFieldData()->GetAbstractArray("FileName") );
      vtkIntArray *materialID = vtkIntArray::SafeDownCast(
        block->GetFieldData()->GetArray("MaterialID") );
      vtkStringArray *identifier = vtkStringArray::SafeDownCast(
        block->GetFieldData()->GetAbstractArray("Identifier") );

      fout << "(Object filename, Material ID): " <<
        (identifier ? identifier->GetValue(0) : fileName->GetValue(0)) <<
        " " << materialID->GetValue(0) << "\n";
      double *pointInside = pointInsideFD->GetTuple3(0);
      fout << "(point inside object): " <<
        pointInside[0] << " " << pointInside[1] << " " << pointInside[2] << "\n";
      }
    iter->GoToNextItem();
    }
  iter->Delete();

  fout.close();
}

//----------------------------------------------------------------------------
int vtkOmicronMeshInputWriter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkOmicronMeshInputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

