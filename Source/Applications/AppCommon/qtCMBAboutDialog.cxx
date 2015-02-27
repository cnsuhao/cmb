/*=========================================================================

  Program:   CMB
  Module:    qtCMBAboutDialog.cxx

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
