/*=========================================================================

  Program:   CMB
  Module:    qtCMBLIDARFilterDialog.h

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
// .NAME qtCMBLIDARFilterDialog - The about dialog for CMB.
// .SECTION Description
// .SECTION Caveats
#ifndef _qtCMBLIDARFilterDialog_h
#define _qtCMBLIDARFilterDialog_h

#include <QDialog>

#include "vtkSMSourceProxy.h"
#include "cmbSystemConfig.h"

class pqServer;

namespace Ui
{
  class qtLIDARFilterDialog;
};

/// Provides an about dialog
class qtCMBLIDARFilterDialog : public QDialog
{
  Q_OBJECT

public:
  qtCMBLIDARFilterDialog(QWidget *parent = NULL);
  virtual ~qtCMBLIDARFilterDialog();

  //sets the thresholdSource's values to dialog's values
  void UpdateThresholdSource(vtkSMSourceProxy* thresholdSource);

  //sets dialog's values to the thresholdSource's values
  void UpdateFilterDialog(vtkSMSourceProxy* threshouldSource);

  //blocks signals before changing information
  void blockAllChildrenSignals(bool block);

//  void setVersionText(const QString& versionText);
//  void setPixmap(const QPixmap& pixMap ) ;

signals:
  void OkPressed();

private slots:
  void OnOk();
  void DialogChanged();
  void CheckUseMinX();
  void CheckUseMinY();
  void CheckUseMinZ();
  void CheckUseMaxX();
  void CheckUseMaxY();
  void CheckUseMaxZ();
  void CheckUseMinRGB();
  void CheckUseMaxRGB();
protected:
  Ui::qtLIDARFilterDialog *InternalWidget;

private:
  qtCMBLIDARFilterDialog(const qtCMBLIDARFilterDialog&);
  qtCMBLIDARFilterDialog& operator=(const qtCMBLIDARFilterDialog&);
;
};

#endif // !_qtCMBLIDARFilterDialog_h
