//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPolyReader.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include <sstream>
#include <string>

vtkStandardNewMacro(vtkCMBPolyReader);

//-----------------------------------------------------------------------------
vtkCMBPolyReader::vtkCMBPolyReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkCMBPolyReader::~vtkCMBPolyReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkCMBPolyReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the info object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if (!fin)
  {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
  }

  int numPts, dimension, numberOfAttributes, hasBoundaryMarkers;
  int status;

  this->UpdateProgress(0.0);

  numPts = -1;

  // get the number of points

  // numPts could actually be 0, indicating points are listed in a separate
  // .node file
  std::stringstream lineStream;
  this->GetNextLineOfData(fin, lineStream);
  lineStream >> numPts >> dimension >> numberOfAttributes >> hasBoundaryMarkers;

  if (dimension == 2)
  {
    status = this->Read2DFile(fin, numPts, numberOfAttributes, hasBoundaryMarkers, output);
  }
  else if (dimension == 3)
  {
    status = this->Read3DFile(fin, numPts, numberOfAttributes, hasBoundaryMarkers, output);
  }
  else
  {
    vtkErrorMacro("Unsupported Dimension!");
    fin.close();
    return 0;
  }
  fin.close();
  return status;
}

int vtkCMBPolyReader::Read3DFile(
  ifstream& fin, int numPts, int numberOfAttributes, int hasBoundaryMarkers, vtkPolyData* output)
{
  // if numPts == 0 then we need to read them from a .node file
  double pt[3];
  int idx;

  std::stringstream lineStream;
  vtkPoints* newPts = vtkPoints::New();
  newPts->SetDataTypeToDouble();
  newPts->Allocate(numPts);

  // now actually read the points
  int ptIndex;
  for (idx = 0; idx < numPts; idx++)
  {
    this->GetNextLineOfData(fin, lineStream);
    lineStream >> ptIndex >> pt[0] >> pt[1] >> pt[2];

    if ((idx % 1000) == 0)
    {
      this->UpdateProgress(0.5 * idx / numPts);
    }
    newPts->InsertNextPoint(pt);

    // read any attributes (just throwing away right now)
    double attribute;
    for (int i = 0; i < numberOfAttributes; i++)
    {
      lineStream >> attribute;
    }

    // get boundary marker
    int boundaryMarker;
    if (hasBoundaryMarkers)
    {
      lineStream >> boundaryMarker;
    }
  }

  vtkIdType numFacets = 0, hasFacetBoundaryMarkers;

  // number of facets
  this->GetNextLineOfData(fin, lineStream);
  lineStream >> numFacets >> hasFacetBoundaryMarkers;

  vtkCellArray* newPolys = vtkCellArray::New();
  // assume triangles (and one poly per facet)
  newPolys->Allocate(4 * numFacets);

  vtkSmartPointer<vtkIntArray> faceIDs = vtkSmartPointer<vtkIntArray>::New();
  faceIDs->SetName(vtkMultiBlockWrapper::GetModelFaceTagName());
  faceIDs->Allocate(numFacets);
  output->GetCellData()->AddArray(faceIDs);

  vtkSmartPointer<vtkIntArray> boundaryMarkerIDs;
  if (hasFacetBoundaryMarkers)
  {
    boundaryMarkerIDs = vtkSmartPointer<vtkIntArray>::New();
    boundaryMarkerIDs->SetName("BoundaryMarkers");
    boundaryMarkerIDs->Allocate(numFacets);
    output->GetCellData()->AddArray(boundaryMarkerIDs);
  }

  int holeIndex, ptsInPoly, indicesSize = 5;
  int numberOfPolygons, numberOfHoles, boundaryMarker;
  vtkIdType* indices = new vtkIdType[5];
  for (int faceIdx = 0; faceIdx < numFacets; faceIdx++)
  {
    numberOfHoles = boundaryMarker = 0;
    this->GetNextLineOfData(fin, lineStream);
    lineStream >> numberOfPolygons >> numberOfHoles >> boundaryMarker;

    // get the polygons making up this facet
    for (int i = 0; i < numberOfPolygons; i++)
    {
      this->GetNextLineOfData(fin, lineStream);
      lineStream >> ptsInPoly;
      // make sure we have enough space to store the indices
      if (ptsInPoly > indicesSize)
      {
        delete[] indices;
        indicesSize = ptsInPoly;
        indices = new vtkIdType[indicesSize];
      }

      for (int j = 0; j < ptsInPoly; j++)
      {
        lineStream >> indices[j];
        indices[j]--; // indices are 1-based (need to convert to 0-based)
      }
      newPolys->InsertNextCell(ptsInPoly, indices);
      faceIDs->InsertNextValue(faceIdx);
      if (hasFacetBoundaryMarkers)
      {
        boundaryMarkerIDs->InsertNextValue(boundaryMarker);
      }
    }

    // and any holes in the facet (throw away for now)
    for (int i = 0; i < numberOfHoles; i++)
    {
      this->GetNextLineOfData(fin, lineStream);
      lineStream >> holeIndex >> pt[0] >> pt[1] >> pt[2];
    }

    this->UpdateProgress(0.5 + 0.5 * faceIdx / numFacets);
  }
  delete[] indices;

  // now get volume holes
  int numberOfVolumeHoles;
  this->GetNextLineOfData(fin, lineStream);
  lineStream >> numberOfVolumeHoles;
  for (int i = 0; i < numberOfVolumeHoles; i++)
  {
    this->GetNextLineOfData(fin, lineStream);
    lineStream >> holeIndex >> pt[0] >> pt[1] >> pt[2];
  }

  // and finally the region attribute list (optional)
  if (this->GetNextLineOfData(fin, lineStream) == 1)
  {
    int numberOfRegions;
    lineStream >> numberOfRegions;
    // no as sure about the format here... will get back to
  }

  this->UpdateProgress(1.0);

  output->SetPoints(newPts);
  newPts->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();

  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBPolyReader::Read2DFile(
  ifstream& fin, int numPts, int numberOfAttributes, int hasBoundaryMarkers, vtkPolyData* output)
{
  // if numPts == 0 then we need to read them from a .node file
  double pt[3];
  int idx;

  std::stringstream lineStream;
  vtkPoints* newPts = vtkPoints::New();
  newPts->SetDataTypeToDouble();
  newPts->Allocate(numPts);

  // now actually read the points
  int ptIndex;
  for (idx = 0; idx < numPts; idx++)
  {
    this->GetNextLineOfData(fin, lineStream);
    lineStream >> ptIndex >> pt[0] >> pt[1];
    pt[2] = 0.0;

    if ((idx % 1000) == 0)
    {
      this->UpdateProgress(0.5 * idx / numPts);
    }
    newPts->InsertNextPoint(pt);

    // read any attributes (just throwing away right now)
    double attribute;
    for (int i = 0; i < numberOfAttributes; i++)
    {
      lineStream >> attribute;
    }

    // get boundary marker
    int boundaryMarker;
    if (hasBoundaryMarkers)
    {
      lineStream >> boundaryMarker;
    }
  }

  vtkIdType numLines = 0, hasLineBoundaryMarkers;

  // number of lines
  this->GetNextLineOfData(fin, lineStream);
  lineStream >> numLines >> hasLineBoundaryMarkers;

  vtkCellArray* newLines = vtkCellArray::New();
  // assume 2 point lines (and one line segment per line)
  newLines->Allocate(2 * numLines);

  vtkSmartPointer<vtkIntArray> lineIDs = vtkSmartPointer<vtkIntArray>::New();
  lineIDs->SetName("SegmentIds");
  lineIDs->Allocate(numLines);
  output->GetCellData()->AddArray(lineIDs);

  vtkSmartPointer<vtkIntArray> boundaryMarkerIDs;
  if (hasLineBoundaryMarkers)
  {
    boundaryMarkerIDs = vtkSmartPointer<vtkIntArray>::New();
    boundaryMarkerIDs->SetName("BoundaryMarkers");
    boundaryMarkerIDs->Allocate(numLines);
    output->GetCellData()->AddArray(boundaryMarkerIDs);
  }

  int boundaryMarker;
  vtkIdType indices[2], lineID;
  for (int lineIdx = 0; lineIdx < numLines; lineIdx++)
  {
    boundaryMarker = 0;
    this->GetNextLineOfData(fin, lineStream);
    lineStream >> lineID >> indices[0] >> indices[1];

    // indices are 1 based not 0 based
    indices[0]--;
    indices[1]--;
    newLines->InsertNextCell(2, indices);
    lineIDs->InsertNextValue(lineID);

    if (hasLineBoundaryMarkers)
    {
      lineStream >> boundaryMarker;
      boundaryMarkerIDs->InsertNextValue(boundaryMarker);
    }

    this->UpdateProgress(0.5 + 0.5 * lineIdx / numLines);
  }

  this->UpdateProgress(1.0);

  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBPolyReader::GetNextLineOfData(ifstream& fin, std::stringstream& lineStream)
{
  // clear the string for the line
  lineStream.str("");
  lineStream.clear();
  char buffer[1024]; // a pretty long line!
  std::string testString;
  while (1)
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
void vtkCMBPolyReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkCMBPolyReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  return 1;
}
