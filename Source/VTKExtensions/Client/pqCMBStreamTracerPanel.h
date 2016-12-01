//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBStreamTracerPanel - Custom object panel for vtkCMBStreamTracer
// .SECTION Description
#include "pqPropertyWidget.h"
#include "cmbSystemConfig.h"

class QFrame;

class pqCMBStreamTracerPanel : public pqPropertyWidget
{
  typedef pqPropertyWidget Superclass;
  Q_OBJECT
public:
  pqCMBStreamTracerPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p=0);

public slots:
  /// accept the changes made to the properties
  /// changes will be propogated down to the server manager
  /// subclasses should only change properties when accept is called to work
  /// properly with undo/redo
  void accept();

protected slots:
  void updateTestLocationsUI();
  void onTestLocationChanged();

protected:
  void SendDouble3Vector(const char *func,
    int index, double *data);

private:
  QFrame* TestLocationsFrame;
  int NumberOfTestLocations;
};
