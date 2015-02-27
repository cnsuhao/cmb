/*=========================================================================

  Program:   CMB
  Module:    qtCMBTINStitcherDialog.cxx

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

#include "qtCMBTINStitcherDialog.h"
#include "ui_qtCMBTINStitcherDialog.h"

//-----------------------------------------------------------------------------
qtCMBTINStitcherDialog::qtCMBTINStitcherDialog(QWidget *parent,
                                                             Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->InternalWidget = new Ui::qtCMBTINStitcherDialog;
  this->InternalWidget->setupUi(this);

  QObject::connect(this->InternalWidget->UserSpecifiedTINType,
    SIGNAL(currentIndexChanged(int)), this, SLOT(tinTypeChanged()));
  QObject::connect(this->InternalWidget->UseQuads,
    SIGNAL(stateChanged(int)), this, SLOT(useQuadsChanged()));
  QObject::connect(this->InternalWidget->AllowInteriorPointInsertion,
    SIGNAL(stateChanged(int)), this, SLOT(allowPointInsertionChanged()));
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
  this->InternalWidget->UseQuads->setCheckState(
    useQuads ? Qt::Checked : Qt::Unchecked);
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
void qtCMBTINStitcherDialog::setAllowInteriorPointInsertion(
  bool allowInteriorPointInsertion)
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
    this->InternalWidget->AllowInteriorPointInsertion->
      setCheckState(Qt::Unchecked);
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
    this->InternalWidget->AllowInteriorPointInsertion->
      setCheckState(Qt::Unchecked);
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
