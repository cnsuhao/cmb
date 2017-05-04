//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBArcModifierInputDialog.h"
#include "ui_qtCMBArcModifierInputDialog.h"

#include "pqCMBSceneTree.h"
#include "pqFileDialog.h"
#include <QPushButton>

qtCMBArcModifierInputDialog::qtCMBArcModifierInputDialog(
  pqCMBSceneTree* tree, QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->Tree = tree;
  this->InternalWidget = new Ui::qtCMBArcModifierInputDialog;
  this->InternalWidget->setupUi(this);
  this->InternalWidget->label->setVisible(false);
  this->InternalWidget->ArcList->setVisible(false);
  QObject::connect(this->InternalWidget->SurfaceList, SIGNAL(itemSelectionChanged()), this,
    SLOT(selectedSourceChanged()));
  QObject::connect(this->InternalWidget->DataTypeGroup, SIGNAL(buttonClicked(int)), this,
    SIGNAL(sourceTypeChanged()));
}

qtCMBArcModifierInputDialog::~qtCMBArcModifierInputDialog()
{
  delete this->InternalWidget;
}

void qtCMBArcModifierInputDialog::insertSourceName(int i, const char* vname)
{
  this->InternalWidget->SurfaceList->insertItem(i, vname);
}

void qtCMBArcModifierInputDialog::removeAllSourceNames()
{
  this->InternalWidget->SurfaceList->blockSignals(true);
  this->InternalWidget->SurfaceList->clear();
  this->InternalWidget->SurfaceList->blockSignals(false);
}

void qtCMBArcModifierInputDialog::setSelectedSourceNames(QList<int>& currentIndices)
{
  QListIterator<int> listIter(currentIndices);
  this->InternalWidget->SurfaceList->blockSignals(true);
  while (listIter.hasNext())
  {
    this->InternalWidget->SurfaceList->item(listIter.next())->setSelected(true);
  }
  this->InternalWidget->SurfaceList->blockSignals(false);
  this->selectedSourceChanged();
}

void qtCMBArcModifierInputDialog::getSelectedSourceNames(QStringList& selectedNames) const
{
  selectedNames.clear();
  const QList<QListWidgetItem*>& selectedItems = this->InternalWidget->SurfaceList->selectedItems();
  QList<QListWidgetItem*>::const_iterator iter = selectedItems.begin();
  for (; iter != selectedItems.end(); iter++)
  {
    selectedNames.push_back((*iter)->text());
  }
}

int qtCMBArcModifierInputDialog::getNumberOfSourceNames() const
{
  return this->InternalWidget->SurfaceList->count();
}

void qtCMBArcModifierInputDialog::selectedSourceChanged()
{
  this->InternalWidget->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(this->InternalWidget->SurfaceList->selectedItems().count() > 0);

  QStringList surfaceNames;
  this->getSelectedSourceNames(surfaceNames);
}

void qtCMBArcModifierInputDialog::insertArcName(int vtkNotUsed(i), const char* vtkNotUsed(vname))
{
}

void qtCMBArcModifierInputDialog::removeAllArcNames()
{
}

void qtCMBArcModifierInputDialog::setSelectedArcNames(QList<int>& vtkNotUsed(currentIndices))
{
}

void qtCMBArcModifierInputDialog::getSelectedArcNames(QStringList& vtkNotUsed(selectedNames)) const
{
}

pqCMBSceneObjectBase::enumObjectType qtCMBArcModifierInputDialog::GetSourceType()
{
  if (this->InternalWidget->Points->isChecked())
  {
    return pqCMBSceneObjectBase::Points;
  }
  else if (this->InternalWidget->Polygons->isChecked())
  {
    return pqCMBSceneObjectBase::Polygon;
  }
  return pqCMBSceneObjectBase::Faceted;
}

int qtCMBArcModifierInputDialog::getNumberOfArcNames() const
{
  return 0;
}

void qtCMBArcModifierInputDialog::setUseNormal(bool b)
{
  this->InternalWidget->UseNormal->setChecked(b);
}

bool qtCMBArcModifierInputDialog::getUseNormal() const
{
  return this->InternalWidget->UseNormal->isChecked();
}
