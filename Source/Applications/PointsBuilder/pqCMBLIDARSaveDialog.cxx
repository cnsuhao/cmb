//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for texturing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "pqCMBLIDARSaveDialog.h"

#include "ui_qtSaveScatterData.h"
#include <QFileInfo>
#include <QLineEdit>
#include <pqFileDialog.h>

//-----------------------------------------------------------------------------
int pqCMBLIDARSaveDialog::getFile(QWidget* parent, pqServer* server, bool enableSavePieces,
  QString* name, bool* saveAsSinglePiece, bool* loadAsDisplayed)
{
  pqCMBLIDARSaveDialog dialog(parent, server, enableSavePieces);
  int status = dialog.exec();
  if (status)
  {
    *name = dialog.SaveDialog->fileNameEntry->text();
    *saveAsSinglePiece = dialog.SaveDialog->saveAsSinglePiece->isChecked();
    *loadAsDisplayed = dialog.SaveDialog->saveAsDisplayed->isChecked();
  }
  return status;
}

//-----------------------------------------------------------------------------
pqCMBLIDARSaveDialog::pqCMBLIDARSaveDialog(QWidget* parent, pqServer* server, bool enableSavePieces)
  : Status(-1)
  , Server(server)
{
  this->MainDialog = new QDialog(parent);
  this->SaveDialog = new Ui::qtSaveScatterData;
  this->SaveDialog->setupUi(MainDialog);

  //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(
    this->SaveDialog->fileBrowserButton, SIGNAL(clicked()), this, SLOT(displayFileBrowser()));

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
void pqCMBLIDARSaveDialog::filesSelected(const QStringList& files)
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
  QString filters =
    "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);; DEM (*.dem);;";

  pqFileDialog file_dialog(this->Server, this->MainDialog, tr("Save File:"), QString(), filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  int retCode = file_dialog.exec();
  if (retCode == QDialog::Accepted)
  {
    this->filesSelected(file_dialog.getSelectedFiles());
  }
}

//-----------------------------------------------------------------------------
