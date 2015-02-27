/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelVertex.cxx

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
