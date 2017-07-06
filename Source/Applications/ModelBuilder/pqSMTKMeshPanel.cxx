//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqSMTKMeshPanel.h"

#include "SimBuilder/pqSMTKUIHelper.h"
#include "pqSMTKModelPanel.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/System.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/LoadJSON.h"
#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/common/View.h"
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtCollapsibleGroupWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/AutoInit.h" // for smtkComponentInitMacro

#include <QDockWidget>

#include <QApplication>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QTextEdit>

#include "vtkPVSMTKModelInformation.h"
#include "vtkSMModelManagerProxy.h"

#include "pqCMBModelManager.h"
#include "qtCMBMeshingMonitor.h"
#include "qtRemusMesherSelector.h"

using namespace std;
using namespace smtk::model;

pqSMTKMeshPanel::pqSMTKMeshPanel(QPointer<pqCMBModelManager> modelManager,
  QPointer<qtCMBMeshingMonitor> monitor, QWidget* p, pqSMTKModelPanel* mp)
  : QDockWidget(p)
  , ModelManager(modelManager)
  , MeshMonitor(monitor)
  , MeshSelector(new qtRemusMesherSelector(
      modelManager->managerProxy()->modelManager(), monitor->connection(), this))
  , RequirementsWidget(new QWidget(this))
  , SubmitterWidget(new QWidget(this))
  , AttSystem()
  , AttUIManager()
  , ActiveModel()
  , ActiveRequirements()
  , CachedAttributes()
{
  this->setObjectName("smtkMeshDockWidget");

  this->modelPanel = mp;

  //construct a widget that holds all the three sections
  //of the mesh panel. Since the meshWidget is parented to the panel
  //it will be properly deleted by Qt when this class is deleted
  QWidget* meshWidget = new QWidget(this);
  QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, meshWidget);
  meshWidget->setLayout(layout);

  //The RequirementsWidget needs a layout explicitly otherwise,
  //smtk will crash when it tries to add elements
  this->RequirementsWidget->setLayout(new QVBoxLayout());

  //construct a mesh button and add it to the SubmitterWidget
  this->SubmitterWidget->setLayout(new QVBoxLayout());
  this->MeshButton = new QPushButton(QString("Mesh"));
  this->MeshButton->setDefault(true);
  this->SubmitterWidget->layout()->addWidget(this->MeshButton);
  //by default the widget is not visible
  this->SubmitterWidget->setVisible(false);

  //mesh operator result log
  this->ResultLog = new QTextEdit(this);
  this->ResultLog->setReadOnly(true);
  this->ResultLog->setStyleSheet("font: \"Monaco\", \"Menlo\", \"Andale Mono\", \"fixed\";");
  this->ResultLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  smtk::extension::qtCollapsibleGroupWidget* gw =
    new smtk::extension::qtCollapsibleGroupWidget(this);
  gw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  gw->setName("Show Meshing Operator Log");
  gw->contentsLayout()->addWidget(this->ResultLog);
  gw->collapse();

  layout->addWidget(this->MeshSelector.data());
  layout->addWidget(this->RequirementsWidget.data());
  layout->addWidget(this->SubmitterWidget.data());
  layout->addWidget(gw);

  this->setWidget(meshWidget);

  QObject::connect(this->MeshSelector, SIGNAL(currentMesherChanged(const smtk::model::Model&,
                                         const QString&, const remus::proto::JobRequirements&)),
    this, SLOT(displayRequirements(
            const smtk::model::Model&, const QString&, const remus::proto::JobRequirements&)));

  QObject::connect(
    this, SIGNAL(visibilityChanged(bool)), this->MeshSelector, SLOT(rebuildModelList(bool)));

  QObject::connect(this->MeshSelector, SIGNAL(noMesherForModel()), this, SLOT(clearActiveMesh()));

  QObject::connect(
    this, SIGNAL(meshingPossible(bool)), this->SubmitterWidget, SLOT(setVisible(bool)));

  QObject::connect(this->MeshButton, SIGNAL(pressed()), this, SLOT(submitMeshJob()));

  if (this->ModelManager)
  {
    QObject::connect(this->ModelManager, SIGNAL(operationLog(const smtk::io::Logger&)), this,
      SLOT(displayMeshOpLog(const smtk::io::Logger&)));
  }
}

pqSMTKMeshPanel::~pqSMTKMeshPanel()
{
}

QPointer<pqCMBModelManager> pqSMTKMeshPanel::modelManager()
{
  return this->ModelManager;
}

void pqSMTKMeshPanel::updateModel(
  QPointer<pqCMBModelManager> mmgr, QPointer<qtCMBMeshingMonitor> monitor)
{
  //we have a new modelproxy and new meshing monitor
  this->MeshSelector->updateModel(mmgr->managerProxy()->modelManager(), monitor->connection());
  this->MeshMonitor = monitor;
}

void pqSMTKMeshPanel::displayRequirements(const smtk::model::Model& modelToDisplay,
  const QString& vtkNotUsed(workerName), const remus::proto::JobRequirements& reqs)
{

  if (modelToDisplay == this->ActiveModel && reqs == this->ActiveRequirements && this->AttSystem &&
    this->AttUIManager)
  {
    //We don't need to clear anything!, use the existing setup we build
    //before. This only works if the AttUIManager hasn't been cleared, which
    //can occur if that mesher type goes away from the avialable list
    //and than the UI moves away from the mesh tab
    emit this->meshingPossible(true);
    return;
  }

  //This check requires
  if (this->AttSystem)
  {
    //At this point we need to cache the current attributes the user
    //could be filling out the requirements for a given mesher and switch
    //to another mesh/model
    smtk::io::Logger inputLogger;
    smtk::io::AttributeWriter writer;
    std::string serializedAttributes;

    writer.writeContents(this->AttSystem, serializedAttributes, inputLogger);
    this->cacheAtts(serializedAttributes);
  }

  //each time a new mesher is selected this needs to rebuild the UI
  this->ActiveModel = modelToDisplay;
  this->ActiveRequirements = reqs;
  this->AttSystem = smtk::attribute::System::create();
  this->AttSystem->setRefModelManager(this->ModelManager->managerProxy()->modelManager());

  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;

  bool err = false;
  if (this->hasCachedAtts(reqs))
  {
    reader.readContents(this->AttSystem, this->fetchCachedAtts(), inputLogger);
  }
  else if (reqs.sourceType() == (remus::common::ContentSource::File))
  { //the requirements are the file name so pass that to the attribute reader
    const std::string p(reqs.requirements(), reqs.requirementsSize());
    err = reader.read(this->AttSystem, p, true, inputLogger);
  }
  else
  { //the requirements are in memory xml contents

    err = reader.readContents(
      this->AttSystem, reqs.requirements(), reqs.requirementsSize(), inputLogger);
  }

  smtk::common::ViewPtr root = this->AttSystem->findTopLevelView();
  const bool useInternalFileBrowser = true;
  this->AttUIManager.reset(new smtk::extension::qtUIManager(this->AttSystem));
  this->AttUIManager->setSMTKView(root, this->RequirementsWidget.data(), useInternalFileBrowser);
  // send signal from UIManager to selection manager
  QObject::connect(this->AttUIManager.get(),
    SIGNAL(sendSelectionsFromAttributePanelToSelectionManager(const smtk::model::EntityRefs&,
      const smtk::mesh::MeshSets&, const smtk::model::DescriptivePhrases&,
      const smtk::extension::SelectionModifier, const std::string&)),
    qtActiveObjects::instance().smtkSelectionManager().get(),
    SLOT(updateSelectedItems(const smtk::model::EntityRefs&, const smtk::mesh::MeshSets&,
      const smtk::model::DescriptivePhrases&, const smtk::extension::SelectionModifier,
      const std::string&)));
  // connect signal and slot for qtModelEntityItem
  QObject::connect(this->AttUIManager.get(),
    SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)), this,
    SLOT(onModelEntityItemCreated(smtk::extension::qtModelEntityItem*)));

  emit this->meshingPossible(true);
}

void pqSMTKMeshPanel::onModelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem)
{
  if (entItem)
  {
    // associated with selected entities
    QObject::connect(
      entItem, SIGNAL(requestEntityAssociation()), this, SLOT(onRequestEntityAssociation()));
    // highlight hoovering entity
    QObject::connect(entItem, SIGNAL(entityListHighlighted(const smtk::common::UUIDs&)), this,
      SLOT(onRequestEntitySelection(const smtk::common::UUIDs&)));
    // send selection to smtkSelectionManager
    QObject::connect(entItem,
      SIGNAL(sendSelectionFromModelEntityToSelectionManager(const smtk::model::EntityRefs&,
        const smtk::mesh::MeshSets&, const smtk::model::DescriptivePhrases&,
        const smtk::extension::SelectionModifier, const std::string)),
      qtActiveObjects::instance().smtkSelectionManager().get(),
      SLOT(updateSelectedItems(const smtk::model::EntityRefs&, const smtk::mesh::MeshSets&,
        const smtk::model::DescriptivePhrases&, const smtk::extension::SelectionModifier,
        const std::string)));
  }
}

void pqSMTKMeshPanel::onRequestEntityAssociation()
{
  smtk::extension::qtModelEntityItem* const entItem =
    qobject_cast<smtk::extension::qtModelEntityItem*>(QObject::sender());
  if (!entItem)
  {
    return;
  }
  if (this->modelPanel)
  {
    pqSMTKUIHelper::process_smtkModelEntityItemSelectionRequest(
      entItem, this->modelPanel->modelView());
  }
}

void pqSMTKMeshPanel::onRequestEntitySelection(const smtk::common::UUIDs& uuids)
{
  // used to hight entity in operator dialog

  // combine current selecton
  smtk::common::UUIDs uuidsCombined;
  if (qtActiveObjects::instance().smtkSelectionManager())
  {
    qtActiveObjects::instance().smtkSelectionManager()->getSelectedEntities(uuidsCombined);
  }
  for (auto uuid : uuids)
  {
    uuidsCombined.insert(uuid);
  }

  // update render view + model tree
  smtk::model::EntityRefs entities;
  for (const auto& uuid : uuidsCombined)
  {
    smtk::model::EntityRef ent(this->modelManager()->managerProxy()->modelManager(), uuid);
    entities.insert(ent);
  }
  if (this->modelPanel)
  {
    this->modelPanel->onSelectionChangedUpdateRenderView(
      entities, smtk::mesh::MeshSets(), smtk::model::DescriptivePhrases(), std::string());
    this->modelPanel->modelView()->onSelectionChangedUpdateModelTree(
      entities, smtk::mesh::MeshSets(), smtk::model::DescriptivePhrases(), std::string());
  }
}

void pqSMTKMeshPanel::clearActiveModel()
{ //this happens we we switch away from the mesh tab
  emit this->meshingPossible(false);
}

void pqSMTKMeshPanel::clearActiveMesh()
{ //this happens when we have no meshers at all!
  this->AttUIManager.reset();
  emit this->meshingPossible(false);
}

bool pqSMTKMeshPanel::submitMeshJob()
{

  smtk::io::Logger inputLogger;
  smtk::io::AttributeWriter writer;
  std::string serializedAttributes;
  //yes this returns false for being a valid, and true when an error occurs
  bool serialized = !writer.writeContents(this->AttSystem, serializedAttributes, inputLogger);
  if (!serialized)
  {
    return false;
  }

  //At this point we need to cache the active attributes. We cache the
  //attributes so that between meshing operations we don't lose the values
  //that the user has specified
  this->cacheAtts(serializedAttributes);

  const std::string meshOperatorName = "mesh";
  const std::string serializedReqs = remus::proto::to_string(this->ActiveRequirements);

  //for the model that we have, call the related sessions mesh operator
  smtk::model::SessionRef session = this->ActiveModel.session();
  const bool is_valid_op = !!session.opDef(meshOperatorName);

  if (is_valid_op)
  {
    //determine if this session has a mesh operator
    smtk::model::OperatorPtr meshOp = session.op(meshOperatorName);
    meshOp->ensureSpecification();

    smtk::attribute::AttributePtr meshSpecification = meshOp->specification();

    //send what model inside the session that we want to operate on
    //currently we will only take the first model
    meshSpecification->findModelEntity("model")->setValue(this->ActiveModel);

    meshSpecification->findString("endpoint")->setValue(this->MeshMonitor->connection().endpoint());

    meshSpecification->findString("remusRequirements")->setValue(serializedReqs);

    //send to the operator the serialized instance information
    meshSpecification->findString("meshingControlAttributes")->setValue(serializedAttributes);

    this->MeshButton->setText("Meshing ...");
    this->MeshButton->setEnabled(false);
    QCoreApplication::processEvents();
    this->MeshButton->repaint();

    const bool meshCreated = this->ModelManager->startOperation(meshOp);

    this->MeshButton->setText("Mesh");
    this->MeshButton->setEnabled(true);

    if (!meshCreated)
    {
      return false;
    }
  }

  return true;
}

void pqSMTKMeshPanel::cacheAtts(const std::string& atts)
{
  AttCacheKey key = { this->ActiveModel, this->ActiveRequirements.workerName() };
  //our key needs to become smaller since reqs is fairly 'heavy'
  this->CachedAttributes[key] = atts;
}

const std::string& pqSMTKMeshPanel::fetchCachedAtts() const
{
  typedef std::map<AttCacheKey, std::string>::const_iterator c_it;

  AttCacheKey key = { this->ActiveModel, this->ActiveRequirements.workerName() };
  c_it result = this->CachedAttributes.find(key);
  return result->second;
}

bool pqSMTKMeshPanel::hasCachedAtts(const remus::proto::JobRequirements& vtkNotUsed(reqs)) const
{
  typedef std::map<AttCacheKey, std::string>::const_iterator c_it;

  AttCacheKey key = { this->ActiveModel, this->ActiveRequirements.workerName() };
  c_it result = this->CachedAttributes.find(key);
  return result != this->CachedAttributes.end();
}

void pqSMTKMeshPanel::displayMeshOpLog(const smtk::io::Logger& log)
{
  QString txt(log.convertToString(false).c_str());
  this->ResultLog->setText(txt);
}

// Force the mesh operator to be available whenever the panel is present.
smtkComponentInitMacro(smtk_remus_mesh_operator);
