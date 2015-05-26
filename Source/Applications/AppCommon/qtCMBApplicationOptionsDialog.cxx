//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBApplicationOptionsDialog.h"

//-----------------------------------------------------------------------------
qtCMBApplicationOptionsDialog::qtCMBApplicationOptionsDialog(
  QWidget* p) : qtCMBOptionsDialog(p)
{
  this->setWindowTitle("Application Settings");
  this->setApplyNeeded(true);
}
