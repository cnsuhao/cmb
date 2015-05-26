//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME SimBuilderCustomExportDialog - Options for exporting CMB simulation file.
// .SECTION Description
// Dialog is customized by input smtk::attribute::System
// .SECTION Caveats


#ifndef __SimBuilderCustomExportDialog_h
#define __SimBuilderCustomExportDialog_h

#include "cmbSystemConfig.h"
#include "pqSimBuilderUIManager.h"

#include "smtk/PublicPointerDefs.h"

#include <string>
#include <QObject>
#include <QPointer>

#include <set>
#include <string>

class QButtonGroup;
class QDialog;
class QFrame;
class QWidget;
class pqServer;

class SimBuilderCustomExportDialog : public QObject
{
  Q_OBJECT

public:
  SimBuilderCustomExportDialog();
  virtual ~SimBuilderCustomExportDialog();

  QWidget *contentWidget() const
  { return this->ContentWidget; }
  smtk::attribute::SystemPtr exportAttSystem(bool baseline=false) const;
  void setExportAttSystem(smtk::attribute::SystemPtr system);
  void setSimAttSystem(smtk::attribute::SystemPtr system);

  void setActiveServer(pqServer* server);
  int exec();
  std::string getPythonScript(bool warnIfMissing=false) const;

protected slots:
  void analysisSelected();
  void multipleSelectChanged(int state);

protected:
  void updatePanel();
  void updateAnalysisTypesWidget();
  smtk::attribute::FileItemPtr
    getPythonScriptItem(bool warnIfMissing=false) const;
  std::string getPythonScriptPath(smtk::attribute::FileItemPtr fileItem,
    bool warnIfMissing=false) const;
  smtk::attribute::ItemPtr getExportSpecItem(const std::string& name,
    bool warnIfMissing=false) const;

private:

  int Status;
  QDialog *MainDialog;
  QFrame *AnalysisTypesContainer;
  QFrame *AnalysisTypesWidget;
  QWidget *ContentWidget;
  QButtonGroup *AnalysisButtonGroup;
  QPointer<pqServer> ActiveServer;
  smtk::attribute::SystemPtr SimAttSystem;
  smtk::attribute::SystemPtr ExportAttSystem;
  QPointer<pqSimBuilderUIManager> ExportUIManager;
  // Indicates if ExportPanel has been updated to current inputs
  bool IsPanelSet;
  // Indicates if multiple selection of analyses is enabled
  bool IsMultipleSelect;
  std::set<std::string> SelectedAnalyses;
};

#endif /* __SimBuilderCustomExportDialog_h */
