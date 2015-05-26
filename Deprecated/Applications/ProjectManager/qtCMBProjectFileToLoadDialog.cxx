//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBProjectFileToLoadDialog.h"
#include "ui_qtProjectFileToLoadDialog.h"

#include "pqFileDialogModel.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QSortFilterProxyModel>


//-----------------------------------------------------------------------------
qtCMBProjectFileToLoadDialog::qtCMBProjectFileToLoadDialog(
 QString const& /*progName*/, QRegExp const& progExt,
 pqFileDialogModel *model, QWidget* parent)
  :
  Superclass(parent),
  fileToLoad(""),
  notCanceled(true),
  Model(model),
  Ui(new Ui::qtProjectFileToLoadDialog())
{
  this->Ui->setupUi(this);

  //Update the dialog to have the name of the program that this dialog is for.
  QString temp = this->Ui->MsgLabel->text();
  temp.replace(QString("<>"),this->programName);
  this->Ui->MsgLabel->setText(temp);

  //set up the view to look at the pqFileDialogModel with
  //all the files from the server
  this->Filter = new QSortFilterProxyModel(this);
  this->Filter->setSourceModel(this->Model);
  this->Ui->ListView->setModel(this->Filter);


  //lets set up the filter
  this->Filter->setFilterRegExp(progExt);


  this->NewFileBtn = this->Ui->buttonBox->addButton("&New File",QDialogButtonBox::YesRole);
  QObject::connect(this->NewFileBtn,SIGNAL(clicked()),this,SLOT(newFile()));

  this->Ui->buttonBox->addButton(QDialogButtonBox::Cancel);

  QObject::connect(this->Ui->buttonBox,SIGNAL(rejected()),this,SLOT(cancelDialog()));
  QObject::connect(this->Ui->buttonBox,SIGNAL(accepted()),this,SLOT(openFile()));

  QObject::connect(this->Ui->ListView,
                   SIGNAL(doubleClicked(const QModelIndex&)),
                   this,
                   SLOT(openFile(const QModelIndex&)));
}

//-----------------------------------------------------------------------------
qtCMBProjectFileToLoadDialog::~qtCMBProjectFileToLoadDialog()
{

}

//-----------------------------------------------------------------------------
void qtCMBProjectFileToLoadDialog::newFile()
{
  this->fileToLoad ="";
  this->notCanceled = true;
  this->accept();
}
//-----------------------------------------------------------------------------
void qtCMBProjectFileToLoadDialog::cancelDialog()
{
  this->notCanceled = false;
}

//-----------------------------------------------------------------------------
void qtCMBProjectFileToLoadDialog::openFile()
{
  if (this->Ui->ListView->selectionModel()->hasSelection())
    {
    QModelIndex index =
      this->Ui->ListView->selectionModel()->currentIndex();
    this->openFile(index);
    }
}
//-----------------------------------------------------------------------------
void qtCMBProjectFileToLoadDialog::openFile(const QModelIndex& index)
{
  QModelIndex si = this->Filter->mapToSource( index );
  this->fileToLoad = this->Model->getFilePaths(si).at(0);
  this->notCanceled = true;
  this->accept();
}
