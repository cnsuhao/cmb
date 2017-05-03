//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBTINStitcherDialog.h"
#include "ui_qtCMBTINStitcherDialog.h"

//-----------------------------------------------------------------------------
qtCMBTINStitcherDialog::qtCMBTINStitcherDialog(QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->InternalWidget = new Ui::qtCMBTINStitcherDialog;
  this->InternalWidget->setupUi(this);

  QObject::connect(this->InternalWidget->UserSpecifiedTINType, SIGNAL(currentIndexChanged(int)),
    this, SLOT(tinTypeChanged()));
  QObject::connect(
    this->InternalWidget->UseQuads, SIGNAL(stateChanged(int)), this, SLOT(useQuadsChanged()));
  QObject::connect(this->InternalWidget->AllowInteriorPointInsertion, SIGNAL(stateChanged(int)),
    this, SLOT(allowPointInsertionChanged()));
}

//-----------------------------------------------------------------------------
qtCMBTINStitcherDialog::~qtCMBTINStitcherDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::setMinimumAngle(double angle)
{
  this->InternalWidget->MinimumAngle->setValue(angle);
}

//-----------------------------------------------------------------------------
double qtCMBTINStitcherDialog::getMinimumAngle() const
{
  return this->InternalWidget->MinimumAngle->value();
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::setUseQuads(bool useQuads)
{
  this->InternalWidget->UseQuads->setCheckState(useQuads ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
bool qtCMBTINStitcherDialog::getUseQuads() const
{
  return this->InternalWidget->UseQuads->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::setUserSpecifiedTINType(int tinType)
{
  if (tinType < 0)
  {
    tinType = 0;
  }
  else if (tinType > 1)
  {
    tinType = 1;
  }
  this->InternalWidget->UserSpecifiedTINType->setCurrentIndex(tinType);
}

//-----------------------------------------------------------------------------
int qtCMBTINStitcherDialog::getUserSpecifiedTINType() const
{
  return this->InternalWidget->UserSpecifiedTINType->currentIndex();
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::setAllowInteriorPointInsertion(bool allowInteriorPointInsertion)
{
  this->InternalWidget->AllowInteriorPointInsertion->setCheckState(
    allowInteriorPointInsertion ? Qt::Checked : Qt::Unchecked);
}

//-----------------------------------------------------------------------------
bool qtCMBTINStitcherDialog::getAllowInteriorPointInsertion() const
{
  return this->InternalWidget->AllowInteriorPointInsertion->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::setTolerance(double tolerance)
{
  this->InternalWidget->Tolerance->setValue(tolerance);
}

//-----------------------------------------------------------------------------
double qtCMBTINStitcherDialog::getTolerance() const
{
  return this->InternalWidget->Tolerance->value();
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::tinTypeChanged()
{
  if (this->getUserSpecifiedTINType() == 1 && this->getUseQuads())
  {
    this->blockSignals(true);
    this->InternalWidget->AllowInteriorPointInsertion->setCheckState(Qt::Unchecked);
    this->blockSignals(false);
    this->InternalWidget->AllowInteriorPointInsertion->setEnabled(false);
    this->InternalWidget->Tolerance->setEnabled(false);
    this->InternalWidget->MinimumAngle->setEnabled(false);
  }
  else
  {
    this->InternalWidget->AllowInteriorPointInsertion->setEnabled(true);
    if (this->getUserSpecifiedTINType() == 0)
    {
      this->InternalWidget->Tolerance->setEnabled(true);
    }
    else if (!this->getAllowInteriorPointInsertion())
    {
      this->InternalWidget->Tolerance->setEnabled(false);
    }
  }
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::useQuadsChanged()
{
  if (this->getUserSpecifiedTINType() == 1 && this->getUseQuads())
  {
    this->InternalWidget->AllowInteriorPointInsertion->setCheckState(Qt::Unchecked);
    this->InternalWidget->AllowInteriorPointInsertion->setEnabled(false);
  }
  else
  {
    this->InternalWidget->AllowInteriorPointInsertion->setEnabled(true);
  }
}

//-----------------------------------------------------------------------------
void qtCMBTINStitcherDialog::allowPointInsertionChanged()
{
  if (this->getAllowInteriorPointInsertion())
  {
    this->InternalWidget->MinimumAngle->setEnabled(true);
    this->InternalWidget->Tolerance->setEnabled(true);
  }
  else
  {
    this->InternalWidget->MinimumAngle->setEnabled(false);
    if (this->getUserSpecifiedTINType() == 1)
    {
      this->InternalWidget->Tolerance->setEnabled(false);
    }
    else
    {
      this->InternalWidget->Tolerance->setEnabled(true);
    }
  }
}
