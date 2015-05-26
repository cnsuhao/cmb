//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtRemusVolumeMesherSubmitter
// .SECTION Description
// Client side display of remus jobs and the interface to fill out the
// requirements of said jobs. After displaying the requirements it will
// send them over to the the server for submission as a remus job
// .SECTION Caveats

#ifndef __qtRemusVolumeMesherSubmitter_h
#define __qtRemusVolumeMesherSubmitter_h

#include <QDialog>
#include <QVariant>

//Don't let QMOC see remus headers that include boost headers
//or bad things happen
#ifndef Q_MOC_RUN
  #include <remus/client/ServerConnection.h>
  #include <remus/proto/Job.h>
  #include <remus/proto/JobRequirements.h>
#endif

class pqCMBModel;

class qtRemusVolumeMesherSubmitter : public QDialog
{
  Q_OBJECT
public:
  qtRemusVolumeMesherSubmitter( QString endpoint,
                      QWidget* parent );

  remus::proto::Job submitRequirements(
                           pqCMBModel* model,
                           const QString& modelFilePath,
                           const remus::proto::JobRequirements& reqs );

private:
  remus::client::ServerConnection Connection;
  pqCMBModel* Model;
};

#endif
