///=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __qtCMBHelpDialog_h
#define __qtCMBHelpDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui { class qtHelpDialog; }

class CMBAPPCOMMON_EXPORT qtCMBHelpDialog : public QDialog
{
  Q_OBJECT

public:
  qtCMBHelpDialog(const char *helpFileResource, QWidget* Parent);
  ~qtCMBHelpDialog() override;

  void setToTop();

private:
  Ui::qtHelpDialog* const Ui;
};

#endif
