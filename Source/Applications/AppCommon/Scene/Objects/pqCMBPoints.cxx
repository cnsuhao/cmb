/*=========================================================================

  Program:   CMB
  Module:    pqCMBPoints.cxx

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



#include "pqCMBPoints.h"

#include "pqActiveServer.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include <vtkProcessModule.h>
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMNewWidgetRepresentationProxy.h"
#include <vtkSMPropertyHelper.h>
#include "vtkSMRepresentationProxy.h"
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>
#include "vtkSMProxyManager.h"
#include "vtkImageData.h"
#include <QFileInfo>
#include <QVariant>

#include "vtkNew.h"

//-----------------------------------------------------------------------------
pqCMBPoints::pqCMBPoints() : pqCMBTexturedObject()
{
  this->DoubleDataPrecision = false;
  this->InitialSurfaceTranslation[0] = InitialSurfaceTranslation[1] =
    InitialSurfaceTranslation[2] = 0.0;
  this->setPieceId( -1 );
  this->PieceOnRatio = 1;
  this->PieceTotalNumberOfPoints = -1;
  this->UserDefinedType = "Points";
}
//-----------------------------------------------------------------------------
pqCMBPoints::pqCMBPoints(pqPipelineSource *source,
                               pqRenderView *view,
                               pqServer *server,
                               const char *filename)
  : pqCMBTexturedObject(source, view, server)
{
  this->setPieceId( -1 );
  this->PieceOnRatio = 1;
  this->PieceTotalNumberOfPoints = -1;

  this->FileName = filename;
  this->DoubleDataPrecision = false;
  this->InitialSurfaceTranslation[0] = InitialSurfaceTranslation[1] =
    InitialSurfaceTranslation[2] = 0.0;

  this->UserDefinedType = "Points";
}

//-----------------------------------------------------------------------------
pqCMBPoints::pqCMBPoints(pqPipelineSource *source,
                               pqRenderView *view,
                               pqServer *server,
                               bool updateRep)
  : pqCMBTexturedObject(source, view, server)
{
  this->setPieceId( -1 );
  this->PieceOnRatio = 1;
  this->PieceTotalNumberOfPoints = -1;

  this->DoubleDataPrecision = false;
  this->InitialSurfaceTranslation[0] = InitialSurfaceTranslation[1] =
    InitialSurfaceTranslation[2] = 0.0;

  this->UserDefinedType = "Points";

  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
pqCMBPoints::pqCMBPoints(const char *filename,
                               pqServer *server,
                               pqRenderView *view,
                               int maxNumberOfPoints,
                               bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << filename;

  builder->blockSignals(true);
  pqPipelineSource* source;
  QList<pqCMBPoints*> dataObjects;
  source =  builder->createReader("sources", "LIDARReader", files, server);

  vtkSMPropertyHelper(source->getProxy(), "MaxNumberOfPoints").Set(maxNumberOfPoints);
  vtkSMPropertyHelper(source->getProxy(), "LimitToMaxNumberOfPoints").Set(1);
  source->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();
  builder->blockSignals(false);

  this->initialize(source, server, view, updateRep);
  this->setFileName(filename);
  this->setReaderSource( source );
  this->setPieceId( -1 );
  this->PieceOnRatio = 1;
  this->PieceTotalNumberOfPoints = -1;
  this->DoubleDataPrecision = false;
  source->getProxy()->UpdatePropertyInformation();
  vtkNew<vtkPVSceneGenObjectInformation> info;
  source->getProxy()->GatherInformation(info.GetPointer());
  int totalNumberOfPoints = pqSMAdaptor::getElementProperty(
    source->getProxy()->GetProperty("TotalNumberOfPoints")).toInt();
  this->setPieceTotalNumberOfPoints( totalNumberOfPoints );
  this->UserDefinedType = "Points";

  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
pqCMBPoints::pqCMBPoints(pqServer *server,
                               pqRenderView *view,
                               pqPipelineSource* source,
                               int pieceIndex,
                               int onRatio,
                               bool doublePrecision)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  vtkSMSourceProxy *sourceProxy =
    vtkSMSourceProxy::SafeDownCast( source->getProxy() );

  QList<QVariant> pieceOnRatioList;
  pieceOnRatioList << pieceIndex << onRatio;

  pqSMAdaptor::setMultipleElementProperty(
    sourceProxy->GetProperty("RequestedPiecesForRead"), pieceOnRatioList);
  pqSMAdaptor::setElementProperty(
    sourceProxy->GetProperty("OutputDataTypeIsDouble"), doublePrecision);
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();

  int numberOfPointsInPiece;
  pqPipelineSource *pdSource = builder->createSource("sources",
    "HydroModelPolySource", server);
  // if from the LAS reader then need to extract the output from the multiblock
  if (!strcmp("vtkLASReader", sourceProxy->GetVTKClassName()))
    {
    pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

    // expecting only 1 block
    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), 0);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

    vtkSMDataSourceProxy::SafeDownCast(
      pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(extract->getProxy()) );
    pdSource->updatePipeline();

    vtkNew<vtkPVLASOutputBlockInformation> info;
    pdSource->getProxy()->GatherInformation(info.GetPointer());
    numberOfPointsInPiece = info->GetNumberOfPointsInClassification();
    builder->destroy(extract);
    }
  else
    {
    vtkSMDataSourceProxy::SafeDownCast( pdSource->getProxy() )->CopyData(sourceProxy);
    // get total # of points in piece, and save the value
    pqSMAdaptor::setElementProperty(
      sourceProxy->GetProperty("PieceIndex"), pieceIndex);
    sourceProxy->UpdateVTKObjects();
    sourceProxy->UpdatePropertyInformation();

    numberOfPointsInPiece = pqSMAdaptor::getElementProperty(
      sourceProxy->GetProperty("NumberOfPointsInPiece")).toInt();
    }
  this->initialize(pdSource, server, view, false);
  this->setPieceTotalNumberOfPoints( numberOfPointsInPiece );
  this->setReaderSource(source);
  this->setPieceId(pieceIndex);
  this->setPieceOnRatio(onRatio);
  this->setDoubleDataPrecision(doublePrecision);
  this->UserDefinedType = "Points";
}

//-----------------------------------------------------------------------------
pqCMBPoints::~pqCMBPoints()
{
}

//-----------------------------------------------------------------------------
void pqCMBPoints::setReaderSource(pqPipelineSource *source)
{
  this->ReaderSource = source;
}

//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBPoints::getReaderSource() const
{
  return this->ReaderSource;
}

//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBPoints::getTransformedSource(pqServer *server) const
{
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  this->getTransform(transform);
  vtkMatrix4x4 *matrix = transform->GetMatrix();
  // if non-identity transform... need to transform the data
  if (matrix->Element[0][0] != 1 || matrix->Element[0][1] != 0 ||
    matrix->Element[0][2] != 0 || matrix->Element[0][3] != 0 ||
    matrix->Element[1][0] != 0 || matrix->Element[1][1] != 1 ||
    matrix->Element[1][2] != 0 || matrix->Element[1][3] != 0 ||
    matrix->Element[2][0] != 0 || matrix->Element[2][1] != 0 ||
    matrix->Element[2][2] != 1 || matrix->Element[2][3] != 0 ||
    matrix->Element[3][0] != 0 || matrix->Element[3][1] != 0 ||
    matrix->Element[3][2] != 0 || matrix->Element[3][3] != 1)
    {
    QList<QVariant> values;
    for (int i = 0; i < 4; i++)
      {
      for (int j = 0; j < 4; j++)
        {
        values << matrix->Element[i][j];
        }
      }
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

    vtkSMProxy *transformProxy = builder->createProxy("transforms", "Transform",
      server, "transforms");
    pqSMAdaptor::setMultipleElementProperty(
      transformProxy->GetProperty("Matrix"), values);
    transformProxy->UpdateVTKObjects();

    pqPipelineSource *transformFilter = builder->createFilter( "filters",
      "TransformFilter", this->Source);
    pqSMAdaptor::setProxyProperty(
      transformFilter->getProxy()->GetProperty("Transform"), transformProxy);
    transformFilter->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( transformFilter->getProxy() )->UpdatePipeline();

    pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", server);
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(transformFilter->getProxy()));
    builder->destroy(transformFilter);

    return pdSource;
    }
  return this->Source;
}


//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBPoints::duplicate(pqServer *server,
                                                    pqRenderView *view,
                                                    bool updateRep)
  {
    pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
    vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));

  pqCMBPoints *nobj = new pqCMBPoints(pdSource, view, server,
                                                    this->FileName.c_str());
  this->duplicateInternals(nobj);

  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  nobj->DoubleDataPrecision = this->DoubleDataPrecision;
  nobj->InitialSurfaceTranslation[0] = this->InitialSurfaceTranslation[0];
  nobj->InitialSurfaceTranslation[1] = this->InitialSurfaceTranslation[1];
  nobj->InitialSurfaceTranslation[2] = this->InitialSurfaceTranslation[2];
  nobj->ReaderSource = this->ReaderSource;
  nobj->FileName = this->FileName;
  nobj->PieceTotalNumberOfPoints = this->PieceTotalNumberOfPoints;
  nobj->PieceId = this->PieceId;
  nobj->PieceOnRatio = this->PieceOnRatio;
  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBPoints::initialize(pqPipelineSource* source,
                                pqServer *server,
                                pqRenderView *view,
                                bool updateRep)
{
  this->InitialSurfaceTranslation[0] = InitialSurfaceTranslation[1] =
    InitialSurfaceTranslation[2] = 0.0;
  this->Source = source;
  this->prepTexturedObject(server, view);
  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
bool pqCMBPoints::isPointsFile(const char *filename)
{
  QFileInfo finfo(filename);
  if (finfo.completeSuffix().toLower() == "pts" ||
     finfo.completeSuffix().toLower() == "bin" ||
     finfo.completeSuffix().toLower() == "bin.pts" ||
     finfo.completeSuffix().toLower() == "las")
    {
    return true;
    }
  return false;
}
//-----------------------------------------------------------------------------
void pqCMBPoints::setInitialSurfaceTranslation(double translation[3])
{
    this->InitialSurfaceTranslation[0] = translation[0];
    this->InitialSurfaceTranslation[1] = translation[1];
    this->InitialSurfaceTranslation[2] = translation[2];
    }
//-----------------------------------------------------------------------------
void pqCMBPoints::getInitialSurfaceTranslation(double translation[3]) const
{
  translation[0] = this->InitialSurfaceTranslation[0];
  translation[1] = this->InitialSurfaceTranslation[1];
  translation[2] = this->InitialSurfaceTranslation[2];
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBPoints::getType() const
{
  return pqCMBSceneObjectBase::Points;
}
//-----------------------------------------------------------------------------
