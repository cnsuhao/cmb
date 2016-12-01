//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBMeshReaderPanel.h"

#include <QCheckBox>

pqCMBMeshReaderPanel::pqCMBMeshReaderPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p)
: pqPropertyWidget(pxy, p)
{
  // connect actions
  QObject::connect(this->findChild<QCheckBox*>("CreateMeshMaterialIdArray"),
    SIGNAL(stateChanged(int)), this, SLOT(updateMaterialControls(int)));
  // initialize
  updateMaterialControls(this->findChild<QCheckBox*>("CreateMeshMaterialIdArray")->checkState());
}

void pqCMBMeshReaderPanel::updateMaterialControls(int state)
{
  this->findChild<QWidget*>("RenameMaterialAsRegion")->setEnabled(state != 0);
  this->findChild<QWidget*>("_labelForRenameMaterialAsRegion")->setEnabled(state != 0);
}
