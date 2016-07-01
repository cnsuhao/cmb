//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBModelManager.h"

#include "vtkDataObject.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkPVSMTKMeshInformation.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSmartPointer.h"
#include "vtkSMSourceProxy.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqRenderView.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqSMTKModelInfo.h"

#include "smtk/common/UUID.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include "smtk/io/ImportJSON.h"
#include "cJSON.h"

#include <fstream>
#include <iostream>

#include "vtksys/SystemTools.hxx"
#include "cJSON.h"

#include <map>
#include <set>
#include <QDebug>

namespace
{

/// Fetch children for volum and group entities.
bool _internal_ContainsGroup(
  const smtk::common::UUID& grpId, const smtk::model::EntityRef& toplevel)
  {
  if(toplevel.isGroup())
    {
    if(grpId == toplevel.entity())
      return true;
    smtk::model::EntityRefs members =
      toplevel.as<smtk::model::Group>().members<smtk::model::EntityRefs>();
    for (smtk::model::EntityRefs::const_iterator it = members.begin();
       it != members.end(); ++it)
      {
      if(it->entity() == grpId)
        return true;
      // Do this recursively since a group may contain other groups
      if(_internal_ContainsGroup(grpId, *it))
        return true;
      }
    }
  return false;
  }
}

//-----------------------------------------------------------------------------
class pqCMBModelManager::qInternal
{
public:

  // <ModelEnity, modelInfo>
  std::map<smtk::common::UUID, pqSMTKModelInfo> ModelInfos;
  typedef std::map<smtk::common::UUID, pqSMTKModelInfo >::iterator itModelInfo;
  typedef std::map<smtk::common::UUID, pqSMTKMeshInfo >::iterator itMeshInfo;
  std::map<smtk::common::UUID, smtk::common::UUID> Entity2Models;

  pqServer* Server;
  vtkSmartPointer<vtkSMModelManagerProxy> ManagerProxy;
  vtkNew<vtkDiscreteLookupTable> ModelColorLUT;
  std::vector<vtkTuple<double, 3> > LUTColors;
  QPointer<pqDataRepresentation> ActiveRepresentation;
  QPointer<pqDataRepresentation> previousActiveRepresentation;

  void updateEntityGroupFieldArrayAndAnnotations(const smtk::model::EntityRef& model)
  {
    pqSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    modelInfo->grp_annotations.clear();
    smtk::model::Groups modGroups =
      model.as<smtk::model::Model>().groups();
    // if there are no groups at all in the model, just return
    if(modGroups.size() == 0)
      return;
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      modelInfo->RepSource->getProxy());
    vtkSMPropertyHelper(smSource,"AddGroupArray").Set(0);
    smSource->UpdateVTKObjects();
    vtkSMPropertyHelper(smSource,"AddGroupArray").Set(1);
    smSource->UpdateVTKObjects();
    modelInfo->RepSource->updatePipeline();

    for(smtk::model::Groups::iterator it = modGroups.begin();
       it != modGroups.end(); ++it)
      {
      modelInfo->grp_annotations.push_back( it->entity().toString());
      modelInfo->grp_annotations.push_back( it->name());
      }
    modelInfo->grp_annotations.push_back( "no group");
    modelInfo->grp_annotations.push_back( "no group");
  }

  void updateGeometryEntityAnnotations(const smtk::model::EntityRef& model)
  {
    smtk::model::ManagerPtr mgr = this->ManagerProxy->modelManager();

    pqSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    vtkPVSMTKModelInformation* pvinfo = modelInfo->Info;
    modelInfo->ent_annotations.clear();
    modelInfo->vol_annotations.clear();
    std::map<smtk::common::UUID, unsigned int>::const_iterator it =
      pvinfo->GetUUID2BlockIdMap().begin();
    for(; it != pvinfo->GetUUID2BlockIdMap().end(); ++it)
      {
      this->Entity2Models[it->first] = model.entity();
      modelInfo->ent_annotations.push_back( it->first.toString());
      modelInfo->ent_annotations.push_back( mgr->name(it->first));
      }

    smtk::model::CellEntities modVols =
      model.as<smtk::model::Model>().cells();
    for(smtk::model::CellEntities::iterator it = modVols.begin();
       it != modVols.end(); ++it)
      {
      if((*it).isVolume())
        {
        modelInfo->vol_annotations.push_back( it->entity().toString());
        modelInfo->vol_annotations.push_back( it->name());
        }
      }
  }

  bool addModelRepresentation(smtk::model::Model& model,
    pqRenderView* view, vtkSMModelManagerProxy* smProxy,
    const std::string& filename, const smtk::model::SessionRef& sref)
  {
    bool loadOK = true;
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      {
      // if this is a submodel, we don't want it to be child of the session.
      // Instead its parent should be the child of the session.
      if(!model.parent().isModel())
        model.setSession(sref);
      // for any model that has cells, we will create a representation for it.
      if(model.cells().size() > 0 || model.groups().size() > 0)
        {
        pqDataRepresentation* rep = NULL;
        pqApplicationCore* core = pqApplicationCore::instance();
        pqObjectBuilder* builder = core->getObjectBuilder();
        pqPipelineSource* modelSrc = builder->createSource(
            "ModelBridge", "SMTKModelSource", this->Server);
        if(modelSrc)
          {
          // ModelManagerWrapper Proxy
          vtkSMProxyProperty* smwrapper =
            vtkSMProxyProperty::SafeDownCast(
            modelSrc->getProxy()->GetProperty("ModelManagerWrapper"));
          smwrapper->RemoveAllProxies();
          smwrapper->AddProxy(smProxy);
          vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set(
            model.entity().toString().c_str());

          modelSrc->getProxy()->UpdateVTKObjects();
          modelSrc->updatePipeline();

          pqPipelineSource* repSrc = builder->createFilter(
            "ModelBridge", "SMTKModelFieldArrayFilter", modelSrc);
          smwrapper =
            vtkSMProxyProperty::SafeDownCast(
            repSrc->getProxy()->GetProperty("ModelManagerWrapper"));
          smwrapper->RemoveAllProxies();
          smwrapper->AddProxy(smProxy);
          repSrc->getProxy()->UpdateVTKObjects();
          repSrc->updatePipeline();

          rep = builder->createDataRepresentation(
            repSrc->getOutputPort(0), view);
          if(rep)
            {
            smtk::model::ManagerPtr mgr = smProxy->modelManager();
            this->ModelInfos[model.entity()].init(modelSrc, repSrc, rep, filename, mgr);

            vtkSMPropertyHelper(rep->getProxy(), "PointSize").Set(8.0);
            this->updateGeometryEntityAnnotations(model);
            this->updateEntityGroupFieldArrayAndAnnotations(model);
            this->resetColorTable(model);
            RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
              rep->getProxy(), NULL, vtkDataObject::FIELD);
            }
          }
        loadOK = (rep != NULL);
        if(!loadOK)
          {
          qCritical() << "Failed to create a pqPipelineSource or pqRepresentation for the model: "
            << model.entity().toString().c_str();
          }
        }

      // we will also try to create a representation for each submodel of the model
      smtk::model::Models msubmodels = model.submodels();
      smtk::model::Models::iterator it;
      for (it = msubmodels.begin(); it != msubmodels.end(); ++it)
        {
        loadOK &= this->addModelRepresentation(*it, view, smProxy, filename, sref);
        }
      return loadOK;
      }
    else
      {
//      qCritical() << "There is already a pqPipelineSource for the model: "
//        << model.entity().toString().c_str();
      return loadOK;
      }
  }

  void removeModelRepresentations(const smtk::common::UUIDs& modeluids,
                                 pqRenderView* view)
  {
    bool modelRemoved = false;
    for (smtk::common::UUIDs::const_iterator mit = modeluids.begin();
        mit != modeluids.end(); ++mit)
      {
      if(this->ModelInfos.find(*mit) == this->ModelInfos.end())
        {
        continue;
        }

      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        this->ModelInfos[*mit].RepSource);
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        this->ModelInfos[*mit].ModelSource);
      std::map<smtk::common::UUID, unsigned int>::const_iterator it;

      for(it = this->ModelInfos[*mit].Info->GetUUID2BlockIdMap().begin();
        it != this->ModelInfos[*mit].Info->GetUUID2BlockIdMap().end(); ++it)
        {
        this->Entity2Models.erase(it->first);
        }

      this->ModelInfos.erase(*mit);
      modelRemoved = true;
      }
  if(modelRemoved)
    view->render();
  }

  void updateModelRepresentation(const smtk::model::EntityRef& model)
  {
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      return;
    pqSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    pqPipelineSource* modelSrc = modelInfo->ModelSource;
    vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set("");
     modelSrc->getProxy()->UpdateVTKObjects();
    vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set(
      modelInfo->Info->GetModelUUID().toString().c_str());

    modelSrc->getProxy()->InvokeCommand("MarkDirty");
    modelSrc->getProxy()->UpdateVTKObjects();

    modelSrc->updatePipeline();
    modelSrc->getProxy()->UpdatePropertyInformation();
    vtkSMRepresentationProxy::SafeDownCast(
        modelInfo->Representation->getProxy())->UpdatePipeline();

    modelInfo->updateBlockInfo(
      this->ManagerProxy->modelManager());
    this->updateGeometryEntityAnnotations(model);
    this->updateEntityGroupFieldArrayAndAnnotations(model);
    this->resetColorTable(model);
    modelInfo->Representation->renderViewEventually();
  }

  void createMeshRepresentation(smtk::model::ManagerPtr manager,
                                const smtk::model::EntityRef& model,
                                pqRenderView* view,
                                bool visible)
  {
    smtk::model::EntityRef related_model  = model;
    smtk::model::Model smtkModel(manager, model.entity());

    //The model relationship could be on the model itself, or a submodel
    //So determine where the relationship is
    bool containsModel = (this->ModelInfos.find(model.entity()) != this->ModelInfos.end());
    if(!containsModel && smtkModel.submodels().size() > 0)
      {
      smtk::model::Models submodels = smtkModel.submodels();
      for(std::size_t i=0; !containsModel && i < submodels.size(); ++i)
        {
        related_model = submodels[i];
        containsModel = (this->ModelInfos.find(related_model.entity()) != this->ModelInfos.end());
        }
      }

    if(!containsModel)
      {
      return;
      }

    // find a mesh collection that's not in the mesh info already
    smtk::mesh::ManagerPtr meshMgr = model.manager()->meshes();
    std::vector<smtk::mesh::CollectionPtr> meshCollections =
      meshMgr->associatedCollections(model);
    if(meshCollections.size() == 0)
      {
      return;
      }

    pqSMTKModelInfo* modelInfo = &this->ModelInfos[related_model.entity()];
    std::vector<smtk::mesh::CollectionPtr> newMCs;
    if(modelInfo->MeshInfos.size() > 0)
      {
      std::vector<smtk::mesh::CollectionPtr>::const_iterator it;
      for(it = meshCollections.begin(); it != meshCollections.end(); ++it)
        {
        if(modelInfo->MeshInfos.find((*it)->entity()) == modelInfo->MeshInfos.end())
          {
          newMCs.push_back(*it);
          }
        }
      }
    else
      {
      newMCs.insert(newMCs.end(), meshCollections.begin(), meshCollections.end()); // take all mesh collections
      }
    if(newMCs.size() == 0)
      return;

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    std::vector<smtk::mesh::CollectionPtr>::const_iterator cit;
    for(cit = newMCs.begin(); cit != newMCs.end(); ++cit)
      {
      smtk::common::UUID cid = (*cit)->entity();
      pqDataRepresentation* rep = NULL;
      pqPipelineSource* meshSrc = builder->createSource(
          "ModelBridge", "SMTKMeshSource", this->Server);
      if(meshSrc)
        {
        // ModelManagerWrapper Proxy
        vtkSMModelManagerProxy* smProxy = this->ManagerProxy;
        vtkSMProxyProperty* smwrapper =
          vtkSMProxyProperty::SafeDownCast(
          meshSrc->getProxy()->GetProperty("ModelManagerWrapper"));
        smwrapper->RemoveAllProxies();
        smwrapper->AddProxy(smProxy);

        vtkSMPropertyHelper(meshSrc->getProxy(), "ModelEntityID").Set(
          model.entity().toString().c_str());
        vtkSMPropertyHelper(meshSrc->getProxy(), "MeshCollectionID").Set(
          cid.toString().c_str());

        meshSrc->getProxy()->UpdateVTKObjects();
        meshSrc->updatePipeline();

        pqPipelineSource* repSrc = builder->createFilter(
          "ModelBridge", "SMTKModelFieldArrayFilter", meshSrc);
        smwrapper =
          vtkSMProxyProperty::SafeDownCast(
          repSrc->getProxy()->GetProperty("ModelManagerWrapper"));
        smwrapper->RemoveAllProxies();
        smwrapper->AddProxy(smProxy);

  //      vtkSMPropertyHelper(repSrc->getProxy(),"AddGroupArray").Set(1);
        repSrc->getProxy()->UpdateVTKObjects();
        repSrc->updatePipeline();

        rep = builder->createDataRepresentation(
          repSrc->getOutputPort(0), view);
        if(rep)
          {
          smtk::model::ManagerPtr mgr = smProxy->modelManager();
          modelInfo->MeshInfos[cid].init(meshSrc, repSrc, rep,
            (*cit)->readLocation().absolutePath(), mgr, modelInfo);

          vtkSMPropertyHelper(rep->getProxy(),
                          "Representation").Set("Surface With Edges");
          vtkSMPropertyHelper(rep->getProxy(), "PointSize").Set(8.0);

          RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
            rep->getProxy(), NULL, vtkDataObject::FIELD);

          rep->setVisible(visible);
          }
        }

      if(rep == NULL)
        {
        qCritical() << "Failed to create a mesh pqRepresentation for the model: "
          << model.entity().toString().c_str();
        }
    }
  }

  void updateModelMeshRepresentations(const smtk::model::Model& model)
  {
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      return;
    // find a mesh collection that's not in the mesh info already
    smtk::mesh::ManagerPtr meshMgr = model.manager()->meshes();
    std::vector<smtk::mesh::CollectionPtr> meshCollections =
      meshMgr->associatedCollections(model);
    if(meshCollections.size() == 0)
      {
      return;
      }

    pqSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    for(qInternal::itMeshInfo meshiter = modelInfo->MeshInfos.begin();
        meshiter != modelInfo->MeshInfos.end(); ++meshiter)
      {
      pqPipelineSource* source = meshiter->second.RepSource;
      if(!source)
        continue;
      // clear selection
      pqOutputPort* outport = source->getOutputPort(0);
      if(outport)
        {
        outport->setSelectionInput(0, 0);
        }

      pqPipelineSource* meshSrc = meshiter->second.MeshSource;
      vtkSMPropertyHelper(meshSrc->getProxy(), "ModelEntityID").Set("");
      vtkSMPropertyHelper(meshSrc->getProxy(), "MeshCollectionID").Set("");
      meshSrc->getProxy()->UpdateVTKObjects();

      vtkSMPropertyHelper(meshSrc->getProxy(), "ModelEntityID").Set(
        model.entity().toString().c_str());
      vtkSMPropertyHelper(meshSrc->getProxy(), "MeshCollectionID").Set(
        meshiter->second.Info->GetMeshCollectionID().toString().c_str());

      meshSrc->getProxy()->InvokeCommand("MarkDirty");
      meshSrc->getProxy()->UpdateVTKObjects();
      meshSrc->updatePipeline();
      meshSrc->getProxy()->UpdatePropertyInformation();

      vtkSMRepresentationProxy::SafeDownCast(
          meshiter->second.Representation->getProxy())->UpdatePipeline();

      meshiter->second.updateBlockInfo(
        this->ManagerProxy->modelManager());

      meshiter->second.Representation->renderViewEventually();
      }
  }

  void removeMeshRepresentations(smtk::mesh::ManagerPtr meshMgr,
                                 const smtk::common::UUIDs& collectionids,
                                 pqRenderView* view)
  {
    bool meshRemoved = false;
    for (smtk::common::UUIDs::const_iterator cit = collectionids.begin();
        cit != collectionids.end(); ++cit)
      {
      smtk::mesh::CollectionPtr collection = meshMgr->collection(*cit);
      if(!collection->isValid())
        {
        continue;
        }
      smtk::common::UUID modelid = collection->associatedModel();
      if(this->ModelInfos.find(modelid) == this->ModelInfos.end())
        {
        continue;
        }
      pqSMTKModelInfo* modelInfo = &this->ModelInfos[modelid];
      for(qInternal::itMeshInfo meshiter = modelInfo->MeshInfos.begin();
          meshiter != modelInfo->MeshInfos.end(); ++meshiter)
        {
        if(meshiter->second.Info->GetMeshCollectionID() == *cit)
          {
          pqApplicationCore::instance()->getObjectBuilder()->destroy(
            meshiter->second.RepSource);
          pqApplicationCore::instance()->getObjectBuilder()->destroy(
            meshiter->second.MeshSource);
          meshRemoved = true;
          break;
          }
        }
      if(meshRemoved)
        {
        modelInfo->MeshInfos.erase(*cit);
        meshMgr->removeCollection(collection);
        }
      else
        {
        std::cerr << "Mesh collection is not removed from client: " << collection->name() << std::endl;
        }
      }
  if(meshRemoved)
    view->render();
  }

  // If the group has already been removed from the model, the modelInfo(entityID)
  // will not return the modelInfo it requests, because the record is removed from
  // model manager. This method, intead, will go through all grp_annotations info
  // cached on client to find the the modelInfo associated with the group.
  pqSMTKModelInfo* modelInfoForRemovedGroup(const smtk::common::UUID& entid)
  {
    for(itModelInfo mit = this->ModelInfos.begin(); mit != this->ModelInfos.end(); ++mit)
      {
      if(std::find(mit->second.grp_annotations.begin(),
                  mit->second.grp_annotations.end(),
                  entid.toString())
        != mit->second.grp_annotations.end())
        {
        return &mit->second;
        }
      }

    return NULL;
  }

  void clear()
  {
    for(itModelInfo mit = this->ModelInfos.begin(); mit != this->ModelInfos.end(); ++mit)
      {
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        mit->second.RepSource);
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        mit->second.ModelSource);
      for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
          meshiter != mit->second.MeshInfos.end(); ++meshiter)
        {
        pqApplicationCore::instance()->getObjectBuilder()->destroy(
          meshiter->second.RepSource);
        pqApplicationCore::instance()->getObjectBuilder()->destroy(
          meshiter->second.MeshSource);
        }
      }
    this->Entity2Models.clear();
    this->ModelInfos.clear();
  }

  qInternal(pqServer* server): Server(server)
  {
    // this->rebuildColorTable(256);
  }

  void rebuildColorTable(vtkIdType numOfColors)
  {
    if(numOfColors < 1)
      return;

//    this->ModelColorLUT->Modified();
    this->ModelColorLUT->SetNumberOfValues(numOfColors);
    this->ModelColorLUT->Build();
    // raw interleaved array with the layout [X1, R1, G1, B1, X2, R2, G2,
    // B2, ..., Xn, Rn, Gn, Bn]
    this->LUTColors.resize(this->ModelColorLUT->GetNumberOfAvailableColors());
 //   double* rawData = this->ModelColorLUT->GetDataPointer();
    double rgba[4];
    for (vtkIdType i = 0; i < this->ModelColorLUT->GetNumberOfAvailableColors(); ++i)
      {
      this->ModelColorLUT->GetIndexedColor(i, rgba);
      this->LUTColors[i].GetData()[0] = rgba[0];
      this->LUTColors[i].GetData()[1] = rgba[1];
      this->LUTColors[i].GetData()[2] = rgba[2];
      }
  }

  void resetColorTable(const smtk::model::EntityRef& model)
  {
    pqSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    this->resetColorTable(modelInfo, modelInfo->EntityLUT, modelInfo->ent_annotations);
    this->resetColorTable(modelInfo, modelInfo->VolumeLUT, modelInfo->vol_annotations);
    this->resetColorTable(modelInfo, modelInfo->GroupLUT, modelInfo->grp_annotations);
   }

  void resetColorTable(pqSMTKModelInfo* modelInfo,
                       vtkSMProxy* lutProxy,
                       const std::vector<std::string> & in_annotations)
  {
    if(!modelInfo)
      return;

    pqDataRepresentation* rep = modelInfo->Representation;
    this->rebuildColorTable(in_annotations.size()/2);
    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), lutProxy,
      this->LUTColors, in_annotations);
  }
};

//----------------------------------------------------------------------------
pqCMBModelManager::pqCMBModelManager(pqServer* server)
{
  this->Internal = new qInternal(server);
  this->initialize();
}

//----------------------------------------------------------------------------
pqCMBModelManager::~pqCMBModelManager()
{
  this->clear();
  delete this->Internal;
}

//----------------------------------------------------------------------------
pqServer* pqCMBModelManager::server()
{
  return this->Internal->Server;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::initialize()
{
  if(!this->Internal->ManagerProxy)
    {
    // create block selection source proxy
    vtkSMSessionProxyManager *proxyManager =
      vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();
    vtkSMProxy* proxy = proxyManager->NewProxy("ModelBridge", "ModelManager");
    if(proxy)
      {
      this->Internal->ManagerProxy.TakeReference(
        vtkSMModelManagerProxy::SafeDownCast(proxy));
      }
    if(!this->Internal->ManagerProxy)
      {
      qCritical() << "Failed to create an Model Manager Proxy!";
      }
    else
      {
      QObject::connect(pqApplicationCore::instance()->getPluginManager(),
        SIGNAL(pluginsUpdated()),
        this, SLOT(onPluginLoaded()));
      this->Internal->ManagerProxy->sessionNames();//fetch session names
      }
    }
}

//----------------------------------------------------------------------------
vtkSMModelManagerProxy* pqCMBModelManager::managerProxy()
{
  this->initialize();
  return this->Internal->ManagerProxy;
}

// This will recursively check the submodels of the input model
//-----------------------------------------------------------------------------
pqSMTKModelInfo* internal_getModelInfo(
  const smtk::model::Model& inModel,
  std::map<smtk::common::UUID, pqSMTKModelInfo>& modelInfos)
{
  smtk::common::UUID modelId = inModel.entity();
  if(!modelId.isNull() && modelInfos.find(modelId) != modelInfos.end())
    {
    return &modelInfos[modelId];
    }

  pqSMTKModelInfo* res = NULL;
  smtk::model::Models msubmodels = inModel.submodels();
  smtk::model::Models::iterator it;
  for (it = msubmodels.begin(); it != msubmodels.end(); ++it)
    {
    if((res = internal_getModelInfo(*it, modelInfos)))
      {
      return res;
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
pqSMTKModelInfo* pqCMBModelManager::modelInfo(const smtk::model::EntityRef& selentity)
{
  smtk::common::UUID modelId;
  if(selentity.isModel())
    {
    return internal_getModelInfo(selentity.as<smtk::model::Model>(), this->Internal->ModelInfos);
    }
  else if(selentity.isVolume())
    {
    modelId = selentity.as<smtk::model::Volume>().model().entity();
    }
  else if(selentity.isGroup()) // group is tricky
    {
    smtk::model::Models modelEnts = this->Internal->ManagerProxy->
      modelManager()->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY);
    for (smtk::model::Models::iterator it = modelEnts.begin();
        it != modelEnts.end(); ++it)
      {
      if(it->isValid())
        {
        smtk::model::Groups modGroups = it->groups();
        for(smtk::model::Groups::iterator grit = modGroups.begin();
           grit != modGroups.end(); ++grit)
          {
          if(_internal_ContainsGroup(selentity.entity(), *grit))
            {
            modelId = it->entity();
            break;
            }
          }
        }
      // if found, break;
      if(!modelId.isNull())
        break;
      }
    }
  else if(this->Internal->Entity2Models.find(selentity.entity())
    != this->Internal->Entity2Models.end())
    {
    modelId = this->Internal->Entity2Models[selentity.entity()];
    }

  if(!modelId.isNull() &&
    this->Internal->ModelInfos.find(modelId) !=
     this->Internal->ModelInfos.end())
    return &this->Internal->ModelInfos[modelId];

  return NULL;
}

//----------------------------------------------------------------------------
pqSMTKModelInfo* pqCMBModelManager::modelInfo(pqDataRepresentation* rep)
{
  if(!rep)
    return NULL;

  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(mit->second.Representation == rep)
      {
      return &mit->second;
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
pqSMTKMeshInfo* pqCMBModelManager::meshInfo(const smtk::mesh::MeshSet& mesh)
{
  smtk::common::UUID modelId = mesh.collection()->associatedModel();
  pqSMTKModelInfo* modInfo = internal_getModelInfo(
    smtk::model::Model(this->Internal->ManagerProxy->modelManager(), modelId),
    this->Internal->ModelInfos);

  if(modInfo)
    {
    smtk::common::UUID cid = mesh.collection()->entity();
    for(qInternal::itMeshInfo meshiter = modInfo->MeshInfos.begin();
        meshiter != modInfo->MeshInfos.end(); ++meshiter)
      {
      if(meshiter->first == cid)
        {
        return &meshiter->second;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
pqSMTKMeshInfo* pqCMBModelManager::meshInfo(pqDataRepresentation* rep)
{
  if(!rep)
    return NULL;

  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
        meshiter != mit->second.MeshInfos.end(); ++meshiter)
      {
      if(meshiter->second.Representation == rep)
        {
        return &meshiter->second;
        }
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
QList<pqSMTKModelInfo*>  pqCMBModelManager::selectedModels() const
{
  QList<pqSMTKModelInfo*> selModels;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.RepSource)
      {
      continue;
      }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      mit->second.RepSource->getProxy());
    if(smSource && smSource->GetSelectionInput(0))
      {
      selModels.append(&mit->second);
      }
    }
  return selModels;
}

//----------------------------------------------------------------------------
QList<pqSMTKModelInfo*>  pqCMBModelManager::allModels() const
{
  QList<pqSMTKModelInfo*> selModels;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    selModels.append(&mit->second);
    }
  return selModels;
}

//----------------------------------------------------------------------------
QList<pqSMTKMeshInfo*>  pqCMBModelManager::allMeshes() const
{
  QList<pqSMTKMeshInfo*> allmeshes;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
        meshiter != mit->second.MeshInfos.end(); ++meshiter)
      {
      allmeshes.append(&meshiter->second);
      }
    }
  return allmeshes;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::clearModelSelections()
{
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.RepSource)
      {
      continue;
      }
    pqPipelineSource* source = mit->second.RepSource;
    pqOutputPort* outport = source->getOutputPort(0);
    if(outport)
      {
      outport->setSelectionInput(0, 0);
      }
    }
}

//----------------------------------------------------------------------------
void pqCMBModelManager::clearMeshSelections()
{
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
        meshiter != mit->second.MeshInfos.end(); ++meshiter)
      {
      if(!meshiter->second.RepSource)
        {
        continue;
        }
      pqPipelineSource* source = meshiter->second.RepSource;
      pqOutputPort* outport = source->getOutputPort(0);
      if(outport)
        {
        outport->setSelectionInput(0, 0);
        }
      }
    }
}

//----------------------------------------------------------------------------
int pqCMBModelManager::numberOfModels()
{
  return (int)this->Internal->ModelInfos.size();
}

//----------------------------------------------------------------------------
pqSMTKModelInfo* pqCMBModelManager::activateModelRepresentation()
{
  pqSMTKModelInfo* minfo = NULL;
  if(!this->Internal->ActiveRepresentation)
    {
    if(this->Internal->previousActiveRepresentation)
      {
      minfo = this->modelInfo(this->Internal->previousActiveRepresentation);
      }
    else
      {
      QList<pqSMTKModelInfo*> selModels(this->selectedModels());
      QList<pqSMTKModelInfo*> allModels(this->allModels());
      if(selModels.count() > 0) // use first selected model
        {
        minfo = selModels[0];
        }
      else if(allModels.count() > 0) // use first model
        {
        minfo = allModels[0];
        }
      }

    if(minfo)
      {
      // this should trigger setActiveModelRepresentation()
      pqActiveObjects::instance().setActiveSource(minfo->RepSource);
      }
    }
  else
    {
    minfo = this->modelInfo(this->Internal->ActiveRepresentation);
    }

  return minfo;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::setActiveModelRepresentation(pqDataRepresentation* rep)
{
  if(this->Internal->ActiveRepresentation)
    this->Internal->previousActiveRepresentation =
      this->Internal->ActiveRepresentation;
  this->Internal->ActiveRepresentation = rep;
}

//----------------------------------------------------------------------------
QList<pqDataRepresentation*> pqCMBModelManager::modelRepresentations() const
{
  QList<pqDataRepresentation*> result;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(mit->second.Representation)
      {
      result.append(mit->second.Representation);
      }
    }
  return result;
}
//----------------------------------------------------------------------------
QList<pqDataRepresentation*> pqCMBModelManager::meshRepresentations() const
{
  QList<pqDataRepresentation*> result;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
        meshiter != mit->second.MeshInfos.end(); ++meshiter)
      {
      if(meshiter->second.Representation)
        {
        result.append(meshiter->second.Representation);
        }
      }
    }
  return result;
}

//----------------------------------------------------------------------------
std::set<std::string> pqCMBModelManager::supportedFileTypes(
  const std::string& sessionName)
{
  std::set<std::string> resultSet;
  if(this->managerProxy())
    {
    smtk::model::StringData bftypes = this->Internal->ManagerProxy->supportedFileTypes(sessionName);
    smtk::model::PropertyNameWithStrings typeIt;
    QString filetype, descr, ext;
    for(typeIt = bftypes.begin(); typeIt != bftypes.end(); ++typeIt)
      {
      for (smtk::model::StringList::iterator tpit = typeIt->second.begin();
        tpit != typeIt->second.end(); ++tpit)
        {
        // ".cmb (Conceptual Model Builder)"
        // we want to convert the format into Paraview format for its fileOpen dialog
        // "Conceptual Model Builder (*.cmb)"
        filetype = (*tpit).c_str();
        int idx = filetype.indexOf('(');
        descr = filetype.mid(idx + 1, filetype.indexOf(')') - idx -1);
        ext = filetype.left(filetype.indexOf('('));
        resultSet.insert(
                descr.append(" (*").append(ext.simplified()).append(")").toStdString());
        }
      }
    }
  return resultSet;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::supportedColorByModes(QStringList& types)
{
  types.clear();
  types << "None"
        << vtkModelMultiBlockSource::GetEntityTagName()
        << vtkModelMultiBlockSource::GetGroupTagName()
        << vtkModelMultiBlockSource::GetVolumeTagName()
        << vtkModelMultiBlockSource::GetAttributeTagName();
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateEntityColorTable(
  pqDataRepresentation* rep,
  const QMap<smtk::model::EntityRef, QColor >& colorEntities,
  const QString& colorByMode)
{
  if(colorEntities.count() == 0)
    return;
  pqSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;
  vtkSMProxy* lutProxy = NULL;
  std::vector<std::string>* annList;
  if(colorByMode == vtkModelMultiBlockSource::GetVolumeTagName())
    {
    lutProxy = modInfo->VolumeLUT;
    annList = &modInfo->vol_annotations;
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetGroupTagName())
    {
    lutProxy = modInfo->GroupLUT;
//    annList = modInfo->grp_annotations.GetPointer();
    annList = &modInfo->grp_annotations;
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetEntityTagName())
    {
    lutProxy = modInfo->EntityLUT;
//    annList = modInfo->ent_annotations.GetPointer();
    annList = &modInfo->ent_annotations;
    }
  if(!lutProxy || annList->size() == 0)
    return;
  // Assuming numbers in the LUT color is equal or greater than the
  // number of annotations, and the order of mapping from annotation
  // to the LUT is the same while switching colorby arrays.
  vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
  std::vector<double> rgbColors = indexedColors.GetDoubleArray();
  int numColors = (int)rgbColors.size()/3;
  if(numColors < 1)
    return;
  int idx;
  bool changed = false;
  std::vector<std::string>::const_iterator cit;
  foreach(smtk::model::EntityRef entref, colorEntities.keys())
    {
    cit = std::find(annList->begin(), annList->end(),
                    entref.entity().toString());
    if(cit == annList->end())
      continue;
//    idx = annList->GetIndex(entref.entity().toString().c_str());
    idx = cit - annList->begin();
    if(idx >= 0 && (idx%2) == 0 && (idx/2) < numColors)
      {
      idx = 3*idx/2;

      rgbColors[idx]   = colorEntities[entref].redF();
      rgbColors[idx+1] = colorEntities[entref].greenF();
      rgbColors[idx+2] = colorEntities[entref].blueF();
      changed = true;
      }
    }

  if (changed )
    {
    indexedColors.Set(&rgbColors[0],
      static_cast<unsigned int>(rgbColors.size()));
    lutProxy->UpdateVTKObjects();
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      colorByMode.toStdString().c_str(), lutProxy);
    }

}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateAttributeColorTable(
  pqDataRepresentation* rep,
  vtkSMProxy* lutProxy,
  const QMap<std::string, QColor >& colorAtts,
  const std::vector<std::string>& annList)
{
  if(colorAtts.count() == 0)
    return;
  pqSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;

  if(!lutProxy || annList.size() == 0)
    return;
  // Assuming numbers in the LUT color is equal or greater than the
  // number of annotations, and the order of mapping from annotation
  // to the LUT is the same while switching colorby arrays.
  vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
  std::vector<double> rgbColors = indexedColors.GetDoubleArray();
  int numColors = (int)rgbColors.size()/3;
  if(numColors < 1)
    return;
  int idx;
  bool changed = false;
  std::vector<std::string>::const_iterator cit;
  foreach(std::string strkey, colorAtts.keys())
    {
    cit = std::find(annList.begin(), annList.end(),
                    strkey);
    if(cit == annList.end())
      continue;
    idx = cit - annList.begin();
    if(idx >= 0 && (idx%2) == 0 && (idx/2) < numColors)
      {
      idx = 3*idx/2;

      rgbColors[idx]   = colorAtts[strkey].redF();
      rgbColors[idx+1] = colorAtts[strkey].greenF();
      rgbColors[idx+2] = colorAtts[strkey].blueF();
      changed = true;
      }
    }

  if (changed )
    {
    indexedColors.Set(&rgbColors[0],
      static_cast<unsigned int>(rgbColors.size()));
    lutProxy->UpdateVTKObjects();
    }

}

//----------------------------------------------------------------------------
void pqCMBModelManager::colorRepresentationByAttribute(
    pqDataRepresentation* rep, smtk::attribute::SystemPtr attsys,
    const QString& attDefType, const QString& attItemName)
{
 // set rep colorByArray("...")
  pqSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;

  modInfo->ColorMode = vtkModelMultiBlockSource::GetAttributeTagName();
  vtkSMProxy* lutProxy = modInfo->AttributeLUT;

  // rebuild the lookup table with selected attribute and its item if it exists
  std::vector<smtk::attribute::AttributePtr> result;
  attsys->findAttributes(attDefType.toStdString(), result);
  if(result.size() == 0)
    {
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
      rep->getProxy(),
      NULL,
      lutProxy, vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);
    return;
    }

  std::string simContents;
  smtk::io::AttributeWriter xmlw;
  xmlw.includeDefinitions(true);
  xmlw.includeInstances(true);
  xmlw.includeModelInformation(true);
  xmlw.includeViews(false);

  smtk::io::Logger logger;
  bool errStatus = xmlw.writeContents( *attsys, simContents, logger);
  if(errStatus)
    {
    std::cerr << logger.convertToString() << std::endl;
    return;
    }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    modInfo->RepSource->getProxy());
  vtkSMPropertyHelper(smSource,"AttributeDefinitionType").Set("");
  vtkSMPropertyHelper(smSource,"AttributeItemName").Set("");
  vtkSMPropertyHelper(smSource,"AttributeSystemContents").Set("");
  vtkSMPropertyHelper(smSource,"AddGroupArray").Set(0);
  smSource->UpdateVTKObjects();

  vtkSMPropertyHelper(smSource,"AttributeDefinitionType").Set(
    attDefType.toStdString().c_str());
  if(!attItemName.isEmpty())
    vtkSMPropertyHelper(smSource,"AttributeItemName").Set(
      attItemName.toStdString().c_str());
  vtkSMPropertyHelper(smSource,"AttributeSystemContents").Set(
    simContents.c_str());

//  smSource->InvokeCommand("MarkDirty");
  smSource->UpdateVTKObjects();
  modInfo->RepSource->updatePipeline();

  QMap<std::string, QColor> attWithColors;
  smtk::common::UUIDs assignedAtts;
  std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
  std::vector<std::string> att_annotations;
  for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
    {
    smtk::common::UUIDs associatedEntities = (*itAtt)->associatedModelEntityIds();
    if(associatedEntities.size() > 0 &&
       assignedAtts.find((*itAtt)->id()) == assignedAtts.end())
      {
      assignedAtts.insert((*itAtt)->id());
      smtk::attribute::ItemPtr attitem;
      std::string keystring = (*itAtt)->name();
      std::string valstring = (*itAtt)->name();
      std::string stritemval;
      // Figure out which variant of the item to use, if it exists
      if(!attItemName.isEmpty() && (attitem = (*itAtt)->find(attItemName.toStdString())))
        {
        if(attitem->type() == smtk::attribute::Item::DOUBLE)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(attitem)->valueAsString();
          }
        else if (attitem->type() == smtk::attribute::Item::INT)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(attitem)->valueAsString();
          }
        else if (attitem->type() == smtk::attribute::Item::STRING)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(attitem)->value();
          }
        if(!stritemval.empty())
          {
          valstring = stritemval;
          keystring = stritemval;
          }
        }
      if(std::find(att_annotations.begin(), att_annotations.end(), keystring) ==
         att_annotations.end())
        {
        att_annotations.push_back(keystring);
        att_annotations.push_back(valstring);
        if((*itAtt)->isColorSet())
          {
          const double* attcolor = (*itAtt)->color();
          attWithColors[keystring] =
            QColor::fromRgbF(attcolor[0], attcolor[1], attcolor[2]);
          }
        }
      }
    }

  if(assignedAtts.size() > 0)
    {
    att_annotations.push_back("no attribute");
    att_annotations.push_back("no attribute");
    this->Internal->rebuildColorTable(assignedAtts.size() + 1);

    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), lutProxy,
      this->Internal->LUTColors, att_annotations);

    this->updateAttributeColorTable(rep,
      lutProxy, attWithColors, att_annotations);
    }

  // reference vtkSMTKModelFieldArrayFilter for array name
  std::string arrayname = attDefType.toStdString();
  if(!attItemName.isEmpty())
    arrayname.append(" (").append(attItemName.toStdString()).append(")");
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
    rep->getProxy(),
    (assignedAtts.size() > 0) ? arrayname.c_str() : NULL,
    lutProxy, vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);

  vtkSMProxyProperty* barProp = vtkSMProxyProperty::SafeDownCast(
                      rep->getProxy()->GetProperty("ScalarBarActor"));
  if (barProp && barProp->GetNumberOfProxies())
    {
    vtkSMProxy* scalarBarProxy = barProp->GetProxy(0);
    if(scalarBarProxy && !arrayname.empty())
      vtkSMPropertyHelper(scalarBarProxy,"ComponentTitle").Set(
        arrayname.c_str());
    }

/*
    }
  else
    {
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
      rep->getProxy(), attDefType.toStdString().c_str(), vtkDataObject::FIELD);
    }
*/
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateModelRepresentation(const smtk::model::EntityRef& model)
{
  this->clearModelSelections();
  this->Internal->updateModelRepresentation(model);
}
//----------------------------------------------------------------------------
void pqCMBModelManager::updateModelRepresentation(pqSMTKModelInfo* modinfo)
{
  if(!modinfo)
    return;
  smtk::model::EntityRef model(this->Internal->ManagerProxy->modelManager(),
                               modinfo->Info->GetModelUUID());
  this->updateModelRepresentation(model);
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateModelMeshRepresentations(const smtk::model::Model& model)
{
  this->Internal->updateModelMeshRepresentations(model);
}

//----------------------------------------------------------------------------
smtk::model::StringData pqCMBModelManager::fileModelSessions(const std::string& filename)
{
  smtk::model::StringData retBrEns;
  if(!this->Internal->ManagerProxy)
    {
    return retBrEns;
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::model::StringList bnames = pxy->sessionNames();
  for (smtk::model::StringList::iterator it = bnames.begin(); it != bnames.end(); ++it)
    {
    smtk::model::StringData bftypes = pxy->supportedFileTypes(*it);
    smtk::model::PropertyNameWithStrings typeIt;
    for(typeIt = bftypes.begin(); typeIt != bftypes.end(); ++typeIt)
      {
      for (smtk::model::StringList::iterator tpit = typeIt->second.begin();
        tpit != typeIt->second.end(); ++tpit)
        {
        if (tpit->find(lastExt) == 0)
          {
          std::string sesstype = *it;
          retBrEns[sesstype].push_back(typeIt->first);
          }
        }
      }
    }

  return retBrEns;
}

//----------------------------------------------------------------------------
smtk::model::OperatorPtr pqCMBModelManager::createFileOperator(
  const std::string& filename)
{
  this->initialize();
  if(!this->Internal->ManagerProxy)
    {
    return smtk::model::OperatorPtr();
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);
  if( lastExt == ".smtk")
    return this->managerProxy()->smtkFileOperator(filename);

  smtk::model::StringData sessionTypes = this->fileModelSessions(filename);
  if (sessionTypes.size() == 0)
    {
    std::cerr << "Could not identify a modeling kernel to use.\n";
    return smtk::model::OperatorPtr();
    }

  smtk::model::PropertyNameWithStrings typeIt = sessionTypes.begin();
  std::string sessionType, engineType;
  sessionType = typeIt->first.c_str();
  if(typeIt->second.size() > 0)
    engineType = (*typeIt->second.begin()).c_str();

  // If there are more than one session or more than one engine on a session
  // can handle this file, we need to pick the session and/or engine.
  if((sessionTypes.size() > 1 || typeIt->second.size() > 1) &&
     !this->DetermineFileReader(filename, sessionType, engineType, sessionTypes))
    {
    return smtk::model::OperatorPtr();
    }

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
//  std::cout << "Should start session \"" << sessionType << "\"\n";
  smtk::common::UUID sessId = pxy->beginSession(sessionType);
//  std::cout << "Started " << sessionType << " session: " << sessId << "\n";

  smtk::model::OperatorPtr fileOp = this->managerProxy()->newFileOperator(
    filename, sessionType, engineType);

  return fileOp;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startOperation(const smtk::model::OperatorPtr& brOp)
{
  this->initialize();
  if(!this->Internal->ManagerProxy || !brOp)
    {
    return false;
    }
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::common::UUID sessId = brOp->session()->sessionId();

  if(!pxy->validSession(sessId))
    {
    return false;
    }

  smtk::model::OperatorResult result = brOp->operate();
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed: " << brOp->name() << "\n";
    brOp->eraseResult(result);
//    pxy->endSession(sessId);
    return false;
    }

  bool hasNewModels = false, bModelGeometryChanged = false, hasNewMeshes = false;
  bool sucess = this->handleOperationResult(result, sessId,
    hasNewModels, bModelGeometryChanged, hasNewMeshes);
  if(sucess)
    {
    smtk::model::SessionRef sref(pxy->modelManager(), sessId);
    emit this->operationFinished(result, sref,
      hasNewModels, bModelGeometryChanged, hasNewMeshes);
    }

  return sucess;
}

void internal_updateEntityList(const smtk::model::EntityRef& ent,
                          smtk::common::UUIDs& modellist)
{
  if(ent.isModel())
    {
    modellist.insert(ent.entity());
    }
  else if (ent.isCellEntity() && !ent.isVolume() )
    {
    modellist.insert(
      ent.as<smtk::model::CellEntity>().model().entity());
    }
}
bool internal_isNewGeometricBlock(const smtk::model::EntityRef& ent)
{
  return (ent.isCellEntity() && !ent.isVolume() &&
        !ent.hasIntegerProperty("block_index"));
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::handleOperationResult(
  const smtk::model::OperatorResult& result,
  const smtk::common::UUID& sessionId,
  bool &hasNewModels, bool& bModelGeometryChanged,
  bool &hasNewMeshes)
{
/*
  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::forOperatorResult(result, json);
  std::cout << "Result " << cJSON_Print(json) << "\n";
*/
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed\n";
    return false;
    }

  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;

  smtk::common::UUIDs geometryChangedModels;
  smtk::common::UUIDs newMeshesModels;
  smtk::common::UUIDs groupChangedModels;
  smtk::common::UUIDs generalModifiedModels;

  pqSMTKModelInfo* minfo = NULL;
  smtk::attribute::ModelEntityItem::Ptr remEntities =
    result->findModelEntity("expunged");
  smtk::model::EntityRefArray::const_iterator it;
  for(it = remEntities->begin(); it != remEntities->end(); ++it)
    {
    // if this is a block index, its pv representation needs to be updated
    if(this->Internal->Entity2Models.find(it->entity()) !=
       this->Internal->Entity2Models.end())
      {
      geometryChangedModels.insert(this->Internal->Entity2Models[it->entity()]);
      this->Internal->Entity2Models.erase(it->entity());
      }
    else
      {
      minfo = this->Internal->modelInfoForRemovedGroup(it->entity());
      if(minfo)
        {
        groupChangedModels.insert(minfo->Info->GetModelUUID());
        }
      }
    }

  smtk::attribute::ModelEntityItem::Ptr modelWithMeshes =
    result->findModelEntity("mesh_created");
  if(modelWithMeshes)
    {
    for(it = modelWithMeshes->begin(); it != modelWithMeshes->end(); ++it)
      {
      internal_updateEntityList(*it, newMeshesModels);
      }
    }
  hasNewMeshes = newMeshesModels.size() > 0;

  smtk::attribute::ModelEntityItem::Ptr tessChangedEntities =
    result->findModelEntity("tess_changed");
  if(tessChangedEntities )
    {
    for(it = tessChangedEntities->begin(); it != tessChangedEntities->end(); ++it)
      {
      internal_updateEntityList(*it, geometryChangedModels);
      }
    }

  // process "modified" in result to figure out if models are changed
  // or there are new cell entities or new groups
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("modified");
  for(it = resultEntities->begin(); it != resultEntities->end(); ++it)
      {
      if(it->isModel())
        {
        generalModifiedModels.insert(it->entity()); // TODO: check what kind of operations on the model
        }
      else if (internal_isNewGeometricBlock(*it)) // a new block
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        }
      else if(it->isGroup() && (minfo = this->modelInfo(*it)))
        {
        groupChangedModels.insert(minfo->Info->GetModelUUID());
        }
      }

  hasNewModels = false;
  // process "created" in result to figure out if there are new cell entities
  smtk::attribute::ModelEntityItem::Ptr newEntities =
    result->findModelEntity("created");
  if(newEntities)
    for(it = newEntities->begin(); it != newEntities->end(); ++it)
      {
      if(it->isModel())
        {
        // The new models will be handled later on by checking all the models
        // in the Manager against the internal ModelInfos map here
        hasNewModels = true;
        continue;
        }

      if (internal_isNewGeometricBlock(*it)) // a new entity?
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        }
      else if(it->isGroup() && (minfo = this->modelInfo(*it)))
        {
        groupChangedModels.insert(minfo->Info->GetModelUUID());
        }
      }
  bModelGeometryChanged = geometryChangedModels.size() > 0;

  // check if there is "selection", such as from "grow" operator.
  //
  smtk::attribute::MeshSelectionItem::Ptr meshSelections =
    result->findMeshSelection("selection");

  smtk::common::UUIDs modelids =
    pxy->modelManager()->entitiesMatchingFlags(
    smtk::model::MODEL_ENTITY);
  smtk::common::UUIDs remmodels;
  // Clean out models that are not in the manager after operation
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(modelids.find(mit->first) == modelids.end())
      remmodels.insert(mit->first);
    }
  this->Internal->removeModelRepresentations(remmodels, view);

  // remove expunged mesh collection representations
  smtk::attribute::MeshItem::Ptr remMeshes =
    result->findMesh("mesh_expunged");
  if(remMeshes)
    {
    smtk::common::UUIDs meshcollections;
    smtk::mesh::MeshList::const_iterator mit;
    for(mit = remMeshes->begin(); mit != remMeshes->end(); ++mit)
      {
      meshcollections.insert((*mit).collection()->entity());
      }
    this->Internal->removeMeshRepresentations(
      pxy->modelManager()->meshes(), meshcollections, view);
    }

  smtk::model::Models modelEnts =
    pxy->modelManager()->entitiesMatchingFlagsAs<smtk::model::Models>(
    smtk::model::MODEL_ENTITY);
  bool success = true;
  smtk::model::SessionRef sref(pxy->modelManager(), sessionId);
  for (smtk::model::Models::iterator it = modelEnts.begin();
      it != modelEnts.end(); ++it)
    {
    if(it->isValid() && !it->parent().isModel()) // ingore submodels
      {
      smtk::model::Model newModel;
      if(internal_getModelInfo(*it, this->Internal->ModelInfos) == NULL)
        {
        hasNewModels = true;
        // if this is a submodel, use its parent
        newModel =  it->parent().isModel() ? it->parent() : *it;
        success = this->Internal->addModelRepresentation(
          newModel, view, this->Internal->ManagerProxy, "", sref);
        }
      // update representation
      else if(geometryChangedModels.find(it->entity()) != geometryChangedModels.end())
        {
        this->updateModelRepresentation(*it);
        this->updateModelMeshRepresentations(*it);
        }
      // update group LUT
      else if(groupChangedModels.find(it->entity()) != groupChangedModels.end())
        {
        this->Internal->updateEntityGroupFieldArrayAndAnnotations(*it);
        pqSMTKModelInfo* modelInfo = this->modelInfo(*it);
        this->Internal->resetColorTable(modelInfo, modelInfo->GroupLUT, modelInfo->grp_annotations);
        }
      // for grow "selection" operations, the model is writen to "modified" result
      else if(meshSelections/* && meshSelections->numberOfValues() > 0 */&&
        generalModifiedModels.find(it->entity()) != generalModifiedModels.end())
        {
        if((minfo = this->modelInfo(*it)))
          emit this->requestMeshSelectionUpdate(meshSelections, minfo);
        }

      // Handle new meshes for a model
      if(newMeshesModels.find(it->entity()) != newMeshesModels.end())
        {
        // If models are also created from the same operation, we only show models,
        // and set mesh representation invisible.
        this->Internal->createMeshRepresentation(this->managerProxy()->modelManager(),
                                                 *it, view, !hasNewModels);
        // make the new model source the active source
        if(hasNewModels && newModel.isValid())
          {
          pqSMTKModelInfo* newModelInfo = this->modelInfo(newModel);
          if(newModelInfo)
            pqActiveObjects::instance().setActiveSource(newModelInfo->RepSource);
          }
        }

      }
    }

  return success;
}

//-----------------------------------------------------------------------------
bool pqCMBModelManager::DetermineFileReader(
  const std::string& filename,
  std::string& sessionType,
  std::string& engineType,
  const smtk::model::StringData& sessionTypes)
{
  QString readerType,readerGroup;
  vtkNew<vtkStringList> list;
  smtk::model::StringData::const_iterator typeIt;
  std::string desc;
  for(typeIt = sessionTypes.begin(); typeIt != sessionTypes.end(); ++typeIt)
    {
    for (smtk::model::StringList::const_iterator tpit = typeIt->second.begin();
      tpit != typeIt->second.end(); ++tpit)
      {
      desc = typeIt->first;
      list->AddString(desc.c_str()); // session
      list->AddString((*tpit).c_str()); // engine
      engineType = (*tpit).c_str();
      sessionType = desc.c_str();
      desc += "::";
      desc += *tpit;
      list->AddString(desc.c_str()); // session::engine
      }
    }

  if(list->GetLength() > 3)
    {
    // If more than one readers found.
    pqSelectReaderDialog prompt(filename.c_str(), NULL,
      list.GetPointer(), pqCoreUtilities::mainWidget());
    if (prompt.exec() == QDialog::Accepted)
      {
      engineType = prompt.getReader().toStdString();
      sessionType = prompt.getGroup().toStdString();
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::clear()
{
  this->Internal->clear();
  if(this->Internal->ManagerProxy)
    this->Internal->ManagerProxy->endSessions();
  this->Internal->ManagerProxy = NULL;
  emit currentModelCleared();
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startNewSession(const std::string& sessionName)
{
  smtk::common::UUID sessionId =
    this->managerProxy()->beginSession(sessionName, 
                                       smtk::common::UUID::null(), true);
  smtk::model::SessionRef sref(
    this->managerProxy()->modelManager(), sessionId);

  if (!sref.session())
    {
    std::cerr << "Could not start new session of type \"" << sessionName << "\"\n";
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::onPluginLoaded()
{
  // force remote server to refetch sessions incase a new session is loaded
  if(this->managerProxy())
    {
    QStringList newFileTypes;
    QStringList newSessionNames;
    smtk::model::StringList oldBnames = this->Internal->ManagerProxy->sessionNames();
    smtk::model::StringList newBnames = this->Internal->ManagerProxy->sessionNames(true);

    for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
      {
      // if there is the new session
      if(std::find(oldBnames.begin(), oldBnames.end(), *it) == oldBnames.end())
        {
        newSessionNames << (*it).c_str();
        std::set<std::string> bftypes = this->supportedFileTypes(*it);
        for (std::set<std::string>::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
          {
          newFileTypes << (*tpit).c_str();
          }
        }
      }
    if(newSessionNames.count() > 0)
      {
      emit newSessionLoaded(newSessionNames);
      }
    if(newFileTypes.count() > 0)
      {
      emit newFileTypesAdded(newFileTypes);
      }
    }
}

//----------------------------------------------------------------------------
QList<pqSMTKMeshInfo*>  pqCMBModelManager::selectedMeshes() const
{
  QList<pqSMTKMeshInfo*> selMeshInfos;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    for(qInternal::itMeshInfo meshiter = mit->second.MeshInfos.begin();
        meshiter != mit->second.MeshInfos.end(); ++meshiter)
      {
      if(!meshiter->second.RepSource)
        continue;
      vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
        meshiter->second.RepSource->getProxy());
      if(smSource && smSource->GetSelectionInput(0))
        {
        selMeshInfos.append(&meshiter->second);
        }
      }
    }
  return selMeshInfos;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::setActiveModelSource(const smtk::common::UUID& entid)
{
  smtk::model::EntityRef entity(this->managerProxy()->modelManager(), entid);
  pqSMTKModelInfo* activeInfo = this->modelInfo(entity);
  pqPipelineSource* activeSource =  activeInfo ? activeInfo->RepSource : NULL;
  pqActiveObjects::instance().setActiveSource(activeSource);
}
