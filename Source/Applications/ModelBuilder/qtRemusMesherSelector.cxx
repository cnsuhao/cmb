/*=========================================================================

  Program:   CMB
  Module:    qtRemusMesherSelector.cxx

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
#include "qtRemusMesherSelector.h"
#include "pqCMBModelManager.h"

#include "ui_qtMesherSelector.h"

#include "smtk/model/Manager.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkPVSMTKModelInformation.h"


//needed to use remus requirements inside a qvariant
Q_DECLARE_METATYPE(remus::proto::JobRequirements)
//needed to use smtk uuid inside a qvariant
Q_DECLARE_METATYPE(smtk::common::UUID)

namespace
{
template<typename T>
T fromItemData(QComboBox* cb, int index)
  {
  return cb->itemData(index, Qt::UserRole).value< T >();
  }
}

//-----------------------------------------------------------------------------
class qtRemusMesherSelector::pqInternal : public Ui::qtMesherSelector { };

//-----------------------------------------------------------------------------
qtRemusMesherSelector::qtRemusMesherSelector(
          QPointer<pqCMBModelManager> modelManager,
          const remus::client::ServerConnection& connection,
          QWidget* parent):
  QWidget(parent),
  Internal(new pqInternal),
  ModelManager( modelManager ),
  Client( connection ) //connect to the server using the passed in connection
{
  this->Internal->setupUi(this);

  connect( this->Internal->cb_models, SIGNAL(currentIndexChanged ( int )),
           this, SLOT( modelChanged( int )) );

  connect( this->Internal->cb_meshers, SIGNAL(currentIndexChanged ( int )),
           this, SLOT( mesherChanged( int )) );

  this->rebuildModelList();
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::rebuildModelList()
{
  smtk::model::ManagerPtr mgr = this->ModelManager->managerProxy()->modelManager();

  //remove any current models
  this->Internal->cb_models->clear();

  //fill the model combo box based on the contents of the model Manager.
  //todo, everytime a new model is added/removed we need to refresh this list
  QList<cmbSMTKModelInfo*> allModels = this->ModelManager->allModels();
  typedef QList<cmbSMTKModelInfo*>::const_iterator cit;
  int index = 1;
  for(cit i=allModels.begin(); i != allModels.end(); ++i, ++index)
    {
    const smtk::common::UUID modelId = (*i)->Info->GetModelUUID();
    const int modelDimension = mgr->dimension(modelId);
    std::stringstream fancyModelName;
    fancyModelName << mgr->name( modelId ) << " (" << modelDimension << "D)";

    this->Internal->cb_models->insertItem(index,
                                          QString::fromStdString( fancyModelName.str() ),
                                          QVariant::fromValue(modelId) );
    //the Variant should be the uuid?
    }
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::modelChanged(int index)
{
  //clear any existing elements in the combo box.
  this->Internal->cb_meshers->clear();

  smtk::model::ManagerPtr mgr = this->ModelManager->managerProxy()->modelManager();

  //grab the current model to determine the dimension
  smtk::common::UUID modelId =
      fromItemData<smtk::common::UUID>( this->Internal->cb_models, index);

  int modelDimension = mgr->dimension(modelId);

  remus::common::MeshIOType modelIOType( (remus::meshtypes::Model()), (remus::meshtypes::Model()) );
  remus::common::MeshIOType meshIOType;
  if(modelDimension == 2)
    {
    meshIOType = remus::common::MeshIOType( (remus::meshtypes::Mesh2D()), (remus::meshtypes::Model()));
    }
  else if(modelDimension == 3)
    {
    meshIOType = remus::common::MeshIOType( (remus::meshtypes::Mesh3D()), (remus::meshtypes::Model()));
    }

  const bool supportsModelMeshing = this->Client.canMesh(modelIOType);
  const bool supportsMeshMeshing= meshIOType.valid() && this->Client.canMesh(meshIOType);

  if(supportsModelMeshing  || supportsMeshMeshing )
    {

    //combine the discrete and model works into a single list
    remus::proto::JobRequirementsSet possibleWorkers;
    remus::proto::JobRequirementsSet modelWorkers = this->Client.retrieveRequirements( modelIOType );
    possibleWorkers.insert(modelWorkers.begin(),modelWorkers.end());

    if(supportsMeshMeshing)
      {
      remus::proto::JobRequirementsSet meshWorkers = this->Client.retrieveRequirements( meshIOType );
      possibleWorkers.insert(meshWorkers.begin(),meshWorkers.end());
      }

    remus::proto::JobRequirementsSet::const_iterator i;

    int index = 1;
    for(i = possibleWorkers.begin(); i != possibleWorkers.end(); ++i, ++index)
      {
      this->Internal->cb_meshers->insertItem(index,
                                      QString::fromStdString(i->workerName()),
                                      QVariant::fromValue(*i) );
      }
    }

}

//-----------------------------------------------------------------------------
qtRemusMesherSelector::~qtRemusMesherSelector()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
smtk::common::UUID qtRemusMesherSelector::currentModelUUID() const
{
  return fromItemData<smtk::common::UUID>(this->Internal->cb_models,
                                          this->Internal->cb_models->currentIndex());
}

//-----------------------------------------------------------------------------
QString qtRemusMesherSelector::currentMesherName() const
{
  return this->Internal->cb_meshers->currentText();
}

//-----------------------------------------------------------------------------
remus::proto::JobRequirements
qtRemusMesherSelector::currentMesherRequirements() const
{
  return fromItemData<remus::proto::JobRequirements>(this->Internal->cb_meshers,
                                                     this->Internal->cb_meshers->currentIndex());
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::mesherChanged( int index )
{
  smtk::common::UUID modelId = this->currentModelUUID();
  QString workerName = this->Internal->cb_meshers->itemText( index );
  remus::proto::JobRequirements reqs  =
      fromItemData<remus::proto::JobRequirements>(this->Internal->cb_meshers,
                                                  index);
  emit this->currentMesherChanged( modelId, workerName, reqs );
}

