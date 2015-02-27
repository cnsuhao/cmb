/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDelosMeshReader.cxx

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive, Suite 204,
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

#include "vtkDelosMeshReader.h"
#include "vtkNew.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockWrapper.h"

#include <string>
#include <vtksys/SystemTools.hxx>
#include <sstream>
#include <sys/stat.h>

vtkStandardNewMacro(vtkDelosMeshReader);


//----------------------------------------------------------------------------
vtkDelosMeshReader::vtkDelosMeshReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkDelosMeshReader::~vtkDelosMeshReader()
{
  delete[] this->FileName;
}

//----------------------------------------------------------------------------
int vtkDelosMeshReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->FileName || (strlen(this->FileName) == 0))
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  struct stat fs;
  if (stat(this->FileName, &fs) != 0)
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
    }

  ifstream infile(this->FileName, ios::in | ios::binary);
  if (infile.fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
    }

  vtkDebugMacro(<< "Executing Delos Mesh file reader");
  // Read the coordinates
  vtkNew<vtkPoints> points;
  output->SetPoints(points.GetPointer());
  std::string mode;
  infile >> mode;
  if (mode != "DEBXYZ")
    {
    vtkErrorMacro(<< "Missing Coordinate Block: "
                  << "Header Tag = " << mode);
    return 0;

    }
  int numPoints, dim, i;
  infile >> numPoints >> dim;
  points->SetNumberOfPoints(numPoints);
  double x, y, z = 0.0;
  if (dim == 2)
    {
    for (i = 0; i < numPoints; i++)
      {
      infile >> x >> y;
      points->SetPoint(i, x, y, 0.0);
      }
    }
  else
    {
    for (i = 0; i < numPoints; i++)
      {
      infile >> x >> y >> z;
      points->SetPoint(i, x, y, z);
      }
    }
  // Read close of section
  infile >> mode;
  if (mode != "FINXYZ")
    {
    vtkErrorMacro(<< "Missing Coordinate End of Block: "
                  << "Close Tag = " << mode);
    return 0;
    }

  // Read in Cells
  infile >> mode;
  if (mode != "DEBILM")
    {
    vtkErrorMacro(<< "Missing Connectivity Block: "
                  << "Header Tag = " << mode);
    return 0;

    }
  int numCells, numSides, id, code, regId, j;
  infile >> numCells;
  vtkNew<vtkCellArray> cells;
  vtkNew<vtkIntArray>  regionIds;
  regionIds->SetName("Model Entity ID");
  regionIds->SetNumberOfValues(numCells);
  output->GetCellData()->AddArray(regionIds.GetPointer());
  for (i = 0; i < numCells; i++)
    {
    infile >> numSides;
    cells->InsertNextCell(numSides);
    for (j = 0; j < numSides; j++)
      {
      infile >> id;
      // Correct to have a start index of 0 and not 1
      --id;
      cells->InsertCellPoint(id);
      }
    infile >> code >> regId;
    regionIds->SetValue(i, regId);
    }
  output->SetPolys(cells.GetPointer());
  // Read close of section
  infile >> mode;
  if (mode != "FINILM")
    {
    vtkErrorMacro(<< "Missing Connectivity End of Block: "
                  << "Close Tag = " << mode);
    return 0;
    }
  vtkDebugMacro(<< "Executed CMB Mesh file reader");
  return 1;
}

//----------------------------------------------------------------------------
int vtkDelosMeshReader::CanReadFile(const char *fname)
{
  struct stat fs;
  if (!fname || strlen(fname) == 0 || stat(fname, &fs) != 0)
    {
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkDelosMeshReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: " <<
    (this->FileName ? this->FileName : "(none)") << endl;
}

//----------------------------------------------------------------------------
int vtkDelosMeshReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}
