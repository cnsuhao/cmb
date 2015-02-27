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
#include "vtkCUBITReader.h"

#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"

#include <vtksys/ios/sstream>


vtkStandardNewMacro(vtkCUBITReader);

//-----------------------------------------------------------------------------
vtkCUBITReader::vtkCUBITReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkCUBITReader::~vtkCUBITReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkCUBITReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
    }

  int idx, numPts, numFacets;
  double pt[3];

  this->UpdateProgress(0.0);

  // get the number of points and facets
  std::stringstream lineStr;
  this->GetNextLineOfData(fin, lineStr);
  lineStr >> numPts >> numFacets;

  // now read the points
  vtkNew<vtkPoints> newPts;
  newPts->SetDataTypeToDouble();
  newPts->Allocate(numPts);
  int id;
  for (idx = 0;  idx < numPts; idx++)
    {
    this->GetNextLineOfData(fin, lineStr);
    lineStr >> id >> pt[0] >> pt[1] >> pt[2];
    if ((idx%1000)==0)
      {
      this->UpdateProgress( 0.4 * idx / numPts );
      }
    newPts->InsertNextPoint(pt);
    }

  // now read the facets
  vtkNew<vtkCellArray> newPolys;
  // assume all quads (4 + 1 for how many points in the cell)
  newPolys->Allocate(4 * numFacets);
  vtkIdType facetPts[4];
  for (int faceIdx = 0;  faceIdx < numFacets; faceIdx++)
    {
    this->GetNextLineOfData(fin, lineStr);
    facetPts[3] = -1;
    lineStr >> id >> facetPts[0] >> facetPts[1] >> facetPts[2] >> facetPts[3];
    vtkIdType numPtsInFacet = 3;
    if (facetPts[3] != -1)
      {
      numPtsInFacet = 4;
      }
    newPolys->InsertNextCell(numPtsInFacet, facetPts);

    if ((faceIdx%1000) == 0)
      {
      this->UpdateProgress( 0.4 + 0.5 * faceIdx / numFacets );
      }
    }

  fin.close();

  vtkNew<vtkPolyData> tempPD;
  tempPD->SetPoints(newPts.GetPointer());
  tempPD->SetPolys(newPolys.GetPointer());

  // merge points, and then make sure all normals point out
  vtkNew<vtkCleanPolyData> clean;
  clean->SetInputData( tempPD.GetPointer() );
  clean->Update();
  this->UpdateProgress( 0.95 );

  vtkNew<vtkPolyDataNormals> normals;
  normals->SetInputConnection( clean->GetOutputPort() );
  normals->ComputePointNormalsOff();
  normals->ComputeCellNormalsOn();
  normals->SplittingOff();
  normals->AutoOrientNormalsOn();
  normals->Update();
  this->UpdateProgress( 1.0 );

  output->ShallowCopy( normals->GetOutput() );

  return 1;
}

//-----------------------------------------------------------------------------
int vtkCUBITReader::GetNextLineOfData(ifstream &fin,
                                         std::stringstream &lineStream)
{
  // clear the string for the line
  lineStream.str("");
  lineStream.clear();
  char buffer[1024]; // a pretty long line!
  std::string testString;
  while(1)
    {
    fin.getline(buffer, 1024);
    if (fin.eof())
      {
      return 0;
      }

    testString = buffer;
    // see if it is a comment or blank line
    if (testString == "" || testString.find("#") == 0)
      {
      continue;
      }

    // set the output stream
    lineStream << testString;
    return 1;
    }
}

//-----------------------------------------------------------------------------
void vtkCUBITReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
int vtkCUBITReader::RequestInformation(
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

