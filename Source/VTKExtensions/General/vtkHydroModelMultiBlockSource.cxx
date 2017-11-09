//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkHydroModelMultiBlockSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkHydroModelMultiBlockSource);

vtkHydroModelMultiBlockSource::vtkHydroModelMultiBlockSource()
{
  this->Source = vtkMultiBlockDataSet::New();
  this->SetNumberOfInputPorts(0);
}

vtkHydroModelMultiBlockSource::~vtkHydroModelMultiBlockSource()
{
  this->Source->Delete();
}

void vtkHydroModelMultiBlockSource::CopyData(vtkMultiBlockDataSet* source)
{
  this->Source->ShallowCopy(source);

  //for(unsigned int ui=0; ui<source->GetNumberOfBlocks(); ui++)
  //  {
  //  // shallow copy and replace...
  //  vtkDataObject* copy = vtkDataObject::SafeDownCast(
  //    vtkInstantiator::CreateInstance(source->GetBlock(ui)->GetClassName()));
  //  copy->ShallowCopy(source->GetBlock(ui));
  //  copy->Modified();
  //  this->Source->SetBlock(ui, copy);
  //  copy->Delete();
  //  }

  this->Modified();
}

int vtkHydroModelMultiBlockSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the ouptut
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  // now move the input through to the output
  output->ShallowCopy(this->Source);
  return 1;
}

void vtkHydroModelMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Source: " << this->Source << "\n";
}
