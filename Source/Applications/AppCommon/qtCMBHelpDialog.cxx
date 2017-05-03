//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBHelpDialog.h"
#include "ui_qtHelpDialog.h"

#include <QFile>
#include <QScrollBar>
#include <QTextStream>

qtCMBHelpDialog::qtCMBHelpDialog(const char* helpFileResource, QWidget* Parent)
  : QDialog(Parent)
  , Ui(new Ui::qtHelpDialog())
{
  this->Ui->setupUi(this);
  this->setWindowTitle(QApplication::translate("HelpDialog", "Help (Release Notes)", 0
#if QT_VERSION < 0x050000
    ,
    QApplication::UnicodeUTF8
#endif
    ));
  this->setObjectName("qtCMBHelpDialog");
  this->Ui->textBrowser->clear();

  QFile inputFile(helpFileResource);
  inputFile.open(QIODevice::ReadOnly);
  QTextStream in(&inputFile);
  QString line = in.readAll();
  inputFile.close();

  this->Ui->textBrowser->append(line);
}

qtCMBHelpDialog::~qtCMBHelpDialog()
{
  delete this->Ui;
}

void qtCMBHelpDialog::setToTop()
{
  this->Ui->textBrowser->verticalScrollBar()->setSliderPosition(
    this->Ui->textBrowser->verticalScrollBar()->minimum());
}
