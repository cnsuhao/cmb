/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: pqSMTKMeshPanel,v $

 =========================================================================*/
#include "pqSMTKMeshPanel.h"


#include "smtk/model/Manager.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/view/Instanced.h"
#include "smtk/view/Root.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QtGui/QDockWidget>

#include <QPointer>
#include <QString>
#include <QBoxLayout>

#include "vtkSMModelManagerProxy.h"
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
      smtk::view::InstancedPtr view =
          smtk::view::Instanced::New((*defIter)->type());

      smtk::attribute::AttributePtr instance =
        attSystem->createAttribute((*defIter)->type());

      attSystem->rootView()->addSubView(view);
      view->addInstance(instance);
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
  MeshSelector( new qtRemusMesherSelector(//modelManager,
                                          monitor->connection(),
                                          this ) ),
  RequirementsWidget( new QWidget(this) ),
  SubmitterWidget( new QWidget(this) ),
  AttSystem(),
  AttUIManager()
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
  this->RequirementsWidget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom));


  layout->addWidget(this->MeshSelector.data());
  layout->addWidget(this->RequirementsWidget.data());
  layout->addWidget(this->SubmitterWidget.data());

  this->setWidget(meshWidget);

  if(!this->MeshSelector->currentMesherName().isEmpty())
    {
    //we have found atleast a single mesher, and we don't have any connections
    //made, so lets manaully invoke displayRequirements
    this->displayRequirements(this->MeshSelector->currentMesherName(),
                              this->MeshSelector->currentMesherRequirements());
    }

  QObject::connect(this->MeshSelector,
    SIGNAL(currentMesherChanged(const QString&, const remus::proto::JobRequirements&)),
    this, SLOT( displayRequirements(const QString&, const remus::proto::JobRequirements&) ));
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
void pqSMTKMeshPanel::displayRequirements(const QString & name,
                                          const remus::proto::JobRequirements& reqs)
{
  //we need to get the raw manager
  smtk::model::ManagerPtr model_manager = this->ModelManager->managerProxy()->modelManager();

  //now that we have a requirements lets display them in the dock widget,
  //each time a new mesher is selected this needs to rebuild the UI
  //build up the smtk attributes.
  this->AttSystem.reset( new smtk::attribute::System() );
  this->AttSystem->setRefModelManager(model_manager);

  this->AttUIManager.reset( new smtk::attribute::qtUIManager( *this->AttSystem) );
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

  // If manager contains no views, create InstancedView by default
  const bool useInternalFileBrowser = true;
  if (this->AttSystem->rootView()->numberOfSubViews() == 0)
    {
    make_InstancedView(this->AttSystem);
    }
  this->AttUIManager->initializeUI(this->RequirementsWidget.data(),
                                   useInternalFileBrowser);
}


//-----------------------------------------------------------------------------
void pqSMTKMeshPanel::submitMeshJob()
{

}

