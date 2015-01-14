/*=========================================================================

  Program:   CMB
  Module:    SimBuilderEMSExportDialog.h

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
// .NAME SimBuilderEMSExportDialog - Options for exporting EMS simulation file.
// .SECTION Description
// .SECTION Caveats


#ifndef __SimBuilderEMSExportDialog_h
#define __SimBuilderEMSExportDialog_h

#include <QObject>
#include <QPointer>

#include "smtk/PublicPointerDefs.h"
#include "cmbSystemConfig.h"

class QDialog;
class pqServer;

namespace Ui
{
  class  qtSimBuilderEMSExportDialog;
};

class SimBuilderEMSExportDialog : public QObject
{
  Q_OBJECT

public:
  SimBuilderEMSExportDialog();
  virtual ~SimBuilderEMSExportDialog();

  void setModelManager(smtk::model::ManagerPtr modelMgr);
  void setAttSystem(smtk::attribute::SystemPtr system);

  QString getFileName() const;
  QString getPythonScriptName() const;
  QString getAnalysisName() const;

  void setActiveServer(pqServer* server);
  int exec();

protected slots:
  void accept();
  void cancel();
  void displayFileBrowser();
  void displayPythonScriptBrowser();
  void pythonScriptChanged();
  void analysisSelected();
  void validate();

protected:
  void setAcceptable(bool state);

private:

  int Status;
  Ui::qtSimBuilderEMSExportDialog *Dialog;
  QDialog *MainDialog;
  QPointer<pqServer> ActiveServer;
  smtk::model::ManagerPtr qtModelManager;
  smtk::attribute::SystemPtr AttSystem;
  static QString LastPythonScriptParsed;
  static QString SelectedAnalysis;
};

#endif /* __SimBuilderEMSExportDialog_h */
