//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _qtCMBApplicationOptionsDialog_h
#define _qtCMBApplicationOptionsDialog_h


#include "cmbAppCommonExport.h"
#include "qtCMBOptionsDialog.h"
#include "cmbSystemConfig.h"

/// qtCMBApplicationOptionsDialog dialog class that allows editing of application
/// wide settings.
class CMBAPPCOMMON_EXPORT qtCMBApplicationOptionsDialog : public qtCMBOptionsDialog
{
  Q_OBJECT
public:
  qtCMBApplicationOptionsDialog(QWidget *parent=0);

protected slots:

private:
  Q_DISABLE_COPY(qtCMBApplicationOptionsDialog)
};

#endif
