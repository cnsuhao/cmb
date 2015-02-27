/*=========================================================================

  Program:   CMB
  Module:    qtRemusVolumeMesherSubmitter.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "qtRemusVolumeMesherSubmitter.h"

#include "pqCMBModel.h"

#include "pqSMAdaptor.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include <QBoxLayout>
#include <QDialogButtonBox>

#include <remus/client/Client.h>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/Definition.h>
#include <smtk/attribute/Manager.h>
#include <smtk/model/Model.h>
#include <smtk/Qt/qtUIManager.h>
#include <smtk/util/AttributeReader.h>
#include <smtk/util/AttributeWriter.h>
#include <smtk/util/Logger.h>
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
void make_InstancedView(smtk::attribute::Manager& manager)
{
  // Generate list of all concrete definitions in the manager
  typedef std::vector<smtk::attribute::DefinitionPtr>::const_iterator
                                                            DefIterType;

  std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
  manager.findBaseDefinitions(baseDefinitions);
  for (DefIterType baseIter = baseDefinitions.begin();
       baseIter != baseDefinitions.end();baseIter++)
    {
    std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
    manager.findAllDerivedDefinitions(*baseIter, true, derivedDefs);

    // Instantiate attribute for each concrete definition
    for (DefIterType defIter = derivedDefs.begin();
         defIter != derivedDefs.end(); defIter++)
      {
      smtk::view::InstancedPtr view =
          smtk::view::Instanced::New((*defIter)->type());

      smtk::attribute::AttributePtr instance =
        manager.createAttribute((*defIter)->type());

      manager.rootView()->addSubView(view);
      view->addInstance(instance);
      }
    }
}

void make_DefaultView(smtk::attribute::Manager& manager)
{
  typedef std::vector<smtk::attribute::DefinitionPtr> DefinitionVector;

  smtk::view::InstancedPtr instanced(smtk::view::Instanced::New("Default"));

  DefinitionVector defList;
  manager.findBaseDefinitions(defList);

  for (DefinitionVector::const_iterator defIter = defList.begin();
       defIter != defList.end(); ++defIter)
    {
    if ((*defIter)->isAbstract())
      {
      // For abstract definitions, retrieve all derived & concrete defs
      std::vector<smtk::attribute::DefinitionPtr> derivedList;
      manager.findAllDerivedDefinitions(*defIter, true, derivedList);
      for (DefinitionVector::const_iterator derivedIter = derivedList.begin();
           derivedIter != derivedList.end(); ++derivedIter)
        {
        instanced->addInstance(manager.createAttribute((*derivedIter)->type()));
        }
      }
    else
      {
      instanced->addInstance(manager.createAttribute((*defIter)->type()));
      }
    }
  manager.rootView()->addSubView(instanced);
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
  smtk::attribute::Manager manager;
  smtk::util::AttributeReader reader;
  smtk::util::Logger inputLogger;

  smtk::model::ModelPtr smtkModel(new smtk::model::Model());
  manager.setRefModel(smtkModel);

  smtk::attribute::qtUIManager uiManager(manager);

  bool err = false;
  if(reqs.sourceType() == (remus::common::ContentSource::File) )
    { //the requirements are the file name so pass that to the attribute reader
    const std::string p(reqs.requirements(), reqs.requirementsSize());
    err = reader.read(manager, p, true, inputLogger);
    }
  else
    { //the requirements are in memory xml contents
    err = reader.readContents(manager, reqs.requirements(),
                              reqs.requirementsSize(), inputLogger);
    }

  if (err)
    {
    return resultingJob;
    }

  // If manager contains no views, create InstancedView by default
  const bool useInternalFileBrowser = true;
  if (manager.rootView()->numberOfSubViews() == 0)
    {
    make_InstancedView(manager);
    uiManager.initializeUI(this, useInternalFileBrowser);
    }
  else
    {
    //we have views so we need to create
    make_DefaultView(manager);
    uiManager.initializeView(this, manager.rootView(), useInternalFileBrowser);
    }

  const bool requirementsAccepted = this->exec() == QDialog::Accepted;

  if(!requirementsAccepted)
    {
    return resultingJob;
    }

  smtk::util::AttributeWriter writer;
  std::string serializedAttributes;

  //yes this returns false for being a valid, and true when an error occurs
  bool serialized = !writer.writeContents(manager,
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
  return resultingJob;
}
