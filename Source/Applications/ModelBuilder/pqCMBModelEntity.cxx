/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelEntity.cxx

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
#include "pqCMBModelEntity.h"

#include "pqCMBTreeItem.h"
#include "pqCMBModel.h"
#include "pqActiveServer.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelItemIterator.h"

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
#include "vtkPVDataInformation.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyLink.h"
#include "vtkModelEntityOperatorClient.h"
#include "vtkModelGeometricEntity.h"

#include "vtkCMBModelEntityMesh.h"
#include "vtkPVModelGeometryInformation.h"
#include "vtkNew.h"

#include <QVariant>
//-----------------------------------------------------------------------------
pqCMBModelEntity::pqCMBModelEntity()
{
  this->init();
}
//-----------------------------------------------------------------------------
pqCMBModelEntity::~pqCMBModelEntity()
  {
  this->clearWidgets();
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->Representation)
    {
    builder->destroy(this->Representation);
    this->Representation = NULL;
    }

  if (this->PolyProvider)
    {
    builder->destroy(this->PolyProvider);
    this->PolyProvider = NULL;
    }

  if (this->MeshRepresentation)
    {
    builder->destroy(this->MeshRepresentation);
    this->MeshRepresentation = NULL;
    }

  if (this->MeshBathymetryFilter)
    {
    builder->destroy(this->MeshBathymetryFilter);
    this->MeshBathymetryFilter = NULL;
    }
  if (this->MeshPolyProvider)
    {
    builder->destroy(this->MeshPolyProvider);
    this->MeshPolyProvider = NULL;
    }
}
//-----------------------------------------------------------------------------
void pqCMBModelEntity::init()
{
  this->ModelEntity = NULL;
  this->UniqueEntityId = -1;
  this->PolyProvider = NULL;
  this->Representation = NULL;
  this->MeshEntity = NULL;
  this->MeshPolyProvider = NULL;
  this->MeshRepresentation = NULL;
  this->MeshBathymetryFilter = NULL;
  this->MeshEdgePointsVisible = 0;
  this->MeshFaceRepresentationType = "Wireframe"; //wire frame
  this->clearWidgets();
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::setModelEntity(vtkModelEntity* entity)
{
  this->ModelEntity = entity;
  this->UniqueEntityId = this->ModelEntity->GetUniquePersistentId();
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::addModelWidget(pqCMBTreeItem* widget)
{
  if(!this->Widgets.contains(widget))
    {
    this->Widgets.append(widget);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::removeModelWidget(pqCMBTreeItem* widget)
{
  int findex = this->Widgets.indexOf(widget);
  if(findex >=0)
    {
    this->Widgets.removeAt(findex);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::clearWidgets()
{
  this->Widgets.clear();
}

//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBModelEntity::getSource() const
{
  return this->PolyProvider;
}

//-----------------------------------------------------------------------------
pqDataRepresentation * pqCMBModelEntity::getRepresentation() const
{
  return this->Representation;
}
//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBModelEntity::getMeshSource() const
{
  return (!this->MeshBathymetryFilter ? this->MeshPolyProvider :
    this->MeshBathymetryFilter);
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::updateRepresentation(vtkSMProxy* /*modelWrapper*/)
{
  if(!this->Representation)
    {
    return;
    }
  vtkSMSourceProxy* polyProvider = vtkSMSourceProxy::SafeDownCast(
    this->PolyProvider->getProxy());

  pqSMAdaptor::setElementProperty(
    polyProvider->GetProperty("EntityId"),  -1);
  polyProvider->UpdateVTKObjects();

  pqSMAdaptor::setElementProperty(
    polyProvider->GetProperty("EntityId"),
    this->ModelEntity->GetUniquePersistentId());
  polyProvider->UpdateVTKObjects();
  polyProvider->UpdatePipeline();

  vtkSMRepresentationProxy::SafeDownCast(
    this->Representation->getProxy())->UpdatePipeline();
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::setLODMode(int mode, bool updateRep)
{
  if(this->Representation)
    {
    vtkSMIntVectorProperty::SafeDownCast(this->Representation->
      getProxy()->GetProperty("SuppressLOD"))->SetElements1(mode);
    if (updateRep)
      {
      this->Representation->getProxy()->UpdateVTKObjects();
      }
    }
}
//-----------------------------------------------------------------------------
bool pqCMBModelEntity::getBounds(double bounds[6]) const
{
  if(this->Representation)
    {
    this->getRepresentation()->getOutputPortFromInput()->
      getDataInformation()->GetBounds(bounds);
    return true;
    }
  // Get the data bounds
  else if(this->PolyProvider)
    {
    vtkSMSourceProxy* polyProvider = vtkSMSourceProxy::SafeDownCast(
      this->PolyProvider->getProxy());
    polyProvider->UpdateVTKObjects();
    polyProvider->UpdatePipeline();
    polyProvider->UpdatePropertyInformation();
    polyProvider->GetDataInformation()->GetBounds(bounds);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setVisibility(int visible)
{
  this->ModelEntity->SetVisibility(visible);
  this->setRepresentationVisibility(this->Representation, visible);
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setRepresentationVisibility(
  pqDataRepresentation* rep, int visible)
{
  if(rep)
    {
    pqSMAdaptor::setElementProperty(
      rep->getProxy()->GetProperty("Visibility"), visible);
    pqSMAdaptor::setElementProperty(
      rep->getProxy()->GetProperty("SelectionVisibility"), visible);
  //  rep->getRepresentationProxy()->SetVisibility(visible);
  //  rep->getRepresentationProxy()->SetSelectionVisibility(visible);

    rep->getProxy()->UpdateVTKObjects();
    vtkSMRepresentationProxy::SafeDownCast(rep->getProxy())->UpdatePipeline();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setSelectionInput(vtkSMSourceProxy* selSource)
{
  vtkSMSourceProxy::SafeDownCast(this->PolyProvider->getProxy())->
    SetSelectionInput(0, selSource, 0);
}

//-----------------------------------------------------------------------------
int pqCMBModelEntity::getVisibility()
{
  if(this->Representation)
    {
    vtkSMProxy *rep = this->Representation->
      getProxy();
    return pqSMAdaptor::getElementProperty(
      rep->GetProperty("Visibility")).toInt();
    }
  else
    {
    return this->ModelEntity->GetVisibility();
    }
}

//-----------------------------------------------------------------------------
int pqCMBModelEntity::meshRepresentationVisible()
{
  if(this->MeshRepresentation)
    {
    vtkSMProxy *rep = this->MeshRepresentation->
      getProxy();
    return pqSMAdaptor::getElementProperty(
      rep->GetProperty("Visibility")).toInt();
    }
  return 0;
}

//-----------------------------------------------------------------------------
int pqCMBModelEntity::getPickable()
{
  if(this->Representation)
    {
    vtkSMProxy *rep = this->Representation->
      getProxy();
    return pqSMAdaptor::getElementProperty(
      rep->GetProperty("Pickable")).toInt();
    }
  else if(vtkModelGeometricEntity::SafeDownCast(this->ModelEntity))
    {
    return vtkModelGeometricEntity::SafeDownCast(
      this->ModelEntity)->GetPickable();
    }
  return 1;
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::select()
{
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
    pxm->NewProxy("sources", "cmbIDSelectionSource"));
  selSource->UpdateProperty("RemoveAllIDs", 1);

  pqSMAdaptor::setElementProperty(
    selSource->GetProperty("InsideOut"), 1);
  selSource->UpdateVTKObjects();

  this->setSelectionInput(selSource);
  selSource->Delete();
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::deselect()
{
  this->setSelectionInput(0);
}

//----------------------------------------------------------------------------
bool pqCMBModelEntity::hasSelectionInput()
{
  return vtkSMSourceProxy::SafeDownCast(
    this->PolyProvider->getProxy())->GetSelectionInput(0) ? true : false;
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setHighlight(int highlight)
{
  if(!this->getVisibility())
    {
    return;
    }

  if(highlight)
    {
    this->select();
    }
  else
    {
    this->deselect();
    }
}

//----------------------------------------------------------------------------
double* pqCMBModelEntity::getColor()
{
  return this->ModelEntity->GetColor();
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::getColor(float RGBA[4])
{
  double* rgba = this->getColor();
  for(int i=0;i<4;i++)
    {
    RGBA[i] = rgba[i];
    }
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setColor(
                                double r, double g, double b, double a)
{
  return this->ModelEntity->SetColor(r, g, b, a);
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::getColor(vtkModelEntity* modelEntity, double *rgb,
                                double &opacity, vtkDiscreteLookupTable* lut)
{
  int tIndex =  modelEntity ? modelEntity->GetUniquePersistentId() : -1;
  // should use average here? or a special color?
  // data->RootInfo->GetBCColorWithId(tIndex, RGBA);

  double RGBA[4] = {-1.0, -1.0, -1.0, 1.0};
  if(modelEntity)
    {
    modelEntity->GetColor(RGBA);
    }

  // if there is no color defined, pick one from the color table.
  // Note: The color from color table may collide
  // with the user defined colors
  if(RGBA[0]<0 || RGBA[1]<0.0 || RGBA[2]<0.0)
    {
    if(tIndex >=0)
      {
      int numColors = static_cast<int>(lut->GetNumberOfValues());
      lut->GetColor(tIndex%numColors,rgb);
      opacity = lut->GetOpacity(tIndex%numColors);
      }
    else
      {
      rgb[0] = rgb[1] = rgb[2] = 1.0;
      }
    }
  else
    {
    rgb[0]=RGBA[0];
    rgb[1]=RGBA[1];
    rgb[2]=RGBA[2];
    opacity = RGBA[3]<0 ? 1.0 : RGBA[3];
    }
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::setPickable(int pickable)
{
  if(this->ModelWrapper && this->ModelEntity)
    {
    vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(
      vtkModelGeometricEntity::SafeDownCast(this->ModelEntity)->GetModel());
    vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEntityOperatorClient>::New();
    ModelEntityOperator->SetItemType(this->ModelEntity->GetType());
    ModelEntityOperator->SetId(this->ModelEntity->GetUniquePersistentId());
    ModelEntityOperator->SetPickable(pickable);

    bool result = ModelEntityOperator->Operate(
      model, this->ModelWrapper);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelEntity::setRepresentationColor(
  double r, double g, double b, double opacity)
{
 // this->ModelEntity->SetRepresentationColor(r, g, b, opacity);

  if(this->ModelWrapper && this->ModelEntity)
    {
    vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(
      vtkModelGeometricEntity::SafeDownCast(this->ModelEntity)->GetModel());
    vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEntityOperatorClient>::New();
    ModelEntityOperator->SetItemType(this->ModelEntity->GetType());
    ModelEntityOperator->SetId(this->ModelEntity->GetUniquePersistentId());
    ModelEntityOperator->SetRepresentationRGBA(r, g, b, opacity);

    bool result = ModelEntityOperator->Operate(
      model, this->ModelWrapper);
    }


  if(this->Representation)
    {
    double rgb[3];
    rgb[0]=r;rgb[1]=g;rgb[2]=b;
    pqDataRepresentation* rep = this->getRepresentation();
    vtkSMPropertyHelper(rep->getProxy(), "DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(rep->getProxy(), "AmbientColor").Set(rgb, 3);
    vtkSMPropertyHelper(rep->getProxy(), "Opacity").Set(opacity);
    rep->getProxy()->UpdateVTKObjects();
    }
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::setMeshRepresentationColor(double r, double g, double b)
{
  if(this->MeshRepresentation && this->MeshRepresentation->getProxy())
    {
    double rgb[3];
    rgb[0]=r; rgb[1]=g; rgb[2]=b;
    vtkSMPropertyHelper(this->MeshRepresentation->getProxy(),
      "DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(this->MeshRepresentation->getProxy(),
      "AmbientColor").Set(rgb, 3);
    this->MeshRepresentation->getProxy()->UpdateVTKObjects();
    }
}
//-----------------------------------------------------------------------------
bool pqCMBModelEntity::getMeshRepresentationColor(double color[4]) const
{
  if(this->MeshRepresentation)
    {
    vtkSMPropertyHelper(this->MeshRepresentation->getProxy(),
      "DiffuseColor").Get(color, 3);
    color[3] = vtkSMPropertyHelper(this->MeshRepresentation->getProxy(),
      "Opacity").GetAsDouble();
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
vtkCMBModelEntityMesh* pqCMBModelEntity::getMeshEntity()
{
  return this->MeshEntity;
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::setMeshEntity(vtkCMBModelEntityMesh* entMesh)
{
  if(this->MeshEntity == entMesh)
    {
    return;
    }
  this->MeshEntity = entMesh;
  // clear whatever existing
  if(this->MeshPolyProvider)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    builder->destroy(this->MeshRepresentation);
    builder->destroy(this->MeshPolyProvider);
    this->MeshPolyProvider = NULL;
    this->MeshRepresentation = NULL;
    }
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::updateMeshEntity(pqCMBModel* model)
{
  if(!this->MeshEntity)
    {
    return;
    }
  if(this->MeshEntity->IsModelEntityMeshed() == false)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    if (this->MeshRepresentation)
      {
      builder->destroy(this->MeshRepresentation);
      this->MeshRepresentation = NULL;
      }
    if (this->MeshBathymetryFilter)
      {
      builder->destroy(this->MeshBathymetryFilter);
      this->MeshBathymetryFilter = NULL;
      }

    if (this->MeshPolyProvider)
      {
      builder->destroy(this->MeshPolyProvider);
      this->MeshPolyProvider = NULL;
      }
    return;
    }
  if(!this->MeshPolyProvider)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->MeshPolyProvider = builder->createSource(
      "CMBSimBuilderMeshGroup", "MeshPolyDataProvider",
      this->PolyProvider->getServer());
    }
  vtkSMSourceProxy* polySource = vtkSMSourceProxy::SafeDownCast(
    this->MeshPolyProvider->getProxy());
  vtkSMProxy* meshWrapper = model ? model->getServerMeshProxy() : NULL;
  if(meshWrapper)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(polySource->GetProperty("MeshWrapper"));
    proxyproperty->RemoveAllProxies();
    proxyproperty->AddProxy(meshWrapper);
    }
  pqSMAdaptor::setElementProperty(polySource->GetProperty("EntityId"), -1);
  polySource->UpdateVTKObjects();
  pqSMAdaptor::setElementProperty(polySource->GetProperty("EntityId"),
    this->UniqueEntityId);
  pqSMAdaptor::setElementProperty(polySource->GetProperty("ItemType"),
    this->ModelEntity->GetType());
  pqSMAdaptor::setElementProperty(
    polySource->GetProperty("CreateEdgePointVerts"),
    this->MeshEdgePointsVisible);

  polySource->MarkModified(NULL);
  polySource->UpdateVTKObjects();
  polySource->UpdatePipeline();

  // Add in the ApplyBathymetry Filter, by default it is just
  // a ShallowCopy (NoOP)
  pqPipelineSource* meshBathymetryFilter = this->getMeshBathymetryFilter();

  if(!this->MeshRepresentation && model && model->modelRepresentation())
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->MeshRepresentation =
      builder->createDataRepresentation(meshBathymetryFilter->getOutputPort(0),
      model->modelRepresentation()->getView(), "GeometryRepresentation");
    vtkSMProxy* repProxy = this->MeshRepresentation->getProxy();
    //vtkSMPropertyHelper(repProxy, "Opacity"), 0.99); // so it will show and not get picked
    vtkSMPropertyHelper(repProxy, "LineWidth").Set(2);
    vtkSMPropertyHelper(repProxy, "Pickable").Set(0);
    vtkSMPropertyHelper(repProxy, "PointSize").Set(6.0);

    double rgb[3] ={0, 0.8, 0};
    vtkSMPropertyHelper(repProxy, "DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(repProxy, "AmbientColor").Set(rgb, 3);
    vtkSMPropertyHelper(repProxy, "Representation").Set("Wireframe"); // WireFrame
    repProxy->UpdateVTKObjects();
    vtkSMRepresentationProxy::SafeDownCast(
      this->MeshRepresentation->getProxy())->UpdatePipeline();
    }
}

//----------------------------------------------------------------------------
pqPipelineSource* pqCMBModelEntity::getMeshBathymetryFilter()
{
  if(!this->MeshBathymetryFilter)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->MeshBathymetryFilter =
      builder->createFilter("filters",
      "CmbApplyBathymetry",  this->MeshPolyProvider);
    vtkSMPropertyHelper(this->MeshBathymetryFilter->getProxy(),
      "NoOP").Set(1);
    }

  this->MeshBathymetryFilter->getProxy()->UpdateVTKObjects();
    // force pipeline update
  vtkSMSourceProxy::SafeDownCast(
    this->MeshBathymetryFilter->getProxy() )->UpdatePipeline();
  return this->MeshBathymetryFilter;
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::applyMeshBathymetry(
  vtkSMProxy* bathymetrySource, double eleRaius,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow)
{
  pqPipelineSource* meshBathymetryFilter = this->getMeshBathymetryFilter();
  vtkSMSourceProxy* smFilter = vtkSMSourceProxy::SafeDownCast(
    meshBathymetryFilter->getProxy());
  vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
    smFilter->GetProperty("Source"));
  pSource->RemoveAllProxies();
  pSource->AddProxy(bathymetrySource);
  vtkSMPropertyHelper(smFilter, "ElevationRadius").Set(eleRaius);
  vtkSMPropertyHelper(smFilter, "HighestZValue").Set(eleHigh);
  vtkSMPropertyHelper(smFilter, "UseHighestZValue").Set(useHighLimit);
  vtkSMPropertyHelper(smFilter, "LowestZValue").Set(eleLow);
  vtkSMPropertyHelper(smFilter, "UseLowestZValue").Set(useLowLimit);

  vtkSMPropertyHelper(smFilter, "NoOP").Set(0);
  smFilter->UpdateVTKObjects();
  smFilter->UpdatePipeline();
}
//----------------------------------------------------------------------------
void pqCMBModelEntity::unapplyMeshBathymetry()
{
  if(this->MeshBathymetryFilter)
    {
    vtkSMSourceProxy* smFilter = vtkSMSourceProxy::SafeDownCast(
      this->MeshBathymetryFilter->getProxy());
    vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
      smFilter->GetProperty("Source"));
    pSource->RemoveAllProxies();
    vtkSMPropertyHelper(smFilter, "NoOP").Set(1);
    smFilter->UpdateVTKObjects();
    smFilter->UpdatePipeline();
    }
}
//-----------------------------------------------------------------------------
void pqCMBModelEntity::setEdgeMeshPointsVisibility(int visible)
{
  if(this->MeshEdgePointsVisible == visible)
    {
    return;
    }
  this->MeshEdgePointsVisible = visible;
  if(!this->MeshEntity || !this->MeshEntity->IsModelEntityMeshed() ||
    !this->MeshPolyProvider || !this->MeshRepresentation)
    {
    return;
    }

  vtkSMSourceProxy* polyProvider = vtkSMSourceProxy::SafeDownCast(
    this->MeshPolyProvider->getProxy());
  if(!polyProvider)
    {
    return;
    }
  pqSMAdaptor::setElementProperty(
    polyProvider->GetProperty("CreateEdgePointVerts"),
    this->MeshEdgePointsVisible);
  this->updateMeshEntity(NULL);
}
//-----------------------------------------------------------------------------
void pqCMBModelEntity::synchronizeMeshRepresentation(
  vtkSMProperty *changedProp, const char* propertyName)
{
  if(!changedProp || !this->MeshRepresentation ||
     !propertyName || !(*propertyName))
    {
    return;
    }
  vtkSMRepresentationProxy *repProxy =
    vtkSMRepresentationProxy::SafeDownCast(
      this->MeshRepresentation->getProxy());
  repProxy->GetProperty(propertyName)->
    Copy(changedProp);
  repProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBModelEntity::setMeshRepresentationType(const char* strType)
{
  if(!strType || this->MeshFaceRepresentationType.compare(strType,
    Qt::CaseInsensitive) == 0)
    {
    return;
    }
  this->MeshFaceRepresentationType = strType;
  if(!this->MeshEntity || !this->MeshEntity->IsModelEntityMeshed() ||
    !this->MeshPolyProvider)
    {
    return;
    }
  pqSMAdaptor::setEnumerationProperty(
    this->MeshRepresentation->getProxy()->GetProperty("Representation"),
    this->MeshFaceRepresentationType.toAscii().constData());
  this->MeshRepresentation->getProxy()->UpdateVTKObjects();
  vtkSMRepresentationProxy::SafeDownCast(
    this->MeshRepresentation->getProxy())->UpdatePipeline();
}
