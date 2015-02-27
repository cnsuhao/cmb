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
#include "qtCMBSceneSurfaceMesherDialog.h"
#include "ui_qtCMBSceneSurfaceMesherDialog.h"

#include "pqCMBSceneTree.h"
#include <QPushButton>

//-----------------------------------------------------------------------------
qtCMBSceneSurfaceMesherDialog::qtCMBSceneSurfaceMesherDialog(pqCMBSceneTree *tree,
                                                   QWidget *parent,
                                                   Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->Tree = tree;
  this->InternalWidget = new Ui::qtCMBSceneSurfaceMesherDialog;
  this->InternalWidget->setupUi(this);
  QObject::connect(this->InternalWidget->SurfaceList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(surfaceSelectionChanged()));
}

//-----------------------------------------------------------------------------
qtCMBSceneSurfaceMesherDialog::~qtCMBSceneSurfaceMesherDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::insertSurfaceName(int i, const char *vname)
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
void qtCMBSceneSurfaceMesherDialog::setSelectedSurfaceNames(QList<int> &currentIndices)
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
void qtCMBSceneSurfaceMesherDialog::getSelectedSurfaceNames(QStringList &selectedNames) const
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
int qtCMBSceneSurfaceMesherDialog::getNumberOfSurfaceNames() const
{
  return this->InternalWidget->SurfaceList->count();
}

//-----------------------------------------------------------------------------
void qtCMBSceneSurfaceMesherDialog::insertVOIName(int i, const char *vname)
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
  this->InternalWidget->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
    this->InternalWidget->SurfaceList->selectedItems().count() > 0 );
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
