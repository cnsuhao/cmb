/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "pqCMBModelManager.h"

#include "vtkDataObject.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkModelMultiBlockSource.h"
#include "vtkNew.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSmartPointer.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringList.h"

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
#include "vtksys/SystemTools.hxx"

#include "smtk/common/UUID.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include <map>
#include <set>
#include <QDebug>

//----------------------------------------------------------------------------
void cmbSMTKModelInfo::init(
  pqPipelineSource* source, pqDataRepresentation* rep,
  const std::string& filename, smtk::model::ManagerPtr mgr)
{
  this->Source = source;
  this->FileName = filename;
  this->Representation = rep;
  this->Info = vtkSmartPointer<vtkPVSMTKModelInformation>::New();
  this->Source->getProxy()->GatherInformation(this->Info);

  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    this->Info->GetUUID2BlockIdMap().begin();
  for(; it != this->Info->GetUUID2BlockIdMap().end(); ++it)
    {
    mgr->setIntegerProperty(it->first, "block_index", it->second);
    }

  // create block selection source proxy
  vtkSMSessionProxyManager *proxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  this->SelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "BlockSelectionSource"));
  smtk::model::ModelEntity modelEntity(mgr,this->Info->GetModelUUID());
  if (modelEntity.isValid())
    this->Bridge = modelEntity.bridge();

}
/// Copy constructor.
cmbSMTKModelInfo::cmbSMTKModelInfo(const cmbSMTKModelInfo& other)
{
  this->Source = other.Source;
  this->FileName = other.FileName;
  this->Representation = other.Representation;
  this->Info = other.Info;
  this->SelectionSource = other.SelectionSource;
}

//-----------------------------------------------------------------------------
class pqCMBModelManager::qInternal
{
public:

  // <ModelEnity, modelInfo>
  std::map<smtk::common::UUID, cmbSMTKModelInfo> ModelInfos;
  typedef std::map<smtk::common::UUID, cmbSMTKModelInfo >::iterator itModelInfo;
  std::map<smtk::common::UUID, smtk::common::UUID> Entity2Models;
  typedef std::map<smtk::common::UUID, cmbSMTKModelInfo >::iterator itModelEnt;

  pqServer* Server;
  vtkSmartPointer<vtkSMModelManagerProxy> ManagerProxy;
  vtkNew<vtkDiscreteLookupTable> ModelColorLUT;
  std::vector<vtkTuple<double, 3> > LUTColors;

  bool addModelRepresentation(const smtk::model::Cursor& model,
    pqRenderView* view, vtkSMModelManagerProxy* smProxy,
    const std::string& filename)
  {
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      {
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

        pqDataRepresentation* rep = builder->createDataRepresentation(
          modelSrc->getOutputPort(0), view);
        if(rep)
          {
          std::cout << "Add model: " << model.entity().toString().c_str() << "\n";

          smtk::model::ManagerPtr mgr = smProxy->modelManager();
          this->ModelInfos[model.entity()].init(modelSrc, rep, filename, mgr);
  
//          std::vector<vtkTuple<const char*, 2> > ent_annotations;
//          std::vector<vtkTuple<const char*, 2> > vol_annotations;
//          std::vector<vtkTuple<const char*, 2> > grp_annotations;

          vtkNew<vtkStringList> ent_annotations;
          vtkNew<vtkStringList> vol_annotations;
          vtkNew<vtkStringList> grp_annotations;

          vtkPVSMTKModelInformation* pvinfo = this->ModelInfos[model.entity()].Info;
          std::map<smtk::common::UUID, unsigned int>::const_iterator it =
            pvinfo->GetUUID2BlockIdMap().begin();
          for(; it != pvinfo->GetUUID2BlockIdMap().end(); ++it)
            {
            this->Entity2Models[it->first] = model.entity();
//            const char* value[2] = {
//                it->first.toString().c_str(),
//                mgr->name(it->first).c_str()};
//            ent_annotations.push_back(vtkTuple<const char*, 2>(value));
            ent_annotations->AddString(it->first.toString().c_str());
            ent_annotations->AddString(mgr->name(it->first).c_str());
            }

          smtk::model::GroupEntities modGroups =
            model.as<smtk::model::ModelEntity>().groups();
          for(smtk::model::GroupEntities::iterator it = modGroups.begin();
             it != modGroups.end(); ++it)
            {
//            const char* value[2] = {
//                (*it).entity().toString().c_str(),
//                (*it).name().c_str()};
//            grp_annotations.push_back(vtkTuple<const char*, 2>(value));
            grp_annotations->AddString((*it).entity().toString().c_str());
            grp_annotations->AddString((*it).name().c_str());
            }

          smtk::model::CellEntities modVols =
            model.as<smtk::model::ModelEntity>().cells();
          for(smtk::model::CellEntities::iterator it = modVols.begin();
             it != modVols.end(); ++it)
            {
            if((*it).isVolume())
              {
//              const char* value[2] = {
//                  (*it).entity().toString().c_str(),
//                  (*it).name().c_str()};
//              vol_annotations.push_back(vtkTuple<const char*, 2>(value));
              vol_annotations->AddString((*it).entity().toString().c_str());
              vol_annotations->AddString((*it).name().c_str());
              }
            }

//          vtkSMPropertyHelper(rep->getProxy(), "SuppressLOD").Set(1);
          RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
            rep->getProxy(), vtkModelMultiBlockSource::GetEntityTagName(),
            this->LUTColors, ent_annotations.GetPointer());
          RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
            rep->getProxy(), vtkModelMultiBlockSource::GetGroupTagName(),
            this->LUTColors, grp_annotations.GetPointer());
          RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
            rep->getProxy(), vtkModelMultiBlockSource::GetVolumeTagName(),
            this->LUTColors, vol_annotations.GetPointer());

          RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
            rep->getProxy(), NULL, vtkDataObject::CELL);


          return true;         
          }
        }
      // Should not get here.
      qCritical() << "Failed to create a pqPipelineSource or pqRepresentation for the model: "
        << model.entity().toString().c_str();
      return false;
      }
    else
      {
      qCritical() << "There is already a pqPipelineSource for the model: "
        << model.entity().toString().c_str();
      return false;
      }
  }

  void clear()
  {
    for(itModelInfo mit = this->ModelInfos.begin(); mit != this->ModelInfos.end(); ++mit)
      {
      pqApplicationCore::instance()->getObjectBuilder()->destroy(mit->second.Source);
      }
    this->Entity2Models.clear();
    this->ModelInfos.clear();
//    this->ManagerProxy = NULL;
  }

  qInternal(pqServer* server): Server(server)
  {
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
      this->LUTColors[i].GetData()[2] = rgba[1];
      }
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
      }
    }
} 

//----------------------------------------------------------------------------
vtkSMModelManagerProxy* pqCMBModelManager::managerProxy()
{
  this->initialize();
  return this->Internal->ManagerProxy;
}

//----------------------------------------------------------------------------
cmbSMTKModelInfo* pqCMBModelManager::modelInfo(const smtk::model::Cursor& selentity)
{
/*
//  smtk::model::Cursor entity(this->managerProxy()->modelManager(), uid);
  smtk::model::ModelEntity modelEntity = entity.isModelEntity() ?
      entity.as<smtk::model::ModelEntity>() : entity.owningModel();

  if(modelEntity.isValid() && this->Internal->ModelInfos.find(modelEntity.entity()) !=
    this->Internal->ModelInfos.end())
    {
    return &this->Internal->ModelInfos[modelEntity.entity()];
    }
*/
  /*    (entity.isCellEntity() ? entity.as<smtk::model::CellEntity>().model() :
       entity.owningModel());*/
/*
  std::cout << "modelInfo Id: " << uid.toString().c_str() << std::endl;
  std::cout << "Model is valid? " << modelEntity.isValid() << std::endl;

  if(modelEntity.isValid() && this->Internal->ModelInfos.find(modelEntity.entity()) !=
    this->Internal->ModelInfos.end())
    {
    return &this->Internal->ModelInfos[modelEntity.entity()];
    }
*/

  if(this->Internal->Entity2Models.find(selentity.entity())
    != this->Internal->Entity2Models.end())
    {
    return &this->Internal->ModelInfos[ this->Internal->Entity2Models[selentity.entity()] ];
    }

  return NULL;
}

//----------------------------------------------------------------------------
cmbSMTKModelInfo* pqCMBModelManager::modelInfo(pqDataRepresentation* rep)
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
QList<cmbSMTKModelInfo*>  pqCMBModelManager::selectedModels()
{
  QList<cmbSMTKModelInfo*> selModels;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Source)
      {
      continue;
      }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      mit->second.Source->getProxy());
    if(smSource && smSource->GetSelectionInput(0))
      {
      selModels.append(&mit->second);
      }
    }
  return selModels;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::clearModelSelections()
{
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Source)
      {
      continue;
      }
    pqPipelineSource* source = mit->second.Source;
    pqOutputPort* outport = source->getOutputPort(0);
    if(outport)
      {
      outport->setSelectionInput(0, 0);
      }
    }
}

//----------------------------------------------------------------------------
int pqCMBModelManager::numberOfModels()
{
  return (int)this->Internal->ModelInfos.size();
}

//----------------------------------------------------------------------------
pqDataRepresentation* pqCMBModelManager::activeModelRepresentation()
{
  return pqActiveObjects::instance().activeRepresentation();
}

//----------------------------------------------------------------------------
QList<pqDataRepresentation*> pqCMBModelManager::modelRepresentations()
{
  QList<pqDataRepresentation*> result;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Representation)
      {
      continue;
      }
    result.append(mit->second.Representation);
    }
  return result;
}

//----------------------------------------------------------------------------
std::set<std::string> pqCMBModelManager::supportedFileTypes(
  const std::string& bridgeName)
{
  std::set<std::string> resultSet;
  if(this->Internal->ManagerProxy)
    {
    smtk::model::StringData bftypes = this->Internal->ManagerProxy->supportedFileTypes(bridgeName);
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
smtk::model::StringData pqCMBModelManager::fileModelBridges(const std::string& filename)
{
  smtk::model::StringData retBrEns;
  if(!this->Internal->ManagerProxy)
    {
    return retBrEns;
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::model::StringList bnames = pxy->bridgeNames();
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
          retBrEns[*it].push_back(typeIt->first);
          }
        }
      }
    }

  return retBrEns;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::loadModel(const std::string& filename, pqRenderView* view)
{
  this->initialize();
  if(!this->Internal->ManagerProxy)
    {
    return false;
    }

  smtk::model::StringData bridgeTypes = this->fileModelBridges(filename);
  if (bridgeTypes.size() == 0)
    {
    std::cerr << "Could not identify a modeling kernel to use.\n";
    return false;
    }

  smtk::model::PropertyNameWithStrings typeIt = bridgeTypes.begin();
  std::string bridgeType, engineType;
  bridgeType = typeIt->first.c_str();
  if(typeIt->second.size() > 0)
    engineType = (*typeIt->second.begin()).c_str();

  // If there are more than one bridge or more than one engine on a bridge
  // can handle this file, we need to pick the bridge and/or engine.
  if((bridgeTypes.size() > 1 || typeIt->second.size() > 1) &&
     !this->DetermineFileReader(filename, bridgeType, engineType, bridgeTypes))
    {
    return false;
    }

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  std::cout << "Should start bridge \"" << bridgeType << "\"\n";
  smtk::common::UUID sessId = pxy->beginBridgeSession(bridgeType);
  std::cout << "Started " << bridgeType << " session: " << sessId << "\n";

  smtk::model::OperatorResult result = pxy->readFile(filename, bridgeType, engineType);
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
//    pxy->endBridgeSession(sessId);
    return false;
    }
  bool hasNewModels = false;
  bool success = this->handleOperationResult(result, sessId, hasNewModels);
  if(success)
    {
    emit this->operationFinished(result, hasNewModels);
    }

  return success;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startOperation(const smtk::model::OperatorPtr& brOp)
{
  this->initialize();
  if(!this->Internal->ManagerProxy || !brOp || !brOp->ableToOperate())
    {
    return false;
    }
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::common::UUID sessId = brOp->bridge()->sessionId();
  std::cout << "Found session: " << sessId << "\n";

 // sessId = pxy->beginBridgeSession("cgm");

  if(!pxy->validBridgeSession(sessId))
    {
    return false;
    }

  smtk::model::OperatorResult result = brOp->operate();
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed: " << brOp->name() << "\n";
//    pxy->endBridgeSession(sessId);
    return false;
    }

  bool hasNewModels = false;
  bool sucess = this->handleOperationResult(result, sessId, hasNewModels);
  if(sucess)
    {
    emit this->operationFinished(result, hasNewModels);
    }

  return sucess;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::handleOperationResult(
  const smtk::model::OperatorResult& result,
  const smtk::common::UUID& bridgeSessionId,
  bool &hadNewModels)
{
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed\n";
    return false;
    }
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  pxy->fetchWholeModel();

  smtk::model::ModelEntities modelEnts =
    pxy->modelManager()->entitiesMatchingFlagsAs<smtk::model::ModelEntities>(
    smtk::model::MODEL_ENTITY);
  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  bool success = true;
  hadNewModels = false;
  smtk::model::BridgePtr bridge = pxy->modelManager()->findBridgeSession(bridgeSessionId);
  for (smtk::model::ModelEntities::iterator it = modelEnts.begin();
      it != modelEnts.end(); ++it)
    {
//   if(!mit->bridge() || mit->bridge()->name() == "native") // a new model
     if((*it).isValid() && this->Internal->ModelInfos.find((*it).entity()) ==
      this->Internal->ModelInfos.end())
      {
      hadNewModels = true;
      pxy->modelManager()->setBridgeForModel(bridge, (*it).entity());
      success = this->Internal->addModelRepresentation(
        *it, view, this->Internal->ManagerProxy, "");
      // fetch again for "block_index" property of entities, which are set
      // while building multi-block dataset
      // pxy->fetchWholeModel();
      }
    }
  pxy->modelManager()->assignDefaultNames();

  return success;
}

//-----------------------------------------------------------------------------
bool pqCMBModelManager::DetermineFileReader(
  const std::string& filename, 
  std::string& bridgeType,
  std::string& engineType,
  const smtk::model::StringData& bridgeTypes)
{
  QString readerType,readerGroup;
  vtkNew<vtkStringList> list;
  smtk::model::StringData::const_iterator typeIt;
  std::string desc;
  for(typeIt = bridgeTypes.begin(); typeIt != bridgeTypes.end(); ++typeIt)
    {
    for (smtk::model::StringList::const_iterator tpit = typeIt->second.begin();
      tpit != typeIt->second.end(); ++tpit)
      {
      desc = typeIt->first;
      list->AddString(desc.c_str()); // bridge
      list->AddString((*tpit).c_str()); // engine
      engineType = (*tpit).c_str();
      bridgeType = desc.c_str();
      desc += "::";
      desc += *tpit;
      list->AddString(desc.c_str()); // bridge::engine
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
      bridgeType = prompt.getGroup().toStdString();
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
    this->Internal->ManagerProxy->endBridgeSessions();
  this->Internal->ManagerProxy = NULL;
  emit currentModelCleared();
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startSession(const std::string& bridgeName)
{
  smtk::common::UUID bridgeId =
    this->Internal->ManagerProxy->beginBridgeSession(bridgeName, true);
  smtk::model::BridgePtr bridge =
    this->managerProxy()->modelManager()->findBridgeSession(bridgeId);

  if (!bridge)
    {
    std::cerr << "Could not start new bridge of type \"" << bridgeName << "\"\n";
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::onPluginLoaded()
{
  // force remote server to refetch bridges incase a new bridge is loaded
  if(this->Internal->ManagerProxy)
    {
    QStringList newFileTypes;
    QStringList newBridgeNames;
    smtk::model::StringList oldBnames = this->Internal->ManagerProxy->bridgeNames();
    smtk::model::StringList newBnames = this->Internal->ManagerProxy->bridgeNames(true);

    for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
      {
      // if there is the new bridge
      if(std::find(oldBnames.begin(), oldBnames.end(), *it) == oldBnames.end())
        {
        newBridgeNames << (*it).c_str();
        std::set<std::string> bftypes = this->supportedFileTypes(*it);
        for (std::set<std::string>::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
          {
          newFileTypes << (*tpit).c_str();
          }
        }
      }
    if(newBridgeNames.count() > 0)
      {
      emit newBridgeLoaded(newBridgeNames);
      }
    if(newFileTypes.count() > 0)
      {
      emit newFileTypesAdded(newFileTypes);
      }
    }
}
