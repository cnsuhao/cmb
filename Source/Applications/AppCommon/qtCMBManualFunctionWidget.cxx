//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBManualFunctionWidget.h"
#include "cmbManualProfileFunction.h"
#include "pqCMBModifierArc.h"
#include "ui_qtCMBManualFunctionWidget.h"

qtCMBManualFunctionWidget::qtCMBManualFunctionWidget(cmbManualProfileFunction* fun, QWidget* parent)
  : QWidget(parent)
  , Ui(new Ui_qtCMBManualFunctionWidget)
  , function(fun)
{
  Ui->setupUi(this);
  connect(this->Ui->dispacementMinDepthValue, SIGNAL(valueChanged(double)), this,
    SLOT(updateDepthMin(double)));
  connect(this->Ui->displacementMaxDepthValue, SIGNAL(valueChanged(double)), this,
    SLOT(updateDepthMax(double)));
  connect(this->Ui->leftValue, SIGNAL(valueChanged(double)), this, SLOT(updateDistMin(double)));
  connect(this->Ui->rightValue, SIGNAL(valueChanged(double)), this, SLOT(updateDistMax(double)));
  connect(this->Ui->DisplacementSplineCont, SIGNAL(toggled(bool)), this, SLOT(dispSplineBox(bool)));
  connect(
    this->Ui->WeightingSplineControl, SIGNAL(toggled(bool)), this, SLOT(weightSplineBox(bool)));
  this->Ui->weightingChartFrame->setVisible(false);
  this->Ui->WeightRange->setVisible(false);
  this->Ui->WeightingSplineControl->setVisible(false);

  connect(this->Ui->weightingBox, SIGNAL(toggled(bool)), this->Ui->weightingChartFrame,
    SLOT(setVisible(bool)));
  connect(
    this->Ui->weightingBox, SIGNAL(toggled(bool)), this->Ui->WeightRange, SLOT(setVisible(bool)));
  connect(this->Ui->weightingBox, SIGNAL(toggled(bool)), this->Ui->WeightingSplineControl,
    SLOT(setVisible(bool)));
  {
    QGridLayout* gridlayout = new QGridLayout(this->Ui->displacementChartFrame);
    gridlayout->setMargin(0);
    this->DisplacementProfile = new pqGeneralTransferFunctionWidget();
    gridlayout->addWidget(this->DisplacementProfile);
    this->DisplacementProfile->addFunction(NULL, false);
    connect(this->Ui->DisplacementSplineCont, SIGNAL(toggled(bool)), this->DisplacementProfile,
      SLOT(setFunctionType(bool)));
    connect(this->DisplacementProfile, SIGNAL(controlPointsModified()), this->DisplacementProfile,
      SLOT(render()));
    connect(this->Ui->dispacementMinDepthValue, SIGNAL(valueChanged(double)),
      this->DisplacementProfile, SLOT(setMinY(double)));
    connect(this->Ui->displacementMaxDepthValue, SIGNAL(valueChanged(double)),
      this->DisplacementProfile, SLOT(setMaxY(double)));
    connect(this->Ui->leftValue, SIGNAL(valueChanged(double)), this->DisplacementProfile,
      SLOT(setMinX(double)));
    connect(this->Ui->rightValue, SIGNAL(valueChanged(double)), this->DisplacementProfile,
      SLOT(setMaxX(double)));
  }

  {
    QGridLayout* gridlayout = new QGridLayout(this->Ui->weightingChartFrame);
    gridlayout->setMargin(0);
    this->WeightingFunction = new pqGeneralTransferFunctionWidget();
    gridlayout->addWidget(this->WeightingFunction);
    this->WeightingFunction->addFunction(NULL, false);
    this->WeightingFunction->setMinY(0);
    this->WeightingFunction->setMaxY(1);
    connect(this->Ui->WeightingSplineControl, SIGNAL(toggled(bool)), this->WeightingFunction,
      SLOT(setFunctionType(bool)));
    connect(this->WeightingFunction, SIGNAL(controlPointsModified()), this->WeightingFunction,
      SLOT(render()));
    connect(this->Ui->leftValue, SIGNAL(valueChanged(double)), this->WeightingFunction,
      SLOT(setMinX(double)));
    connect(this->Ui->rightValue, SIGNAL(valueChanged(double)), this->WeightingFunction,
      SLOT(setMaxX(double)));
  }

  this->Ui->dispacementMinDepthValue->setValue(function->getDepthRange(pqCMBModifierArc::MIN));
  this->Ui->displacementMaxDepthValue->setValue(function->getDepthRange(pqCMBModifierArc::MAX));
  this->Ui->leftValue->setValue(function->getDistanceRange(pqCMBModifierArc::MIN));
  this->Ui->rightValue->setValue(function->getDistanceRange(pqCMBModifierArc::MAX));
  bool isSymmetric = function->isSymmetric();
  this->setSemetricMode(isSymmetric);
  this->Ui->Symmetric->setChecked(isSymmetric);
  connect(this->Ui->Symmetric, SIGNAL(clicked(bool)), this, SLOT(setSemetricMode(bool)));

  this->Ui->DisplacementSplineCont->setChecked(function->isDispSpline());
  this->Ui->WeightingSplineControl->setChecked(function->isWeightSpline());

  this->DisplacementProfile->changeFunction(0, function->getDisplacementProfile(), true);
  this->WeightingFunction->changeFunction(0, function->getWeightingFunction(), true);

  this->updateDistMin(function->getDistanceRange(pqCMBModifierArc::MIN));
  this->updateDistMax(function->getDistanceRange(pqCMBModifierArc::MAX));
  this->updateDepthMin(function->getDepthRange(pqCMBModifierArc::MIN));
  this->updateDepthMax(function->getDepthRange(pqCMBModifierArc::MAX));

  this->WeightingFunction->setMinY(0);
  this->WeightingFunction->setMaxY(1);
}

qtCMBManualFunctionWidget::~qtCMBManualFunctionWidget()
{
  delete this->WeightingFunction;
  this->WeightingFunction = NULL;
  delete this->DisplacementProfile;
  this->DisplacementProfile = NULL;
  delete Ui;
}

void qtCMBManualFunctionWidget::dispSplineBox(bool v)
{
  this->function->setDispSpline(v);
  this->DisplacementProfile->changeFunction(0, function->getDisplacementProfile(), true);
  this->DisplacementProfile->setMinX(function->getDistanceRange(pqCMBModifierArc::MIN));
  this->DisplacementProfile->setMaxX(function->getDistanceRange(pqCMBModifierArc::MAX));
  this->DisplacementProfile->setMinY(function->getDepthRange(pqCMBModifierArc::MIN));
  this->DisplacementProfile->setMaxY(function->getDepthRange(pqCMBModifierArc::MAX));
}

void qtCMBManualFunctionWidget::weightSplineBox(bool v)
{
  this->function->setWeightSpline(v);
  this->WeightingFunction->changeFunction(0, function->getWeightingFunction(), true);
  this->WeightingFunction->setMinX(function->getDistanceRange(pqCMBModifierArc::MIN));
  this->WeightingFunction->setMaxX(function->getDistanceRange(pqCMBModifierArc::MAX));
  this->WeightingFunction->setMinY(0);
  this->WeightingFunction->setMaxY(1);
}

void qtCMBManualFunctionWidget::setSemetricMode(bool sm)
{
  if (sm)
  {
    this->Ui->leftValue->setEnabled(false);
  }
  else
  {
    this->Ui->leftValue->setEnabled(true);
  }
  this->function->setSymmetric(sm);
  this->DisplacementProfile->changeFunction(0, function->getDisplacementProfile(), true);
  this->WeightingFunction->changeFunction(0, function->getWeightingFunction(), true);
  this->WeightingFunction->setMinY(0);
  this->WeightingFunction->setMaxY(1);
  this->Ui->dispacementMinDepthValue->setValue(function->getDepthRange(pqCMBModifierArc::MIN));
  this->Ui->displacementMaxDepthValue->setValue(function->getDepthRange(pqCMBModifierArc::MAX));
  this->Ui->leftValue->setValue(function->getDistanceRange(pqCMBModifierArc::MIN));
  this->Ui->rightValue->setValue(function->getDistanceRange(pqCMBModifierArc::MAX));
  this->render();
}

void qtCMBManualFunctionWidget::updateDepthMax(double d)
{
  this->DisplacementProfile->setMaxY(d);
  this->function->setDepthRange(pqCMBModifierArc::MAX, d);
  this->render();
}

void qtCMBManualFunctionWidget::updateDepthMin(double d)
{
  this->DisplacementProfile->setMinY(d);
  this->function->setDepthRange(pqCMBModifierArc::MIN, d);
  this->render();
}

void qtCMBManualFunctionWidget::updateDistMax(double d)
{
  this->DisplacementProfile->setMinX(d);
  this->WeightingFunction->setMaxX(d);
  this->function->setDistanceRange(pqCMBModifierArc::MAX, d);
  this->render();
}

void qtCMBManualFunctionWidget::updateDistMin(double d)
{
  this->DisplacementProfile->setMinX(d);
  this->WeightingFunction->setMinX(d);
  this->function->setDistanceRange(pqCMBModifierArc::MIN, d);
  this->render();
}

void qtCMBManualFunctionWidget::render()
{
  this->DisplacementProfile->render();
  this->WeightingFunction->render();
}
