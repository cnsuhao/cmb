//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================



#include "pqCMBUniformGrid.h"


#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkGeometryRepresentation.h"
#include <vtkProcessModule.h>
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMIdTypeVectorProperty.h>
#include "vtkSMNewWidgetRepresentationProxy.h"
#include <vtkSMPropertyHelper.h>
#include "vtkSMRepresentationProxy.h"
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>
#include <vtkXMLDataSetWriter.h>
#include "vtkSMProxyManager.h"
#include "vtkImageData.h"
#include <QFileInfo>
#include <QVariant>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

//-----------------------------------------------------------------------------
pqCMBUniformGrid::pqCMBUniformGrid() : pqCMBSceneObjectBase()
{
  HasInvalidValue = false;
}
//-----------------------------------------------------------------------------
pqCMBUniformGrid::pqCMBUniformGrid(pqPipelineSource *source,
                                         pqRenderView *view,
                                         pqServer *server,
                                         const char *filename,
                                         bool updateRep)
  : pqCMBSceneObjectBase(source)
{
  this->FileName = filename;
  QFileInfo finfo(filename);
  this->prepGridObject(server, view, updateRep,
                       finfo.completeSuffix().toLower() != "tif");
  HasInvalidValue = false;
}

//-----------------------------------------------------------------------------
pqCMBUniformGrid::pqCMBUniformGrid(const char *filename,
                                             pqServer *server,
                                             pqRenderView *view,
                                             bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << filename;

  builder->blockSignals(true);
  pqPipelineSource* source;
  QFileInfo finfo(filename);
  if (this->isRawDEM(filename))
      {
      source =  builder->createReader("sources", "RawDEMReader", files, server);
      vtkSMPropertyHelper(source->getProxy(), "OutputImageData").Set(1);
      }
  else if(finfo.completeSuffix().toLower() == "dem" || finfo.completeSuffix().toLower() == "tif")
      {
      //TEST_DEM_TO_MESH(filename);
      source =  builder->createReader("sources", "GDALRasterReader", files, server);
      HasInvalidValue = true;
      }
  else
      {
      source =  builder->createReader("sources", "XMLImageDataReader", files, server);
      }
  pqPipelineSource* mesh = builder->createFilter("filters", "cmbStructedToMesh", source);
  vtkSMPropertyHelper(mesh->getProxy(), "UseScalerForZ").Set(0);
  builder->blockSignals(false);
  this->Source = mesh;
  this->ImageSource = source;
  this->setFileName(filename);
  this->prepGridObject(server, view, updateRep, true);
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::prepGridObject(pqServer *server,
                                      pqRenderView *view,
                                      bool updateRep,
                                      bool transferColor)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  // this->Representation =
  //   builder->createDataRepresentation(
  //   this->Source->getOutputPort(0), view, "UniformGridRepresentation");
  this->setRepresentation(
      builder->createDataRepresentation(
        this->Source->getOutputPort(0), view));
  // If there is an elevation field on the points then use it.
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->getRepresentation()->getProxy(),
    "Elevation", vtkDataObject::POINT);

  // use CMB Elevation 2 Color Table
  if(transferColor)
    {
    vtkSMProxy* lut = builder->createProxy("lookup_tables", "PVLookupTable",
                                         server, "transfer_functions");
    QList<QVariant> values;
    values << -5000.0  << 0.0 << 0.0 << 0
	   << -1000.0  << 0.0 << 0.0 << 1.0
	   <<  -100.0  << 0.129412 << 0.345098 << 0.996078
	   <<   -50.0  << 0.0 << 0.501961 << 1.0
	   <<   -10.0  << 0.356863 << 0.678431 << 1.0
	   <<    -0.0  << 0.666667 << 1.0 << 1.0
	   <<     0.01 << 0.0 << 0.250998 << 0.0
	   <<    10.0 << 0.301961 << 0.482353 << 0.0
	   <<    25.0 << 0.501961 << 1.0 << 0.501961
	   <<   500.0 << 0.188224 << 1.0 << 0.705882
	   <<  1000.0 << 1.0 << 1.0 << 0.0
	   <<  2500.0 << 0.505882 << 0.211765 << 0.0
	   <<  3200.0 << 0.752941 << 0.752941 << 0.752941
	   <<  6000.0 << 1.0 << 1.0 << 1.0;
    pqSMAdaptor::setMultipleElementProperty(lut->GetProperty("RGBPoints"), values);
    vtkSMPropertyHelper(lut, "ColorSpace").Set(0);
    vtkSMPropertyHelper(lut, "Discretize").Set(0);
    lut->UpdateVTKObjects();
    pqSMAdaptor::setProxyProperty(
      this->getRepresentation()->getProxy()->GetProperty("LookupTable"), lut);
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "SelectionVisibility").Set(0);
    }
  else
    {
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "MapScalars").Set(0);
    }
  this->Source->getProxy()->UpdateVTKObjects();
  if (updateRep)
      {
      this->getRepresentation()->getProxy()->UpdateVTKObjects();
      }
}
//-----------------------------------------------------------------------------
pqCMBUniformGrid::~pqCMBUniformGrid()
{
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::updatePolyDataStats()
{
}

//-----------------------------------------------------------------------------
pqCMBUniformGrid::enumObjectType pqCMBUniformGrid::getType() const
{
  return pqCMBSceneObjectBase::UniformGrid;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBUniformGrid::duplicate(pqServer *server,
                                               pqRenderView *view,
                                               bool updateRep)
{
  pqCMBUniformGrid *dup = new pqCMBUniformGrid(this->FileName.c_str(),
                                                     server, view, false);
  if (this->isRawDEM())
    {
      dup->setOnRatio(this->getOnRatio());
      vtkIdType re[2], ce[2];
      this->getExtents(re, ce);
      dup->setExtents(re,ce);
      dup->setReadGroupOfFiles(this->getReadGroupOfFiles());
    }
  if (updateRep)
    {
    dup->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  return dup;
}


//-----------------------------------------------------------------------------
bool pqCMBUniformGrid::isRawDEM(const QString &filename)
{
  QFileInfo finfo(filename);
  if (finfo.completeSuffix().toLower() == "flt" ||
    finfo.completeSuffix().toLower() == "ftw" ||
    finfo.completeSuffix().toLower() == "hdr")
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBUniformGrid::isRawDEM() const
{
  return isRawDEM(QString(this->FileName.c_str()));
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::getDimensions(vtkIdType dims[2])
{
  this->ImageSource->getProxy()->InvokeCommand("GatherDimensions");
  this->ImageSource->getProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "Dimensions").Get(dims, 2);
}
//-----------------------------------------------------------------------------
void pqCMBUniformGrid::setReadGroupOfFiles(bool mode)
{
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "ReadSetOfFiles").Set(mode);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
bool pqCMBUniformGrid::getReadGroupOfFiles() const
{
  int mode;
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "ReadSetOfFiles").Get(&mode, 1);
  return (mode != 0);
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::setOnRatio(int r)
{
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "OnRatio").Set(r);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
int pqCMBUniformGrid::getOnRatio() const
{
  int r;
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "OnRatio").Get(&r, 1);
  return r;
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::setExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2])
{
  vtkSMPropertyHelper(this->ImageSource->getProxy(),
                      "RowReadExtents").Set(rowExtents, 2);
  vtkSMPropertyHelper(this->ImageSource->getProxy(),
                      "ColumnReadExtents").Set(columnExtents, 2);
  this->Source->getProxy()->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBUniformGrid::getExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2]) const
{
  vtkSMPropertyHelper(this->ImageSource->getProxy(),
                      "RowReadExtents").Get(rowExtents, 2);
  vtkSMPropertyHelper(this->ImageSource->getProxy(),
                      "ColumnReadExtents").Get(columnExtents, 2);
}
//-----------------------------------------------------------------------------
void pqCMBUniformGrid::getAreaStats(double* areaStats)
{
  // Calculate the number of cells
  double n = static_cast<double>(this->getNumberOfPoints());
  double a = this->getSurfaceArea();
  if (n != 0.0)
    {
    areaStats[0] = areaStats[1] = areaStats[2] = a / n;
    }
  else
    {
    areaStats[0] = areaStats[1] = areaStats[2] = 0.0;
    }
}
//-----------------------------------------------------------------------------
void pqCMBUniformGrid::getGeometryBounds(double* geoBounds) const
{
  this->getBounds(geoBounds);
}
//-----------------------------------------------------------------------------
void pqCMBUniformGrid::getPolySideStats(double* polySide)
{
  polySide[0] = polySide[1] = polySide[2] = 4;
}
//-----------------------------------------------------------------------------
double pqCMBUniformGrid::getSurfaceArea()
{
  double b[6];
  this->getBounds(b);
  double a = (b[1] - b[0]) * (b[3] - b[2]);
  return a;
}
//-----------------------------------------------------------------------------
vtkIdType pqCMBUniformGrid::getNumberOfPoints()
{
  vtkPVDataInformation *imageInfo = this->Source->getOutputPort(0)->getDataInformation();
  return imageInfo->GetNumberOfPoints();
}
//-----------------------------------------------------------------------------
vtkIdType pqCMBUniformGrid::getNumberOfPolygons()
{
  vtkPVDataInformation *imageInfo = this->Source->getOutputPort(0)->getDataInformation();
  return imageInfo->GetNumberOfCells();
}
//-----------------------------------------------------------------------------
bool pqCMBUniformGrid::hasInvalidValue()
{
  return HasInvalidValue;
}
//-----------------------------------------------------------------------------
double pqCMBUniformGrid::invalidValue()
{
  double v;
  vtkSMPropertyHelper(this->ImageSource->getProxy(), "InvalidValue").Get(&v, 1);
  return v;
}
