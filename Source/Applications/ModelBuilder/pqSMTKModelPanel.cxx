//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqSMTKModelPanel.h"

#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtModelPanel.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/Group.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags
#include "smtk/model/Operator.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeView>
#include <QtGui/QDockWidget>
#include <QPointer>
#include <QString>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QMessageBox>
#include <QFileInfo>

#include <pqActiveObjects.h>
#include <pqDataRepresentation.h>
#include <pqRenderView.h>
#include <pqSMAdaptor.h>
#include <pqPipelineSource.h>
#include <pqOutputPort.h>
#include <pqApplicationCore.h>
#include <pqSelectionManager.h>
#include <pqTimer.h>
#include "pqCMBModelManager.h"
#include "pqSMTKModelInfo.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkPVSelectionInformation.h"
#include "vtkUnsignedIntArray.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkPVSMTKMeshInformation.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMDoubleMapProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMDoubleMapPropertyIterator.h"
#include "vtksys/SystemTools.hxx"
#include "SimBuilder/pqSMTKUIHelper.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <stdlib.h>

using namespace std;
using namespace smtk::model;
using namespace smtk::extension;

//-----------------------------------------------------------------------------
class pqSMTKModelPanel::qInternal
{
public:
  QPointer<qtModelPanel> ModelPanel;
//  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  bool ModelLoaded;
  QPointer<pqCMBModelManager> smtkManager;
  bool ignorePropertyChange;

  // [meshItem, <opName, sessionId>]
  QMap<QPointer<smtk::extension::qtMeshSelectionItem>,
    QPair<std::string, smtk::common::UUID> >SelectionOperations;
  QPointer<smtk::extension::qtMeshSelectionItem> CurrentMeshSelectItem;

  qInternal()
    {
//    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    this->ModelLoaded = false;
    }

  pqOutputPort* setSelectionInput(vtkSMProxy* selectionSource,
                         const std::vector<vtkIdType>& ids,
                         pqPipelineSource* source)
  {
    vtkSMPropertyHelper prop(selectionSource, "Blocks");
    prop.SetNumberOfElements(0);
    selectionSource->UpdateVTKObjects();
    // set selected blocks
    if (ids.size() > 0)
      {
      prop.Set(&ids[0], static_cast<unsigned int>(ids.size()));
      selectionSource->UpdateVTKObjects();
      }

    pqOutputPort* outport = source->getOutputPort(0);
    if(outport)
      {
      vtkSMSourceProxy *selectionSourceProxy =
        vtkSMSourceProxy::SafeDownCast(selectionSource);
      outport->setSelectionInput(selectionSourceProxy, 0);
      }
    return outport;
  }

  void updateSelectionView()
  {
    pqRenderView* renView = qobject_cast<pqRenderView*>(
         pqActiveObjects::instance().activeView());
    renView->render();
  }

};

//-----------------------------------------------------------------------------
pqSMTKModelPanel::pqSMTKModelPanel(pqCMBModelManager* mmgr, QWidget* p)
: QDockWidget(p)
{
  this->Internal = new pqSMTKModelPanel::qInternal();
  this->setObjectName("smtkModelDockWidget");
  this->Internal->smtkManager = mmgr;
//  this->connect(&this->Internal->UpdateUITimer, SIGNAL(timeout()),
//                this, SLOT(updateModelTreeView()));

  this->resetUI();
  //QSizePolicy expandPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //this->setSizePolicy(expandPolicy);

}

//-----------------------------------------------------------------------------
pqSMTKModelPanel::~pqSMTKModelPanel()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
pqCMBModelManager* pqSMTKModelPanel::modelManager()
{
  return this->Internal->smtkManager;
}

smtk::extension::qtModelView* pqSMTKModelPanel::modelView()
{
  return this->Internal->ModelPanel ?
    this->Internal->ModelPanel->getModelView() : NULL;
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::clearUI()
{
  if(this->Internal->ModelPanel)
    {
    this->Internal->ModelPanel->getModelView()->onOperationPanelClosing();
    delete this->Internal->ModelPanel;
    this->Internal->ModelPanel = NULL;
    }
  this->Internal->ModelLoaded = false;
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::setBlockVisibility(
    const smtk::common::UUID& sessid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, bool visible)
{
  int vis = visible ? 1 : 0;
  this->Internal->ModelPanel->getModelView()->syncEntityVisibility(
    sessid, entids, meshes, vis);
  this->Internal->ModelPanel->update();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::setBlockColor(
    const smtk::common::UUID& sessid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, const QColor& color)
{
  this->Internal->ModelPanel->getModelView()->syncEntityColor(
    sessid, entids, meshes, color);
  this->Internal->ModelPanel->update();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::resetUI()
{
  if(/*this->Internal->ModelLoaded || */!this->Internal->smtkManager)
    {
    return;
    }
/*
  this->Internal->smtkManager->managerProxy()->UpdatePropertyInformation();
  std::string json = pqSMAdaptor::getElementProperty(
    this->Internal->smtkManager->managerProxy()->GetProperty(
    "JSONModel")).toString().toStdString();

  std::string filename = pqSMAdaptor::getElementProperty(
    this->Internal->smtkManager->managerProxy()->GetProperty(
    "FileName")).toString().toStdString();
  filename = vtksys::SystemTools::GetFilenameName(filename);
*/

  smtk::model::ManagerPtr modelMgr = this->Internal->smtkManager->managerProxy()->modelManager();
//  smtk::io::ImportJSON::intoModelManager(json.c_str(), model);
//  model->assignDefaultNames();

//  QFileInfo fInfo(this->Internal->smtkManager->currentFile().c_str());
//  this->setWindowTitle(fInfo.fileName().toAscii().constData());
  if(!this->Internal->ModelPanel)
    {
    this->Internal->ModelPanel = new qtModelPanel(this);
    this->setWidget(this->Internal->ModelPanel);
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(entitiesSelected(const smtk::model::EntityRefs& )),
      this, SLOT(selectEntityRepresentations(const smtk::model::EntityRefs& )));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(meshesSelected(const smtk::mesh::MeshSets& )),
      this, SLOT(selectMeshRepresentations(const smtk::mesh::MeshSets& )));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)),
      this, SLOT(onFileItemCreated(smtk::extension::qtFileItem*)));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)),
      this, SLOT(onModelEntityItemCreated(smtk::extension::qtModelEntityItem*)));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(operationRequested(const smtk::model::OperatorPtr& )),
      this->Internal->smtkManager,
      SLOT(startOperation( const smtk::model::OperatorPtr& )));
    QObject::connect(this->Internal->ModelPanel->getModelView()->operatorsWidget(),
       SIGNAL(activateOperationTarget(const smtk::common::UUID&)),
       this->Internal->smtkManager,
       SLOT(setActiveModelSource(const smtk::common::UUID&)));

    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(operationCancelled(const smtk::model::OperatorPtr& )),
      this,
      SLOT(cancelOperation( const smtk::model::OperatorPtr& )));
    QObject::connect(this->Internal->smtkManager,
      SIGNAL(requestMeshSelectionUpdate(
        const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*)),
      this,
      SLOT(updateMeshSelection(
           const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*)));

    }

//  this->linkRepresentations();

  this->Internal->ModelPanel->resetView(
    qtModelPanel::VIEW_BY_TOPOLOGY, modelMgr);

  this->Internal->ModelLoaded = true;
  QObject::connect(
    this->Internal->smtkManager, SIGNAL(operationLog(const smtk::io::Logger&)),
    this->modelView()->operatorsWidget(), SLOT(displayResult(const smtk::io::Logger&)));
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::onEntitiesExpunged(
  const smtk::model::EntityRefs& expungedEnts)
{
  if(!this->Internal->ModelPanel)
    return;
  this->Internal->ModelPanel->getModelView()->onEntitiesExpunged(expungedEnts);
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::selectEntityRepresentations(const smtk::model::EntityRefs& entities)
{
  //clear current selections
  this->Internal->smtkManager->clearModelSelections();
  smtk::model::ManagerPtr modelMan =
    this->Internal->smtkManager->managerProxy()->modelManager();

  // create vector of selected block ids
  QMap<pqSMTKModelInfo*, std::vector<vtkIdType> > selmodelblocks;
  for(smtk::model::EntityRefs::const_iterator it = entities.begin(); it != entities.end(); ++it)
    {
    unsigned int flatIndex;
    //std::cout << "UUID: " << (*it).toString().c_str() << std::endl;
    if(modelMan->hasIntegerProperty((*it).entity(), "block_index"))
      {
      pqSMTKModelInfo* minfo = this->Internal->smtkManager->modelInfo(
        *it);
      const IntegerList& prop((*it).integerProperty("block_index"));
      //the flatIndex is 1 more than blockId, because the root is index 0
      if(minfo && minfo->Representation && !prop.empty())
        {
        flatIndex = prop[0];
        selmodelblocks[minfo].push_back(static_cast<vtkIdType>(flatIndex+1));
        }
      }
    }

  foreach(pqSMTKModelInfo* modinfo, selmodelblocks.keys())
    {
    this->Internal->setSelectionInput(
                                      modinfo->BlockSelectionSource,
                                      selmodelblocks[modinfo],
                                      modinfo->RepSource);
    }

  // set the last selected rep to be active source
  if(selmodelblocks.count() > 0)
    {
    pqSMTKModelInfo* minfo = selmodelblocks.keys()[selmodelblocks.keys().count() - 1];
    pqActiveObjects::instance().setActiveSource(minfo->RepSource);
    }

  this->Internal->updateSelectionView();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::selectMeshRepresentations(
  const smtk::mesh::MeshSets& selmeshes)
{
  //clear current selections
  this->Internal->smtkManager->clearMeshSelections();

  // create vector of selected block ids
  QMap<pqSMTKMeshInfo*, std::vector<vtkIdType> > selmeshblocks;
  for(smtk::mesh::MeshSets::const_iterator it = selmeshes.begin();
      it != selmeshes.end(); ++it)
    {
    unsigned int flatIndex;
    smtk::mesh::CollectionPtr c = it->collection();
    if(c->hasIntegerProperty(*it, "block_index"))
      {
      pqSMTKMeshInfo* minfo = this->Internal->smtkManager->meshInfo(*it);
      const IntegerList& prop(c->integerProperty(*it, "block_index"));
      //the flatIndex is 1 more than blockId, because the root is index 0
      if(minfo && minfo->Representation && !prop.empty())
        {
        flatIndex = prop[0];
        selmeshblocks[minfo].push_back(static_cast<vtkIdType>(flatIndex+1));
        }
      }
    }

  // update the selection manager
  foreach(pqSMTKMeshInfo* meshinfo, selmeshblocks.keys())
    {
    this->Internal->setSelectionInput(
                                      meshinfo->BlockSelectionSource,
                                      selmeshblocks[meshinfo],
                                      meshinfo->RepSource);
    }

  // set the last selected rep to be active source
  if(selmeshblocks.count() > 0)
    {
    pqSMTKMeshInfo* minfo = selmeshblocks.keys()[selmeshblocks.keys().count() - 1];
    pqActiveObjects::instance().setActiveSource(minfo->RepSource);
    }

  this->Internal->updateSelectionView();
}
//-----------------------------------------------------------------------------
void pqSMTKModelPanel::updateTreeSelection()
{
  smtk::mesh::MeshSets meshes;
  smtk::common::UUIDs uuids;
  QList<pqSMTKModelInfo*> selModels = this->Internal->smtkManager->selectedModels();
  foreach(pqSMTKModelInfo* modinfo, selModels)
    {
    this->gatherSelectionInfo(modinfo->RepSource, modinfo->Info, uuids, meshes);
    }

  QList<pqSMTKMeshInfo*> selMeshes = this->Internal->smtkManager->selectedMeshes();
  foreach(pqSMTKMeshInfo* meshinfo, selMeshes)
    {
    this->gatherSelectionInfo(meshinfo->RepSource, meshinfo->Info, uuids, meshes);
    }

  this->modelView()->selectItems(uuids, meshes, true); // block selection signal
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onFileItemCreated(smtk::extension::qtFileItem* fileItem)
{
  if(fileItem)
    {
    QObject::connect(fileItem, SIGNAL(launchFileBrowser()),
      this, SLOT(onLaunchFileBrowser()));
    }
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onLaunchFileBrowser()
{
  smtk::extension::qtFileItem* const fileItem =
    qobject_cast<smtk::extension::qtFileItem*>(QObject::sender());
  if(!fileItem)
    {
    return;
    }
  pqSMTKUIHelper::process_smtkFileItemRequest(
    fileItem, this->Internal->smtkManager->server(), fileItem->widget());
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onModelEntityItemCreated(
  smtk::extension::qtModelEntityItem* entItem)
{
  if(entItem)
    {
    QObject::connect(entItem, SIGNAL(requestEntityAssociation()),
      this, SLOT(onRequestEntityAssociation()));
    QObject::connect(entItem, SIGNAL(entityListHighlighted(const smtk::common::UUIDs&)),
      this, SLOT(requestEntitySelection(const smtk::common::UUIDs&)));
    }
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::requestEntityAssociation(
  smtk::extension::qtModelEntityItem* entItem)
{
  if(!entItem)
    {
    return;
    }

  pqSMTKUIHelper::process_smtkModelEntityItemSelectionRequest(
    entItem, this->modelView());
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onRequestEntityAssociation()
{
  smtk::extension::qtModelEntityItem* const entItem =
    qobject_cast<smtk::extension::qtModelEntityItem*>(QObject::sender());
  this->requestEntityAssociation(entItem);
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::requestEntitySelection(const smtk::common::UUIDs& uuids)
{
  this->modelView()->selectEntityItems(uuids, false); // not block selection signal
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::addMeshSelectionOperation(
    smtk::extension::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid)
{
  if(meshItem)
    this->Internal->SelectionOperations[meshItem] = qMakePair(opName, uuid);
}
//----------------------------------------------------------------------------
void pqSMTKModelPanel::setCurrentMeshSelectionItem(
    smtk::extension::qtMeshSelectionItem* meshItem)
{
  this->Internal->CurrentMeshSelectItem = meshItem;
}
//----------------------------------------------------------------------------
void pqSMTKModelPanel::startMeshSelectionOperation(
  const QList<pqOutputPort*> & selPorts)
{
  smtk::extension::qtMeshSelectionItem* currSelItem =
    this->Internal->CurrentMeshSelectItem;
  if(!currSelItem)
    return;
  smtk::attribute::MeshSelectionItemPtr MeshSelectionItem =
    smtk::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(
    currSelItem->getObject());
  if(!MeshSelectionItem)
    {
    return;
    }
  if(!currSelItem || !this->Internal->SelectionOperations.contains(currSelItem))
    return;

  // expecting a ModelEntity is set for grow selection
  const smtk::attribute::MeshSelectionItemDefinition *itemDef =
    dynamic_cast<const smtk::attribute::MeshSelectionItemDefinition*>(MeshSelectionItem->definition().get());
  smtk::attribute::ModelEntityItem::Ptr inputEntities =
    currSelItem->refModelEntityItem();
  if(!inputEntities)
    return;

  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  int isCtrlKeyDown = view->
    getRenderViewProxy()->GetInteractor()->GetControlKey();
  currSelItem->setUsingCtrlKey(isCtrlKeyDown ? true : false);

  std::map<smtk::common::UUID, std::set<int> >selectionValues;
  for(int p=0; p<selPorts.count(); p++)
    {
    pqOutputPort* opPort = selPorts.value(p);
    pqPipelineSource *source = opPort? opPort->getSource() : NULL;
    if(!source )
      continue;

    pqDataRepresentation* rep = opPort->getRepresentation(view);
    pqSMTKModelInfo* modInfo = this->modelManager()->modelInfo(rep);
    if(!modInfo || !inputEntities->has(modInfo->Info->GetModelUUID()))
      continue;
    smtk::common::UUID entid;
    vtkSMSourceProxy* selSource = opPort->getSelectionInput();
    if(selSource && selSource->GetProperty("IDs"))
      {
      // [composite_index, process_id, index]
      vtkSMPropertyHelper selIDs(selSource, "IDs");
      unsigned int count = selIDs.GetNumberOfElements();
      unsigned int flat_idx, selid;
      for (unsigned int cc=0; cc < (count/3); cc++)
        {
        flat_idx = selIDs.GetAsInt(3*cc);
        entid = modInfo->Info->GetModelEntityId(flat_idx - 1);
        selid = selIDs.GetAsInt(3*cc+2);
        selectionValues[entid].insert(selid);
        }
      opPort->setSelectionInput(NULL, 0);
      }
    }

  currSelItem->updateInputSelection(selectionValues);

  // no need to start the operation if there is no selection at all.
  if(MeshSelectionItem->numberOfValues() != 0)
    this->modelView()->requestOperation(
      this->Internal->SelectionOperations[currSelItem].first,
      this->Internal->SelectionOperations[currSelItem].second, true);
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::updateMeshSelection(
  const smtk::attribute::MeshSelectionItemPtr& meshSelectionItem,
  pqSMTKModelInfo* minfo)
{
  if(!minfo)
    return;

  smtk::extension::qtMeshSelectionItem* currSelItem =
    this->Internal->CurrentMeshSelectItem;
  if(!currSelItem)
    return;

  std::map<smtk::common::UUID, std::set<int> > outSelectionValues;
  currSelItem->syncWithCachedSelection(meshSelectionItem,
                                       outSelectionValues);

  pqPipelineSource* modelSrc = minfo->RepSource;
  vtkSMSourceProxy* smModelSource = vtkSMSourceProxy::SafeDownCast(
  modelSrc->getProxy());

  vtkSMProxy* selectionSource = minfo->CompositeDataIdSelectionSource;
  smtk::model::ManagerPtr mgr =
    this->Internal->smtkManager->managerProxy()->modelManager();

  unsigned int flatIndex;
  vtkIdType selCompIdx;
  std::vector<vtkIdType> ids;
  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for(mapIt = outSelectionValues.begin(); mapIt != outSelectionValues.end(); ++mapIt)
    {
    //std::cout << "UUID: " << (*it).toString().c_str() << std::endl;
    if(mgr->hasIntegerProperty(mapIt->first, "block_index"))
      {
      smtk::model::EntityRef entRef(mgr, mapIt->first);
      const smtk::model::IntegerList& prop(entRef.integerProperty("block_index"));
      //the flatIndex is 1 more than blockId, because the root is index 0
      if(!prop.empty())
        {
        flatIndex = prop[0];
        selCompIdx = static_cast<vtkIdType>(flatIndex+1);
        std::set<int>::const_iterator it;
        for(it = mapIt->second.begin(); it != mapIt->second.end(); ++it)
          {
          ids.push_back(selCompIdx); // composite_index
          ids.push_back(0); // process_id
          ids.push_back(*it); // cell_id in block
          }
        }
      }
    }


  vtkSMPropertyHelper newSelIDs(selectionSource, "IDs");
  newSelIDs.Set(&ids[0], static_cast<unsigned int>(ids.size()));
  selectionSource->UpdateVTKObjects();

  smModelSource->SetSelectionInput(0,
    vtkSMSourceProxy::SafeDownCast(selectionSource), 0);
  smModelSource->UpdatePipeline();

/*
  pqSelectionManager *selectionManager =
    qobject_cast<pqSelectionManager*>(
      pqApplicationCore::instance()->manager("SelectionManager"));

  if(outport && selectionManager)
    {
    outport->setSelectionInput(selectionSourceProxy, 0);
//    this->requestRender();
    this->updateSMTKSelection();
    selectionManager->blockSignals(true);
    pqPVApplicationCore::instance()->selectionManager()->select(outport);
    selectionManager->blockSignals(false);
//    pqActiveObjects::instance().setActivePort(outport);
    }
*/
  pqRenderView* renView = qobject_cast<pqRenderView*>(
     pqActiveObjects::instance().activeView());
  renView->render();
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::resetMeshSelectionItems()
{
  foreach(smtk::extension::qtMeshSelectionItem* meshItem,
    this->Internal->SelectionOperations.keys())
    if(meshItem)
      meshItem->resetSelectionState();
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::cancelOperation(const smtk::model::OperatorPtr& op)
{
  smtk::extension::qtMeshSelectionItem* currSelItem =
    this->Internal->CurrentMeshSelectItem;
  if(!op || !currSelItem ||
     !this->Internal->SelectionOperations.contains(currSelItem))
    {
    return;
    }
  // if currentMeshSelectItem belongs to the cancelled op, reset the item ui
  if(this->Internal->SelectionOperations[currSelItem].first == op->name())
    {
    currSelItem->resetSelectionState(true);
    }
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::gatherSelectionInfo(pqPipelineSource* source,
                                           vtkPVInformation* pvInfo,
                                           smtk::common::UUIDs& uuids,
                                           smtk::mesh::MeshSets& meshes)
{
  vtkPVSMTKModelInformation* modinfo = vtkPVSMTKModelInformation::SafeDownCast(pvInfo);
  vtkPVSMTKMeshInformation* meshinfo = vtkPVSMTKMeshInformation::SafeDownCast(pvInfo);
  if(!modinfo && !meshinfo)
    {
    return;
    }

  vtkNew<vtkPVSelectionInformation> selInfo;
  if(vtkSelectionNode* selnode =
    pqCMBSelectionHelperUtil::gatherSelectionNode(source, selInfo.GetPointer()))
    {
    unsigned int flat_idx;
    vtkUnsignedIntArray* blockIds = selnode->GetSelectionList() ?
      vtkUnsignedIntArray::SafeDownCast(selnode->GetSelectionList()) : NULL;
    if(blockIds)
      {
      for(vtkIdType ui=0;ui<blockIds->GetNumberOfTuples();ui++)
        {
        flat_idx = blockIds->GetValue(ui);
        // blockId is child index, which is one less of flat_index
        flat_idx--;
        if(modinfo)
          {
          uuids.insert(modinfo->GetModelEntityId(flat_idx));
          }
        else if(meshinfo)
          {
          meshes.insert(meshinfo->GetMeshSet(flat_idx));
          }
        }
      }
    // this may be the a ID selection
    else
      {
      vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
        source->getProxy())->GetSelectionInput(0);
      vtkSMPropertyHelper selIDs(selSource, "IDs");
      unsigned int count = selIDs.GetNumberOfElements();
      // [composite_index, process_id, index]
      for (unsigned int cc=0; cc < (count/3); cc++)
        {
        flat_idx = selIDs.GetAsInt(3*cc);
        // blockId is child index, which is one less of flat_index
        flat_idx--;
        if(modinfo)
          {
          uuids.insert(modinfo->GetModelEntityId(flat_idx));
          }
        else if(meshinfo)
          {
          meshes.insert(meshinfo->GetMeshSet(flat_idx));
          }
        }
      }
    }
}
