/*=========================================================================

   Program: CMB
   Module:    qtCMBHelpDialog.cxx

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
   ========================================================================*/
#include "qtCMBHelpDialog.h"
#include "ui_qtHelpDialog.h"

#include <QFile>
#include <QTextStream>
#include <QScrollBar>

//-----------------------------------------------------------------------------
qtCMBHelpDialog::qtCMBHelpDialog(const char *helpFileResource, QWidget* Parent) :
  QDialog(Parent),
  Ui(new Ui::qtHelpDialog())
{
  this->Ui->setupUi(this);
  this->setWindowTitle(QApplication::translate("HelpDialog",
                                               "Help (Release Notes)",
                                               0, QApplication::UnicodeUTF8));
  this->setObjectName("qtCMBHelpDialog");
  this->Ui->textBrowser->clear();

  QFile inputFile(helpFileResource);
  inputFile.open(QIODevice::ReadOnly);
  QTextStream in(&inputFile);
  QString line = in.readAll();
  inputFile.close();

  this->Ui->textBrowser->append(line);
}

//-----------------------------------------------------------------------------
qtCMBHelpDialog::~qtCMBHelpDialog()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
void qtCMBHelpDialog::setToTop()
  {
  this->Ui->textBrowser->verticalScrollBar()->setSliderPosition(
    this->Ui->textBrowser->verticalScrollBar()->minimum() );
  }
