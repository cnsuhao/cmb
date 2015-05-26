//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBAboutDialog - The about dialog for CMB.
// .SECTION Description
// .SECTION Caveats
#ifndef _qtCMBAboutDialog_h
#define _qtCMBAboutDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui { class qtAboutDialog; }

class QPixmap;

/// Provides an about dialog
class CMBAPPCOMMON_EXPORT qtCMBAboutDialog : public QDialog
{
  Q_OBJECT

public:
  qtCMBAboutDialog(QWidget* Parent);
  virtual ~qtCMBAboutDialog();

  void setVersionText(const QString& versionText);
  void setPixmap(const QPixmap& pixMap ) ;

private:
  qtCMBAboutDialog(const qtCMBAboutDialog&);
  qtCMBAboutDialog& operator=(const qtCMBAboutDialog&);

  Ui::qtAboutDialog* const Ui;
};

#endif // !_qtCMBAboutDialog_h
