/*=========================================================================

  Program:   CMB
  Module:    vtkExtractLine.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
#include "vtkExtractLine.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkExtractLine);

//----------------------------------------------------------------------------
vtkExtractLine::vtkExtractLine()
{
  this->LineId = -1;
}

//----------------------------------------------------------------------------
int vtkExtractLine::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetPoints( input->GetPoints() );
  if (this->LineId < 0)
    {
    vtkWarningMacro("LineId hasn't been set, thus no line in output.");
    return 1;
    }

  if (this->LineId > input->GetNumberOfLines())
    {
    vtkWarningMacro("LineId (" << this->LineId << ") too large.  Only " <<
      input->GetNumberOfLines() << "lines in input.  No line in output.");
    return 1;
    }

  vtkIdType npts = 0, *pts = NULL;

  vtkCellArray *inputLines = input->GetLines();
  inputLines->InitTraversal();
  for (int i = 0; i <= this->LineId; i++)
    {
    inputLines->GetNextCell(npts, pts);
    }

  vtkCellArray *lines = vtkCellArray::New();
  lines->InsertNextCell(npts, pts);
  output->SetLines(lines);
  lines->Delete();

  output->GetCellData()->CopyData(input->GetCellData(),
    input->GetNumberOfVerts() + this->LineId, 0);
  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractLine::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "LineId: " << this->LineId << "\n";
}
