//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBSceneSurfaceMesherDialog.h"
#include "ui_qtCMBSceneSurfaceMesherDialog.h"

#include "pqCMBSceneTree.h"
#include <QPushButton>

//-----------------------------------------------------------------------------
qtCMBSceneSurfaceMesherDialog::qtCMBSceneSurfaceMesherDialog(
  pqCMBSceneTree* tree, QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->Tree = tree;
  this->InternalWidget = new Ui::qtCMBSceneSurfaceMesherDialog;
  this->InternalWidget->setupUi(this);
  QObject::connect(this->InternalWidget->SurfaceList, SIGNAL(itemSelectionChanged()), this,
    SLOT(surfaceSelectionChanged()));
}

//-----------------------------------------------------------------------------
qtCMBSceneSurfaceMesherDialog::~qtCMBSceneSurfaceMesherDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::insertSurfaceName(int i, const char* vname)
{
  this->InternalWidget->SurfaceList->insertItem(i, vname);
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::removeAllSurfaceNames()
{
  this->InternalWidget->SurfaceList->blockSignals(true);
  this->InternalWidget->SurfaceList->clear();
  this->InternalWidget->SurfaceList->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::setSelectedSurfaceNames(QList<int>& currentIndices)
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
void qtCMBSceneSurfaceMesherDialog::getSelectedSurfaceNames(QStringList& selectedNames) const
{
  selectedNames.clear();
  const QList<QListWidgetItem*>& selectedItems = this->InternalWidget->SurfaceList->selectedItems();
  QList<QListWidgetItem*>::const_iterator iter = selectedItems.begin();
  for (; iter != selectedItems.end(); iter++)
  {
    selectedNames.push_back((*iter)->text());
  }
}

//-----------------------------------------------------------------------------
int qtCMBSceneSurfaceMesherDialog::getNumberOfSurfaceNames() const
{
  return this->InternalWidget->SurfaceList->count();
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::insertVOIName(int i, const char* vname)
{
  this->InternalWidget->VOI->insertItem(i, vname);
}
//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::removeVOIName(int i)
{
  this->InternalWidget->VOI->removeItem(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::removeAllVOINames()
{
  this->InternalWidget->VOI->clear();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneSurfaceMesherDialog::getVOIName(int i) const
{
  return this->InternalWidget->VOI->itemText(i);
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::setCurrentVOINameIndex(int i)
{
  this->InternalWidget->VOI->setCurrentIndex(i);
}

//-----------------------------------------------------------------------------
int qtCMBSceneSurfaceMesherDialog::getCurrentVOINameIndex() const
{
  return this->InternalWidget->VOI->currentIndex();
}

//-----------------------------------------------------------------------------
QString qtCMBSceneSurfaceMesherDialog::getCurrentVOIName() const
{
  return this->InternalWidget->VOI->currentText();
}

//-----------------------------------------------------------------------------
int qtCMBSceneSurfaceMesherDialog::getNumberOfVOINames() const
{
  return this->InternalWidget->VOI->count();
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::surfaceSelectionChanged()
{
  this->InternalWidget->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(this->InternalWidget->SurfaceList->selectedItems().count() > 0);
}

//-----------------------------------------------------------------------------
double qtCMBSceneSurfaceMesherDialog::getElevationWeightRadius()
{
  return this->InternalWidget->ElevationWeightRadius->value();
}

//-----------------------------------------------------------------------------
bool qtCMBSceneSurfaceMesherDialog::getMeshVisibleArcSets()
{
  return this->InternalWidget->MeshVisibleArcSets->isChecked();
}
