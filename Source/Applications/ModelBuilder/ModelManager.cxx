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

#include "vtkSMModelManagerProxy.h"
#include "vtkSMProxyManager.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "smtk/model/StringData.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/model/Operator.h"
#include "vtksys/SystemTools.hxx"

//----------------------------------------------------------------------------
ModelManager::ModelManager(pqServer* server) :
  m_Server(server), m_ManagerProxy(NULL), m_modelSource(NULL)
{
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
  if(!this->m_ManagerProxy)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->m_modelSource = builder->createSource(
      "CMBModelGroup", "ModelManager", m_Server);

    this->m_ManagerProxy = vtkSMModelManagerProxy::SafeDownCast(
      this->m_modelSource->getProxy());
    }
  this->m_CurrentFile = "";
}

//----------------------------------------------------------------------------
vtkSMModelManagerProxy* ModelManager::managerProxy()
{
  return this->m_ManagerProxy;
}

//----------------------------------------------------------------------------
pqPipelineSource* ModelManager::modelSource()
{
  return this->m_modelSource;
}

//----------------------------------------------------------------------------
pqDataRepresentation* ModelManager::modelRepresentation()
{
  return this->m_modelRepresentation;
}

//----------------------------------------------------------------------------
std::vector<std::string> ModelManager::supportedFileTypes()
{
  if(this->m_ManagerProxy)
    {
    return this->m_ManagerProxy->supportedFileTypes("cmb");
    }
  std::vector<std::string> resultVec;
  return resultVec;
}

//----------------------------------------------------------------------------
bool ModelManager::loadModel(const std::string& filename, pqRenderView* view)
{
  this->initialize();
  if(!this->m_ManagerProxy)
    {
    return false;
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::cout << "File        " << filename << "   ext " << lastExt << "\n";
  std::string bridgeType;

  vtkSMModelManagerProxy* pxy = this->m_ManagerProxy;
  smtk::model::StringList bnames = pxy->bridgeNames();
  for (smtk::model::StringList::iterator it = bnames.begin(); bridgeType.empty() && it != bnames.end(); ++it)
    {
    std::cout << "Bridge      " << *it << "\n";
    smtk::model::StringList bftypes = pxy->supportedFileTypes(*it);
    for (smtk::model::StringList::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
      {
      std::cout << "  File type " << *tpit << "\n";
      if (tpit->find(lastExt) == 0)
        {
        bridgeType = *it;
        break;
        }
      }
    }
  if (bridgeType.empty())
    {
    std::cerr << "Could not identify a modeling kernel to use.\n";
    return false;
    }

  std::cout << "Should start bridge \"" << bridgeType << "\"\n";
  smtk::util::UUID sessId = pxy->beginBridgeSession(bridgeType);
  std::cout << "Started \"cmb\" session: " << sessId << "\n";

  smtk::model::OperatorResult result = pxy->readFile(filename, bridgeType);
  if (
    result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
    pxy->endBridgeSession(sessId);
    return false;
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

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->m_modelRepresentation = builder->createDataRepresentation(
        this->modelSource()->getOutputPort(0), view);
//  qobject_cast<pqPipelineRepresentation*>(modelRep)->colorByArray(
//    "ModelFaceColor",
//    vtkGeometryRepresentation::CELL_DATA);

  this->m_CurrentFile = filename;
  pxy->endBridgeSession(sessId);
  return true;
}

//----------------------------------------------------------------------------
void ModelManager::clear()
{
  if(this->m_ManagerProxy)
    {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->m_modelSource);
    this->m_ManagerProxy = 0;
    }
  this->m_CurrentFile = "";
  emit currentModelCleared();
}
