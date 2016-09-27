//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  ~qtCMBLIDARFilterDialog() override;

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
