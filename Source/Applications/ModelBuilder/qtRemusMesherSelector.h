//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtRemusMesherSelector
// .SECTION Description
// Simply displays all the different meshers that are available to the
// user, as a combo box widget
// .SECTION Caveats

#ifndef __qtRemusMesherSelector_h
#define __qtRemusMesherSelector_h

#include <QWidget>
#include <QPointer>

#ifndef Q_MOC_RUN
# include <remus/client/Client.h>
# include <remus/proto/JobRequirements.h>
#endif

#include "smtk/common/UUID.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

class qtRemusMesherSelector : public QWidget
{
  Q_OBJECT
public:
  qtRemusMesherSelector(smtk::model::ManagerPtr modelManager,
                        const remus::client::ServerConnection& connection,
                        QWidget* parent);

  ~qtRemusMesherSelector();

  smtk::model::Model currentModel() const;

  QString currentMesherName() const;
  remus::proto::JobRequirements currentMesherRequirements() const;

public slots:
  void rebuildModelList();

signals:
  void currentModelChanged( );

  void currentMesherChanged(const std::vector<smtk::model::Model>& models,
                            const QString & workerName,
                            const remus::proto::JobRequirements& reqs);

protected slots:
  void modelChanged( int index );
  void mesherChanged( int index );

private:
  class pqInternal;
  pqInternal* Internal;

  smtk::model::ManagerPtr ModelManager;
  remus::client::Client Client;
};

#endif
