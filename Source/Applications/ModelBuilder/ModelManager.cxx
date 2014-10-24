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

#include "ModelManager.h"

#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSmartPointer.h"
#include "vtkSMSourceProxy.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "vtksys/SystemTools.hxx"

#include "smtk/common/UUID.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/model/StringData.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include <map>
#include <set>
#include <QDebug>

//----------------------------------------------------------------------------
void cmbSMTKModelInfo::init(
  pqPipelineSource* source, pqDataRepresentation* rep, const std::string& filename)
{
  this->Source = source;
  this->FileName = filename;
  this->Representation = rep;
  this->Info = vtkSmartPointer<vtkPVSMTKModelInformation>::New();
  this->Source->getProxy()->GatherInformation(this->Info);

  // create block selection source proxy
  vtkSMSessionProxyManager *proxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  this->SelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "BlockSelectionSource"));

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
class ModelManager::qInternal
{
public:

  // <ModelEnity, modelInfo>
  std::map<smtk::common::UUID, cmbSMTKModelInfo> Models;
  typedef std::map<smtk::common::UUID, cmbSMTKModelInfo >::iterator itModelEnt;

  pqServer* Server;
  vtkSmartPointer<vtkSMModelManagerProxy> ManagerProxy;

  bool addModelRepresentation(const smtk::model::Cursor& model,
    pqRenderView* view, vtkSMModelManagerProxy* smProxy,
    const std::string& filename)
  {
    if(this->Models.find(model.entity()) == this->Models.end())
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
          //cmbSMTKModelInfo modinfo;
          //modinfo.init(modelSrc, rep, filename);
           std::cout << "Add model: " << model.entity().toString().c_str() << "\n";

          this->Models[model.entity()].init(modelSrc, rep, filename);
          /*.insert(std::pair<smtk::common::UUID, cmbSMTKModelInfo&>(
                              model.entity(), modinfo));*/
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
    for(itModelEnt mit = this->Models.begin(); mit != this->Models.end(); ++mit)
      {
      pqApplicationCore::instance()->getObjectBuilder()->destroy(mit->second.Source);
      }
    this->ManagerProxy = NULL;
  }

  qInternal(pqServer* server): Server(server)
  {
  }
};

//----------------------------------------------------------------------------
ModelManager::ModelManager(pqServer* server)
{
  this->Internal = new qInternal(server);
  this->initialize();
}

//----------------------------------------------------------------------------
ModelManager::~ModelManager()
{
  this->clear();
}

//----------------------------------------------------------------------------
void ModelManager::initialize()
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
vtkSMModelManagerProxy* ModelManager::managerProxy()
{
  return this->Internal->ManagerProxy;
}

//----------------------------------------------------------------------------
cmbSMTKModelInfo* ModelManager::modelInfo(const smtk::common::UUID& uid)
{
/*
  smtk::model::Cursor entity(this->managerProxy()->modelManager(), uid);
  smtk::model::ModelEntity modelEntity = entity.isModelEntity() ?
      entity.as<smtk::model::ModelEntity>() : entity.owningModel();
*/
  /*    (entity.isCellEntity() ? entity.as<smtk::model::CellEntity>().model() :
       entity.owningModel());*/
/*
  std::cout << "modelInfo Id: " << uid.toString().c_str() << std::endl;
  std::cout << "Model is valid? " << modelEntity.isValid() << std::endl;

  if(modelEntity.isValid() && this->Internal->Models.find(modelEntity.entity()) !=
    this->Internal->Models.end())
    {
    return &this->Internal->Models[modelEntity.entity()];
    }
*/
  for(qInternal::itModelEnt mit = this->Internal->Models.begin();
      mit != this->Internal->Models.end(); ++mit)
    {
    if(!mit->second.Info)
      {
      continue;
      }

    smtk::common::UUIDs ids = mit->second.Info->GetBlockUUIDs();
    if(std::find(ids.begin(), ids.end(), uid) != ids.end())
      {
      return &(mit->second);
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
QList<cmbSMTKModelInfo*>  ModelManager::selectedModels()
{
  QList<cmbSMTKModelInfo*> selModels;
  for(qInternal::itModelEnt mit = this->Internal->Models.begin();
      mit != this->Internal->Models.end(); ++mit)
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
int ModelManager::numberOfModels()
{
  return (int)this->Internal->Models.size();
}

//----------------------------------------------------------------------------
pqDataRepresentation* ModelManager::activeModelRepresentation()
{
  return pqActiveObjects::instance().activeRepresentation();
}

//----------------------------------------------------------------------------
std::vector<std::string> ModelManager::supportedFileTypes(
  const std::string& bridgeName)
{
  std::vector<std::string> resultVec;
  if(this->Internal->ManagerProxy)
    {
    smtk::model::StringList bftypes = this->Internal->ManagerProxy->supportedFileTypes(bridgeName);
    for (smtk::model::StringList::iterator tpit = bftypes.begin();
      tpit != bftypes.end(); ++tpit)
      {
      // ".cmb (Conceptual Model Builder)"
      // we want to convert the format into Paraview format for its fileOpen dialog
      // "Conceptual Model Builder (*.cmb)"
      QString filetype = (*tpit).c_str();
      int idx = filetype.indexOf('(');

      QString descr = filetype.mid(idx + 1, filetype.indexOf(')') - idx -1);
      QString ext = filetype.left(filetype.indexOf('('));
      resultVec.push_back(
              descr.append(" (*").append(ext.simplified()).append(")").toStdString());
      }
    }
  return resultVec;
}

//----------------------------------------------------------------------------
std::string ModelManager::fileModelBridge(const std::string& filename)
{
  if(!this->Internal->ManagerProxy)
    {
    return "";
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::model::StringList bnames = pxy->bridgeNames();
  for (smtk::model::StringList::iterator it = bnames.begin(); it != bnames.end(); ++it)
    {
    smtk::model::StringList bftypes = pxy->supportedFileTypes(*it);
    for (smtk::model::StringList::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
      {
      if (tpit->find(lastExt) == 0)
        {
        return *it;
        }
      }
    }

  return "";
}

//----------------------------------------------------------------------------
bool ModelManager::loadModel(const std::string& filename, pqRenderView* view)
{
  this->initialize();
  if(!this->Internal->ManagerProxy)
    {
    return false;
    }

  std::string bridgeType = this->fileModelBridge(filename);
  if (bridgeType.empty())
    {
    std::cerr << "Could not identify a modeling kernel to use.\n";
    return false;
    }

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  std::cout << "Should start bridge \"" << bridgeType << "\"\n";
  smtk::common::UUID sessId = pxy->beginBridgeSession(bridgeType);
  std::cout << "Started " << bridgeType << " session: " << sessId << "\n";

  smtk::model::OperatorResult result = pxy->readFile(filename, bridgeType);
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
    pxy->endBridgeSession(sessId);
    return false;
    }

  smtk::model::ModelEntities modelEnts =
    pxy->modelManager()->entitiesMatchingFlagsAs<smtk::model::ModelEntities>(
    smtk::model::MODEL_ENTITY);
  bool success = false;
  for (smtk::model::ModelEntities::iterator it = modelEnts.begin();
      it != modelEnts.end(); ++it)
    {
    if((*it).isValid() && this->Internal->Models.find((*it).entity()) ==
      this->Internal->Models.end())
      {
      success = this->Internal->addModelRepresentation(
        *it, view, this->Internal->ManagerProxy, filename);
      }
    }

/*
  smtk::model::ModelEntity model = result->findModelEntity("model")->value();
  manager->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  smtk::model::DescriptivePhrase::Ptr dit;
  smtk::model::EntityPhrase::Ptr ephr = smtk::model::EntityPhrase::create()->setup(model);
  smtk::model::SimpleModelSubphrases::Ptr spg = smtk::model::SimpleModelSubphrases::create();
  ephr->setDelegate(spg);
  prindent(std::cout, 0, ephr);

  // List model operators
  smtk::model::StringList opNames = model.operatorNames();
  if (!opNames.empty())
    {
    std::cout << "\nFound operators:\n";
    for (smtk::model::StringList::const_iterator it = opNames.begin(); it != opNames.end(); ++it)
      {
      std::cout << "  " << *it << "\n";
      }
    }
*/

/*
  smtk::model::ManagerPtr storage = this->modelManager();
  smtk::model::QEntityItemModel* qmodel =
    new smtk::model::QEntityItemModel;
  smtk::model::QEntityItemDelegate* qdelegate =
    new smtk::model::QEntityItemDelegate;

  ModelBrowser* view = new ModelBrowser;
  smtk::model::Cursors cursors;
  smtk::model::Cursor::CursorsFromUUIDs(
    cursors, storage, storage->entitiesMatchingFlags(mask, false));
  view->setup(
    storage, qmodel, qdelegate,
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate(
        smtk::model::SimpleModelSubphrases::create()));
*/
  pxy->endBridgeSession(sessId);
  return success;
}

//----------------------------------------------------------------------------
void ModelManager::clear()
{
  this->Internal->clear();
  emit currentModelCleared();
}

//----------------------------------------------------------------------------
void ModelManager::onPluginLoaded()
{
  // force remote server to refetch bridges incase a new bridge is loaded
  if(this->Internal->ManagerProxy)
    {
    QStringList newFileTypes;
    smtk::model::StringList oldBnames = this->Internal->ManagerProxy->bridgeNames();
    smtk::model::StringList newBnames = this->Internal->ManagerProxy->bridgeNames(true);

    for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
      {
      // if there is the new bridge
      if(std::find(oldBnames.begin(), oldBnames.end(), *it) == oldBnames.end())
        {
        smtk::model::StringList bftypes = this->supportedFileTypes(*it);
        for (smtk::model::StringList::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
          {
          newFileTypes << (*tpit).c_str();
          }
        }
      }
    if(newFileTypes.count() > 0)
      {
      emit newBridgeLoaded(newFileTypes);
      }
    }
}
