//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBWriter.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkXMLPolyDataWriter.h"

vtkStandardNewMacro(vtkCMBWriter);

vtkCMBWriter::vtkCMBWriter()
{
  this->SetNumberOfOutputPorts(0);
  this->FileName = 0;
  this->BinaryOutput = 1;
}

vtkCMBWriter::~vtkCMBWriter()
{
  this->SetFileName(0);
}

void vtkCMBWriter::Write()
{
  this->Modified();
  this->Update();
}

int vtkCMBWriter::RequestData(vtkInformation* /*request*/, vtkInformationVector** inputVector,
  vtkInformationVector* /*outputVector*/)
{
  vtkDebugMacro("Writing CMB File");
  // get the info object
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

  // get the input and output
  vtkMultiBlockDataSet* input =
    vtkMultiBlockDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input)
  {
    vtkWarningMacro("The input did not pass in a vtkMultiBlockDataSet object");
    return 0;
  }

  vtkNew<vtkPolyData> poly;
  vtkNew<vtkMultiBlockWrapper> mbw;
  mbw->SetMultiBlock(input);
  mbw->ProcessForWriting(poly.GetPointer());

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(poly.GetPointer());
  writer->SetFileName(this->FileName);
  if (this->BinaryOutput)
  {
    writer->SetDataModeToBinary();
  }
  else
  {
    writer->SetDataModeToAscii();
  }
  writer->Write();
  return 1;
}

void vtkCMBWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  std::cout << indent << "FileName: " << (this->FileName ? this->FileName : "NULL") << "\n";
  std::cout << indent << "BinaryOutput: " << this->BinaryOutput << "\n";
}
