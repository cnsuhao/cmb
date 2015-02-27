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
#include "qtCMBArcModifierInputDialog.h"
#include "ui_qtCMBArcModifierInputDialog.h"

#include "pqFileDialog.h"
#include "pqCMBSceneTree.h"
#include <QPushButton>

//-----------------------------------------------------------------------------
qtCMBArcModifierInputDialog::qtCMBArcModifierInputDialog(pqCMBSceneTree *tree,
                                                   QWidget *parent,
                                                   Qt::WindowFlags flags)
: QDialog(parent, flags)
{
  this->Tree = tree;
  this->InternalWidget = new Ui::qtCMBArcModifierInputDialog;
  this->InternalWidget->setupUi(this);
  this->InternalWidget->label->setVisible(false);
  this->InternalWidget->ArcList->setVisible(false);
  QObject::connect(this->InternalWidget->SurfaceList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(selectedSourceChanged()));
  QObject::connect(this->InternalWidget->DataTypeGroup, SIGNAL(buttonClicked(int)),
                   this, SIGNAL(sourceTypeChanged()));
}

//-----------------------------------------------------------------------------
qtCMBArcModifierInputDialog::~qtCMBArcModifierInputDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void qtCMBArcModifierInputDialog::insertSourceName(int i, const char *vname)
{
  this->InternalWidget->SurfaceList->insertItem(i, vname);
}

//-----------------------------------------------------------------------------
void qtCMBArcModifierInputDialog::removeAllSourceNames()
{
  this->InternalWidget->SurfaceList->blockSignals(true);
  this->InternalWidget->SurfaceList->clear();
  this->InternalWidget->SurfaceList->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBArcModifierInputDialog::setSelectedSourceNames(QList<int> &currentIndices)
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

//-----------------------------------------------------------------------------
void qtCMBArcModifierInputDialog::getSelectedSourceNames(QStringList &selectedNames) const
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
int qtCMBArcModifierInputDialog::getNumberOfSourceNames() const
{
  return this->InternalWidget->SurfaceList->count();
}

//-----------------------------------------------------------------------------
void qtCMBArcModifierInputDialog::selectedSourceChanged()
{
  this->InternalWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                                                                            this->InternalWidget->SurfaceList->selectedItems().count() > 0 );

  QStringList surfaceNames;
  this->getSelectedSourceNames(surfaceNames);
}

void qtCMBArcModifierInputDialog::insertArcName(int i, const char *vname)
{

}

void qtCMBArcModifierInputDialog::removeAllArcNames()
{

}

void qtCMBArcModifierInputDialog::setSelectedArcNames(QList<int> &currentIndices)
{

}

void qtCMBArcModifierInputDialog::getSelectedArcNames(QStringList &selectedNames) const
{

}

pqCMBSceneObjectBase::enumObjectType qtCMBArcModifierInputDialog::GetSourceType()
{
  if(this->InternalWidget->Points->isChecked())
  {
    return pqCMBSceneObjectBase::Points;
  }
  else if(this->InternalWidget->Polygons->isChecked())
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
