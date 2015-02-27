/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelEdge.cxx

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
#include "pqCMBModelEdge.h"
#include "pqCMBModel.h"
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
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelLoopUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkEdgeSplitOperatorClient.h"

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
#include "vtkDiscreteModelVertex.h"

#include <QVariant>

//-----------------------------------------------------------------------------
pqCMBModelEdge::pqCMBModelEdge()
{
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBModelEdge::init()
{
  this->Superclass::init();
}

//-----------------------------------------------------------------------------
pqCMBModelEdge::~pqCMBModelEdge()
{
}

//-----------------------------------------------------------------------------
vtkDiscreteModelEdge* pqCMBModelEdge::getModelEdgeEntity() const
{
  return vtkDiscreteModelEdge::SafeDownCast(this->ModelEntity);
}

//-----------------------------------------------------------------------------
pqCMBModelEdge* pqCMBModelEdge::createObject(
  vtkSMProxy* modelWrapper, vtkDiscreteModelEdge* cmbModelEdge,
  pqServer *server, pqRenderView *view, bool visible)
{
  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  QPointer<pqPipelineSource> polySource = builder->createSource(
        "CMBModelGroup", "PolyDataProvider", server);

  if(polySource)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(polySource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->AddProxy(modelWrapper);
    pqSMAdaptor::setElementProperty(polySource->getProxy()->GetProperty("EntityId"),
      cmbModelEdge->GetUniquePersistentId());
    pqSMAdaptor::setElementProperty(polySource->getProxy()->GetProperty("ItemType"),  vtkModelEdgeType);
    polySource->getProxy()->UpdateVTKObjects();

    return pqCMBModelEdge::createObject(
      polySource, server, view, cmbModelEdge, modelWrapper, visible);
    }

  return NULL;
}

//-----------------------------------------------------------------------------
pqCMBModelEdge *pqCMBModelEdge::createObject(pqPipelineSource* source,
                                                     pqServer * /*server*/,
                                                     pqRenderView * /*view*/,
                                                     vtkDiscreteModelEdge* cmbModelEdge,
                                                     vtkSMProxy* modelWrapper,
                                                     bool /*visible*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBModelEdge *obj = new pqCMBModelEdge();
  obj->PolyProvider = source;
  obj->setModelEntity(cmbModelEdge);
  obj->setModelWrapper(modelWrapper);

  return obj;
}

//-----------------------------------------------------------------------------
void pqCMBModelEdge::setEdgePointsVisibility(int visible)
{
  vtkSMSourceProxy* polyProvider = vtkSMSourceProxy::SafeDownCast(
    this->PolyProvider->getProxy());

  if(!polyProvider)
    {
    return;
    }
  pqSMAdaptor::setElementProperty(
    polyProvider->GetProperty("CreateEdgePointVerts"),  visible);
  polyProvider->UpdateVTKObjects();
  polyProvider->UpdatePipeline();
  this->updateRepresentation(NULL);
}

//-----------------------------------------------------------------------------
bool pqCMBModelEdge::splitSelectedNodes(const QList<vtkIdType>& selPointIds,
  pqCMBModel* CMBModel, vtkSMProxy* ModelWrapper,
  QList<vtkIdType>& newEdges, QList<vtkIdType>& newVTXs)
{
  if( selPointIds.count() ==0 )
    {
    return false;
    }
  vtkDiscreteModelEdge* cmbModelEdge = vtkDiscreteModelEdge::SafeDownCast(
    this->ModelEntity);
  if(!cmbModelEdge)
    {
    return false;
    }

  vtkSmartPointer<vtkEdgeSplitOperatorClient> ModelEntityOperator =
    vtkSmartPointer<vtkEdgeSplitOperatorClient>::New();
  ModelEntityOperator->SetEdgeId(this->getUniqueEntityId());

  vtkDiscreteModelVertex* cmbModelVertex1 =
    vtkDiscreteModelVertex::SafeDownCast(cmbModelEdge->GetAdjacentModelVertex(0));
  vtkDiscreteModelVertex* cmbModelVertex2 =
    vtkDiscreteModelVertex::SafeDownCast(cmbModelEdge->GetAdjacentModelVertex(1));
  vtkIdType vtx1 = cmbModelVertex1 ? cmbModelVertex1->GetUniquePersistentId() : -1;
  vtkIdType vtx2 = cmbModelVertex2 ? cmbModelVertex2->GetUniquePersistentId() : -1;
  pqCMBModelVertex* cmbVert1 = vtx1<0 ? NULL : qobject_cast<pqCMBModelVertex*>(
    CMBModel->Get2DVertexID2VertexMap()[vtx1]);
  pqCMBModelVertex* cmbVert2 = vtx2<0 ? NULL : qobject_cast<pqCMBModelVertex*>(
    CMBModel->Get2DVertexID2VertexMap()[vtx2]);

  // Find the first point Id that is not a model vertex already
  int numSelPoints = selPointIds.count();
  vtkIdType pointId = -1;
  for (int cc=0; cc < numSelPoints; cc++)
    {
    vtkIdType pId = selPointIds.value(cc);
    if((cmbVert1 && cmbVert1->getPointId() == pId) ||
       (cmbVert2 && cmbVert2->getPointId() == pId))
      {
      continue;
      }
    pointId = pId;
    break;
    }
  if(pointId < 0)
    {
    return false;
    }
  ModelEntityOperator->SetPointId(pointId);

  if(ModelEntityOperator->Operate(CMBModel->getModel(), ModelWrapper))
    {
    newEdges.append(ModelEntityOperator->GetCreatedModelEdgeId());
    newVTXs.append(ModelEntityOperator->GetCreatedModelVertexId());
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
void pqCMBModelEdge::colorByColorMode(
  vtkDiscreteLookupTable* lut, int colorMode)
{
  if(!lut)
    {
    return;
    }

  vtkDiscreteModelEdge* edgeEntity = this->getModelEdgeEntity();
  vtkModelItemIterator* iterFace = edgeEntity->NewAdjacentModelFaceIterator();
  iterFace->Begin();
  vtkModelFace* faceEntity = vtkModelFace::SafeDownCast(iterFace->GetCurrentItem());
  iterFace->Delete();

  vtkModelEntity* modelEntity = NULL;
  vtkModelItemIterator* matIte = NULL;
  switch (colorMode)
    {
    case ColorByEntity:
      modelEntity = edgeEntity;
      break;
    case ColorByDomain:
      modelEntity = faceEntity;
      break;
    case ColorByMaterial:
      matIte = faceEntity->NewIterator(vtkModelMaterialType);
      matIte->Begin();
      modelEntity = vtkModelMaterial::SafeDownCast(matIte->GetCurrentItem());
      matIte->Delete();
      break;
    case ColorByBCS:
      if(edgeEntity->GetNumberOfModelEntityGroups())
        {
        vtkModelItemIterator* iter=edgeEntity->NewIterator(vtkDiscreteModelEntityGroupType);
        iter->Begin();
        modelEntity = vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem());
        iter->Delete();
        }
      break;
    default:
      break;
    }

  double rgb[3];
  double opacity = 1.0;
  this->getColor(modelEntity, rgb, opacity, lut);
  this->setRepresentationColor(rgb);
}

//-----------------------------------------------------------------------------
int pqCMBModelEdge::GetAdjacentModelFaces(QList<vtkModelEntity*>& faces)
{
  int numAdjFaces = 0;
  vtkModelItemIterator* edgeUses =
    this->ModelEntity->NewIterator(vtkModelEdgeUseType);
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelItemIterator* loopUses =
      edgeUses->GetCurrentItem()->NewIterator(vtkModelLoopUseType);
    for(loopUses->Begin();!loopUses->IsAtEnd();loopUses->Next())
      {
      vtkModelFace* face = vtkModelLoopUse::SafeDownCast(
        loopUses->GetCurrentItem())->GetModelFace();
      if(!faces.contains(face))
        {
        faces.append(face);
        numAdjFaces++;
        }
      }
    loopUses->Delete();
    }
  edgeUses->Delete();

  return numAdjFaces;
}
