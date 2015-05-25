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

#include "qtCMBAboutDialog.h"

#include "ui_qtAboutDialog.h"
#include "pqCMBAppCommonConfig.h"

#include <QHeaderView>
#include <QPixmap>
#include "vtksys/ios/sstream"

//-----------------------------------------------------------------------------
qtCMBAboutDialog::qtCMBAboutDialog(QWidget* Parent) :
  QDialog(Parent),
  Ui(new Ui::qtAboutDialog())
{
  this->Ui->setupUi(this);
  this->setObjectName("qtCMBAboutDialog");
  this->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(CMB_VERSION_FULL));
}

void qtCMBAboutDialog::setVersionText(const QString& versionText)
{
  this->Ui->VersionLabel->setText(versionText);
}

//-----------------------------------------------------------------------------
void qtCMBAboutDialog::setPixmap(const QPixmap& pixMap )
{
  this->Ui->label->setPixmap(pixMap);
}

//-----------------------------------------------------------------------------
qtCMBAboutDialog::~qtCMBAboutDialog()
{
  delete this->Ui;
}
