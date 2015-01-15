/*=========================================================================
Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive, Suite 204,
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
// .NAME SimBuilderCustomExportDialog - Options for exporting CMB simulation file.
// .SECTION Description
// Dialog is customized by input smtk::attribute::System
// .SECTION Caveats


#ifndef __SimBuilderCustomExportDialog_h
#define __SimBuilderCustomExportDialog_h

#include "cmbSystemConfig.h"
#include "pqSMTKUIManager.h"

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
  QPointer<pqSMTKUIManager> ExportUIManager;
  // Indicates if ExportPanel has been updated to current inputs
  bool IsPanelSet;
  // Indicates if multiple selection of analyses is enabled
  bool IsMultipleSelect;
  std::set<std::string> SelectedAnalyses;
};

#endif /* __SimBuilderCustomExportDialog_h */
