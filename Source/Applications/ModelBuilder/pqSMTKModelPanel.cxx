/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: qtSMTKModelPanel.cxx,v $

 =========================================================================*/
#include "qtSMTKModelPanel.h"

#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtModelPanel.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/SimpleModelSubphrases.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeView>
#include <QtGui/QDockWidget>

#include <iomanip>
#include <iostream>
#include <fstream>

#include <stdlib.h>

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
#include "vtkSmartPointer.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkPVSelectionInformation.h"
#include "vtkUnsignedIntArray.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtksys/SystemTools.hxx"

#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"
#include "SimBuilder/cmbSMTKUIHelper.h"
#include "vtkSMDoubleMapProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMDoubleMapPropertyIterator.h"
#include "pqCMBModelManager.h"

using namespace std;
using namespace smtk::model;

//-----------------------------------------------------------------------------
class qtSMTKModelPanel::qInternal
{
public:
  QPointer<qtModelPanel> ModelPanel;
//  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  bool ModelLoaded;
  QPointer<pqCMBModelManager> smtkManager;
  bool ignorePropertyChange;

  qInternal()
    {
//    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    this->ModelLoaded = false;
    }
};

//-----------------------------------------------------------------------------
qtSMTKModelPanel::qtSMTKModelPanel(pqCMBModelManager* mmgr, QWidget* p)
: QDockWidget(p)
{
  this->Internal = new qtSMTKModelPanel::qInternal();
  this->setObjectName("smtkModelDockWidget");
  this->Internal->smtkManager = mmgr;
//  this->connect(&this->Internal->UpdateUITimer, SIGNAL(timeout()),
//                this, SLOT(updateModelTreeView()));

  this->onDataUpdated();
  //QSizePolicy expandPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //this->setSizePolicy(expandPolicy);

}

//-----------------------------------------------------------------------------
qtSMTKModelPanel::~qtSMTKModelPanel()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
pqCMBModelManager* qtSMTKModelPanel::modelManager()
{
  return this->Internal->smtkManager;
}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::clearUI()
{
  if(this->Internal->ModelPanel)
    {
    delete this->Internal->ModelPanel;
    this->Internal->ModelPanel = NULL;
    }
  this->Internal->ModelLoaded = false;
}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::setBlockVisibility(pqDataRepresentation* rep,
    const QList<unsigned int>& blockids, bool visible)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::BridgePtr br = modInfo->Bridge;
  if(!br)
    return;

  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockVisibilites;
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
void qtSMTKModelPanel::showOnlyBlocks(pqDataRepresentation* rep,
    const QList<unsigned int>& indices)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::BridgePtr br = modInfo->Bridge;
  if(!br)
    return;

  QList<unsigned int> tmpList(indices);
//  if(!tmpList.count())
//    rep->setVisible(false);

  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockVisibilites;
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
void qtSMTKModelPanel::showAllBlocks(pqDataRepresentation* rep)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::BridgePtr br = modInfo->Bridge;
  if(!br)
    return;

  rep->setVisible(true);
  smtk::model::ManagerPtr modelMan =
    this->Internal->smtkManager->managerProxy()->modelManager();

  // we want to show the whole model
  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockVisibilites;

  smtk::model::ModelEntity modelEntity(modelMan,
    modInfo->Info->GetModelUUID());

  BlockVisibilites[br].insert(modelEntity.entity());

  smtk::model::CellEntities cellents = modelEntity.cells();
  for (smtk::model::CellEntities::const_iterator it = cellents.begin();
    it != cellents.end(); ++it)
    {
    BlockVisibilites[br].insert((*it).entity());
    }

  smtk::model::GroupEntities groups = modelEntity.groups();
  for (smtk::model::GroupEntities::iterator git = groups.begin();
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
void qtSMTKModelPanel::setBlockColor(pqDataRepresentation* rep,
    const QList<unsigned int>& blockids, const QColor& color)
{
  if(!rep)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(rep);
  if(!modInfo)
    return;
  smtk::model::BridgePtr br = modInfo->Bridge;
  if(!br)
    return;

  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockColors;
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
void qtSMTKModelPanel::onDataUpdated()
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
  smtk::model::BitFlags mask = smtk::model::BRIDGE_SESSION;

  smtk::model::ManagerPtr model = this->Internal->smtkManager->managerProxy()->modelManager();
//  smtk::io::ImportJSON::intoModel(json.c_str(), model);
//  model->assignDefaultNames();

//  QFileInfo fInfo(this->Internal->smtkManager->currentFile().c_str());
//  this->setWindowTitle(fInfo.fileName().toAscii().constData());
  if(!this->Internal->ModelPanel)
    {
    this->Internal->ModelPanel = new qtModelPanel(this);
    this->setWidget(this->Internal->ModelPanel);
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(entitiesSelected(const smtk::model::Cursors& )),
      this, SLOT(selectEntities(const smtk::model::Cursors& )));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
      this, SLOT(onFileItemCreated(smtk::attribute::qtFileItem*)));
    QObject::connect(this->Internal->ModelPanel->getModelView(),
      SIGNAL(operationRequested(const smtk::model::OperatorPtr& )),
      this->Internal->smtkManager,
      SLOT(startOperation( const smtk::model::OperatorPtr& )));

    }

//  this->linkRepresentations();

  qtModelView* modelview = this->Internal->ModelPanel->getModelView();
  QPointer<smtk::model::QEntityItemModel> qmodel = modelview->getModel();
  qmodel->clear();

  smtk::model::Cursors cursors;
  smtk::model::Cursor::CursorsFromUUIDs(
    cursors, model, model->entitiesMatchingFlags(mask, true));
  std::cout << std::setbase(10) << "Found " << cursors.size() << " entries\n";
  qmodel->setRoot(
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate( // set the subphrase generator:
        smtk::model::SimpleModelSubphrases::create()));

  this->Internal->ModelLoaded = true;
}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::selectEntities(const smtk::model::Cursors& entities)
{
  //clear current selections
  this->Internal->smtkManager->clearModelSelections();
  smtk::model::ManagerPtr modelMan =
    this->Internal->smtkManager->managerProxy()->modelManager();

std::cout << "client before converting to blockid" << std::endl;
  // create vector of selected block ids
  QMap<cmbSMTKModelInfo*, std::vector<vtkIdType> > selmodelblocks;
  for(smtk::model::Cursors::const_iterator it = entities.begin(); it != entities.end(); ++it)
    {
    unsigned int flatIndex;
    //std::cout << "UUID: " << (*it).toString().c_str() << std::endl;
    if(modelMan->hasIntegerProperty((*it).entity(), "block_index"))
    //if((*it).hasIntegerProperty("block_index"))
      {
      if (it->as<CellEntity>().isValid())
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
    }
std::cout << "After convert to blockid, before selecting" << std::endl;
  // update the selection manager
  pqSelectionManager *selectionManager =
    qobject_cast<pqSelectionManager*>(
      pqApplicationCore::instance()->manager("SelectionManager"));
  pqOutputPort* outport = NULL;
  pqPipelineSource* source = NULL;
  foreach(cmbSMTKModelInfo* modinfo, selmodelblocks.keys())
    {
    vtkSMProxy* selectionSource = modinfo->SelectionSource;
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

std::cout << "done selecting" << std::endl;

}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::updateTreeSelection()
{

  std::cout << "update tree selection, before converting blockid to uuid" << std::endl;
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
  std::cout << "update tree selection, after converting blockid to uuid" << std::endl;

  qtModelView* modelview = this->Internal->ModelPanel->getModelView();
  modelview->blockSignals(true);
  modelview->clearSelection();
  modelview->blockSignals(false);
  modelview->selectEntities(uuids);
  std::cout << "update tree selection, done" << std::endl;
}

//----------------------------------------------------------------------------
void qtSMTKModelPanel::onFileItemCreated(smtk::attribute::qtFileItem* fileItem)
{
  if(fileItem)
    {
    QObject::connect(fileItem, SIGNAL(launchFileBrowser()),
      this, SLOT(onLaunchFileBrowser()));
    }
}

//----------------------------------------------------------------------------
void qtSMTKModelPanel::onLaunchFileBrowser()
{
  smtk::attribute::qtFileItem* const fileItem =
    qobject_cast<smtk::attribute::qtFileItem*>(QObject::sender());
  if(!fileItem)
    {
    return;
    }
  cmbSMTKUIHelper::process_smtkFileItemRequest(
    fileItem, this->Internal->smtkManager->server(), fileItem->widget());
}
/*
//----------------------------------------------------------------------------
void qtSMTKModelPanel::linkRepresentations()
{
  // disconnect from previous representations
  this->Internal->VTKConnect->Disconnect();
  foreach(pqDataRepresentation* repr,
          this->Internal->smtkManager->modelRepresentations())
    {
    this->linkRepresentation(repr);
    }
}

//----------------------------------------------------------------------------
void qtSMTKModelPanel::linkRepresentation(pqDataRepresentation *representation)
{
  if(representation)
    {
    // listen to property changes
    vtkSMProxy *proxy = representation->getProxy();
    vtkSMProperty *visibilityProperty = proxy->GetProperty("BlockVisibility");
    if(visibilityProperty)
      {
      this->Internal->VTKConnect->Connect(visibilityProperty,
        vtkCommand::ModifiedEvent,
        this, SLOT(propertyChanged(vtkObject*,  unsigned long, void*)), representation);
      }

    vtkSMProperty *colorProperty = proxy->GetProperty("BlockColor");
    if(colorProperty)
      {
      this->Internal->VTKConnect->Connect(colorProperty,
        vtkCommand::ModifiedEvent,
        this, SLOT(propertyChanged(vtkObject*,  unsigned long, void*)), representation);
      }
    }
}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::propertyChanged(
  vtkObject* caller, unsigned long, void* clientdata)
{
  if(this->Internal->ignorePropertyChange)
    return;
  vtkSMProperty* prop = vtkSMProperty::SafeDownCast(caller);
  pqDataRepresentation* representation = reinterpret_cast<pqDataRepresentation*>(clientdata);
  if(!prop || !representation)
    return;
  if(strcmp(prop->GetXMLName(), "BlockVisibility") == 0)
    {
    this->updateEntityVisibility(
      vtkSMIntVectorProperty::SafeDownCast(prop), representation);
    }
  else if(strcmp(prop->GetXMLName(), "BlockColor") == 0)
    {
    this->updateEntityColor(
      vtkSMDoubleMapProperty::SafeDownCast(prop), representation);
    }
}


//-----------------------------------------------------------------------------
void qtSMTKModelPanel::updateEntityVisibility(vtkSMIntVectorProperty* ivp,
  pqDataRepresentation* representation, )
{
  if(!ivp)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(representation);
  if(!modInfo)
    return;

  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockVisibilites;
  vtkIdType nbElems = static_cast<vtkIdType>(ivp->GetNumberOfElements());
  int vis = 1;
  for(vtkIdType i = 0; i + 1 < nbElems; i += 2)
    {
    smtk::common::UUID uid = modInfo->Info->GetModelEntityId(ivp->GetElement(i)-1);
    smtk::model::BridgePtr br = this->Internal->smtkManager->modelBridge(uid);
    if(br)
      {
      BlockVisibilites[br].insert(uid);
      vis = ivp->GetElement(i+1); // for now, we use same value for all
      }
    }
  // the pqCMBModelManager shouldn't need to emit signal while syncing
  this->Internal->smtkManager->blockSignals(true);
  this->Internal->ModelPanel->getModelView()->syncEntityVisibility(
    BlockVisibilites, vis);
  this->Internal->smtkManager->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtSMTKModelPanel::updateEntityColor(vtkSMDoubleMapProperty* dmp,
  pqDataRepresentation* representation)
{
  if(!dmp)
    return;
  cmbSMTKModelInfo* modInfo =
    this->Internal->smtkManager->modelInfo(representation);
  if(!modInfo)
    return;

  QMap<smtk::model::BridgePtr, smtk::common::UUIDs> BlockColors;
  vtkSMDoubleMapPropertyIterator *iter = dmp->NewIterator();
  QColor newcolor;
  for(iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
    smtk::common::UUID uid = modInfo->Info->GetModelEntityId(iter->GetKey());
    smtk::model::BridgePtr br = this->Internal->smtkManager->modelBridge(uid);
    if(br)
      {
      BlockColors[br].insert(uid);
      // for now, we use same value for all
      newcolor.setRedF(iter->GetElementComponent(0));
      newcolor.setGreenF(iter->GetElementComponent(1));
      newcolor.setBlueF(iter->GetElementComponent(2));
      }
    }
  iter->Delete();

  // the pqCMBModelManager shouldn't need to emit signal while syncing
  this->Internal->smtkManager->blockSignals(true);
  this->Internal->ModelPanel->getModelView()->syncEntityColor(
    BlockColors, newcolor);
  this->Internal->smtkManager->blockSignals(false);
}
*/
