//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtRemusVolumeMesherSubmitter.h"

#include "pqSMAdaptor.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include <QBoxLayout>
#include <QDialogButtonBox>

#include <remus/client/Client.h>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/Definition.h>
#include <smtk/attribute/System.h>
#include <smtk/extension/qt/qtUIManager.h>
#include <smtk/io/AttributeReader.h>
#include <smtk/io/AttributeWriter.h>
#include <smtk/io/Logger.h>
#include <smtk/view/Instanced.h>
#include <smtk/view/Root.h>

namespace
{

remus::proto::Job make_invalidJob()
{
  remus::meshtypes::MeshTypeBase baseType;
  return remus::proto::Job(boost::uuids::uuid(),
                           remus::common::MeshIOType(baseType,baseType));
}
void make_InstancedView(smtk::attribute::System& system)
{
  // Generate list of all concrete definitions in the system
  typedef std::vector<smtk::attribute::DefinitionPtr>::const_iterator
                                                            DefIterType;

  std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
  system.findBaseDefinitions(baseDefinitions);
  for (DefIterType baseIter = baseDefinitions.begin();
       baseIter != baseDefinitions.end();baseIter++)
    {
    std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
    system.findAllDerivedDefinitions(*baseIter, true, derivedDefs);

    // Instantiate attribute for each concrete definition
    for (DefIterType defIter = derivedDefs.begin();
         defIter != derivedDefs.end(); defIter++)
      {
      smtk::view::InstancedPtr view =
          smtk::view::Instanced::New((*defIter)->type());

      smtk::attribute::AttributePtr instance =
        system.createAttribute((*defIter)->type());

      system.rootView()->addSubView(view);
      view->addInstance(instance);
      }
    }
}

void make_DefaultView(smtk::attribute::System& system)
{
  typedef std::vector<smtk::attribute::DefinitionPtr> DefinitionVector;

  smtk::view::InstancedPtr instanced(smtk::view::Instanced::New("Default"));

  DefinitionVector defList;
  system.findBaseDefinitions(defList);

  for (DefinitionVector::const_iterator defIter = defList.begin();
       defIter != defList.end(); ++defIter)
    {
    if ((*defIter)->isAbstract())
      {
      // For abstract definitions, retrieve all derived & concrete defs
      std::vector<smtk::attribute::DefinitionPtr> derivedList;
      system.findAllDerivedDefinitions(*defIter, true, derivedList);
      for (DefinitionVector::const_iterator derivedIter = derivedList.begin();
           derivedIter != derivedList.end(); ++derivedIter)
        {
        instanced->addInstance(system.createAttribute((*derivedIter)->type()));
        }
      }
    else
      {
      instanced->addInstance(system.createAttribute((*defIter)->type()));
      }
    }
  system.rootView()->addSubView(instanced);
}
}


//-----------------------------------------------------------------------------
qtRemusVolumeMesherSubmitter::qtRemusVolumeMesherSubmitter( QString endpoint,
                                        QWidget* parent  ):
  QDialog(parent),
  Connection(remus::client::make_ServerConnection(endpoint.toStdString()))
{
  //the layout is required for smtk to display properly. Without the layout
  //smtk will segfault. We want the layout to be bottom to top since we want
  //to insert the Accept/Cancel buttons at the bottom of the dialog, with
  //the smtk generated widgets above them.
  QBoxLayout *layout = new QBoxLayout(QBoxLayout::BottomToTop,this);
  this->setLayout(layout);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
  buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  layout->addWidget(buttonBox);

  //connect that accept and reject signals of the button box to ourself
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

}

//-----------------------------------------------------------------------------
remus::proto::Job qtRemusVolumeMesherSubmitter::submitRequirements(
                          pqCMBModel* model,
                          const QString& modelFilePath,
                          const remus::proto::JobRequirements& reqs )
{
  remus::proto::Job resultingJob = make_invalidJob();
  if(!model)
    {
    return resultingJob;
    }

  //build up the smtk attributes
  smtk::attribute::System system;
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;

  // FIXME: There is no more smtk::model::Model
  //smtk::model::ModelPtr smtkModel(new smtk::model::Model());
  //system.setRefModel(smtkModel);

  smtk::attribute::qtUIManager uiManager(system);

  bool err = false;
  if(reqs.sourceType() == (remus::common::ContentSource::File) )
    { //the requirements are the file name so pass that to the attribute reader
    const std::string p(reqs.requirements(), reqs.requirementsSize());
    err = reader.read(system, p, true, inputLogger);
    }
  else
    { //the requirements are in memory xml contents
    err = reader.readContents(system, reqs.requirements(),
                              reqs.requirementsSize(), inputLogger);
    }

  if (err)
    {
    return resultingJob;
    }

  // If system contains no views, create InstancedView by default
  const bool useInternalFileBrowser = true;
  if (system.rootView()->numberOfSubViews() == 0)
    {
    make_InstancedView(system);
    uiManager.initializeUI(this, useInternalFileBrowser);
    }
  else
    {
    //we have views so we need to create
    make_DefaultView(system);
    uiManager.initializeView(this, system.rootView(), useInternalFileBrowser);
    }

  const bool requirementsAccepted = this->exec() == QDialog::Accepted;

  if(!requirementsAccepted)
    {
    return resultingJob;
    }

  smtk::io::AttributeWriter writer;
  std::string serializedAttributes;

  //yes this returns false for being a valid, and true when an error occurs
  bool serialized = !writer.writeContents(system,
                                          serializedAttributes,
                                          inputLogger);
  if(!serialized)
    {
    return resultingJob;
    }

  //we now have the serialized data so we can send them down to the worker
  //without issue.

  //create new requirements and job content ( zero copy to reduce memory )
  remus::proto::JobSubmission submission(reqs);
  remus::proto::JobContent instanceValues(reqs.formatType(),
                                          serializedAttributes.c_str(),
                                          serializedAttributes.size() );

  //create a JobContent that contains the location were to save the output
  //of the mesher
  submission["instance"] = instanceValues;
  submission["model_file_path"] = remus::proto::make_JobContent(
                                                  modelFilePath.toStdString());

  //helper tag that we might need later
  submission["model_file_path"].tag("input model file location");
/*
  //we have the submission and we have a connection object, so lets bundle
  //everything up and send it down to the server to send. We can't send
  //from the client as the full model is on the server
  vtkSMOperatorProxy* jobSubmitProxy;
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  jobSubmitProxy = vtkSMOperatorProxy::SafeDownCast(
                    pxm->NewProxy("CMBModelGroup", "MeshServerJobSubmitter"));
  if(!jobSubmitProxy)
    {
    return resultingJob;
    }

  jobSubmitProxy->SetLocation(model->getModelWrapper()->GetLocation());

  pqSMAdaptor::setElementProperty(jobSubmitProxy->GetProperty("Endpoint"),
                  QString::fromStdString(this->Connection.endpoint()));
  pqSMAdaptor::setElementProperty(jobSubmitProxy->GetProperty("Submission"),
                  QString::fromStdString(remus::proto::to_string(submission)));

  jobSubmitProxy->Operate(model->getModel(), model->getModelWrapper());

  // check to see if the operation succeeded on the server
  jobSubmitProxy->UpdatePropertyInformation();
  const int succeeded = pqSMAdaptor::getElementProperty(
                jobSubmitProxy->GetProperty("OperateSucceeded")).toInt();

  if(succeeded)
    {
    //fetch the remus::proto::Job back from the server, so the
    //client can monitor the job
    const QString serializedJob = pqSMAdaptor::getElementProperty(
                    jobSubmitProxy->GetProperty("LastSubmittedJob")).toString();

    //resultingJob now becomes valid
    resultingJob = remus::proto::to_Job(serializedJob.toStdString());
    }
*/
  return resultingJob;
}
