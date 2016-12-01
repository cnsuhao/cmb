//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBMeshReaderPanel - Custom object panel for vtkCMBMeshReader
// .SECTION Description
// Custom adjustments to the automatically generated object panel for the
// vtkCMBMeshReader
#include "pqPropertyWidget.h"
#include "cmbSystemConfig.h"

class pqCMBMeshReaderPanel : public pqPropertyWidget
{
  Q_OBJECT
public:
  pqCMBMeshReaderPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p=0);

protected slots:
  void updateMaterialControls(int state);
};
