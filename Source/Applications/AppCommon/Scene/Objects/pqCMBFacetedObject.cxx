//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBFacetedObject.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkImageData.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRepresentationProxy.h"
#include <QFileInfo>
#include <QVariant>
#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>

pqCMBFacetedObject::pqCMBFacetedObject()
  : pqCMBTexturedObject()
{
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

pqCMBFacetedObject::pqCMBFacetedObject(
  pqPipelineSource* source, pqRenderView* view, pqServer* server, const char* filename)
  : pqCMBTexturedObject(source, view, server)
{
  this->FileName = filename;
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

pqCMBFacetedObject::pqCMBFacetedObject(
  pqPipelineSource* source, pqServer* server, pqRenderView* view, bool updateRep)
  : pqCMBTexturedObject(source, view, server)
{
  if (updateRep)
  {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
  }
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

pqCMBFacetedObject::pqCMBFacetedObject(
  const char* filename, pqServer* server, pqRenderView* view, bool /*updateRep*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << filename;

  builder->blockSignals(true);
  pqPipelineSource* source;
  QFileInfo finfo(filename);
  source = builder->createReader("sources", "CMBGeometryReader", files, server);
  builder->blockSignals(false);
  this->Source = source;
  this->prepTexturedObject(server, view);
  this->setFileName(filename);
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

pqCMBFacetedObject::~pqCMBFacetedObject()
{
}

pqCMBFacetedObject::enumObjectType pqCMBFacetedObject::getType() const
{
  return pqCMBSceneObjectBase::Faceted;
}

pqPipelineSource* pqCMBFacetedObject::getTransformedSource(pqServer* server) const
{
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  this->getTransform(transform);
  vtkMatrix4x4* matrix = transform->GetMatrix();
  // if non-identity transform... need to transform the data
  if (matrix->Element[0][0] != 1 || matrix->Element[0][1] != 0 || matrix->Element[0][2] != 0 ||
    matrix->Element[0][3] != 0 || matrix->Element[1][0] != 0 || matrix->Element[1][1] != 1 ||
    matrix->Element[1][2] != 0 || matrix->Element[1][3] != 0 || matrix->Element[2][0] != 0 ||
    matrix->Element[2][1] != 0 || matrix->Element[2][2] != 1 || matrix->Element[2][3] != 0 ||
    matrix->Element[3][0] != 0 || matrix->Element[3][1] != 0 || matrix->Element[3][2] != 0 ||
    matrix->Element[3][3] != 1)
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

    vtkSMProxy* transformProxy =
      builder->createProxy("transforms", "Transform", server, "transforms");
    pqSMAdaptor::setMultipleElementProperty(transformProxy->GetProperty("Matrix"), values);
    transformProxy->UpdateVTKObjects();

    pqPipelineSource* transformFilter =
      builder->createFilter("filters", "TransformFilter", this->Source);
    pqSMAdaptor::setProxyProperty(
      transformFilter->getProxy()->GetProperty("Transform"), transformProxy);
    transformFilter->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(transformFilter->getProxy())->UpdatePipeline();

    pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
      ->CopyData(vtkSMSourceProxy::SafeDownCast(transformFilter->getProxy()));
    builder->destroy(transformFilter);

    return pdSource;
  }
  return this->Source;
}

pqPipelineSource* pqCMBFacetedObject::duplicatePipelineSource(pqServer* server)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));
  return pdSource;
}

pqCMBSceneObjectBase* pqCMBFacetedObject::duplicate(
  pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));

  pqCMBFacetedObject* nobj = new pqCMBFacetedObject(pdSource, view, server, this->FileName.c_str());
  this->duplicateInternals(nobj);
  nobj->SurfaceType = this->SurfaceType;

  if (updateRep)
  {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
  }

  return nobj;
}

std::string pqCMBFacetedObject::getSurfaceTypeAsString() const
{
  return this->convertSurfaceTypeToString(this->SurfaceType);
}
