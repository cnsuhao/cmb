//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  smtk::model::ManagerPtr pqCMBModelManager;
  smtk::attribute::SystemPtr AttSystem;
  static QString LastPythonScriptParsed;
  static QString SelectedAnalysis;
};

#endif /* __SimBuilderEMSExportDialog_h */
