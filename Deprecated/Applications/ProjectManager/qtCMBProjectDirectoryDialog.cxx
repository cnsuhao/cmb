//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBProjectDirectoryDialog.h"
#include "ui_qtProjectDirectoryDialog.h"

#include "pqApplicationCore.h"
#include "pqFileDialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QString>


//-----------------------------------------------------------------------------
qtCMBProjectDirectoryDialog::qtCMBProjectDirectoryDialog(
  QString const& progName, QString const& dir, QWidget* parentObject)
  :
  Superclass(parentObject),
  programName(progName),
  defaultDir(dir),
  selectedDirectory(dir),
  Ui(new Ui::qtProjectDirectoryDialog())
{
  this->Ui->setupUi(this);
  this->updateDialogWithProgramName();

  //setup the project directory
  QObject::connect(this->Ui->dirButton,SIGNAL(clicked()),this,SLOT(open()));
}

//-----------------------------------------------------------------------------
qtCMBProjectDirectoryDialog::~qtCMBProjectDirectoryDialog()
{

}

//-----------------------------------------------------------------------------
void qtCMBProjectDirectoryDialog::updateDialogWithProgramName()
{
  //Update the dialog to have the name of the program that this dialog is for.
  QString temp = this->Ui->MsgLabel->text();
  temp.replace(QString("<>"),this->programName);
  this->Ui->MsgLabel->setText(temp);

  temp = this->Ui->dirLabel->text();
  temp.replace(QString("<>"),this->programName);
  this->Ui->dirLabel->setText(temp);

  //setup the default directory into the label
  this->Ui->dirLineEdit->setText(this->defaultDir);
}


//-----------------------------------------------------------------------------
void qtCMBProjectDirectoryDialog::open()
{
  //spawn a pqFileDialog
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
    this,"Select Project Folder",this->defaultDir);
  dialog.setFileMode(pqFileDialog::Directory);
  if (dialog.exec() == QDialog::Accepted)
    {
    this->selectedDirectory = dialog.getSelectedFiles()[0];

    //update the line endit with the new folder
    this->Ui->dirLineEdit->setText(this->selectedDirectory);
    }
}