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
#include "vtkCMBWriter.h"
#include "vtkPolyData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCellData.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkNew.h"
#include "vtkXMLPolyDataWriter.h"

vtkStandardNewMacro(vtkCMBWriter);

//----------------------------------------------------------------------------
vtkCMBWriter::vtkCMBWriter()
{
  this->SetNumberOfOutputPorts(0);
  this->FileName = 0;
  this->BinaryOutput = 1;
}

//----------------------------------------------------------------------------
vtkCMBWriter::~vtkCMBWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkCMBWriter::Write()
{
  this->Modified();
  this->Update();
}

//-----------------------------------------------------------------------------
int vtkCMBWriter::RequestData(
  vtkInformation * /*request*/,
  vtkInformationVector **inputVector,
  vtkInformationVector * /*outputVector*/)
{
  vtkDebugMacro("Writing CMB File");
  // get the info object
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // get the input and output
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!input)
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
  if(this->BinaryOutput)
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

//-----------------------------------------------------------------------------
void vtkCMBWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  std::cout << indent << "FileName: " << (this->FileName ? this->FileName : "NULL") << "\n";
  std::cout << indent << "BinaryOutput: " << this->BinaryOutput << "\n";

}
