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

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "smtk/attribute/System.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/View.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtRootView.h"

#include <QtGui/QDockWidget>

#include <QPointer>
#include <QString>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPushButton>

#include "vtkSMModelManagerProxy.h"
#include "vtkPVSMTKModelInformation.h"

#include "qtCMBMeshingMonitor.h"
#include "pqCMBModelManager.h"
#include "qtRemusMesherSelector.h"


using namespace std;
using namespace smtk::model;


namespace
{
//-----------------------------------------------------------------------------
void make_InstancedView(smtk::attribute::SystemPtr attSystem)
{
  // Generate list of all concrete definitions in the manager
  typedef std::vector<smtk::attribute::DefinitionPtr>::const_iterator
                                                            DefIterType;

  smtk::common::ViewPtr root = attSystem->findViewByType("Root");
  int pos = root->details().findChild("Views");
  smtk::common::View::Component &vcomp = root->details().child(pos);
  std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
  
  attSystem->findBaseDefinitions(baseDefinitions);
  for (DefIterType baseIter = baseDefinitions.begin();
       baseIter != baseDefinitions.end();baseIter++)
    {
    std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
    attSystem->findAllDerivedDefinitions(*baseIter, true, derivedDefs);

    // Instantiate attribute for each concrete definition
    for (DefIterType defIter = derivedDefs.begin();
         defIter != derivedDefs.end(); defIter++)
      {
      smtk::common::ViewPtr view =
        smtk::common::View::New("Instanced", (*defIter)->type());
      smtk::common::View::Component &comp = view->details().addChild("InstancedAttributes");

      smtk::attribute::AttributePtr instance =
        attSystem->createAttribute((*defIter)->type());
      comp.addChild("Att").setAttribute("Type", instance->definition()->type())
        .setAttribute("Name",instance->name());
      vcomp.addChild("View").setAttribute("Title", view->title());
      }
    }
}
}

//-----------------------------------------------------------------------------
pqSMTKMeshPanel::pqSMTKMeshPanel(QPointer<pqCMBModelManager> modelManager,
                                 QPointer<qtCMBMeshingMonitor> monitor,
                                 QWidget* p)
: QDockWidget(p),
  ModelManager(modelManager),
  MeshMonitor(monitor),
  MeshSelector( new qtRemusMesherSelector(modelManager->managerProxy()->modelManager(),
                                          monitor->connection(),
                                          this ) ),
  RequirementsWidget( new QWidget(this) ),
  SubmitterWidget( new QWidget(this) ),
  AttSystem(),
  AttUIManager(),
  ActiveModels( ),
  ActiveRequirements()
{
  this->setObjectName("smtkMeshDockWidget");

  //construct a widget that holds all the three sections
  //of the mesh panel. Since the meshWidget is parented to the panel
  //it will be properly deleted by Qt when this class is deleted
  QWidget* meshWidget = new QWidget(this);
  QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom,meshWidget);
  meshWidget->setLayout(layout);

  //The RequirementsWidget needs a layout explicitly otherwise,
  //smtk will crash when it tries to add elements
  this->RequirementsWidget->setLayout(new QVBoxLayout());

  //construct a mesh button and add it to the SubmitterWidget
  this->SubmitterWidget->setLayout(new QVBoxLayout());
  QPushButton* meshButton = new QPushButton(QString("Mesh"));
  this->SubmitterWidget->layout()->addWidget(meshButton);
  //by default the widget is not visible
  this->SubmitterWidget->setVisible(false);

  layout->addWidget(this->MeshSelector.data());
  layout->addWidget(this->RequirementsWidget.data());
  layout->addWidget(this->SubmitterWidget.data());

  this->setWidget(meshWidget);

  QObject::connect(
    this->MeshSelector,
    SIGNAL(currentMesherChanged(const std::vector<smtk::model::Model>&, const QString&, const remus::proto::JobRequirements&)),
    this,
    SLOT( displayRequirements(const std::vector<smtk::model::Model>&, const QString&, const remus::proto::JobRequirements&) ) );

  QObject::connect(
    this,
    SIGNAL( visibilityChanged(bool) ),
    this->MeshSelector,
    SLOT( rebuildModelList() ) );

  QObject::connect(
    this->MeshSelector,
    SIGNAL( currentModelChanged( ) ),
    this,
    SLOT( clearActiveModel() ) );

  QObject::connect(
    this,
    SIGNAL( meshingPossible(bool) ),
    this->SubmitterWidget,
    SLOT( setVisible(bool) ) );

  QObject::connect(
    meshButton,
    SIGNAL( pressed() ),
    this,
    SLOT( submitMeshJob() ) );
}

//-----------------------------------------------------------------------------
pqSMTKMeshPanel::~pqSMTKMeshPanel()
{
}

//-----------------------------------------------------------------------------
QPointer<pqCMBModelManager> pqSMTKMeshPanel::modelManager()
{
  return this->ModelManager;
}

//-----------------------------------------------------------------------------
void pqSMTKMeshPanel::updateModel(QPointer<pqCMBModelManager> mmgr,
                                   QPointer<qtCMBMeshingMonitor> monitor)
{
  //we have a new modelproxy and new meshing monitor
  this->MeshSelector->updateModel(mmgr->managerProxy()->modelManager(),
                                  monitor->connection());
  this->MeshMonitor = monitor;
}

//-----------------------------------------------------------------------------
void pqSMTKMeshPanel::displayRequirements(const std::vector<smtk::model::Model>& modelsToDisplay,
                                          const QString & workerName,
                                          const remus::proto::JobRequirements& reqs)
{
  (void) workerName;
  //determine the session that this modelId is part of.
  //we can
  this->ActiveModels = modelsToDisplay;
  this->ActiveRequirements = reqs;

  //now that we have a requirements lets display them in the dock widget,
  //each time a new mesher is selected this needs to rebuild the UI
  //build up the smtk attributes.
  this->AttSystem.reset( new smtk::attribute::System() );
  this->AttSystem->setRefModelManager( this->ModelManager->managerProxy()->modelManager() );

  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;


  bool err = false;
  if(reqs.sourceType() == (remus::common::ContentSource::File) )
    { //the requirements are the file name so pass that to the attribute reader
    const std::string p(reqs.requirements(), reqs.requirementsSize());
    err = reader.read(*this->AttSystem, p, true, inputLogger);
    }
  else
    { //the requirements are in memory xml contents
    err = reader.readContents(*this->AttSystem,
                              reqs.requirements(),
                              reqs.requirementsSize(),
                              inputLogger);
    }
  // Matching the old View logic in 3.0 in which everything goes through a Root View
  // Assuming the 
  smtk::common::ViewPtr root = this->AttSystem->findTopLevelView();
  if (!root)
    {
    // Create a new Root View called MeshView
    root = smtk::common::View::New("Root", ("MeshView"));
    root->details().setAttribute("TopLevel", "true");
    this->AttSystem->addView(root);
    }
  // Get the Views Component ifit exists - else create it
  int pos = root->details().findChild("Views");
  if (pos < 0)
    {
    root->details().addChild("Views");
    pos = root->details().findChild("Views");
    }
  
  smtk::common::View::Component &vcomp = root->details().child(pos);

  // If manager contains no views, create InstancedView by default
  const bool useInternalFileBrowser = true;
  if (vcomp.numberOfChildren() == 0)
    {
    make_InstancedView(this->AttSystem);
    }
  this->AttUIManager.reset( new smtk::attribute::qtUIManager( *this->AttSystem));
                            this->AttUIManager->setSMTKView(root, this->RequirementsWidget.data(),
                                                            useInternalFileBrowser);
  QObject::connect(this->AttUIManager.get(), SIGNAL(entitiesSelected(const smtk::common::UUIDs&)),
    this, SIGNAL(entitiesSelected(const smtk::common::UUIDs&)));

  emit this->meshingPossible( true );
}

//-----------------------------------------------------------------------------
void pqSMTKMeshPanel::clearActiveModel()
  {
  this->ActiveModels = std::vector<smtk::model::Model>();
  this->ActiveRequirements = remus::proto::JobRequirements();
  this->AttUIManager.reset();
  this->AttSystem.reset();

  emit this->meshingPossible( false );
  }

//-----------------------------------------------------------------------------
bool pqSMTKMeshPanel::submitMeshJob()
{
  if(this->ActiveModels.empty())
    {
    return false;
    }

  smtk::io::Logger inputLogger;
  smtk::io::AttributeWriter writer;
  std::string serializedAttributes;

  //yes this returns false for being a valid, and true when an error occurs
  bool serialized = !writer.writeContents(*this->AttSystem,
                                          serializedAttributes,
                                          inputLogger);
  if(!serialized)
    {
    return false;
    }

  const std::string meshOperatorName = "mesh";
  const std::string serializedReqs = remus::proto::to_string(this->ActiveRequirements);

  //for each model that we have, call the related sessions mesh operator
  std::vector< smtk::model::Model >::const_iterator model_iter;
  for( model_iter = this->ActiveModels.begin();
       model_iter != this->ActiveModels.end();
       ++model_iter)
    {
    smtk::model::SessionRef session = model_iter->session();
    const bool is_valid_op = !!session.opDef(meshOperatorName);
    if(is_valid_op)
      {
      //determine if this session has a mesh operator
      smtk::model::OperatorPtr meshOp = session.op(meshOperatorName);
      meshOp->ensureSpecification();

      smtk::attribute::AttributePtr meshSpecification = meshOp->specification();

      //send what model inside the session that we want to operate on
      //currently we will only take the first model
      meshSpecification->findModelEntity("model")->setValue( *model_iter );

      meshSpecification->findString("endpoint")->setValue(this->MeshMonitor->connection().endpoint());


      meshSpecification->findString("remusRequirements")->setValue( serializedReqs );

      //send to the operator the serialized instance information
      meshSpecification->findString("meshingControlAttributes")->setValue(serializedAttributes);

      const bool meshCreated = this->ModelManager->startOperation( meshOp );
      if(!meshCreated)
        {
        return false;
        }
      }
    }
  return true;
}

