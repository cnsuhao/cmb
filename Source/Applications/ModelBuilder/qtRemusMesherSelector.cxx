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

#include "ui_qtMesherSelector.h"

#include <remus/client/Client.h>


//needed to use remus requirements inside a qvariant
Q_DECLARE_METATYPE(remus::proto::JobRequirements)

//-----------------------------------------------------------------------------
class qtRemusMesherSelector::pqInternal : public Ui::qtMesherSelector { };

//-----------------------------------------------------------------------------
qtRemusMesherSelector::qtRemusMesherSelector(
          const remus::client::ServerConnection& connection,
          QWidget* parent):
  QWidget(parent),
  Internal(new pqInternal)
{
  this->Internal->setupUi(this);

  //First we use the passed in connection endpoint, because we can't be
  //sure that we are connecting to a local server, or even that the server
  //is bound the default port
  remus::client::Client c(connection);

  using namespace remus::meshtypes;
  remus::common::MeshIOType volumeMeshingToFile( (Model()), (Mesh3D()) );
  remus::common::MeshIOType surfaceMeshingToFile( (Model()), (Mesh2D()) );
  remus::common::MeshIOType modelMeshing( (Model()), (Model()) );

  //if the server supports volume meshes, we go forward and add an item
  //to MesherList that is the mesher name, and set custom UserRole data
  //to be the mesher requirements
  const bool supportsVolumeMeshingToFile = c.canMesh(volumeMeshingToFile);
  const bool supportsSurfaceMeshingToFile = c.canMesh(surfaceMeshingToFile);
  const bool supportsModelMeshing = c.canMesh(modelMeshing);
  if(supportsVolumeMeshingToFile  ||
     supportsSurfaceMeshingToFile ||
     supportsModelMeshing         )
    {

    remus::proto::JobRequirementsSet volumeWorkers =c.retrieveRequirements( volumeMeshingToFile );
    remus::proto::JobRequirementsSet surfaceWorkers = c.retrieveRequirements( surfaceMeshingToFile );
    remus::proto::JobRequirementsSet modelWorkers = c.retrieveRequirements( modelMeshing );

    //combine the discrete and model works into a single list
    remus::proto::JobRequirementsSet possibleWorkers;
    possibleWorkers.insert(volumeWorkers.begin(),volumeWorkers.end());
    possibleWorkers.insert(surfaceWorkers.begin(),surfaceWorkers.end());
    possibleWorkers.insert(modelWorkers.begin(),modelWorkers.end());

    remus::proto::JobRequirementsSet::const_iterator i;
    int index = 1;
    for(i = possibleWorkers.begin(); i != possibleWorkers.end(); ++i, ++index)
      {
      this->Internal->cb_meshers->insertItem(index,
                                      QString::fromStdString(i->workerName()),
                                      QVariant::fromValue(*i) );
      }
    }
  connect( this->Internal->cb_meshers, SIGNAL(currentIndexChanged ( int )),
           this, SLOT( mesherChanged( int )) );
}

//-----------------------------------------------------------------------------
qtRemusMesherSelector::~qtRemusMesherSelector()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
int qtRemusMesherSelector::currentIndex() const
{
  return this->Internal->cb_meshers->currentIndex();
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
  const int index = this->Internal->cb_meshers->currentIndex();
  remus::proto::JobRequirements reqs  =
      this->Internal->cb_meshers->itemData(index, Qt::UserRole).value< remus::proto::JobRequirements>();
  return reqs;
}

//-----------------------------------------------------------------------------
void qtRemusMesherSelector::mesherChanged( int index )
{
  QString name = this->Internal->cb_meshers->itemText( index );
  remus::proto::JobRequirements reqs  =
      this->Internal->cb_meshers->itemData(index, Qt::UserRole).value< remus::proto::JobRequirements>();

  emit this->currentMesherChanged( name, reqs );
}

