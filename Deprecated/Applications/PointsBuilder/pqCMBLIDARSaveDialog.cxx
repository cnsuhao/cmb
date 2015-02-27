/*=========================================================================

  Program:   CMB
  Module:    pqCMBLIDARSaveDialog.cxx

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
// .NAME Represents a dialog for texturing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "pqCMBLIDARSaveDialog.h"

#include "ui_qtSaveScatterData.h"
#include <pqFileDialog.h>
#include <QFileInfo>
#include <QLineEdit>

//-----------------------------------------------------------------------------
int pqCMBLIDARSaveDialog::getFile(QWidget *parent, pqServer *server,
                             bool enableSavePieces,
                             QString *name,
                             bool *saveAsSinglePiece,
                             bool *loadAsDisplayed)
{
  pqCMBLIDARSaveDialog dialog(parent,server,enableSavePieces);
  int status =  dialog.exec();
  if (status)
    {
    *name = dialog.SaveDialog->fileNameEntry->text();
    *saveAsSinglePiece = dialog.SaveDialog->saveAsSinglePiece->isChecked();
    *loadAsDisplayed = dialog.SaveDialog->saveAsDisplayed->isChecked();
    }
  return status;
}

//-----------------------------------------------------------------------------
pqCMBLIDARSaveDialog::pqCMBLIDARSaveDialog(QWidget *parent, pqServer *server,
                             bool enableSavePieces) :
  Status(-1), Server(server)
{
  this->MainDialog = new QDialog(parent);
  this->SaveDialog = new Ui::qtSaveScatterData;
  this->SaveDialog->setupUi(MainDialog);

  //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(this->SaveDialog->fileBrowserButton, SIGNAL(clicked()),
                   this, SLOT(displayFileBrowser()));

  if (!enableSavePieces)
    {
    this->SaveDialog->saveAsSinglePiece->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
pqCMBLIDARSaveDialog::~pqCMBLIDARSaveDialog()
{
  if (this->SaveDialog)
    {
    delete SaveDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
int pqCMBLIDARSaveDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARSaveDialog::accept()
{
  this->Status = 1;
  this->FileName = this->SaveDialog->fileNameEntry->text();
  this->SaveAsSinglePiece = this->SaveDialog->saveAsSinglePiece->isChecked();
  this->SaveAsDisplayed = this->SaveDialog->saveAsDisplayed->isChecked();
}
//-----------------------------------------------------------------------------
void pqCMBLIDARSaveDialog::cancel()
{
  this->Status = 0;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARSaveDialog::filesSelected(const QStringList &files)
{
  if (files.size() == 0)
    {
    return;
    }

  this->SaveDialog->fileNameEntry->setText(files[0]);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARSaveDialog::displayFileBrowser()
{
  QString filters = "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);;";

  pqFileDialog file_dialog(this->Server,
                           this->MainDialog,
                           tr("Save File:"),
                           QString(),
                           filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  int retCode = file_dialog.exec();
  if(retCode == QDialog::Accepted)
    {
    this->filesSelected(file_dialog.getSelectedFiles());
    }
}

//-----------------------------------------------------------------------------
