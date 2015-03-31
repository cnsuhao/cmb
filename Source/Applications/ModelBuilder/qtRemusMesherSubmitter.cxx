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
#include "ModelManager.h"

#include "pqSMAdaptor.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include <QBoxLayout>
#include <QDialogButtonBox>

#include <remus/proto/Job.h>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/Definition.h>
#include <smtk/attribute/Manager.h>
#include <smtk/attribute/IntItem.h>
#include <smtk/attribute/StringItem.h>
#include <smtk/attribute/ModelEntityItem.h>
#include <smtk/model/Bridge.h>
#include <smtk/model/Manager.h>
#include <smtk/model/Model.h>
#include <smtk/model/Operator.h>
#include <smtk/Qt/qtUIManager.h>
#include <smtk/util/AttributeReader.h>
#include <smtk/util/AttributeWriter.h>
#include <smtk/util/Logger.h>
#include <smtk/view/Instanced.h>
#include <smtk/view/Root.h>

#include "vtkSMModelManagerProxy.h"

namespace
{

//------------------------------------------------------------------------------
remus::proto::Job make_invalidJob()
{
  remus::meshtypes::MeshTypeBase baseType;
  return remus::proto::Job(boost::uuids::uuid(),
                           remus::common::MeshIOType(baseType,baseType));
}
//-----------------------------------------------------------------------------
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
                          ModelManager* smtkManager,
                          const remus::proto::JobRequirements& reqs )
{
  remus::proto::Job errorJob = make_invalidJob();
  if(!smtkManager)
    {
    return errorJob;
    }

  //build up the current model
  const smtk::util::UUID session = smtkManager->currentSession();

  //we need to use the raw manager since the proxy can't
  //create operators currently
  smtk::model::ManagerPtr model_manager = smtkManager->managerProxy()->modelManager();
  smtk::model::BridgePtr bridge = model_manager->findBridgeSession(session);

  //build up the smtk attributes. Todo use our own att_manager not the global one
  smtk::attribute::Manager att_manager;
  att_manager.setRefModelManager(model_manager);

  smtk::attribute::qtUIManager uiManager(att_manager);
  smtk::util::AttributeReader reader;
  smtk::util::Logger inputLogger;

  bool err = false;
  if(reqs.sourceType() == (remus::common::ContentSource::File) )
    { //the requirements are the file name so pass that to the attribute reader
    const std::string p(reqs.requirements(), reqs.requirementsSize());
    err = reader.read(att_manager, p, true, inputLogger);
    }
  else
    { //the requirements are in memory xml contents
    err = reader.readContents(att_manager, reqs.requirements(),
                              reqs.requirementsSize(), inputLogger);
    }

  // If manager contains no views, create InstancedView by default
  const bool useInternalFileBrowser = true;
  if (att_manager.rootView()->numberOfSubViews() == 0)
    {
    make_InstancedView(att_manager);
    }
  uiManager.initializeUI(this, useInternalFileBrowser);

  const bool requirementsAccepted = this->exec() == QDialog::Accepted;

  if(!requirementsAccepted)
    {
    return errorJob;
    }

  smtk::util::AttributeWriter writer;
  std::string serializedAttributes;

  //yes this returns false for being a valid, and true when an error occurs
  bool serialized = !writer.writeContents(att_manager,
                                          serializedAttributes,
                                          inputLogger);
  if(!serialized)
    {
    return errorJob;
    }

  //we now invoke an operator on the client. That operator
  //will take all the information we have built up and the
  //serialized json model and send it to the worker

  smtk::model::OperatorPtr meshOp = bridge->op("mesh", model_manager);
  if(!meshOp)
    {
    return errorJob;
    }

  meshOp->ensureSpecification();
  smtk::attribute::AttributePtr meshSpecification = meshOp->specification();
  if(!meshSpecification)
    {
    return errorJob;
    }

  smtk::model::ModelEntity modelEnt = smtkManager->currentModel();

  meshSpecification->findString("modelUUID")->setValue(modelEnt.entity().toString());

  meshSpecification->findString("endpoint")->setValue(this->Connection.endpoint());

  std::ostringstream buffer; buffer << reqs;
  meshSpecification->findString("remusRequirements")->setValue( buffer.str() );

  //send to the operator the serialized instance information
  meshSpecification->findString("meshingControlInstance")->setValue(serializedAttributes);

  //send down the location to save the file
  meshSpecification->findString("currentFile")->setValue(smtkManager->currentFile());

  //now invoke the operator so that we submit this as remus job
  smtk::model::OperatorResult result = meshOp->operate();

  //if the operator was valid de-serailize the resulting remus::prot::Job
  if (result->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED)
    {
    //update resultingJob to hold the de-serailized job info
    return remus::proto::to_Job(result->findString("job")->value());
    }

  return errorJob;
}
