/*=========================================================================
Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive
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
#include "vtkCMBPtsWriter.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkIdTypeArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkCompositeDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"

#include <vtksys/SystemTools.hxx>

#define SEPARATOR "  "

vtkStandardNewMacro(vtkCMBPtsWriter);
//----------------------------------------------------------------------------
vtkCMBPtsWriter::vtkCMBPtsWriter()
{
  this->FileName = 0;
  this->Header = 0;
  this->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_POINTS,
    vtkDataSetAttributes::SCALARS);
  this->MyData = 0;
  this->MyGeom = 0;
  this->UseScientificNotation = true;
  this->FloatPrecision = 6;
}

//----------------------------------------------------------------------------
vtkCMBPtsWriter::~vtkCMBPtsWriter()
{
  this->SetFileName(0);
  this->SetHeader(0);
}

//----------------------------------------------------------------------------
void vtkCMBPtsWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

//----------------------------------------------------------------------------
ostream* vtkCMBPtsWriter::OpenFile()
{
  if (!this->FileName || !this->FileName[0])
    {
    vtkErrorMacro("FileName has to be specified.");
    return 0;
    }

  ostream* fp = new ofstream(this->FileName, ios::out);
  if (fp->fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    delete fp;
    return 0;
    }
  return fp;
}

//----------------------------------------------------------------------------
void vtkCMBPtsWriter::CloseFile(ostream* fp)
{
  if (fp)
    {
    delete fp;
    fp = 0;
    }
}

//----------------------------------------------------------------------------
void vtkCMBPtsWriter::WriteData()
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

  //Get the data array to be processed - NOTE: as of now this is not used
  // in the future we could use this to assign intensity or color to each point
  this->MyData =
    vtkIdTypeArray::SafeDownCast(this->GetInputArrayToProcess(0, this->MyGeom));
  ostream* file = this->OpenFile();
  if (!file ||
      !this->WriteHeader(*file) ||
      !this->WritePoints(*file) ||
      !this->WriteFooter(*file))
    {
    vtkErrorMacro("Write failed");
    }
  this->CloseFile(file);
  this->MyData = 0;
  this->MyGeom = 0;
}

//----------------------------------------------------------------------------
bool vtkCMBPtsWriter::WriteHeader(ostream& fp)
{
  if (this->Header && (this->Header[0] != '\0'))
    {
    fp << this->Header << endl;
    }
  fp << this->MyGeom->GetNumberOfPoints() << endl;
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBPtsWriter::WritePoints(ostream& fp)
{
  vtkIdType i, n = this->MyGeom->GetNumberOfPoints();
  double v[3];
  fp.precision(this->FloatPrecision);
  fp.setf(ios::showpoint);
  fp.setf(UseScientificNotation ? ios::scientific : ios::fixed, ios::floatfield);

  for (i = 0; i < n; i++)
    {
    this->MyGeom->GetPoint(i, v);
    fp << v[0] << " " << v[1] << " " << v[2] << " 1" << endl;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBPtsWriter::WriteFooter(ostream& /*fp*/)
{
  return true;
}

//----------------------------------------------------------------------------
int vtkCMBPtsWriter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBPtsWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "FileName = " << this->FileName << endl;
  os << indent << "FloatPrecision = " << this->FloatPrecision << endl;
  os << indent << "UseScientificNotation = " << this->UseScientificNotation << endl;
  this->Superclass::PrintSelf(os, indent);
}
