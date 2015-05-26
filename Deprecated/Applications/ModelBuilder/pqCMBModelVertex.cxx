//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBModelVertex.h"

#include "pqActiveServer.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkMath.h"

#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkGeometryRepresentation.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkPVModelVertexObjectInformation.h"

#include <QVariant>

//-----------------------------------------------------------------------------
pqCMBModelVertex::pqCMBModelVertex()
{
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBModelVertex::init()
{
  this->Superclass::init();
}

//-----------------------------------------------------------------------------
pqCMBModelVertex::~pqCMBModelVertex()
{
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBModelVertex::getPointId()
{
  if(!this->PolyProvider || !this->PolyProvider->getProxy())
    {
    return -1;
    }
  vtkSMSourceProxy::SafeDownCast(
    this->PolyProvider->getProxy() )->UpdatePipeline();

  vtkNew<vtkPVModelVertexObjectInformation> vertexInfo;
  this->PolyProvider->getProxy()->GatherInformation(vertexInfo.GetPointer());
  return vertexInfo->GetPointId();
}
//-----------------------------------------------------------------------------
vtkDiscreteModelVertex* pqCMBModelVertex::getModelVertexEntity() const
{
  return vtkDiscreteModelVertex::SafeDownCast(this->ModelEntity);
}

//-----------------------------------------------------------------------------
pqCMBModelVertex* pqCMBModelVertex::createObject(
  vtkSMProxy* modelWrapper, vtkDiscreteModelVertex* CMBModelVertex,
  pqServer *server, pqRenderView *view, bool updateRep)
{
  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();
  QPointer<pqPipelineSource> polySource = builder->createSource(
    "CMBModelGroup", "PolyDataProvider", server);

  //QPointer<pqPipelineSource> polySource = builder->createSource(
  //      "sources", "SphereSource", server);
  if(polySource)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(polySource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->AddProxy(modelWrapper);
    pqSMAdaptor::setElementProperty(polySource->getProxy()->GetProperty("EntityId"),
      CMBModelVertex->GetUniquePersistentId());
    pqSMAdaptor::setElementProperty(polySource->getProxy()->GetProperty("ItemType"),  vtkModelVertexType);

    polySource->getProxy()->UpdateVTKObjects();

    return pqCMBModelVertex::createObject(
      polySource, view, CMBModelVertex, modelWrapper, updateRep);
    }

  return NULL;
}

//-----------------------------------------------------------------------------
pqCMBModelVertex *pqCMBModelVertex::createObject(pqPipelineSource* source,
                                                    // pqServer *server,
                                                     pqRenderView * /*view*/,
                                                     vtkDiscreteModelVertex* CMBModelVertex,
                                                     vtkSMProxy* modelWrapper,
                                                     bool /*updateRep*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBModelVertex *obj = new pqCMBModelVertex();
  obj->PolyProvider = source;
  obj->setModelEntity(CMBModelVertex);
  obj->setModelWrapper(modelWrapper);
  obj->setColor(0.0, 1.0, 0.0, 1.0);
  obj->setRepresentationColor(0.0, 1.0, 0.0);

  return obj;
}
