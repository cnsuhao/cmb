//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtRemusMesherSelector.h"

#include "ui_qtMesherSelector.h"

//needed to use remus requirements inside a qvariant
Q_DECLARE_METATYPE(remus::proto::JobRequirements)
//needed to use smtk::model::Model inside a qvariant
Q_DECLARE_METATYPE(smtk::model::Model)

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
          smtk::model::ManagerPtr modelManager,
          const remus::client::ServerConnection& connection,
          QWidget* parent):
  QWidget(parent),
  Internal(new pqInternal),
  ModelManager( modelManager ),
  Client( new remus::client::Client( connection ) ) //connect to the server using the passed in connection
{
  this->Internal->setupUi(this);

  connect( this->Internal->cb_models, SIGNAL(currentIndexChanged ( int )),
           this, SIGNAL( currentModelChanged( )) );

  connect( this->Internal->cb_models, SIGNAL(currentIndexChanged ( int )),
           this, SLOT( modelChanged( int )) );

  connect( this->Internal->cb_meshers, SIGNAL(currentIndexChanged ( int )),
           this, SLOT( mesherChanged( int )) );

  this->rebuildModelList(true);
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::rebuildModelList(bool shouldRebuild)
{
  if(!shouldRebuild)
    { //only rebuild when the visibilty / shouldRebuild becomes true
    return;
    }
  smtk::model::EntityRefArray allModels =
      this->ModelManager->findEntitiesOfType( smtk::model::MODEL_ENTITY );

  //First determine if the models need to be cleared
  bool rebuild = true;
  if(static_cast<int>(allModels.size()) == this->Internal->cb_models->count())
    {
    rebuild = false;
    int index = 0;
    smtk::model::EntityRefArray::const_iterator i;
    for(i=allModels.begin(); i != allModels.end() && rebuild == false; ++i, ++index)
      {
      smtk::model::Model new_model = i->as<smtk::model::Model>();
      smtk::model::Model current_model =
      fromItemData<smtk::model::Model>( this->Internal->cb_models, index);
      rebuild = !(new_model == current_model);
      }
    }

  if(rebuild)
    {
    this->Internal->cb_models->blockSignals(true);
    this->Internal->cb_models->clear();

    //fill the model combo box based on the contents of the model Manager.
    //todo, everytime a new model is added/removed we need to refresh this list
    smtk::model::EntityRefArray::const_iterator i;
    for(i=allModels.begin(); i != allModels.end(); ++i)
      {
      smtk::model::Model model = i->as<smtk::model::Model>();
      if(model.isValid())
        {
        std::stringstream fancyModelName;
        fancyModelName << i->name( ) << " (" << i->dimension() << "D)";
        this->Internal->cb_models->addItem(QString::fromStdString( fancyModelName.str() ),
                                           QVariant::fromValue(model) );
        }
      }
    this->Internal->cb_models->blockSignals(false);
    }

  this->modelChanged( this->Internal->cb_models->currentIndex() );


}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::modelChanged(int index)
{

  //grab the current model to determine the dimension
  smtk::model::Model model =
      fromItemData<smtk::model::Model>( this->Internal->cb_models, index);

  const int modelDimension = model.dimension();

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

  const bool supportsModelMeshing = this->Client->canMesh(modelIOType);
  const bool supportsMeshMeshing= meshIOType.valid() && this->Client->canMesh(meshIOType);

  if(supportsModelMeshing  || supportsMeshMeshing )
    {

    //combine the discrete and model works into a single list
    remus::proto::JobRequirementsSet possibleWorkers;
    remus::proto::JobRequirementsSet modelWorkers = this->Client->retrieveRequirements( modelIOType );
    possibleWorkers.insert(modelWorkers.begin(),modelWorkers.end());

    if(supportsMeshMeshing)
      {
      remus::proto::JobRequirementsSet meshWorkers = this->Client->retrieveRequirements( meshIOType );
      possibleWorkers.insert(meshWorkers.begin(),meshWorkers.end());
      }

    bool rebuild = true;
    if(static_cast<int>(possibleWorkers.size()) == this->Internal->cb_meshers->count())
      {
      rebuild = false;
      int indexLocal = 0;
      remus::proto::JobRequirementsSet::const_iterator i;
      for(i = possibleWorkers.begin(); i != possibleWorkers.end() && rebuild == false; ++i, ++indexLocal)
        {
        remus::proto::JobRequirements current_reqs =
              fromItemData<remus::proto::JobRequirements>( this->Internal->cb_meshers, indexLocal);
        rebuild = !(*i == current_reqs);
        }
      }

    if(rebuild)
      {
      //clear any existing elements in the combo box.
      this->Internal->cb_meshers->blockSignals(true);
      this->Internal->cb_meshers->clear();

      remus::proto::JobRequirementsSet::const_iterator i;
      for(i = possibleWorkers.begin(); i != possibleWorkers.end(); ++i, ++index)
        {
        this->Internal->cb_meshers->addItem(QString::fromStdString(i->workerName()),
                                            QVariant::fromValue(*i) );
        }
      this->Internal->cb_meshers->blockSignals(false);
      }

    this->mesherChanged(  this->Internal->cb_meshers->currentIndex() );
    }
  else
    {
    emit this->noMesherForModel();
    }
}

//-----------------------------------------------------------------------------
qtRemusMesherSelector::~qtRemusMesherSelector()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::updateModel(smtk::model::ManagerPtr modelManager,
                                        const remus::client::ServerConnection& connection)
{
  this->ModelManager = modelManager;
  //connect to the server using the passed in connection
  this->Client.reset( new remus::client::Client( connection ) );
}

//-----------------------------------------------------------------------------
smtk::model::Model qtRemusMesherSelector::currentModel() const
{
  return fromItemData<smtk::model::Model>(this->Internal->cb_models,
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
  if(index >= 0)
    {
    QString workerName = this->Internal->cb_meshers->itemText( index );
    remus::proto::JobRequirements reqs  =
        fromItemData<remus::proto::JobRequirements>(this->Internal->cb_meshers,
                                                  index);
    emit this->currentMesherChanged( this->currentModel(), workerName, reqs );
    }
}

