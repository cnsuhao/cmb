/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: pqSMTKModelPanel,v $

 =========================================================================*/
#include "pqSMTKModelPanel.h"

#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
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
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

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
#include "vtksys/SystemTools.hxx"

#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"
#include "SimBuilder/pqSMTKUIHelper.h"
#include "vtkSMDoubleMapProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMDoubleMapPropertyIterator.h"
#include "pqCMBModelManager.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <stdlib.h>

using namespace std;
using namespace smtk::model;

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
  QMap<smtk::attribute::qtMeshSelectionItem*,
    QPair<std::string, smtk::common::UUID> >SelectionOperations;
  QPointer<smtk::attribute::qtMeshSelectionItem> CurrentMeshSelectItem;

  qInternal()
    {
//    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    this->ModelLoaded = false;
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

smtk::model::qtModelView* pqSMTKModelPanel::modelView()
{
  return this->Internal->ModelPanel ?
    this->Internal->ModelPanel->getModelView() : NULL;
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::clearUI()
{
  if(this->Internal->ModelPanel)
    {
    delete this->Internal->ModelPanel;
    this->Internal->ModelPanel = NULL;
    }
  this->Internal->ModelLoaded = false;
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::setBlockVisibility(pqDataRepresentation* rep,
    const QList<unsigned int>& blockids, bool visible)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::SessionPtr br = modInfo->Session;
  if(!br)
    return;

  QMap<smtk::model::SessionPtr, smtk::common::UUIDs> BlockVisibilites;
  int vis = visible ? 1 : 0;
  foreach(unsigned int idx, blockids)
    {
    smtk::common::UUID uid = modInfo->Info->GetModelEntityId(idx-1);
    BlockVisibilites[br].insert(uid);
    }
  this->Internal->ModelPanel->getModelView()->syncEntityVisibility(
    BlockVisibilites, vis);
  this->Internal->ModelPanel->update();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::showOnlyBlocks(pqDataRepresentation* rep,
    const QList<unsigned int>& indices)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::SessionPtr br = modInfo->Session;
  if(!br)
    return;

  QList<unsigned int> tmpList(indices);
//  if(!tmpList.count())
//    rep->setVisible(false);

  QMap<smtk::model::SessionPtr, smtk::common::UUIDs> BlockVisibilites;
  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    modInfo->Info->GetUUID2BlockIdMap().begin();
  for(; it != modInfo->Info->GetUUID2BlockIdMap().end(); ++it)
    {
    if(tmpList.count() && tmpList.contains(it->second+1))
      {
      tmpList.removeAll(it->second+1);
      continue;
      }
    BlockVisibilites[br].insert(it->first);
    }
  this->Internal->ModelPanel->getModelView()->syncEntityVisibility(
    BlockVisibilites, 0);
  this->Internal->ModelPanel->update();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::showAllBlocks(pqDataRepresentation* rep)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::SessionPtr br = modInfo->Session;
  if(!br)
    return;

  rep->setVisible(true);
  smtk::model::ManagerPtr modelMan =
    this->Internal->smtkManager->managerProxy()->modelManager();

  // we want to show the whole model
  QMap<smtk::model::SessionPtr, smtk::common::UUIDs> BlockVisibilites;

  smtk::model::Model modelEntity(modelMan,
    modInfo->Info->GetModelUUID());

  BlockVisibilites[br].insert(modelEntity.entity());

  smtk::model::CellEntities cellents = modelEntity.cells();
  for (smtk::model::CellEntities::const_iterator it = cellents.begin();
    it != cellents.end(); ++it)
    {
    BlockVisibilites[br].insert((*it).entity());
    }

  smtk::model::Groups groups = modelEntity.groups();
  for (smtk::model::Groups::iterator git = groups.begin();
    git != groups.end(); ++git)
    {
    BlockVisibilites[br].insert((*git).entity());
    }
 
  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    modInfo->Info->GetUUID2BlockIdMap().begin();
  for(; it != modInfo->Info->GetUUID2BlockIdMap().end(); ++it)
    {
    BlockVisibilites[br].insert(it->first);
    }

  this->Internal->ModelPanel->getModelView()->syncEntityVisibility(
    BlockVisibilites, 1);

//  this->Internal->ModelPanel->getModelView()->syncModelVisibility(
//    modInfo->Info->GetModelUUID(), 1);
  this->Internal->ModelPanel->update();
}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::setBlockColor(pqDataRepresentation* rep,
    const QList<unsigned int>& blockids, const QColor& color)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::SessionPtr br = modInfo->Session;
  if(!br)
    return;

  QMap<smtk::model::SessionPtr, smtk::common::UUIDs> BlockColors;
  foreach(unsigned int idx, blockids)
    {
    smtk::common::UUID uid = modInfo->Info->GetModelEntityId(idx-1);
    BlockColors[br].insert(uid);
    }

  this->Internal->ModelPanel->getModelView()->syncEntityColor(
    BlockColors, color);
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
  smtk::model::BitFlags mask = smtk::model::SESSION;

  smtk::model::ManagerPtr model = this->Internal->smtkManager->managerProxy()->modelManager();
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
      SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
      this, SLOT(onFileItemCreated(smtk::attribute::qtFileItem*)));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(modelEntityItemCreated(smtk::attribute::qtModelEntityItem*)),
      this, SLOT(onModelEntityItemCreated(smtk::attribute::qtModelEntityItem*)));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(operationRequested(const smtk::model::OperatorPtr& )),
      this->Internal->smtkManager,
      SLOT(startOperation( const smtk::model::OperatorPtr& )));
    QObject::connect(this->Internal->smtkManager,
      SIGNAL(requestMeshSelectionUpdate(
        const smtk::attribute::MeshSelectionItemPtr&, cmbSMTKModelInfo*)),
      this,
      SLOT(updateMeshSelection(
           const smtk::attribute::MeshSelectionItemPtr&, cmbSMTKModelInfo*)));

    }

//  this->linkRepresentations();

  qtModelView* modelview = this->Internal->ModelPanel->getModelView();
  QPointer<smtk::model::QEntityItemModel> qmodel = modelview->getModel();
  qmodel->clear();

  smtk::model::EntityRefs cursors;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    cursors, model, model->entitiesMatchingFlags(mask, true));
  std::cout << std::setbase(10) << "Found " << cursors.size() << " entries\n";

  smtk::model::SimpleModelSubphrases::Ptr spg =
    smtk::model::SimpleModelSubphrases::create();
  spg->setDirectLimit(-1);
  qmodel->setRoot(
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate( spg));// set the subphrase generator

  this->Internal->ModelLoaded = true;
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
  QMap<cmbSMTKModelInfo*, std::vector<vtkIdType> > selmodelblocks;
  for(smtk::model::EntityRefs::const_iterator it = entities.begin(); it != entities.end(); ++it)
    {
    unsigned int flatIndex;
    //std::cout << "UUID: " << (*it).toString().c_str() << std::endl;
    if(modelMan->hasIntegerProperty((*it).entity(), "block_index"))
      {
      cmbSMTKModelInfo* minfo = this->Internal->smtkManager->modelInfo(
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

  // update the selection manager
  pqSelectionManager *selectionManager =
    qobject_cast<pqSelectionManager*>(
      pqApplicationCore::instance()->manager("SelectionManager"));
  pqOutputPort* outport = NULL;
  pqPipelineSource* source = NULL;
  foreach(cmbSMTKModelInfo* modinfo, selmodelblocks.keys())
    {
    vtkSMProxy* selectionSource = modinfo->BlockSelectionSource;
    vtkSMPropertyHelper prop(selectionSource, "Blocks");
    prop.SetNumberOfElements(0);
    selectionSource->UpdateVTKObjects();
    // set selected blocks
    if (selmodelblocks[modinfo].size() > 0)
      {
      prop.Set(&selmodelblocks[modinfo][0], static_cast<unsigned int>(
        selmodelblocks[modinfo].size()));
      }
    selectionSource->UpdateVTKObjects();

    vtkSMSourceProxy *selectionSourceProxy =
      vtkSMSourceProxy::SafeDownCast(selectionSource);
    source = modinfo->Source;
    outport = source->getOutputPort(0);
    if(outport)
      {
      outport->setSelectionInput(selectionSourceProxy, 0);
      }
    }
  // use last selected port as active
  if(selectionManager && outport)
    {
    selectionManager->blockSignals(true);
    selectionManager->select(outport);
    selectionManager->blockSignals(false);
//    pqActiveObjects::instance().setActivePort(outport);
//    std::cout << "set active source: " << source << std::endl;
    }
  else
    {
    pqRenderView* renView = qobject_cast<pqRenderView*>(
       pqActiveObjects::instance().activeView());
    renView->render();
    }

}

//-----------------------------------------------------------------------------
void pqSMTKModelPanel::updateTreeSelection()
{

  smtk::common::UUIDs uuids;
  QList<cmbSMTKModelInfo*> selModels = this->Internal->smtkManager->selectedModels();
  foreach(cmbSMTKModelInfo* modinfo, selModels)
    {
    pqPipelineSource *source = modinfo->Source;
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
    vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
    selSource->UpdatePipeline();
    vtkNew<vtkPVSelectionInformation> selInfo;
    selSource->GatherInformation(selInfo.GetPointer());
    if(selInfo->GetSelection() &&
      selInfo->GetSelection()->GetNumberOfNodes())
      {
//      std::string entId;
      unsigned int flat_idx;
      vtkUnsignedIntArray* blockIds = vtkUnsignedIntArray::SafeDownCast(
        selInfo->GetSelection()->GetNode(0)->GetSelectionList());
      if(blockIds)
        {
        for(vtkIdType ui=0;ui<blockIds->GetNumberOfTuples();ui++)
          {
          flat_idx = blockIds->GetValue(ui);
          // blockId is child index, which is one less of flat_index
          flat_idx--;

          uuids.insert(modinfo->Info->GetModelEntityId(flat_idx));
          }
        }
      // this may be the a ID selection
      else
        {
        vtkSMPropertyHelper selIDs(selSource, "IDs");
        unsigned int count = selIDs.GetNumberOfElements();
        // [composite_index, process_id, index]
        for (unsigned int cc=0; cc < (count/3); cc++)
          {
          flat_idx = selIDs.GetAsInt(3*cc);
          // blockId is child index, which is one less of flat_index
          flat_idx--;
          //entId = modinfo->Info->GetModelEntityId(flat_idx);
          uuids.insert(modinfo->Info->GetModelEntityId(flat_idx));
          }
        }
      }
    }

  this->modelView()->selectEntityItems(uuids, true); // block selection signal
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onFileItemCreated(smtk::attribute::qtFileItem* fileItem)
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
  smtk::attribute::qtFileItem* const fileItem =
    qobject_cast<smtk::attribute::qtFileItem*>(QObject::sender());
  if(!fileItem)
    {
    return;
    }
  pqSMTKUIHelper::process_smtkFileItemRequest(
    fileItem, this->Internal->smtkManager->server(), fileItem->widget());
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::onModelEntityItemCreated(
  smtk::attribute::qtModelEntityItem* entItem)
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
  smtk::attribute::qtModelEntityItem* entItem)
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
  smtk::attribute::qtModelEntityItem* const entItem =
    qobject_cast<smtk::attribute::qtModelEntityItem*>(QObject::sender());
  this->requestEntityAssociation(entItem);
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::requestEntitySelection(const smtk::common::UUIDs& uuids)
{
  this->modelView()->selectEntityItems(uuids, false); // not block selection signal
}

//----------------------------------------------------------------------------
void pqSMTKModelPanel::addMeshSelectionOperation(
    smtk::attribute::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid)
{
  if(meshItem)
    this->Internal->SelectionOperations[meshItem] = qMakePair(opName, uuid);
}
//----------------------------------------------------------------------------
void pqSMTKModelPanel::setCurrentMeshSelectionItem(
    smtk::attribute::qtMeshSelectionItem* meshItem)
{
  this->Internal->CurrentMeshSelectItem = meshItem;
}
//----------------------------------------------------------------------------
void pqSMTKModelPanel::startMeshSelectionOperation(
  const QList<pqOutputPort*> & selPorts)
{
  smtk::attribute::qtMeshSelectionItem* currSelItem =
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
    cmbSMTKModelInfo* modInfo = this->modelManager()->modelInfo(rep);
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
  cmbSMTKModelInfo* minfo)
{
  if(!minfo)
    return;

  smtk::attribute::qtMeshSelectionItem* currSelItem =
    this->Internal->CurrentMeshSelectItem;
  if(!currSelItem)
    return;

  std::map<smtk::common::UUID, std::set<int> > outSelectionValues;
  currSelItem->syncWithCachedSelection(meshSelectionItem,
                                       outSelectionValues);

  pqPipelineSource* modelSrc = minfo->Source;
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
  foreach(smtk::attribute::qtMeshSelectionItem* meshItem,
    this->Internal->SelectionOperations.keys())
    if(meshItem)
      meshItem->resetSelectionState();
}
