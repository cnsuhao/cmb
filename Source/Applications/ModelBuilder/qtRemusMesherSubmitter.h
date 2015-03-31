/*=========================================================================

  Program:   CMB
  Module:    qtRemusVolumeMesherSubmitter.h

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

#ifndef Q_MOC_RUN
# include <remus/client/ServerConnection.h>
# include <remus/proto/Job.h>
# include <remus/proto/JobRequirements.h>
#endif

class pqCMBModel;
class ModelManager;

class qtRemusVolumeMesherSubmitter : public QDialog
{
  Q_OBJECT
public:
  qtRemusVolumeMesherSubmitter( QString endpoint,
                      QWidget* parent );

  remus::proto::Job submitRequirements(ModelManager *smtkManager,
                                       const remus::proto::JobRequirements& reqs );

private:
  remus::client::ServerConnection Connection;
  pqCMBModel* Model;
};

#endif
