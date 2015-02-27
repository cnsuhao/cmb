/*=========================================================================

  Program:   CMB
  Module:    vtkQtCMBConnectComboBox.h

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
