//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkQtCMBConnectComboBox - a combobox which will emit a signal on dropdown.
// .SECTION Description
// .SECTION Caveats

#ifndef _vtkQtCMBConnectComboBox_h
#define _vtkQtCMBConnectComboBox_h

#include "cmbAppCommonExport.h"
#include <QComboBox>
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

class vtkObject;
class vtkEventQtSlotConnect;

class CMBAPPCOMMON_EXPORT vtkQtCMBConnectComboBox : public QComboBox
{
  typedef QComboBox Superclass;
  Q_OBJECT

public:

  vtkQtCMBConnectComboBox(QWidget* p = NULL);
  ~vtkQtCMBConnectComboBox();

  virtual void showPopup();
  virtual void setVTKConnectObject(vtkObject* obj, unsigned long evt);

signals:
  void startingPopup();
  void vtkObjectEventInvoked();

public slots:
  void onVTKEventInvoked();

protected:
  vtkSmartPointer<vtkEventQtSlotConnect> VTKUserEventConnect;

};

#endif // !_vtkQtCMBConnectComboBox_h
