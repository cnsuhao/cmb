/*=========================================================================
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
#include "pqCMBLegacyMesherDialog.h"
#include "ui_qtCMBLegacyMesherDialog.h"

#include "pqFileDialog.h"
#include "pqFileDialogModel.h"

#include <QDir>
#include <QFileInfo>

namespace
{
  QString find_volume_mesher(QString name, pqServer* server)
  {
#ifdef _WIN32
    name += ".exe";
#endif

    QDir root_dir(QApplication::applicationDirPath());

    QString file = root_dir.absoluteFilePath(name);
    pqFileDialogModel fModel(server);

    QString fullPath; //not needed by this code, just required by signature
    if (fModel.fileExists(file,fullPath))
      {
      return file;
      }

    // Mac install
    QDir temp_dir(root_dir);
    temp_dir.cdUp();
    temp_dir.cd("bin");
    file = temp_dir.absoluteFilePath(name);
    if (fModel.fileExists(file,fullPath))
      {
      return file;
      }

    // Mac build
    temp_dir = QDir(root_dir);
    temp_dir.cdUp();
    temp_dir.cdUp();
    temp_dir.cdUp();
    file = temp_dir.absoluteFilePath(name);
    if (fModel.fileExists(file,fullPath))
      {
      return file;
      }

  return QString();
  }
}

//-----------------------------------------------------------------------------
pqCMBLegacyMesherDialog::pqCMBLegacyMesherDialog(pqServer* server,
                                         double modelBounds[6],
                                         QWidget *parent,
                                         Qt::WindowFlags flags)
  : QDialog(parent, flags),
    InternalWidget(new Ui::qtCMBMesherDialog),
    CurrentServer(server),
    ModelBounds(),
    DefaultVolumeMesher( find_volume_mesher("mesh", server) ),
    LastMesherPathIndex(0)
{
  for(int i=0; i < 6; ++i)
    { this->ModelBounds[i]=modelBounds[i]; }


  this->InternalWidget->setupUi(this);
  QObject::connect(this->InternalWidget->Mesher, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(mesherSelectionChanged(int)));
  QObject::connect(this->InternalWidget->FileBrowserButton, SIGNAL(clicked()),
                   this, SLOT(displayFileBrowser()));
  QObject::connect(this->InternalWidget->AdvanceOptions, SIGNAL(clicked(bool)),
                   this, SLOT(displayAdvanceOptions(bool)));
  // Make the advance panel invisible
  this->InternalWidget->AdvanceFrame->setVisible(false);

  // Prep the volume constraint line edit
  QDoubleValidator *validator = new QDoubleValidator(parent);
  validator->setBottom(0.0);
  this->InternalWidget->MeshLength->setValidator( validator );

  this->setMeshLength(0.1, true);

  if (this->DefaultVolumeMesher.size() == 0)
    {
    // internal mesher NOT an option
    this->InternalWidget->Mesher->removeItem(0);
    }

  this->LastMesherPathIndex = 0;
}

//-----------------------------------------------------------------------------
pqCMBLegacyMesherDialog::~pqCMBLegacyMesherDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::displayAdvanceOptions(bool flag)
{
  this->InternalWidget->AdvanceFrame->setVisible(flag);
}

//-----------------------------------------------------------------------------
void  pqCMBLegacyMesherDialog::setMeshLength(double c, bool isRelative)
{
  QString s;
  s.setNum(c);
  this->InternalWidget->MeshLength->setText(s);
  this->InternalWidget->MeshLengthType->setCurrentIndex(isRelative?1:0);
}
//-----------------------------------------------------------------------------
double pqCMBLegacyMesherDialog::getMeshLength(bool &isRelative) const
{
  isRelative = (this->InternalWidget->MeshLengthType->currentIndex() == 1);
  QString s = this->InternalWidget->MeshLength->text();
  return s.toDouble();
}

//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::filesSelected(const QStringList &files)
{
  if (files.size() == 0)
    {
    return;
    }

  this->InternalWidget->FileNameText->setText(files[0]);
}
//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::displayFileBrowser()
{
  QString filters = "Omicron Input File (*.dat);;All files (*)";

  pqFileDialog file_dialog(this->CurrentServer,
                           this,
                           tr("Save File:"),
                           QString(),
                           filters);

  //file_dialog.setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("FileImportDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  QObject::connect(&file_dialog, SIGNAL(filesSelected(const QStringList&)),
    this, SLOT(filesSelected(const QStringList&)));
  file_dialog.exec();
}

//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::setFileName(QString name)
{
  this->InternalWidget->FileNameText->setText(name);
}

//-----------------------------------------------------------------------------
QString pqCMBLegacyMesherDialog::getFileName() const
{
  return  this->InternalWidget->FileNameText->text();
}

//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::setModelBounds(const double bounds[6])
{
  int i;
  for (i = 0; i < 6; i++)
    {
    this->ModelBounds[i] = bounds[i];
    }
}
//-----------------------------------------------------------------------------
double pqCMBLegacyMesherDialog::getMinModelBounds() const
{
  double l[3], minL = 0.01;
  int i;
  l[0] = this->ModelBounds[1] - this->ModelBounds[0];
  l[1] = this->ModelBounds[3] - this->ModelBounds[2];
  l[2] = this->ModelBounds[5] - this->ModelBounds[4];
  bool minSet = false;
  for (i = 0; i < 3; i++)
    {
    if (l[i] < 1.0e-10)
      {
      continue;
      }
    if (!minSet)
      {
      minSet = true;
      minL = l[i];
      continue;
      }
    if (minL > l[i])
      {
      minL = l[i];
      }
    }
  return minL;
}

//-----------------------------------------------------------------------------
void pqCMBLegacyMesherDialog::mesherSelectionChanged(int mesherIndex)
{
  if (this->InternalWidget->Mesher->itemText(mesherIndex) == "Choose Mesher")
    {
    QString filters = "All files (*);;Windows Executable (*.exe)";
    pqFileDialog file_dialog(this->CurrentServer,
                             this,
                             tr("Select surface mesher"),
                             QString(),
                             filters);
    file_dialog.setObjectName("FileOpenDialog");
    file_dialog.setFileMode(pqFileDialog::ExistingFile);

    QObject::connect(&file_dialog, SIGNAL(filesSelected(const QList<QStringList> &)),
                      this, SLOT(selectVolumeMesher(const QList<QStringList> &)));

    //reset to previous selected index before we execute, that way if the
    //user cancels we have the previous selected mesher as the default
    this->InternalWidget->Mesher->setCurrentIndex( this->LastMesherPathIndex );
    file_dialog.exec();
    }
}

//-----------------------------------------------------------------------------
bool pqCMBLegacyMesherDialog::selectVolumeMesher(const QList<QStringList> &files)
{
  if (files.size() == 0 || files[0].size() == 0)
    {
    return false;
    }
  QString newMesher = files[0][0];

  int pos = this->InternalWidget->Mesher->findText(newMesher);
  if(pos >= 0)
    {
    this->InternalWidget->Mesher->setCurrentIndex(pos);
    return true;
    }

  //add an item to the combo box
  const int size = this->InternalWidget->Mesher->count();
  this->blockSignals(true);
  this->InternalWidget->Mesher->insertItem( size - 1, newMesher );
  this->InternalWidget->Mesher->setCurrentIndex( size - 2 );
  this->LastMesherPathIndex = size - 2;
  this->blockSignals(false);
  return true;
}

//-----------------------------------------------------------------------------
QString pqCMBLegacyMesherDialog::getActiveMesher() const
{
  QString activeMesher = this->DefaultVolumeMesher;
  if(this->InternalWidget->Mesher->currentText() != "Internal Mesher")
    {
    activeMesher = this->InternalWidget->Mesher->currentText();
    }
  return activeMesher;
}

//-----------------------------------------------------------------------------
QString pqCMBLegacyMesherDialog::getTetGenOptions() const
{
  QString cmds("pzVMAa");
  QString val;
  double length;
  bool isRelative;
  // lets get the length and convert it to a volume constraint
  length = this->getMeshLength(isRelative);
  // if the length is relative convert it to absolute
  if (isRelative)
    {
    double mlength = this->getMinModelBounds();
    length *= mlength;
    }
  double volumeConstraint = length*length*length/6.0;
  val.setNum(volumeConstraint);
  cmds.append(val);
  // Let see if we are overriding the basic options
  if (this->InternalWidget->AdvanceOptions->isChecked())
    {
    if (this->InternalWidget->TetGenCommand->isChecked())
      {
      if (this->InternalWidget->CommandLineOnly->isChecked())
        {
        // Just need to return the tetgen command
        return this->InternalWidget->TetGenCommands->text();
        }
      //Append options
      cmds.append(this->InternalWidget->TetGenCommands->text());
      }
    // Check preservation modes
    if (this->InternalWidget->PreserveExterior->isChecked())
      {
      cmds.append("Y");
      }
    else if (this->InternalWidget->PreserveSurface->isChecked())
      {
      cmds.append("YY");
      }
    }
  else
    {
    cmds.append("YY");
    }
  return cmds;
}
