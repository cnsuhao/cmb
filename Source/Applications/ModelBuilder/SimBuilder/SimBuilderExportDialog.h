//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME SimBuilderExportDialog - Options for exporting CMB simulation file.
// .SECTION Description
// Dialog is customized by input smtk::attribute::System
// .SECTION Caveats


#ifndef __SimBuilderExportDialog_h
#define __SimBuilderExportDialog_h

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

class SimBuilderExportDialog : public QObject
{
  Q_OBJECT

public:
  SimBuilderExportDialog();
  virtual ~SimBuilderExportDialog();

  QWidget *contentWidget() const
  { return this->ContentWidget; }
  smtk::attribute::SystemPtr exportAttSystem(bool baseline=false) const;
  void setExportAttSystem(smtk::attribute::SystemPtr system);
  void setSimAttSystem(smtk::attribute::SystemPtr system);

  void setActiveServer(pqServer* server);
  int exec();
  std::string getPythonScript() const;

protected slots:
  void analysisSelected();
  void multipleSelectChanged(int state);

protected:
  std::string findPythonScriptPath(
    smtk::attribute::FileItemPtr fileItem,
    bool warnIfMissing = false) const;
  std::string findPythonScriptPath(
    const std::string& name,
    bool warnIfMissing = false) const;
  void updatePanel();
  void updateAnalysisTypesWidget();
  smtk::attribute::FileItemDefinitionPtr getPythonScriptDef(
    const smtk::attribute::SystemPtr attributeSystem,
    bool warnIfMissing=false) const;
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

#endif /* __SimBuilderExportDialog_h */
