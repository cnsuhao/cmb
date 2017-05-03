//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDEMRasterWriter.h"

#include "vtkAppendPolyData.h"
#include "vtkDataArray.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkUnsignedCharArray.h"

#include <sstream>

// GDAL includes
#include <gdal_priv.h>
#include <ogr_spatialref.h>

vtkStandardNewMacro(vtkDEMRasterWriter);

//-----------------------------------------------------------------------------
vtkDEMRasterWriter::vtkDEMRasterWriter()
{
  this->FileName = NULL;
  this->WriteAsSinglePiece = false;
  GDALAllRegister();
}

//-----------------------------------------------------------------------------
vtkDEMRasterWriter::~vtkDEMRasterWriter()
{
}

//----------------------------------------------------------------------------
void vtkDEMRasterWriter::WriteData()
{
  int numInputs = this->GetNumberOfInputConnections(0);
  if (numInputs > 1 && this->WriteAsSinglePiece)
  {
    // 1st append all pieces, then write
    vtkNew<vtkAppendPolyData> append;

    for (int idx = 0; idx < numInputs; ++idx)
    {
      vtkPolyData* inputPoly = vtkPolyData::SafeDownCast(this->GetInputFromPort0(idx));
      if (!inputPoly)
      {
        continue;
      }
      append->AddInputData(inputPoly);
    }
    append->Update();
    this->createOutputFile(this->FileName, append->GetOutput());
  }
  else if (numInputs == 1)
  {
    vtkPolyData* inputPoly = vtkPolyData::SafeDownCast(this->GetInputFromPort0(0));
    this->createOutputFile(this->FileName, inputPoly);
  }
  else
  {
    for (int idx = 0; idx < numInputs; ++idx)
    {
      vtkPolyData* inputPoly = vtkPolyData::SafeDownCast(this->GetInputFromPort0(idx));
      if (!inputPoly)
      {
        continue;
      }
      //create a file name if more than 1
      size_t lastindex = std::string(FileName).find_last_of('.');
      std::string rawname = std::string(FileName).substr(0, lastindex);
      std::stringstream ss;
      ss << idx;
      std::string tmp;
      ss >> tmp;
      rawname += "_" + tmp + ".dem";
      this->createOutputFile(rawname.c_str(), inputPoly);
    }
  }
}

//----------------------------------------------------------------------------
void vtkDEMRasterWriter::AddInputData(int index, vtkDataObject* input)
{
  if (input)
  {
    this->AddInputDataInternal(index, input);
  }
}

//----------------------------------------------------------------------------
int vtkDEMRasterWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkDEMRasterWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkDEMRasterWriter::GetInputFromPort0(int connection)
{
  return this->GetExecutive()->GetInputData(0, connection);
}

void vtkDEMRasterWriter::createOutputFile(std::string fname, vtkPolyData* inputPoly)
{
  if (inputPoly == NULL)
    return;
  std::map<vtkIdType, double> IdToElevation;
  vtkSmartPointer<vtkIncrementalOctreePointLocator> Locator;

  if (RasterSize[0] == -1 || RasterSize[1] == -1 || RadiusX == -1.0 || RadiusY == -1.0)
  {
    vtkErrorMacro("Write failed, INVALID SIZE OR RADIUS");
    return;
  }

  vtkPoints* points = vtkPoints::New();

  double p[3];

  unsigned int total;
  unsigned int count = 0;
  this->UpdateProgress(0.0);

  vtkPoints* inputPoints = inputPoly->GetPoints();
  vtkIdType size = inputPoints->GetNumberOfPoints();
  if (size == 0)
    return;
  inputPoints->GetPoint(0, p);
  double min[2] = { p[0], p[1] };
  double max[2] = { p[0], p[1] };
  double max_ele = fabs(p[2]);
  double min_ele = p[2];
  points->SetNumberOfPoints(size);

  total = size + RasterSize[0] * RasterSize[1] + 2;
  for (vtkIdType i = 0; i < size; ++i)
  {
    //get the point
    inputPoints->GetPoint(i, p);
    // check elevation limit
    //store the z value & flatten
    IdToElevation[i] = p[2];
    if (p[2] < min_ele)
      min_ele = p[2];
    if (p[2] > fabs(max_ele))
      max_ele = fabs(p[2]);
    p[2] = 0.0;
    points->InsertPoint(i, p);
    for (unsigned int a = 0; a < 2; ++a)
    {
      if (min[a] > p[a])
        min[a] = p[a];
      if (max[a] < p[a])
        max[a] = p[a];
    }
    if ((i % 100) == 0)
    {
      this->UpdateProgress(static_cast<double>(++count) / static_cast<double>(total));
      if (this->GetAbortExecute())
      {
        return;
      }
    }
  }

  vtkPolyData* pointSet = vtkPolyData::New();
  pointSet->SetPoints(points);

  Locator = vtkSmartPointer<vtkIncrementalOctreePointLocator>::New();
  Locator->AutomaticOn();
  Locator->SetTolerance(0.0);
  Locator->SetDataSet(pointSet);
  Locator->BuildLocator();

  double space[2] = { (max[0] - min[0]) / RasterSize[0], (max[1] - min[1]) / RasterSize[1] };

  unsigned int nobX = RasterSize[0];
  unsigned int nobY = RasterSize[1];

  double* matix = new double[nobX * nobY];
  double rx2 = this->RadiusX * this->RadiusX;
  double ry2 = this->RadiusY * this->RadiusY;
  double r = std::max(rx2, ry2);

  double max_b = -1;
  for (unsigned int x = 0; x < nobX; ++x)
  {
    double dpoint[3];
    dpoint[0] = min[0] + x * space[0] + space[0] * 0.5;
    assert(dpoint[0] <= max[0]);
    for (unsigned int y = 0; y < nobY; ++y)
    {
      dpoint[1] = min[1] + y * space[1] + space[1] * 0.5;
      assert(dpoint[1] <= max[1]);
      dpoint[2] = 0.0;
      vtkIdList* ids = vtkIdList::New();
      Locator->FindPointsWithinSquaredRadius(r, dpoint, ids);
      double sum = 0;
      double weight = 0;
      std::map<vtkIdType, double>::const_iterator it;
      vtkIdType size2 = ids->GetNumberOfIds();
      for (vtkIdType i = 0; i < size2; ++i)
      {
        it = IdToElevation.find(ids->GetId(i));
        if (it != IdToElevation.end())
        {
          inputPoints->GetPoint(it->first, p);
          double d = ((p[0] - dpoint[0]) * (p[0] - dpoint[0])) / rx2 +
            ((p[1] - dpoint[1]) * (p[1] - dpoint[1])) / ry2;
          if (d >= 1)
            continue;
          double w = 1 - d;
          if (w > 1)
            w = 1.0;
          if (w < 0)
            w = 0.0;
          sum += w * it->second;
          weight += w;
        }
      }
      double v = sum / ((weight != 0) ? weight : 1.0);
      if (max_b < v)
        max_b = v;
      unsigned int at = (nobY - y - 1) * nobX + x;
      assert(at < nobX * nobY);
      if (weight != 0)
      {
        double r2 = v / this->Scale;
        matix[at] = r2;
      }
      else
      {
        matix[at] = -32767;
      }
      ids->Delete();
      if ((y % 100) == 0)
      {
        this->UpdateProgress(static_cast<double>(++count) / static_cast<double>(total));
        if (this->GetAbortExecute())
        {
          return;
        }
      }
    }
  }

  const char* pszFormat = "MEM";
  GDALDriver* poDriver;

  poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

  if (poDriver == NULL)
  {
    vtkErrorMacro("Write failed, GDAL was not built with MEM driver");
    return;
  }

  GDALDataset* ds = poDriver->Create("", nobX, nobY, 1, GDT_Float64, NULL);

  double adfGeoTransform[6] = { min[0], space[0], 0, max[1], 0, -space[1] };
  ds->SetGeoTransform(adfGeoTransform);

  OGRSpatialReference oSRS;
  char* pszSRS_WKT = NULL;

  if (this->Zone != -1)
  {
    oSRS.SetUTM(this->Zone, this->IsNorth);
    oSRS.SetWellKnownGeogCS("NAD27");
    oSRS.exportToWkt(&pszSRS_WKT);
  }
  else if (inputPoly->GetFieldData()->HasArray("GeoInfoProj4"))
  {
    vtkStringArray* vsa =
      vtkStringArray::SafeDownCast(inputPoly->GetFieldData()->GetAbstractArray("GeoInfoProj4"));
    vtkStdString& vss = vsa->GetValue(0);
    oSRS.importFromProj4(vss.c_str());
    oSRS.exportToWkt(&pszSRS_WKT);
  }
  else
  {
    vtkErrorMacro("Write failed.  Cannot process Negative Zone");
    return;
  }

  ds->SetProjection(pszSRS_WKT);
  CPLFree(pszSRS_WKT);

  GDALRasterBand* poBand;
  poBand = ds->GetRasterBand(1);
  poBand->SetNoDataValue(-32767);
  poBand->RasterIO(GF_Write, 0, 0, nobX, nobY, matix, nobX, nobY, GDT_Float64, 0, 0);

  const char* demFormat = "USGSDEM";
  GDALDriver* demDriver;

  demDriver = GetGDALDriverManager()->GetDriverByName(demFormat);
  char** demOptions = NULL;

  std::stringstream ss;
  ss << this->Scale;
  std::string tmp;
  ss >> tmp;

  demOptions = CSLSetNameValue(demOptions, "ZRESOLUTION", tmp.c_str());
  GDALDataset* poDstDS =
    demDriver->CreateCopy(fname.c_str(), ds, FALSE, demOptions, GDALTermProgress, NULL);
  if (poDstDS != NULL)
  {
    GDALClose((GDALDatasetH)poDstDS);
  }
  else
  {
    vtkErrorMacro("Write failed, Could not create a USGSDEM");
  }
  CSLDestroy(demOptions);

  points->FastDelete();
  pointSet->Delete();
  this->UpdateProgress(1.0);
}
