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
#include "vtkRawDEMReader.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkStringArray.h"
#include "vtkMath.h"
#include "vtkGeoSphereTransform.h"
#include "vtkNew.h"
#include "vtkTriangle.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>
#include <vtksys/Glob.hxx>
#include <algorithm>

enum FileReadingStatus
  {
    READ_OK = 0,
    READ_ERROR,
    READ_ABORT
  };

vtkStandardNewMacro(vtkRawDEMReader);

vtkCxxSetObjectMacro(vtkRawDEMReader, Transform, vtkTransform);

struct RowCol
  {
  RowCol(vtkIdType row, vtkIdType column)
    {
    this->Row = row;
    this->Column = column;
    }
  vtkIdType Row;
  vtkIdType Column;
  };

//----------------------------------------------------------------------------
class vtkRawDEMReaderInternals
{
public:
  int GetFilesInfo(const char *fileName, bool readSet);
  int ReadFileInfo(const char *fileName, RawDEMReaderFileInfo &fileInfo);

  std::vector< RawDEMReaderFileInfo > FileInfo;
  double LongitudeMidPoint;
  double LatitudeMidPoint;
  vtkIdType TotalNumberOfPoints;
  vtkIdType NumberOfColumns;
  vtkIdType NumberOfRows;
  vtkIdType OutputNumberOfColumns;
  vtkIdType OutputNumberOfRows;
  vtkIdType Extents[4];
  vtkIdType OnRatio;
  std::vector<RowCol> NoDataValueInstances;
};

//----------------------------------------------------------------------------
bool SortPredicate(RawDEMReaderFileInfo elem1, RawDEMReaderFileInfo elem2)
{
  if (elem1.LatitudeOrigin > elem2.LatitudeOrigin)
    {
    return true;
    }
  else if (elem1.LatitudeOrigin == elem2.LatitudeOrigin &&
    elem1.LongitudeOrigin < elem2.LongitudeOrigin)
    {
    return true;
    }
  return false;
  }

//----------------------------------------------------------------------------
int vtkRawDEMReaderInternals::GetFilesInfo(const char *fileName, bool readSet)
{
  std::vector<std::string> files;
  if (readSet)
    {
    vtksys::Glob glob;
    std::string globString = vtksys::SystemTools::GetFilenamePath(fileName) + "/" +
      vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName);
    globString.erase(globString.end() - 2, globString.end());
    globString += "*.hdr";
    glob.FindFiles(globString.c_str());
    files = glob.GetFiles();
    if (files.size() == 0)
      {
      return 0;
      }
    }
  else
    {
    files.push_back( fileName );
    }

  this->TotalNumberOfPoints = 0;
  this->FileInfo.clear();
  std::vector<std::string>::const_iterator file = files.begin();
  RawDEMReaderFileInfo fileInfo;
  double longitudeRange[2] = {500, -500}, latitudeRange[2] = {500, -500};
  for (file = files.begin(); file != files.end(); file++)
    {
    if (this->ReadFileInfo(file->c_str(), fileInfo) == READ_ERROR)
      {
      std::cerr << "Unable to open file: " << *file << endl;
      return READ_ERROR;
      }
    else
      {
      this->FileInfo.push_back(fileInfo);
      }

    this->TotalNumberOfPoints += fileInfo.NumberOfColumns * fileInfo.NumberOfRows;

    double longitude[2], latitude[2];
    longitude[0] = fileInfo.LongitudeOrigin;
    longitude[1] = fileInfo.LongitudeOrigin +
      (fileInfo.NumberOfColumns - 1) * fileInfo.PointSpacing;
    latitude[1] = fileInfo.LatitudeOrigin;
    latitude[0] = fileInfo.LatitudeOrigin -
      (fileInfo.NumberOfRows - 1) * fileInfo.PointSpacing;

    if (latitude[0] < latitudeRange[0])
      {
      latitudeRange[0] = latitude[0];
      }
    if (latitude[1] > latitudeRange[1])
      {
      latitudeRange[1] = latitude[1];
      }
    if (longitude[0] < longitudeRange[0])
      {
      longitudeRange[0] = longitude[0];
      }
    if (longitude[1] > longitudeRange[1])
      {
      longitudeRange[1] = longitude[1];
      }
    }

  std::sort(this->FileInfo.begin(), this->FileInfo.end(), SortPredicate);

  // figure out how many (total) rows & columns; ASSUMING that every row has same number
  // of columns
  std::vector<RawDEMReaderFileInfo>::iterator fileInfoIter = this->FileInfo.begin();
  double startLatitude = fileInfoIter->LatitudeOrigin;
  double previousLatitude = startLatitude;
  double startLongitude = fileInfoIter->LongitudeOrigin;
  this->NumberOfColumns = 0;
  this->NumberOfRows = 0;
  vtkIdType rowOffset = 0, columnOffset = 0;
  for (; fileInfoIter != this->FileInfo.end(); fileInfoIter++)
    {
    if (fileInfoIter->LatitudeOrigin != previousLatitude)
      {
      previousLatitude = fileInfoIter->LatitudeOrigin;
      rowOffset += (fileInfoIter - 1)->NumberOfRows;
      columnOffset = 0;
      }
    fileInfoIter->Offset[1] = rowOffset;
    fileInfoIter->Offset[0] = columnOffset;
    columnOffset += fileInfoIter->NumberOfColumns;

    // come up with full column and row counts
    if (fileInfoIter->LatitudeOrigin == startLatitude)
      {
      this->NumberOfColumns += fileInfoIter->NumberOfColumns;
      }
    if (fileInfoIter->LongitudeOrigin == startLongitude)
      {
      this->NumberOfRows += fileInfoIter->NumberOfRows;
      }
    }

  // one more time... adjust RowOffset to be from LL instead of UL
  fileInfoIter = this->FileInfo.begin();
  for (; fileInfoIter != this->FileInfo.end(); fileInfoIter++)
    {
    fileInfoIter->Offset[1] = this->NumberOfRows -
      fileInfoIter->NumberOfRows - fileInfoIter->Offset[1];
    }

  this->LatitudeMidPoint = 0.5 * (latitudeRange[0] + latitudeRange[1]);
  this->LongitudeMidPoint = 0.5 * (longitudeRange[0] + longitudeRange[1]);

  return READ_OK;
}

//-----------------------------------------------------------------------------
int vtkRawDEMReaderInternals::ReadFileInfo(const char *fileName,
                                           RawDEMReaderFileInfo &fileInfo)
{
  // read the "hdr" file
  std::string fileNameStr =
    vtksys::SystemTools::GetFilenamePath( fileName ) + "/" +
    vtksys::SystemTools::GetFilenameWithoutLastExtension( fileName ) + ".hdr";
  ifstream fin;

  fin.open(fileNameStr.c_str(), ios::binary);
  if(!fin)
    {
    return READ_ERROR;
    }

  fileInfo.FileName = fileName;

  char buffer[64];
  fin >> buffer >> fileInfo.NumberOfColumns;
  fin >> buffer >> fileInfo.NumberOfRows;
  fin >> buffer >> fileInfo.LongitudeOrigin;
  fin >> buffer >> fileInfo.LatitudeOrigin;
  fin >> buffer >> fileInfo.PointSpacing;
  fin >> buffer >> fileInfo.NoDataValue;
  fin >> buffer >> buffer;
  fin.close();

  // LatitudeOrigin is specified as lower edge, but the data is read in from top
  // edge, so adjust the origin to be "upper-left" corner
  fileInfo.LatitudeOrigin += (fileInfo.NumberOfRows - 1) * fileInfo.PointSpacing;

  return READ_OK;
}


//-----------------------------------------------------------------------------
vtkRawDEMReader::vtkRawDEMReader()
{
  this->FileName = NULL;
  this->FileInfoMode = true;
  this->OnRatio = 239;
  this->SetNumberOfInputPorts(0);
  this->RealNumberOfOutputPoints = 0;
  this->LimitReadToBounds = false;
  this->ReadBounds[0] = this->ReadBounds[2] = this->ReadBounds[4] = VTK_DOUBLE_MAX;
  this->ReadBounds[1] = this->ReadBounds[3] = this->ReadBounds[5] = VTK_DOUBLE_MIN;
  this->Transform = 0;
  this->TransformOutputData = false;

  this->ConvertFromLatLongToXYZ = true;
  this->TransformForZUp = true;
  this->RemoveCurvature = true;

  this->LatLongTransform1 = vtkSmartPointer<vtkGeoSphereTransform>::New();
  this->LatLongTransform2 = vtkSmartPointer<vtkTransform>::New();

  this->MaxNumberOfPoints = 1000000;
  this->LimitToMaxNumberOfPoints = false;
  this->TransformForZOrigin[0] = this->TransformForZOrigin[1] = -500;
  this->OutputImageData = true;
  this->ReadSetOfFiles = false;
  this->ZRotationAngle = 0.0;

  this->RowReadExtents[0] = this->RowReadExtents[1] = -1;
  this->ColumnReadExtents[0] = this->ColumnReadExtents[1] = -1;

  this->OutputDataTypeIsDouble = false;
  this->Internals = new vtkRawDEMReaderInternals;
}

//-----------------------------------------------------------------------------
vtkRawDEMReader::~vtkRawDEMReader()
{
  delete this->Internals;
  this->SetFileName(0);
  this->SetTransform(static_cast<vtkTransform*>(0));
}

//-----------------------------------------------------------------------------
// vtkSetStringMacro except we clear some variables if we update the value
void vtkRawDEMReader::SetFileName(const char *filename)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting FileName to " << filename );
  if (this->FileName == NULL && filename == NULL)
    {
    return;
    }
  if (this->FileName && filename && !strcmp(this->FileName, filename))
    {
    return;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    }
  if (filename)
    {
    size_t n = strlen(filename) + 1;
    char *cp1 =  new char[n];
    const char *cp2 = (filename);
    this->FileName = cp1;
    do
      {
      *cp1++ = *cp2++;
      } while ( --n );
    }
   else
    {
    this->FileName = NULL;
    }

  // only want to clear these values if FileName changes!
  this->DataBounds[0] = this->DataBounds[2] = this->DataBounds[4] = VTK_DOUBLE_MAX;
  this->DataBounds[1] = this->DataBounds[3] = this->DataBounds[5] = VTK_DOUBLE_MIN;
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::SetTransform(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::SetupReadExtents()
{
  vtkIdType readExtents[4] = {0, this->Internals->NumberOfColumns - 1,
    0, this->Internals->NumberOfRows - 1};
  if (this->RowReadExtents[0] >= 0 && this->RowReadExtents[0] < this->Internals->NumberOfRows)
    {
    readExtents[2] = this->RowReadExtents[0];
    }
  if (this->RowReadExtents[1] >= readExtents[2] &&
    this->RowReadExtents[1] < this->Internals->NumberOfRows )
    {
    readExtents[3] = this->RowReadExtents[1];
    }
  if (this->ColumnReadExtents[0] >= 0 && this->ColumnReadExtents[0] < this->Internals->NumberOfColumns)
    {
    readExtents[0] = this->ColumnReadExtents[0];
    }
  if (this->ColumnReadExtents[1] >= readExtents[0] &&
    this->ColumnReadExtents[1] < this->Internals->NumberOfColumns )
    {
    readExtents[1] = this->ColumnReadExtents[1];
    }

  this->Internals->OnRatio = this->OnRatio;
  if (this->LimitToMaxNumberOfPoints)
    {
    if (this->MaxNumberOfPoints >= this->Internals->TotalNumberOfPoints)
      {
      this->Internals->OnRatio = 1;
      }
    else
      {
      this->Internals->OnRatio =
        sqrt(this->Internals->TotalNumberOfPoints / static_cast<double>(this->MaxNumberOfPoints));
      }
    }
  int onRatio = static_cast<int>(floor(sqrt(
    static_cast<double>(this->Internals->OnRatio))));

  int numberOfRangeRows = readExtents[3] - readExtents[2] + 1;
  int numberOfRangeColumns = readExtents[1] - readExtents[0] + 1;
  this->Internals->OutputNumberOfColumns = 1 + (numberOfRangeColumns - 1) / onRatio;
  this->Internals->OutputNumberOfRows = 1 + (numberOfRangeRows - 1) / onRatio;

  this->RealNumberOfOutputPoints =
    this->Internals->OutputNumberOfRows * this->Internals->OutputNumberOfColumns;

  this->Internals->Extents[0] = readExtents[0];
  this->Internals->Extents[1] = readExtents[0] +
    (this->Internals->OutputNumberOfColumns - 1) * onRatio;
  this->Internals->Extents[3] = readExtents[3];
  this->Internals->Extents[2] = readExtents[3] -
    (this->Internals->OutputNumberOfRows - 1) * onRatio;
}

//-----------------------------------------------------------------------------
int vtkRawDEMReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  int res = this->Internals->GetFilesInfo(this->FileName, this->ReadSetOfFiles);
  if(res == READ_ERROR)
    {
    return 0;
    }
  else if(res == READ_ABORT)
    {
    return 1;
    }

  this->SetupReadExtents();

  // setup the ReadBBox, IF we're limiting the read to specifed ReadBounds
  if (this->LimitReadToBounds)
    {
    this->ReadBBox.Reset();
    this->ReadBBox.SetMinPoint(this->ReadBounds[0], this->ReadBounds[2],
      this->ReadBounds[4]);
    this->ReadBBox.SetMaxPoint(this->ReadBounds[1], this->ReadBounds[3],
      this->ReadBounds[5]);
    // the ReadBBox is guaranteed to be "valid", regardless of the whether
    // ReadBounds is valid.  If any of the MonPoint values are greater than
    // the corresponding MaxPoint, the MinPoint component will be set to be
    // the same as the MaxPoint during the SetMaxPoint fn call.
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *pdOutput = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (pdOutput)
    {
    this->ReadPolyDataOutput( pdOutput );
    //pdOutput->GetBounds(this->DataBounds);
    }
  else
    {
    vtkImageData *imageOutput = vtkImageData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->ReadImageDataOutput( imageOutput );
    //imageOutput->GetBounds(this->DataBounds);
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::ReadPolyDataOutput(vtkPolyData *output)
{
  if (!this->OutputImageData && this->ConvertFromLatLongToXYZ && this->TransformForZUp)
    {
    this->LatLongTransform2->Identity();
    double rotationAxis[3], zAxis[3] = {0, 0, 1};
    double pt[3];
    RawDEMReaderFileInfo &fileInfo = this->Internals->FileInfo.front();

    // lat/long as specified, height of 0
    if (this->TransformForZOrigin[0] > -180)
      {
      pt[0] = this->TransformForZOrigin[0];
      pt[1] = this->TransformForZOrigin[1];
      }
    else
      {
      pt[0] = this->Internals->LongitudeMidPoint;
      pt[1] = this->Internals->LatitudeMidPoint;
      }
    pt[2] = 0;
    this->LatLongTransform1->TransformPoint(pt, pt);
    double rotationAngle;
    double planeNormal[3] = {pt[0], pt[1], pt[2]};
    vtkMath::Normalize(planeNormal);
    vtkMath::Cross(planeNormal, zAxis, rotationAxis);
    rotationAngle = vtkMath::DegreesFromRadians( acos(planeNormal[2]) );

    // Figure out Z-rotation
    if(this->ZRotationAngle == 0)
      {
      double pt1[3]={fileInfo.LongitudeOrigin, fileInfo.LatitudeOrigin, 0};
      double pt3[3]={fileInfo.LongitudeOrigin, fileInfo.LatitudeOrigin-
        (this->Internals->NumberOfRows - 1)*fileInfo.PointSpacing, 0};
      this->LatLongTransform1->TransformPoint(pt1, pt1);
      this->LatLongTransform1->TransformPoint(pt3, pt3);

      vtkNew<vtkTransform> tmpTransform;
      tmpTransform->PreMultiply();
      tmpTransform->RotateWXYZ(rotationAngle, rotationAxis);
      tmpTransform->Translate(-pt[0], -pt[1], -pt[2]);
      tmpTransform->TransformPoint(pt3, pt3);
      tmpTransform->TransformPoint(pt1, pt1);

      double temppt[3]={pt1[0]-pt3[0], pt1[1]-pt3[1], pt1[2]-pt3[2]};
      this->ZRotationAngle = atan2(temppt[0],temppt[1]) * 180 / vtkMath::Pi();
      }

    this->LatLongTransform2->PreMultiply();
//    this->LatLongTransform2->Translate(pt[0], pt[1], pt[2]);
    if(this->ZRotationAngle !=0)
      {
      this->LatLongTransform2->RotateZ(this->ZRotationAngle);
      }
    this->LatLongTransform2->RotateWXYZ(rotationAngle, rotationAxis);
    this->LatLongTransform2->Translate(-pt[0], -pt[1], -pt[2]);
    }

  vtkPoints *newPts = vtkPoints::New();
  if (this->OutputDataTypeIsDouble)
    {
    newPts->SetDataTypeToDouble();
    }
  else
    {
    newPts->SetDataTypeToFloat();
    }
  vtkCellArray *newVerts = vtkCellArray::New();
  output->SetPoints( newPts );
  output->SetVerts( newVerts );
  newPts->UnRegister(this);
  newVerts->UnRegister(this);

  vtkIdType totalNumberOfPoints =
    this->Internals->OutputNumberOfRows * this->Internals->OutputNumberOfColumns;
  newPts->Allocate( totalNumberOfPoints );
  newVerts->Allocate( 2*totalNumberOfPoints );

  std::vector<RawDEMReaderFileInfo>::iterator file =
    this->Internals->FileInfo.begin();
  for (; file != this->Internals->FileInfo.end(); file++)
    {
    this->ReadData(*file, 0, newPts, newVerts);
    }
  newVerts->Squeeze();
  newPts->Squeeze();
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::ReadImageDataOutput(vtkImageData *output)
{
  vtkImageData *data = this->AllocateOutputData(output);
  vtkFloatArray *scalars = vtkFloatArray::SafeDownCast(data->GetPointData()->GetScalars());
  scalars;

  this->Internals->NoDataValueInstances.clear();

  std::vector<RawDEMReaderFileInfo>::iterator file =
    this->Internals->FileInfo.begin();
  for (; file != this->Internals->FileInfo.end(); file++)
    {
    this->ReadData(*file, scalars, 0, 0);
    }

  // fixup any "NoDataValue" instances
  std::vector<RowCol>::const_iterator noDataValueIter =
    this->Internals->NoDataValueInstances.begin();
  float noDataValue = this->Internals->FileInfo.front().NoDataValue;
  for (; noDataValueIter != this->Internals->NoDataValueInstances.end(); noDataValueIter++)
    {
    double sum = 0;
    vtkIdType sumCount = 0;
    for (vtkIdType row = noDataValueIter->Row - 1; row < noDataValueIter->Row + 2; row++)
      {
      if (row < 0 || row >= this->Internals->OutputNumberOfRows)
        {
        continue;
        }
      for (vtkIdType col = noDataValueIter->Column - 1; col < noDataValueIter->Column + 2; col++)
        {
        if (col < 0 || col >= this->Internals->OutputNumberOfColumns ||
          (col == noDataValueIter->Column && row == noDataValueIter->Row))
          {
          continue;
          }
        float value =
          scalars->GetValue(row * this->Internals->OutputNumberOfColumns + col);
        if (value != noDataValue)
          {
          sum += value;
          sumCount++;
          }
        }
      if (sumCount)
        {
        scalars->SetValue(noDataValueIter->Row * this->Internals->OutputNumberOfColumns +
          noDataValueIter->Column, sum/sumCount);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::ReadData(RawDEMReaderFileInfo &fileInfo,
                               vtkFloatArray *scalars,
                               vtkPoints *pts,
                               vtkCellArray *verts)
{
  ifstream fin;

  // read the binary "FLT" file
  std::string fileNameStr =
    vtksys::SystemTools::GetFilenamePath( fileInfo.FileName ) + "/" +
    vtksys::SystemTools::GetFilenameWithoutLastExtension( fileInfo.FileName ) + ".FLT";

  fin.open(fileNameStr.c_str(), ios::binary);

  // is there even anything to read from this file
  if (this->Internals->Extents[2] >= fileInfo.Offset[1] + fileInfo.NumberOfRows ||
    this->Internals->Extents[3] < fileInfo.Offset[1] ||
    this->Internals->Extents[0] >= fileInfo.Offset[0] + fileInfo.NumberOfColumns ||
    this->Internals->Extents[1] < fileInfo.Offset[0])
    {
    return;
    }

  // from onRatio and Row/Column offsets, determine if we need to skip at the
  // beginning rows / columns
  vtkIdType onRatio = static_cast<vtkIdType>(floor(sqrt(
    static_cast<double>(this->Internals->OnRatio))));
  vtkIdType topRowRelativeToExtent = this->Internals->Extents[3] -
    (fileInfo.Offset[1] + fileInfo.NumberOfRows - 1);
  vtkIdType initialRowSkip;
  if (topRowRelativeToExtent > 0)
    {
    initialRowSkip = topRowRelativeToExtent % onRatio == 0 ? 0 :
      onRatio - (topRowRelativeToExtent % onRatio);
    }
  else
    {
    initialRowSkip = (fileInfo.Offset[1] + fileInfo.NumberOfRows - 1) -
      this->Internals->Extents[3];
    }
  if (initialRowSkip)
    {
    fin.seekg(4 * initialRowSkip * fileInfo.NumberOfColumns, ios::cur);
    }
  vtkIdType leftColumnRelativeToExtent = fileInfo.Offset[0] - this->Internals->Extents[0];
  vtkIdType initialColumnSkip;
  if (leftColumnRelativeToExtent > 0)
    {
    initialColumnSkip = leftColumnRelativeToExtent % onRatio == 0 ?
      0 : onRatio - (leftColumnRelativeToExtent % onRatio);
    }
  else
    {
    initialColumnSkip = this->Internals->Extents[0] - fileInfo.Offset[0];
    }

  // how many rows/ columns do we not read at the end
  vtkIdType lastPossibleRow = fileInfo.NumberOfRows - 1;
  if (this->Internals->Extents[2] > fileInfo.Offset[1])
    {
    lastPossibleRow -= this->Internals->Extents[2] - fileInfo.Offset[1];
    }
  vtkIdType numberOfRows = 1 + (lastPossibleRow - initialRowSkip) / onRatio;

  vtkIdType lastPossibleColumn = fileInfo.NumberOfColumns - 1;
  if (this->Internals->Extents[1] < fileInfo.Offset[0] + fileInfo.NumberOfColumns - 1)
    {
    lastPossibleColumn -=
      (fileInfo.Offset[0] + fileInfo.NumberOfColumns - 1) - this->Internals->Extents[1];
    }
  vtkIdType numberOfColumns = 1 + (lastPossibleColumn - initialColumnSkip) / onRatio;

  // how much do we skip at the end of each row
  vtkIdType lastActualColumn = initialColumnSkip + (numberOfColumns - 1) * onRatio;
  vtkIdType endRowSkip = (fileInfo.NumberOfColumns - 1) - lastActualColumn;
  // should be integer (no remainder) results
  vtkIdType outputRowOffset = ((fileInfo.Offset[1] + fileInfo.NumberOfRows - 1) -
    initialRowSkip - this->Internals->Extents[2]) / onRatio;
  vtkIdType outputColumnOffset =
    (fileInfo.Offset[0] + initialColumnSkip - this->Internals->Extents[0]) / onRatio;

  // since data comes in from upper-left, we start with 1st row for output
  // as "top" row... thus, due to skipping, we might not output some # of
  // bottom rows, and thus need to adjust origin accordingly
  double dataOrigin[3] = {
     /*this->Internals->LongitudeMidPoint + */fileInfo.LongitudeOrigin +
      static_cast<double>(this->Internals->Extents[0]) * fileInfo.PointSpacing,
    /*this->Internals->LatitudeMidPoint + */fileInfo.LatitudeOrigin -
    static_cast<double>(this->Internals->NumberOfRows - 1 - this->Internals->Extents[2]) * fileInfo.PointSpacing, 0};

  double rowProgressDelta = static_cast<double>(numberOfColumns) /
    (static_cast<double>(this->Internals->OutputNumberOfColumns) * static_cast<double>(this->Internals->OutputNumberOfRows));

  vtkBoundingBox bbox;
  float rawData;
  for (vtkIdType row = 0; row < numberOfRows; row++)
    {
    vtkIdType outputImageOffset = outputColumnOffset +
      (outputRowOffset - row) * this->Internals->OutputNumberOfColumns;
    if (initialColumnSkip)
      {
      fin.ignore(4 * initialColumnSkip);
      }
    for (vtkIdType column = 0; column < numberOfColumns; column++)
      {
      fin.read(reinterpret_cast<char *>(&rawData), 4);

      if (scalars)
        {
        // set, regardless of whether "NoDataValue", but if NoDataValue,
        // save location to allow fixing
        scalars->SetValue(outputImageOffset + column, rawData);
        if (rawData == fileInfo.NoDataValue)
          {
          RowCol tmp(outputRowOffset - row, outputColumnOffset + column);
          this->Internals->NoDataValueInstances.push_back(tmp);
          }
        }
      else
        {
        double pt[3];
        pt[0] = dataOrigin[0] +
          fileInfo.PointSpacing * static_cast<double>(column * onRatio + initialColumnSkip);
        pt[1] = dataOrigin[1] -
          fileInfo.PointSpacing * static_cast<double>(row * onRatio + initialRowSkip);
        pt[2] = rawData;

        if (pt[2] != fileInfo.NoDataValue)
          {
          if (this->ConvertFromLatLongToXYZ)
            {
            this->LatLongTransform1->TransformPoint(pt, pt);
            if (this->TransformForZUp)
              {
              this->LatLongTransform2->TransformPoint(pt, pt);
              if (this->RemoveCurvature)
                {
                pt[2] = rawData;
                }
              }
            }
          bbox.AddPoint(pt[0], pt[1], pt[2]);

          // add the point, but 1st make sure it is in the ReadBounds (if specified);
          // consider the Transform if set (and "on")
          double transformedPt[3];
          bool addPt = true;
          if (this->Transform)
            {
            // only need the transformed pt if we're limiting read based on bounds or
            // we're transforming the output
            if (this->LimitReadToBounds || this->TransformOutputData)
              {
              this->Transform->TransformPoint(pt, transformedPt);
              }
            if (this->LimitReadToBounds &&
              !this->ReadBBox.ContainsPoint(transformedPt[0], transformedPt[1], transformedPt[2]))
              {
              addPt = false;
              }
            }
          else // not transformed, use as read in
            {
            if (this->LimitReadToBounds && !this->ReadBBox.ContainsPoint(pt[0], pt[1], pt[2]))
              {
              addPt = false;
              }
            }

          if(addPt)
            {
            vtkIdType outputIdx;
            if (this->Transform && this->TransformOutputData)
              {
              outputIdx = pts->InsertNextPoint(transformedPt);
              }
            else
              {
              //double tmppts[3]={pt[0], pt[2], pt[1]};
              outputIdx = pts->InsertNextPoint(pt);
              }
            verts->InsertNextCell(1, &outputIdx);
            }
          }
        }

      if (column == numberOfColumns - 1 && endRowSkip) // read the rest of the row
        {
        fin.ignore(4 * endRowSkip);
        }
      else if (onRatio > 1)
        {
        fin.ignore(4 * (onRatio - 1));
        }
      }
    this->UpdateProgress( this->GetProgress() + rowProgressDelta );
//    this->UpdateProgress( (double)(row + 1) / (double)numberOfRows );
    if (this->GetAbortExecute())
      {
      break;
      }

    if (onRatio > 1)
      {
      if (row < numberOfRows - 1) // skip over appropriate # of rows
        {
        fin.seekg(4 * (onRatio - 1) * fileInfo.NumberOfColumns, ios::cur);
        }
      }
    }
  fin.close();
  this->UpdateProgress( 1.0 );

  //// set our DataBounds
  bbox.GetBounds(this->DataBounds);
}

//-----------------------------------------------------------------------------
vtkIdType vtkRawDEMReader::GetTotalNumberOfPoints()
{
  if (this->Internals->GetFilesInfo(this->FileName, this->ReadSetOfFiles) == READ_ERROR)
    {
    return -1;
    }
  this->SetupReadExtents();

  return this->Internals->TotalNumberOfPoints;
}

//-----------------------------------------------------------------------------
vtkIdType vtkRawDEMReader::GetRealNumberOfOutputPoints()
{
  if (this->Internals->GetFilesInfo(this->FileName, this->ReadSetOfFiles) == READ_ERROR)
    {
    return -1;
    }
  this->SetupReadExtents();

  return this->RealNumberOfOutputPoints;
}

//-----------------------------------------------------------------------------
void vtkRawDEMReader::GatherDimensions()
{
  if (this->Internals->GetFilesInfo(this->FileName, this->ReadSetOfFiles) == READ_ERROR)
    {
    this->Dimensions[0] = -1;
    this->Dimensions[1] = -1;
    return;
    }
  this->Dimensions[0] = this->Internals->NumberOfColumns;
  this->Dimensions[1] = this->Internals->NumberOfRows;
}


//-----------------------------------------------------------------------------
void vtkRawDEMReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}

//-----------------------------------------------------------------------------
int vtkRawDEMReader::RequestDataObject(
  vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector) ,
  vtkInformationVector* outputVector)
{
  int outputType = VTK_POLY_DATA;
  if (this->OutputImageData)
    {
    outputType = VTK_IMAGE_DATA;
    }

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if (output && (output->GetDataObjectType() == outputType))
    {
    return 1;
    }

  if (!output || output->GetDataObjectType() != outputType)
    {
    switch (outputType)
      {
      case VTK_POLY_DATA:
        output = vtkPolyData::New();
        break;
      case VTK_IMAGE_DATA:
        output = vtkImageData::New();
        break;
      default:
        return 0;
      }

    this->GetExecutive()->SetOutputData(0, output);
    output->Delete();
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkRawDEMReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  // get the info objects, needed for vtkImageData output
  if (this->OutputImageData && vtksys::SystemTools::FileExists(this->FileName))
  {
    int res = this->Internals->GetFilesInfo(this->FileName, this->ReadSetOfFiles);
    if(res == READ_ERROR)
      {
      return 0;
      }
    else if(res == READ_ABORT)
      {
      return 1;
      }

    if (this->Internals->FileInfo.size() == 0)
      {
      return 0;
      }
    this->SetupReadExtents();

    RawDEMReaderFileInfo &fileInfo = this->Internals->FileInfo.front();
    // since data comes in from upper-left, we start with 1st row for output
    // as "top" row... thus, due to skipping, we might not output some # of
    // bottom rows, and thus need to adjust origin accordingly
    double dataOrigin[3] = {
      fileInfo.LongitudeOrigin + this->Internals->Extents[0] * fileInfo.PointSpacing,
      fileInfo.LatitudeOrigin -
      (this->Internals->NumberOfRows - 1 - this->Internals->Extents[2]) * fileInfo.PointSpacing, 0};

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    int dataExtent[6] = {0,
      static_cast<int>(this->Internals->OutputNumberOfColumns - 1), 0,
      static_cast<int>(this->Internals->OutputNumberOfRows - 1), 0, 0};
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
      dataExtent, 6);
    double spacing[3] = {fileInfo.PointSpacing * this->Internals->OnRatio,
      fileInfo.PointSpacing * this->Internals->OnRatio, 0};
    outInfo->Set(vtkDataObject::SPACING(), spacing, 3);

    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, 1);
    outInfo->Set(vtkDataObject::ORIGIN(), dataOrigin, 3);
    }

  return 1;
}


//----------------------------------------------------------------------------
vtkImageData *vtkRawDEMReader::AllocateOutputData(vtkDataObject *output)
{
  // set the extent to be the update extent
  vtkImageData *out = vtkImageData::SafeDownCast(output);
  if (out)
    {
    // this needs to be fixed -Ken
    vtkStreamingDemandDrivenPipeline *sddp =
      vtkStreamingDemandDrivenPipeline::SafeDownCast(this->GetExecutive());
    int numInfoObj = sddp->GetNumberOfOutputPorts();
    if (sddp && numInfoObj == 1)
      {
      int extent[6];
      sddp->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),extent);
      out->SetExtent(extent);
      }
    else
      {
      vtkWarningMacro( "There are multiple output ports. You cannot use AllocateOutputData" );
      return NULL;
      }
    out->AllocateScalars(VTK_FLOAT, 1);
    }
  return out;
}
