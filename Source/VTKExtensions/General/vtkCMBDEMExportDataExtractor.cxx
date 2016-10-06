//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBDEMExportDataExtractor.h"
#include <vtkPolyData.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkInformationVector.h>
#include <vtkStringArray.h>
#include <vtkInformation.h>
#include <vtkIdList.h>
#include <vtkFieldData.h>

// GDAL includes
#include <gdal_priv.h>
#include <ogr_spatialref.h>

vtkStandardNewMacro(vtkCMBDEMExportDataExtractor);

//-----------------------------------------------------------------------------
vtkCMBDEMExportDataExtractor::vtkCMBDEMExportDataExtractor()
{
  //this->SetNumberOfOutputPorts(0);
  this->Modified();
}

vtkCMBDEMExportDataExtractor::~vtkCMBDEMExportDataExtractor()
{
}

int vtkCMBDEMExportDataExtractor::RequestData(vtkInformation* /*request*/,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* /*outputVector*/)
{
  //NOTE: We might want to reconsider this for varied sampled input data
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData * inputPoly = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkIncrementalOctreePointLocator> Locator;

  double p[3];

  vtkPoints *points = vtkPoints::New();

  vtkPoints *inputPoints = inputPoly->GetPoints();
  vtkIdType size = inputPoints->GetNumberOfPoints();
  if(size == 0) return 0;
  inputPoints->GetPoint(0,p);
  double min[2] = {p[0], p[1]};
  double max[2] = {p[0], p[1]};
  points->SetNumberOfPoints(size);
  double max_z = p[2];
  for (vtkIdType i=0; i < size; ++i)
    {
    //get the point
    inputPoints->GetPoint(i,p);
    if(max_z < fabs(p[2])) max_z = fabs(p[2]);
    p[2] = 0.0;
    points->InsertPoint(i,p);
    for(unsigned int a = 0; a < 2; ++a)
      {
      if(min[a]>p[a])min[a] = p[a];
      if(max[a]<p[a])max[a] = p[a];
      }
    }

  this->Min[0] = min[0];
  this->Min[1] = min[1];

  this->Max[0] = max[0];
  this->Max[1] = max[1];

  //double initSize = sqrt(static_cast<double>(size));
  double w = (this->Max[0]-this->Min[0]);
  double l = (this->Max[1]-this->Min[1]);

  this->Spacing[0] =  this->Spacing[1] = sqrt((w*l)/size);
  //this->Spacing[0] = w/initSize;
  //this->Spacing[1] = l/initSize;
  this->Scale = max_z/32766.0;

  if(inputPoly->GetFieldData()->HasArray("GeoInfoProj4"))
    {
    vtkStringArray * vsa = vtkStringArray::SafeDownCast(inputPoly->GetFieldData()->GetAbstractArray("GeoInfoProj4"));
    vtkStdString & vss = vsa->GetValue(0);
    OGRSpatialReference oSRS;
    oSRS.importFromProj4(vss.c_str());

    int in;
    Zone = oSRS.GetUTMZone(&in);
    IsNorth = (in == TRUE);
    }
  else
    {
    Zone = -1;
    IsNorth = true;
    }

  return 1;
}

int vtkCMBDEMExportDataExtractor::FillInputPortInformation(int vtkNotUsed(port), vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}
