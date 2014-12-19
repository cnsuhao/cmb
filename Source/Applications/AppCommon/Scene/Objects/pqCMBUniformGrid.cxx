/*=========================================================================

  Program:   CMB
  Module:    pqCMBUniformGrid.cxx

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



#include "pqCMBUniformGrid.h"

#include "pqActiveServer.h"
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

#include "vtkGDALRasterReader.h"
#include "vtkDEMToMesh.h"

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"
/*
void TEST_DEM_TO_MESH(std::string fname)
{
  vtkGDALRasterReader * GDAL = vtkGDALRasterReader::New();
  GDAL->SetFileName(fname.c_str());
  GDAL->Update();
  vtkDEMToMesh * Mesher = vtkDEMToMesh::New();
  Mesher->SetUseScalerForZ(false);
  Mesher->SetInputData(GDAL->GetOutput());
  Mesher->Update();
  vtkXMLDataSetWriter* const writer = vtkXMLDataSetWriter::New();
  writer->SetInputData(Mesher->GetOutput());
  writer->SetFileName("test_mesh.vtp");
  writer->Write();
  Mesher->SetUseScalerForZ(true);
  Mesher->SetInputData(GDAL->GetOutput());
  Mesher->Update();
  writer->SetInputData(Mesher->GetOutput());
  writer->SetFileName("test_mesh3D.vtp");
  writer->Write();
}
*/
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
  this->prepGridObject(server, view, updateRep);
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
  else if(finfo.completeSuffix().toLower() == "dem")
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
  this->prepGridObject(server, view, updateRep);
}

//-----------------------------------------------------------------------------
void pqCMBUniformGrid::prepGridObject(pqServer *server,
                                         pqRenderView *view,
                                         bool updateRep)
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

  // use "Bob's" color table
  vtkSMProxy* lut = builder->createProxy("transfer_functions", "ColorTransferFunction",
                                         server, "transfer_functions");
  QList<QVariant> values;
  values << -1000.0 << 0.0 << 0.0 << 0
         << 1000.0 << 0.0 << 0.0 << 0.498
         << 2000.0 << 0.0 << 0.0 << 1.0
         << 2200.0 << 0.0 << 1.0 << 1.0
         << 2400.0 << 0.333 << 1.0 << 0.0
         << 2600.0 << 0.1216 << 0.3725 << 0.0
         << 2800.0 << 1.0 << 1.0 << 0.0
         << 3000.0 << 1.0 << 0.333 << 0.0;
  pqSMAdaptor::setMultipleElementProperty(
      lut->GetProperty("RGBPoints"), values);
  lut->UpdateVTKObjects();
  pqSMAdaptor::setProxyProperty(
      this->getRepresentation()->getProxy()->GetProperty("LookupTable"), lut);
  vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "SelectionVisibility").Set(0);
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
