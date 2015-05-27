//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBSceneMesherDialog.h"
#include "ui_qtCMBSceneMesherDialog.h"

#include "pqFileDialog.h"
#include "pqCMBSceneTree.h"
#include <QPushButton>

//-----------------------------------------------------------------------------
qtCMBSceneMesherDialog::qtCMBSceneMesherDialog(pqCMBSceneTree *tree,
                                                   QWidget *parent,
                                                   Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->Tree = tree;
  this->InternalWidget = new Ui::qtCMBSceneMesherDialog;
  this->InternalWidget->setupUi(this);
  QObject::connect(this->InternalWidget->Mesher, SIGNAL(currentIndexChanged(int)),
                   this, SIGNAL(mesherSelectionChanged(int)));
  QObject::connect(this->InternalWidget->SurfaceList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(surfaceSelectionChanged()));
  QObject::connect(this->InternalWidget->BrowseNewPtsFile, SIGNAL(clicked()),
                   this, SLOT(displayFileBrowser()));
}

//-----------------------------------------------------------------------------
qtCMBSceneMesherDialog::~qtCMBSceneMesherDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::insertMesherPath(int i, const char *mpath)
{
  this->InternalWidget->Mesher->insertItem(i, mpath);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::removeMesherPath(int i)
{
  this->InternalWidget->Mesher->removeItem(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::removeAllMesherPaths()
{
  this->InternalWidget->Mesher->clear();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getMesherPath(int i) const
{
  return this->InternalWidget->Mesher->itemText(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setCurrentMesherPathIndex(int i)
{
  this->InternalWidget->Mesher->setCurrentIndex(i);
}
//-----------------------------------------------------------------------------
int qtCMBSceneMesherDialog::getCurrentMesherPathIndex() const
{
  return this->InternalWidget->Mesher->currentIndex();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getCurrentMesherPath() const
{
  return this->InternalWidget->Mesher->currentText();
}

//-----------------------------------------------------------------------------
int qtCMBSceneMesherDialog::getNumberOfMesherPaths() const
{
  return this->InternalWidget->Mesher->count();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::insertSurfaceName(int i, const char *vname)
{
  this->InternalWidget->SurfaceList->insertItem(i, vname);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::removeAllSurfaceNames()
{
  this->InternalWidget->SurfaceList->blockSignals(true);
  this->InternalWidget->SurfaceList->clear();
  this->InternalWidget->SurfaceList->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setSelectedSurfaceNames(QList<int> &currentIndices)
{
  QListIterator<int> listIter(currentIndices);
  this->InternalWidget->SurfaceList->blockSignals(true);
  while (listIter.hasNext())
    {
    this->InternalWidget->SurfaceList->item(listIter.next())->setSelected(true);
    }
  this->InternalWidget->SurfaceList->blockSignals(false);
  this->surfaceSelectionChanged();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::getSelectedSurfaceNames(QStringList &selectedNames) const
{
  selectedNames.clear();
  const QList<QListWidgetItem*> &selectedItems = this->InternalWidget->SurfaceList->selectedItems();
  QList<QListWidgetItem*>::const_iterator iter = selectedItems.begin();
  for (; iter != selectedItems.end(); iter++)
    {
    selectedNames.push_back( (*iter)->text() );
    }
}

//-----------------------------------------------------------------------------
int qtCMBSceneMesherDialog::getNumberOfSurfaceNames() const
{
  return this->InternalWidget->SurfaceList->count();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::insertVOIName(int i, const char *vname)
{
  this->InternalWidget->VOI->insertItem(i, vname);
}
//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::removeVOIName(int i)
{
  this->InternalWidget->VOI->removeItem(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::removeAllVOINames()
{
  this->InternalWidget->VOI->clear();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getVOIName(int i) const
{
  return this->InternalWidget->VOI->itemText(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setCurrentVOINameIndex(int i)
{
  this->InternalWidget->VOI->setCurrentIndex(i);
}

//-----------------------------------------------------------------------------
int qtCMBSceneMesherDialog::getCurrentVOINameIndex() const
{
  return this->InternalWidget->VOI->currentIndex();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getCurrentVOIName() const
{
  return this->InternalWidget->VOI->currentText();
}

//-----------------------------------------------------------------------------
int qtCMBSceneMesherDialog::getNumberOfVOINames() const
{
  return this->InternalWidget->VOI->count();
}

//-----------------------------------------------------------------------------
void  qtCMBSceneMesherDialog::setMeshLength(double c, bool isRelative)
{
  this->InternalWidget->MeshLength->setValue(c);
  if (isRelative)
    {
    this->InternalWidget->MeshLengthType->setCurrentIndex(1);
    }
  else
    {
    this->InternalWidget->MeshLengthType->setCurrentIndex(0);
    }


}
//-----------------------------------------------------------------------------
double qtCMBSceneMesherDialog::getMeshLength(bool &isRelative) const
{
  isRelative = (this->InternalWidget->MeshLengthType->currentIndex() == 1);
  return this->InternalWidget->MeshLength->value();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setInterpolatingRadius(double c)
{
  this->InternalWidget->InterpolatingRadius->setValue(c);
}

//-----------------------------------------------------------------------------
double qtCMBSceneMesherDialog::getInterpolatingRadius() const
{
  return this->InternalWidget->InterpolatingRadius->value();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::surfaceSelectionChanged()
{
  this->InternalWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
    this->InternalWidget->SurfaceList->selectedItems().count() > 0 );

  QStringList surfaceNames;
  this->getSelectedSurfaceNames(surfaceNames);
  bool enableState = this->Tree->IsTemporaryPtsFileForMesherNeeded(surfaceNames);
  this->InternalWidget->DeleteCreatedPtsFile->setEnabled( enableState );
  this->InternalWidget->BrowseNewPtsFile->setEnabled( enableState );
  this->InternalWidget->TemporaryPtsFileName->setEnabled( enableState );
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::displayFileBrowser()
{
  QString filters = "LIDAR Pts (*.pts *.bin *.bin.pts)";

  pqFileDialog file_dialog(this->Tree->getCurrentServer(),
                                                     this,
                                                     tr("Open File:"),
                                                     QString(),
                                                     filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("NewPtsFileDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  file_dialog.setWindowModality(Qt::WindowModal);
  QObject::connect(&file_dialog, SIGNAL(filesSelected(const QStringList&)),
    this, SLOT(filesSelected(const QStringList&)));
  file_dialog.exec();
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::filesSelected(const QStringList &files)
{
  if (files.size() == 0)
    {
    return;
    }

  this->InternalWidget->TemporaryPtsFileName->setText(files[0]);
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setTemporaryPtsFileName(const char *name)
{
  this->InternalWidget->TemporaryPtsFileName->setText(name);
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getTemporaryPtsFileName()
{
  return this->InternalWidget->TemporaryPtsFileName->text();
}

//-----------------------------------------------------------------------------
bool qtCMBSceneMesherDialog::getDeleteCreatedPtsFile()
{
  return this->InternalWidget->DeleteCreatedPtsFile->isEnabled() &&
    this->InternalWidget->DeleteCreatedPtsFile->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
void qtCMBSceneMesherDialog::setOmicronBaseName(const char *name)
{
  this->InternalWidget->OmicronBaseName->setText(name);
}

//-----------------------------------------------------------------------------
QString qtCMBSceneMesherDialog::getOmicronBaseName()
{
  return this->InternalWidget->OmicronBaseName->text();
}

//-----------------------------------------------------------------------------
