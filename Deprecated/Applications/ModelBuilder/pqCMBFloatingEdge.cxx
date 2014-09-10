/*=========================================================================

  Program:   CMB
  Module:    pqCMBFloatingEdge.cxx

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
#include "pqCMBFloatingEdge.h"

#include "pqActiveServer.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelEdgeOperatorClient.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"

#include "vtkProcessModule.h"
#include "vtkPVModelVertexObjectInformation.h"
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

#include <QTableWidgetItem>

//-----------------------------------------------------------------------------
pqCMBFloatingEdge::pqCMBFloatingEdge()
{
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBFloatingEdge::init()
{
  this->Superclass::init();
  this->PolyProvider = NULL;
  //this->Representation = NULL;
  this->HighlightColor[0] = 1.0;
  this->HighlightColor[1] = 0.0;
  this->HighlightColor[2] = 1.0;
  this->OriginalColor[0] = this->OriginalColor[1] = this->OriginalColor[2] = 1.0;
  this->OrigColorArrayName = "";
  this->selected = false;
}

//-----------------------------------------------------------------------------
pqCMBFloatingEdge::~pqCMBFloatingEdge()
{
}

//-----------------------------------------------------------------------------
vtkDiscreteModelEdge* pqCMBFloatingEdge::getModelEdgeEntity()
{
  return vtkDiscreteModelEdge::SafeDownCast(this->ModelEntity);
}

//-----------------------------------------------------------------------------
int pqCMBFloatingEdge::updateLineSource(
  vtkSMProxy* modelWrapper, vtkDiscreteModelEdge* cmbModelEdge, pqPipelineSource* polySource)
{
  double point1[3], point2[3];
  vtkPVModelVertexObjectInformation *pdSourceInfo =
    vtkPVModelVertexObjectInformation::New();
  vtkDiscreteModelVertex* cmbModelVertex1 =
    vtkDiscreteModelVertex::SafeDownCast(cmbModelEdge->GetAdjacentModelVertex(0));

  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();
  QPointer<pqPipelineSource> vertexSource = builder->createSource(
    "CMBModelGroup", "PolyDataProvider", polySource->getServer());

  vtkSMProxyProperty* proxyproperty =
    vtkSMProxyProperty::SafeDownCast(vertexSource->getProxy()->GetProperty("ModelWrapper"));
  proxyproperty->AddProxy(modelWrapper);
  pqSMAdaptor::setElementProperty(vertexSource->getProxy()->GetProperty("EntityId"),
                                  cmbModelVertex1->GetUniquePersistentId());
  pqSMAdaptor::setElementProperty(vertexSource->getProxy()->GetProperty("ItemType"),
                                  vtkModelVertexType);
  vertexSource->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast( vertexSource->getProxy() )->UpdatePipeline();

  vertexSource->getProxy()->GatherInformation(pdSourceInfo);
  if(!pdSourceInfo->GetIsInfoValid())
    {
    pdSourceInfo->Delete();
    return 0;
    }
  pdSourceInfo->GetLocation(point1);

  vtkDiscreteModelVertex* cmbModelVertex2 =
    vtkDiscreteModelVertex::SafeDownCast(cmbModelEdge->GetAdjacentModelVertex(1));
  pqSMAdaptor::setElementProperty(vertexSource->getProxy()->GetProperty("EntityId"),
                                  cmbModelVertex2->GetUniquePersistentId());
  vertexSource->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast( vertexSource->getProxy() )->UpdatePipeline();

  vertexSource->getProxy()->GatherInformation(pdSourceInfo);
  if(!pdSourceInfo->GetIsInfoValid())
    {
    pdSourceInfo->Delete();
    return 0;
    }

  pdSourceInfo->GetLocation(point2);
  pdSourceInfo->Delete();

  QList<QVariant> values;
  values <<point1[0] << point1[1] << point1[2];
  pqSMAdaptor::setMultipleElementProperty(
    polySource->getProxy()->GetProperty("Point1"), values);
  values.clear();
  values <<point2[0] << point2[1] << point2[2];
  pqSMAdaptor::setMultipleElementProperty(
    polySource->getProxy()->GetProperty("Point2"), values);

  int lineResolution = cmbModelEdge->GetLineResolution();
  polySource->getProxy()->UpdateVTKObjects();
  pqSMAdaptor::setElementProperty(
    polySource->getProxy()->GetProperty("Resolution"),  lineResolution);

  polySource->getProxy()->UpdateVTKObjects();
  polySource->updatePipeline();
  return 1;
}

//-----------------------------------------------------------------------------
pqCMBFloatingEdge* pqCMBFloatingEdge::createObject(
  vtkSMProxy* modelWrapper, vtkDiscreteModelEdge* cmbModelEdge,
  pqServer *server, pqRenderView *view, bool updateRep)
{
  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  QPointer<pqPipelineSource> lineSource = builder->createSource(
        "sources", "ModelLineSource", server);

  if(lineSource)
    {
    if(pqCMBFloatingEdge::updateLineSource(modelWrapper, cmbModelEdge, lineSource))
      {
      //QPointer<pqPipelineSource> glyphLine = builder->createFilter("filters",
      //    "Glyph", lineSource);
      //if(glyphLine)
      //  {
      //  QPointer<pqPipelineSource> sphereSource = builder->createSource(
      //        "sources", "SphereSource", server);
      //  pqSMAdaptor::setElementProperty(
      //    sphereSource->getProxy()->GetProperty("Radius"),  0.2);
      //  sphereSource->getProxy()->UpdateVTKObjects();
      //
      //  vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
      //    glyphLine->getProxy()->GetProperty("Source"));
      //  pp->AddProxy( sphereSource->getProxy() );
      //  pqSMAdaptor::setElementProperty(
      //    glyphLine->getProxy()->GetProperty("FillCellData"),  1);
      //  glyphLine->getProxy()->UpdateVTKObjects();

       // }
      return pqCMBFloatingEdge::createObject(
        lineSource, server, view, cmbModelEdge, modelWrapper, updateRep);
      }
    else
      {
      builder->destroy(lineSource);
      }
    }

  return NULL;
}

//-----------------------------------------------------------------------------
pqCMBFloatingEdge *pqCMBFloatingEdge::createObject(
  pqPipelineSource *linesource, pqServer * /*server*/,
  pqRenderView *view,vtkDiscreteModelEdge* cmbModelEdge,
  vtkSMProxy* modelWrapper, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBFloatingEdge *obj = new pqCMBFloatingEdge();
  obj->PolyProvider = linesource;
  obj->setModelEntity(cmbModelEdge);
  obj->setModelWrapper(modelWrapper);

  vtkSMSourceProxy::SafeDownCast(obj->PolyProvider->getProxy() )->UpdatePipeline();

  obj->Representation =
    builder->createDataRepresentation(obj->PolyProvider->getOutputPort(0),
    view, "GeometryRepresentation");

  pqSMAdaptor::setElementProperty(
    obj->Representation->getProxy()->GetProperty("Pickable"), 0);
  pqSMAdaptor::setElementProperty(
    obj->Representation->getProxy()->GetProperty("LineWidth"), 2);
  pqSMAdaptor::setElementProperty(
    obj->Representation->getProxy()->GetProperty("PointSize"), 4);

  if (updateRep)
    {
    obj->Representation->getProxy()->UpdateVTKObjects();
    }

  return obj;
}

//-----------------------------------------------------------------------------
int pqCMBFloatingEdge::getLineResolution()
{
  return this->getModelEdgeEntity()->GetLineResolution();
  //vtkSMProxy *source = this->PolyProvider->getProxy();
  //return pqSMAdaptor::getElementProperty(
  //  source->GetProperty("Resolution")).toInt();
}

//-----------------------------------------------------------------------------
void pqCMBFloatingEdge::setLineResolution(
  int res, vtkDiscreteModel* Model, vtkSMProxy* ModelWrapper)
{
  if(res >= 1 && res != this->getLineResolution())
    {
    vtkSmartPointer<vtkModelEdgeOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEdgeOperatorClient>::New();
    ModelEntityOperator->SetItemType(vtkModelEdgeType);
    ModelEntityOperator->SetId(this->getUniqueEntityId());
    ModelEntityOperator->SetLineResolution(res);
    if(ModelEntityOperator->Operate(Model, ModelWrapper))
      {
      this->getModelEdgeEntity()->SetLineResolution(res);
      this->updateLineSource(ModelWrapper, this->getModelEdgeEntity(), this->PolyProvider);
      //vtkSMRepresentationProxy::SafeDownCast(
      //  this->Representation->getProxy())->UpdatePipeline();
      }
    }
}

//----------------------------------------------------------------------------
void pqCMBFloatingEdge::select()
{
  this->setRepresentationColor(this->HighlightColor);
  this->selected = true;
}

//----------------------------------------------------------------------------
void pqCMBFloatingEdge::deselect()
{
  this->setRepresentationColor(this->OriginalColor);
  this->selected = false;
}
