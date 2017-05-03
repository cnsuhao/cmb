//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef qtCMBManualFunctionWidget_h_
#define qtCMBManualFunctionWidget_h_

#include "pqGeneralTransferFunctionWidget.h"
#include <QPointer>
#include <QWidget>

class Ui_qtCMBManualFunctionWidget;
class cmbManualProfileFunction;
class QGridLayout;

class qtCMBManualFunctionWidget : public QWidget
{
  Q_OBJECT
public:
  qtCMBManualFunctionWidget(cmbManualProfileFunction* fun, QWidget* parent);
  ~qtCMBManualFunctionWidget() override;

public slots:
  void setSemetricMode(bool);
  void updateDepthMax(double);
  void updateDepthMin(double);
  void updateDistMax(double);
  void updateDistMin(double);
  void dispSplineBox(bool);
  void weightSplineBox(bool);
  void render();

signals:
  void changeDisplacementFunctionType(bool);

protected:
  Ui_qtCMBManualFunctionWidget* Ui;
  cmbManualProfileFunction* function;
  QPointer<pqGeneralTransferFunctionWidget> DisplacementProfile;
  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;
};

#endif
