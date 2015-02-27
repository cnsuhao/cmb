/*=========================================================================

  Program:   CMB
  Module:    qtRemusVolumeMesherSelector.cxx

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
#include "qtRemusVolumeMesherSelector.h"

#include "ui_qtVolumeMesherSelector.h"

#include <remus/client/Client.h>

namespace
{
  QString DefaultMesherName()
    { return QString("Built In: Omicron"); }
}

//needed to use remus requirements inside a qvariant
Q_DECLARE_METATYPE(remus::proto::JobRequirements)


//-----------------------------------------------------------------------------
qtRemusVolumeMesherSelector::qtRemusVolumeMesherSelector(QString serverEndpoint,
                                                   QWidget* parent):
  QDialog(parent),
  ActiveMesher(DefaultMesherName()),
  ActiveMesherData(),
  Connection(remus::client::make_ServerConnection(serverEndpoint.toStdString()))
{
  Ui::qtVolumeMesherSelector dialogUI;
  dialogUI.setupUi(this);

  dialogUI.MesherList->addItem( DefaultMesherName() );


  //First we use the passed in connection endpoint, because we can't be
  //sure that we are connecting to a local server, or even that the server
  //is bound the default port
  remus::client::Client c(this->Connection);

  remus::common::MeshIOType meshTypes( (remus::meshtypes::PiecewiseLinearComplex()),
                                       (remus::meshtypes::Mesh3D()) );

  //if the server supports volume meshes, we go forward and add an item
  //to MesherList that is the mesher name, and set custom UserRole data
  //to be the mesher requirements
  if(c.canMesh(meshTypes))
    {
    remus::proto::JobRequirementsSet possibleWorkers =
                                            c.retrieveRequirements( meshTypes );

    remus::proto::JobRequirementsSet::const_iterator i;
    for(i = possibleWorkers.begin(); i != possibleWorkers.end(); ++i)
      {
      QListWidgetItem* item = new QListWidgetItem(
                                       QString::fromStdString(i->workerName()),
                                       dialogUI.MesherList);
      item->setData( Qt::UserRole, QVariant::fromValue(*i) );
      }
    }

  connect( dialogUI.MesherList, SIGNAL(itemClicked ( QListWidgetItem *)),
           this, SLOT(mesherChanged( QListWidgetItem *)) );

  //http://qt-project.org/doc/qt-4.8/signalsandslots.html
  //If several slots are connected to one signal, the slots will be executed
  //one after the other, in the order they have been connected, when the
  //signal is emitted.
  //So don't change the order of the following two connections or double click
  //to select a mesher will break
  connect( dialogUI.MesherList, SIGNAL(itemDoubleClicked ( QListWidgetItem *)),
           this, SLOT(mesherChanged( QListWidgetItem *)) );
  connect( dialogUI.MesherList, SIGNAL(itemDoubleClicked ( QListWidgetItem *)),
           this, SLOT(accept( )) );
  //Done with setting up double click
}

//-----------------------------------------------------------------------------
bool qtRemusVolumeMesherSelector::chooseMesher()
{
  const int result = this->exec();
  QDialog::DialogCode result_code = static_cast<QDialog::DialogCode>(result);
  return (result_code == QDialog::Accepted);
}

//-----------------------------------------------------------------------------
void qtRemusVolumeMesherSelector::mesherChanged(  QListWidgetItem * item )
{
  this->ActiveMesher = item->text();
  this->ActiveMesherData = item->data( Qt::UserRole ).value<
                                          remus::proto::JobRequirements>();
}

//-----------------------------------------------------------------------------
bool qtRemusVolumeMesherSelector::useLegacyMesher() const
{
  return this->ActiveMesher == DefaultMesherName();
}
