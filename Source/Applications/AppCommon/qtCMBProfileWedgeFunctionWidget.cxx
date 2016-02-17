#include "qtCMBProfileWedgeFunctionWidget.h"
#include "cmbProfileWedgeFunction.h"

#include "cmbProfileWedgeFunction.h"
#include "ui_qtCMBFunctionWedgeEditor.h"

#include <cmath>

qtCMBProfileWedgeFunctionWidget
::qtCMBProfileWedgeFunctionWidget(QWidget * parent, cmbProfileWedgeFunction * fun)
: QWidget(parent), UI(new Ui_qtCMBProfileWedgeFunction()), function(fun)
{
  this->UI->setupUi(this);

  connect(this->UI->symmetric, SIGNAL(toggled(bool)),
          this, SLOT(setSymmetry(bool)));
  connect(this->UI->relative, SIGNAL(toggled(bool)),
          this, SLOT(setRelative(bool)));
  connect(this->UI->clamp, SIGNAL(toggled(bool)),
          this, SLOT(setClamp(bool)));

  connect(this->UI->LeftSlope, SIGNAL(valueChanged(double)),
          this, SLOT(setLeftSlope(double)));
  connect(this->UI->RightSlope, SIGNAL(valueChanged(double)),
          this, SLOT(setRightSlope(double)));
  connect(this->UI->depth, SIGNAL(valueChanged(double)),
          this, SLOT(setDepth(double)));
  connect(this->UI->baseWidth, SIGNAL(valueChanged(double)),
          this, SLOT(setBaseWidth(double)));
  connect(this->UI->WeightingSplineControl, SIGNAL(toggled(bool)),
          this, SLOT(weightSplineBox(bool)));

  {
    QGridLayout* gridlayout = new QGridLayout(this->UI->weightingChartFrame);
    gridlayout->setMargin(0);
    this->WeightingFunction = new pqGeneralTransferFunctionWidget();
    gridlayout->addWidget(this->WeightingFunction);
    this->WeightingFunction->addFunction(NULL, false);
    this->WeightingFunction->setMinY(0);
    this->WeightingFunction->setMaxY(1);

    connect( this->UI->WeightingSplineControl, SIGNAL(toggled(bool)),
            this->WeightingFunction, SLOT(setFunctionType(bool)));
    connect(this->WeightingFunction, SIGNAL(controlPointsModified()),
            this->WeightingFunction, SLOT(render()));
  }

  this->setUp();
  setWeightFunction();
}

qtCMBProfileWedgeFunctionWidget
::~qtCMBProfileWedgeFunctionWidget()
{
  delete UI;
}

void qtCMBProfileWedgeFunctionWidget
::setLeftSlope(double d)
{
  function->setSlopeLeft(d);
  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget
::setRightSlope(double d)
{
  function->setSlopeRight(d);
  if(function->isSymmetric())
  {
    UI->LeftSlope->setValue(d);
  }
  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget
::setDepth(double d)
{
  function->setDepth(d);
  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget
::setBaseWidth(double d)
{
  function->setBaseWidth(d);
  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget
::setRelative(bool b)
{
  function->setRelative(b);
}

void qtCMBProfileWedgeFunctionWidget
::setSymmetry(bool b)
{
  function->setSymmetry(b);
  if(b)
  {
    this->UI->LeftSlope->setEnabled(false);
    this->UI->LeftSlope->setValue(function->getSlopeRight());
  }
  else
  {
    this->UI->LeftSlope->setEnabled(true);
  }

  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget
::setClamp(bool)
{
  //TODO
}

void qtCMBProfileWedgeFunctionWidget
::weightSplineBox(bool v)
{
  this->function->setWeightSpline(v);
  setWeightFunction();
}

void qtCMBProfileWedgeFunctionWidget::setWeightFunction()
{
  this->WeightingFunction->changeFunction(0, function->getWeightingFunction(), true);
  double depth = function->getDepth();
  double slopeLeft = function->getSlopeLeft();
  double slopeRight = function->getSlopeRight();
  double leftWidth = std::abs(function->getBaseWidth()*0.5);
  double rightWidth = leftWidth;
  if(slopeLeft != 0)
  {
    leftWidth += std::abs(depth/slopeLeft);
  }
  if(slopeRight != 0)
  {
    rightWidth += std::abs(depth/slopeRight);
  }

  this->WeightingFunction->setMinX(-leftWidth);
  this->WeightingFunction->setMaxX(rightWidth);
  this->WeightingFunction->setMinY(0);
  this->WeightingFunction->setMaxY(1);
}

void qtCMBProfileWedgeFunctionWidget
::render()
{
  this->WeightingFunction->render();
}

void qtCMBProfileWedgeFunctionWidget::setUp()
{
  this->WeightingFunction->changeFunction(0, function->getWeightingFunction(), true);
  this->UI->LeftSlope->setValue(function->getSlopeLeft());
  this->UI->RightSlope->setValue(function->getSlopeRight());
  this->UI->depth->setValue(function->getDepth());
  this->UI->baseWidth->setValue(function->getBaseWidth());
  this->setSymmetry(function->isSymmetric());
  this->UI->symmetric->setChecked(function->isSymmetric());

  this->UI->WeightingSplineControl->setChecked(function->isWeightSpline());

  this->UI->relative->setChecked(function->isRelative());
}
