//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for importing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBNewSceneUnitsDialog.h"

#include "ui_qtNewSceneUnits.h"
#include "pqCMBSceneObjectBase.h"

//-----------------------------------------------------------------------------
bool qtCMBNewSceneUnitsDialog::getUnits(cmbSceneUnits::Enum initialUnits,
                                           cmbSceneUnits::Enum &newUnits)
{
  qtCMBNewSceneUnitsDialog dialog(initialUnits);
  return dialog.exec(newUnits);
}

//-----------------------------------------------------------------------------
qtCMBNewSceneUnitsDialog::qtCMBNewSceneUnitsDialog(cmbSceneUnits::Enum initialUnits)
{
  this->NewUnits = cmbSceneUnits::Unknown;
  this->Status = false;
  MainDialog = new QDialog();

  this->NewUnitsDialog = new Ui::qtNewSceneUnitsDialog;
  this->NewUnitsDialog->setupUi(MainDialog);

  switch (initialUnits)
    {
    case cmbSceneUnits::inches:
      this->NewUnitsDialog->INButton->setChecked(true);
      break;
    case cmbSceneUnits::feet:
      this->NewUnitsDialog->FTButton->setChecked(true);
      break;
    case cmbSceneUnits::mm:
      this->NewUnitsDialog->MMButton->setChecked(true);
      break;
    case cmbSceneUnits::cm:
      this->NewUnitsDialog->CMButton->setChecked(true);
      break;
    case cmbSceneUnits::m:
      this->NewUnitsDialog->MButton->setChecked(true);
      break;
    case cmbSceneUnits::km:
      this->NewUnitsDialog->KMButton->setChecked(true);
      break;
    default:
      this->NewUnitsDialog->UnknownButton->setChecked(true);
    }
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

//-----------------------------------------------------------------------------
qtCMBNewSceneUnitsDialog::~qtCMBNewSceneUnitsDialog()
{
  if (this->NewUnitsDialog)
    {
    delete NewUnitsDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
bool qtCMBNewSceneUnitsDialog::exec(cmbSceneUnits::Enum &newUnits)
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  newUnits = this->NewUnits;
  return this->Status;
}
//-----------------------------------------------------------------------------
void qtCMBNewSceneUnitsDialog::accept()
{
  if (this->NewUnitsDialog->INButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::inches;
    }
  else if (this->NewUnitsDialog->FTButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::feet;
    }
  else if (this->NewUnitsDialog->MMButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::mm;
    }
  else if (this->NewUnitsDialog->CMButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::cm;
    }
  else if (this->NewUnitsDialog->MButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::m;
    }
  else if (this->NewUnitsDialog->KMButton->isChecked())
    {
    this->NewUnits = cmbSceneUnits::km;
    }
  this->Status = true;
}
//-----------------------------------------------------------------------------
void qtCMBNewSceneUnitsDialog::cancel()
{
}

//-----------------------------------------------------------------------------
