//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBLIDARFilterDialog - Filter dialog for the LIDAR tool.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBLIDARFilterDialog.h"
#include "pqSMAdaptor.h"
#include "ui_qtFilterDialog.h"

//-----------------------------------------------------------------------------
qtCMBLIDARFilterDialog::qtCMBLIDARFilterDialog(QWidget* parent)
  : QDialog(parent)
{
  this->InternalWidget = new Ui::qtLIDARFilterDialog;
  this->InternalWidget->setupUi(this);

  QObject::connect(this->InternalWidget->addFilterButton, SIGNAL(accepted()), this, SLOT(OnOk()));

  QObject::connect(
    this->InternalWidget->minX, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->minZ, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->minY, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->maxX, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->maxZ, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->maxY, SIGNAL(valueChanged(double)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->minX, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMinX()));
  QObject::connect(
    this->InternalWidget->minZ, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMinZ()));
  QObject::connect(
    this->InternalWidget->minY, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMinY()));

  QObject::connect(
    this->InternalWidget->maxX, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMaxX()));
  QObject::connect(
    this->InternalWidget->maxZ, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMaxZ()));
  QObject::connect(
    this->InternalWidget->maxY, SIGNAL(valueChanged(double)), this, SLOT(CheckUseMaxY()));

  QObject::connect(
    this->InternalWidget->useMinX, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->useMinZ, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->useMinY, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->useMaxX, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->useMaxZ, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->useMaxY, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->minR, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->minG, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->minB, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->maxR, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->maxG, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->maxB, SIGNAL(valueChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->minR, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMinRGB()));
  QObject::connect(
    this->InternalWidget->minG, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMinRGB()));
  QObject::connect(
    this->InternalWidget->minB, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMinRGB()));

  QObject::connect(
    this->InternalWidget->maxR, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMaxRGB()));
  QObject::connect(
    this->InternalWidget->maxG, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMaxRGB()));
  QObject::connect(
    this->InternalWidget->maxB, SIGNAL(valueChanged(int)), this, SLOT(CheckUseMaxRGB()));

  QObject::connect(
    this->InternalWidget->useMaxRGB, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));
  QObject::connect(
    this->InternalWidget->useMinRGB, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->invert, SIGNAL(stateChanged(int)), this, SLOT(DialogChanged()));

  QObject::connect(
    this->InternalWidget->previewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnOk()));
  this->setModal(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::blockAllChildrenSignals(bool block)
{
  this->InternalWidget->minX->blockSignals(block);
  this->InternalWidget->minZ->blockSignals(block);
  this->InternalWidget->minY->blockSignals(block);
  this->InternalWidget->maxX->blockSignals(block);
  this->InternalWidget->maxZ->blockSignals(block);
  this->InternalWidget->maxY->blockSignals(block);
  this->InternalWidget->minX->blockSignals(block);
  this->InternalWidget->minZ->blockSignals(block);
  this->InternalWidget->minY->blockSignals(block);
  this->InternalWidget->maxX->blockSignals(block);
  this->InternalWidget->maxZ->blockSignals(block);
  this->InternalWidget->maxY->blockSignals(block);
  this->InternalWidget->useMinX->blockSignals(block);
  this->InternalWidget->useMinZ->blockSignals(block);
  this->InternalWidget->useMinY->blockSignals(block);
  this->InternalWidget->useMaxX->blockSignals(block);
  this->InternalWidget->useMaxZ->blockSignals(block);
  this->InternalWidget->useMaxY->blockSignals(block);
  this->InternalWidget->minR->blockSignals(block);
  this->InternalWidget->minG->blockSignals(block);
  this->InternalWidget->minB->blockSignals(block);
  this->InternalWidget->maxR->blockSignals(block);
  this->InternalWidget->maxG->blockSignals(block);
  this->InternalWidget->maxB->blockSignals(block);
  this->InternalWidget->minR->blockSignals(block);
  this->InternalWidget->minG->blockSignals(block);
  this->InternalWidget->minB->blockSignals(block);
  this->InternalWidget->maxR->blockSignals(block);
  this->InternalWidget->maxG->blockSignals(block);
  this->InternalWidget->maxB->blockSignals(block);
  this->InternalWidget->useMaxRGB->blockSignals(block);
  this->InternalWidget->useMinRGB->blockSignals(block);
  this->InternalWidget->invert->blockSignals(block);
  this->InternalWidget->previewCheckBox->blockSignals(block);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::OnOk()
{
  emit OkPressed();
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::DialogChanged()
{
  if (this->InternalWidget->previewCheckBox->isChecked())
  {
    emit OkPressed();
  }
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::UpdateThresholdSource(vtkSMSourceProxy* thresholdSource)
{

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMinX"), this->InternalWidget->useMinX->isChecked());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMinY"), this->InternalWidget->useMinY->isChecked());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMinZ"), this->InternalWidget->useMinZ->isChecked());

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMaxX"), this->InternalWidget->useMaxX->isChecked());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMaxY"), this->InternalWidget->useMaxY->isChecked());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMaxZ"), this->InternalWidget->useMaxZ->isChecked());

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMinRGB"), this->InternalWidget->useMinRGB->isChecked());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetUseMaxRGB"), this->InternalWidget->useMaxRGB->isChecked());

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetInvert"), this->InternalWidget->invert->isChecked());

  QList<QVariant> minColorList;
  minColorList.push_back(this->InternalWidget->minR->value());
  minColorList.push_back(this->InternalWidget->minG->value());
  minColorList.push_back(this->InternalWidget->minB->value());
  pqSMAdaptor::setMultipleElementProperty(thresholdSource->GetProperty("SetMinRGB"), minColorList);

  QList<QVariant> maxColorList;
  maxColorList.push_back(this->InternalWidget->maxR->value());
  maxColorList.push_back(this->InternalWidget->maxG->value());
  maxColorList.push_back(this->InternalWidget->maxB->value());
  pqSMAdaptor::setMultipleElementProperty(thresholdSource->GetProperty("SetMaxRGB"), maxColorList);

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMinX"), this->InternalWidget->minX->value());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMinY"), this->InternalWidget->minY->value());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMinZ"), this->InternalWidget->minZ->value());

  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMaxX"), this->InternalWidget->maxX->value());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMaxY"), this->InternalWidget->maxY->value());
  pqSMAdaptor::setElementProperty(
    thresholdSource->GetProperty("SetMaxZ"), this->InternalWidget->maxZ->value());

  thresholdSource->InvokeCommand("SetUseMinX");
  thresholdSource->InvokeCommand("SetUseMinY");
  thresholdSource->InvokeCommand("SetUseMinZ");
  thresholdSource->InvokeCommand("SetUseMaxX");
  thresholdSource->InvokeCommand("SetUseMaxY");
  thresholdSource->InvokeCommand("SetUseMaxZ");
  thresholdSource->InvokeCommand("SetUseMinRGB");
  thresholdSource->InvokeCommand("SetUseMaxRGB");

  thresholdSource->InvokeCommand("SetInvert");
  thresholdSource->InvokeCommand("SetMinColorBounds");
  thresholdSource->InvokeCommand("SetMaxColorBounds");
  thresholdSource->InvokeCommand("SetMinX");
  thresholdSource->InvokeCommand("SetMinY");
  thresholdSource->InvokeCommand("SetMinZ");
  thresholdSource->InvokeCommand("SetMaxX");
  thresholdSource->InvokeCommand("SetMaxY");
  thresholdSource->InvokeCommand("SetMaxZ");
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::UpdateFilterDialog(vtkSMSourceProxy* thresholdSource)
{
  if (!thresholdSource)
  {
    return;
  }
  thresholdSource->UpdatePropertyInformation();
  this->blockAllChildrenSignals(true);
  this->InternalWidget->minR->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinR")).toInt());
  this->InternalWidget->minG->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinG")).toInt());
  this->InternalWidget->minB->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinB")).toInt());

  this->InternalWidget->maxR->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxR")).toInt());
  this->InternalWidget->maxG->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxG")).toInt());
  this->InternalWidget->maxB->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxB")).toInt());

  this->InternalWidget->minX->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinX")).toDouble());
  this->InternalWidget->minY->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinY")).toDouble());
  this->InternalWidget->minZ->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMinZ")).toDouble());

  this->InternalWidget->maxX->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxX")).toDouble());
  this->InternalWidget->maxY->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxY")).toDouble());
  this->InternalWidget->maxZ->setValue(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetMaxZ")).toDouble());

  this->InternalWidget->useMinX->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMinX")).toBool());
  this->InternalWidget->useMinY->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMinY")).toBool());
  this->InternalWidget->useMinZ->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMinZ")).toBool());

  this->InternalWidget->useMaxX->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMaxX")).toBool());
  this->InternalWidget->useMaxY->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMaxY")).toBool());
  this->InternalWidget->useMaxZ->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMaxZ")).toBool());

  this->InternalWidget->useMinRGB->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMinRGB")).toBool());
  this->InternalWidget->useMaxRGB->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetUseMaxRGB")).toBool());

  this->InternalWidget->invert->setChecked(
    pqSMAdaptor::getElementProperty(thresholdSource->GetProperty("GetInvert")).toBool());
  this->blockAllChildrenSignals(false);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMinX()
{
  this->InternalWidget->useMinX->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMinY()
{
  this->InternalWidget->useMinY->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMinZ()
{
  this->InternalWidget->useMinZ->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMaxX()
{
  this->InternalWidget->useMaxX->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMaxY()
{
  this->InternalWidget->useMaxY->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMaxZ()
{
  this->InternalWidget->useMaxZ->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMinRGB()
{
  this->InternalWidget->useMinRGB->setChecked(true);
}
//-----------------------------------------------------------------------------
void qtCMBLIDARFilterDialog::CheckUseMaxRGB()
{
  this->InternalWidget->useMaxRGB->setChecked(true);
}
//-----------------------------------------------------------------------------

qtCMBLIDARFilterDialog::~qtCMBLIDARFilterDialog()
{
  if (this->InternalWidget)
  {
    delete this->InternalWidget;
  }
}
