/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelFace.cxx

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
#include "pqCMBModelFace.h"

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
#include "vtkDiscreteModelFace.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"

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
#include "vtkPVModelGeometryInformation.h"

#include <QVariant>
#include <QTableWidgetItem>
#include <QMessageBox>

//-----------------------------------------------------------------------------
pqCMBModelFace::pqCMBModelFace()
{
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBModelFace::init()
{
  this->Superclass::init();
}

//-----------------------------------------------------------------------------
pqCMBModelFace::~pqCMBModelFace()
{

}

//-----------------------------------------------------------------------------
vtkDiscreteModelFace* pqCMBModelFace::getModelFaceEntity() const
{
  return vtkDiscreteModelFace::SafeDownCast(this->ModelEntity);
}

//-----------------------------------------------------------------------------
void pqCMBModelFace::colorByColorMode(
                                        vtkDiscreteLookupTable* lut, int colorMode)
{
  if(!lut)
    {
    return;
    }

  vtkDiscreteModelFace* faceEntity = this->getModelFaceEntity();
  vtkModelEntity* modelEntity = NULL;
  vtkDiscreteModelRegion* modelRegion = NULL;
  switch (colorMode)
    {
    case ColorByEntity:
      modelEntity = faceEntity;
      break;
    case ColorByDomain:
      modelEntity = faceEntity->GetModelRegion(0);
      break;
    case ColorByMaterial:
      modelRegion = vtkDiscreteModelRegion::SafeDownCast(
        faceEntity->GetModelRegion(0));
      modelEntity = modelRegion ? modelRegion->GetMaterial() :
        faceEntity->GetMaterial();
      break;
    case ColorByBCS:
      //int startIndex = data->FaceIDToRepMap[faceId].ModelFaceInfo->GetInfoArrayBCStartIndex();
      if(faceEntity->GetNumberOfModelEntityGroups())
        {
        vtkModelItemIterator* iter=faceEntity->NewIterator(vtkDiscreteModelEntityGroupType);
        iter->Begin();
        modelEntity = vtkDiscreteModelEntityGroup::SafeDownCast(iter->GetCurrentItem());
        iter->Delete();
        }
      break;
    default:
      break;
    }

  double rgb[3]={1.0,1.0,1.0};
  double opacity = 1.0;
  if(ModelEntity)
    {
    this->getColor(modelEntity, rgb, opacity, lut);
    }

  this->setRepresentationColor(rgb);
}
//-----------------------------------------------------------------------------
void pqCMBModelFace::colorByEdgeDomainMode(
  vtkDiscreteLookupTable* lut, int colorMode)
{
  if(!lut)
    {
    return;
    }

  vtkDiscreteModelFace* faceEntity = this->getModelFaceEntity();
  vtkModelEntity* modelEntity = NULL;
  switch (colorMode)
  {
  case PolygonFaceColorByModelFace:
    modelEntity = faceEntity;
    break;
  case PolygonFaceColorByMaterial:
    modelEntity = faceEntity->GetMaterial();
    break;
  default:
    break;
  }

  double rgb[3];
  double opacity = 1.0;

  this->getColor(modelEntity, rgb, opacity, lut);
  this->setRepresentationColor(rgb, opacity);
}

//-----------------------------------------------------------------------------
pqCMBModelFace* pqCMBModelFace::createObject(
  vtkSMProxy* modelWrapper, vtkDiscreteModelFace* cmbModelFace,
  pqServer *server, pqRenderView *view, bool updateRep)
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
      cmbModelFace->GetUniquePersistentId());
    pqSMAdaptor::setElementProperty(polySource->getProxy()->GetProperty("ItemType"),
      vtkModelFaceType);
    polySource->getProxy()->UpdateVTKObjects();

    return pqCMBModelFace::createObject(
      polySource, server, view, cmbModelFace, modelWrapper, updateRep);
    }

  return NULL;
}

//-----------------------------------------------------------------------------
pqCMBModelFace *pqCMBModelFace::createObject(pqPipelineSource* source,
                                                     pqServer * /*server*/,
                                                     pqRenderView * /*view*/,
                                                     vtkDiscreteModelFace* cmbModelFace,
                                                     vtkSMProxy* modelWrapper,
                                                     bool /*updateRep*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBModelFace *obj = new pqCMBModelFace();
  obj->PolyProvider = source;
  obj->setModelEntity(cmbModelFace);
  obj->setModelWrapper(modelWrapper);

  return obj;
}
