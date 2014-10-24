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
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
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
#include <pqSMAdaptor.h>
#include <pqPipelineSource.h>
#include <pqOutputPort.h>
#include <pqApplicationCore.h>
#include <pqSelectionManager.h>

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

#include "ModelManager.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"

using namespace std;
using namespace smtk::model;

//-----------------------------------------------------------------------------
class qtSMTKModelPanel::qInternal
{
public:
  QPointer<qtModelPanel> ModelPanel;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  bool ModelLoaded;
  QPointer<ModelManager> smtkManager;

  qInternal()
    {
    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    this->ModelLoaded = false;
    }
};

qtSMTKModelPanel::qtSMTKModelPanel(ModelManager* mmgr, QWidget* p)
: QDockWidget(p)
{
  this->Internal = new qtSMTKModelPanel::qInternal();
  this->setObjectName("smtkModelDockWidget");
  this->Internal->smtkManager = mmgr;
  this->Internal->VTKConnect->Connect(
    this->Internal->smtkManager->managerProxy(), vtkCommand::UpdateDataEvent,
    this, SLOT(onDataUpdated()));

  this->onDataUpdated();
  //QSizePolicy expandPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //this->setSizePolicy(expandPolicy);

}

qtSMTKModelPanel::~qtSMTKModelPanel()
{
  delete this->Internal;
}

void qtSMTKModelPanel::clearUI()
{
  if(this->Internal->ModelPanel)
    {
    delete this->Internal->ModelPanel;
    this->Internal->ModelPanel = NULL;
    }
  this->Internal->ModelLoaded = false;
}

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
  smtk::model::BitFlags mask = smtk::model::MODEL_ENTITY;

  smtk::model::ManagerPtr model = this->Internal->smtkManager->managerProxy()->modelManager();
//  smtk::io::ImportJSON::intoModel(json.c_str(), model);
//  model->assignDefaultNames();

//  QFileInfo fInfo(this->Internal->smtkManager->currentFile().c_str());
//  this->setWindowTitle(fInfo.fileName().toAscii().constData());
  if(!this->Internal->ModelPanel)
    {
    this->Internal->ModelPanel = new qtModelPanel(this);
    this->setWidget(this->Internal->ModelPanel);
    }
  qtModelView* modelview = this->Internal->ModelPanel->getModelView();
  QPointer<smtk::model::QEntityItemModel> qmodel = modelview->getModel();
  qmodel->clear();

  QObject::connect(modelview, SIGNAL(entitiesSelected(const smtk::common::UUIDs& )),
      this, SLOT(selectEntities(const smtk::common::UUIDs& )));

  smtk::model::Cursors cursors;
  smtk::model::Cursor::CursorsFromUUIDs(
    cursors, model, model->entitiesMatchingFlags(mask, false));
  std::cout << std::setbase(10) << "Found " << cursors.size() << " entries\n";
  qmodel->setRoot(
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate( // set the subphrase generator:
        smtk::model::SimpleModelSubphrases::create()));

  this->Internal->ModelLoaded = true;
}

void qtSMTKModelPanel::selectEntities(const smtk::common::UUIDs& ids)
{
  // create vector of selected block ids
  QMap<cmbSMTKModelInfo*, std::vector<vtkIdType> > selmodelblocks;
  for(smtk::common::UUIDs::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
    unsigned int flatIndex;
    cmbSMTKModelInfo* minfo = this->Internal->smtkManager->modelInfo(*it);
    //std::cout << "UUID: " << (*it).toString().c_str() << std::endl;

    if(minfo && minfo->Info->GetBlockId((*it).toString(), flatIndex))
      {
      //the flatIndex is 1 more than blockId, because the root is index 0
      selmodelblocks[minfo].push_back(static_cast<vtkIdType>(flatIndex+1));
      }
    }

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
    pqPipelineSource* source = modinfo->Source;
    pqOutputPort* outport = source->getOutputPort(0);
    if(outport)
      {
      outport->setSelectionInput(selectionSourceProxy, 0);
      }

    // update the selection manager
    pqSelectionManager *selectionManager =
      qobject_cast<pqSelectionManager*>(
        pqApplicationCore::instance()->manager("SelectionManager"));
    if(selectionManager && outport)
      {
      selectionManager->select(outport);
      outport->renderAllViews();
      pqActiveObjects::instance().setActiveSource(source);
      }    
    }
}

void qtSMTKModelPanel::updateTreeSelection()
{
  QList<std::string> uuids;
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
      std::string entId;
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

          entId = modinfo->Info->GetModelEntityId(flat_idx);
          if(!uuids.contains(entId))
            {
            uuids.append(entId);
            }
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
          entId = modinfo->Info->GetModelEntityId(flat_idx);
          if(!uuids.contains(entId))
            {
            uuids.append(entId);
            }
          }
        }
      }
    }
  qtModelView* modelview = this->Internal->ModelPanel->getModelView();
  modelview->selectEntities(uuids);
}
