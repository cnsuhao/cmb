#include "qtCMBManualProfilePointFunctionModifier.h"

#include "ui_qtCMBFunctionEditor.h"

#include "cmbManualProfileFunction.h"
#include "cmbProfileWedgeFunction.h"

#include <cassert>

qtCMBManualProfilePointFunctionModifier
::qtCMBManualProfilePointFunctionModifier(QWidget * parent,
                                          std::vector<cmbProfileFunction*> funs,
                                          pqCMBModifierArc::pointFunctionWrapper & w)
:QWidget(parent), UI(new Ui_qtCMBFunctionEditor), functions(funs), wrapper(w)
{
  this->UI->setupUi(this);

  this->setUp();

  connect(this->UI->useDefaults, SIGNAL(toggled(bool)),
          this, SLOT(setUseDefaults(bool)));
  connect(this->UI->leftDist, SIGNAL(valueChanged(double)),
          this, SLOT(setLeftDist(double)));
  connect(this->UI->rightDist, SIGNAL(valueChanged(double)),
          this, SLOT(setRightDist(double)));
  connect(this->UI->minDepth, SIGNAL(valueChanged(double)),
          this, SLOT(setMinDepth(double)));
  connect(this->UI->maxDepth, SIGNAL(valueChanged(double)),
          this, SLOT(setMaxDepth(double)));


  connect(this->UI->wedgeDepth, SIGNAL(valueChanged(double)),
          this, SLOT(setDepth(double)));
  connect(this->UI->baseWidth, SIGNAL(valueChanged(double)),
          this, SLOT(setBaseDistance(double)));
  connect(this->UI->leftSlope, SIGNAL(valueChanged(double)),
          this, SLOT(setLeftSlope(double)));
  connect(this->UI->rightSlope, SIGNAL(valueChanged(double)),
          this, SLOT(setRightSlope(double)));

  {
    QVariant vdata;
    vdata.setValue(static_cast<void*>(NULL));
    this->UI->function->addItem("", vdata);
  }

  cindex = 0;
  for( unsigned int i = 0; i < functions.size(); ++i)
  {
    if(functions[i] == w.getFunction())
    {
      cindex = i+1;
    }
    QVariant vdata;
    vdata.setValue(static_cast<void*>(functions[i]));
    this->UI->function->addItem(functions[i]->getName().c_str(), vdata);
  }
  this->UI->function->setCurrentIndex(cindex);
  connect(this->UI->function, SIGNAL(currentIndexChanged(int)),
          this, SLOT(functionIndexChange(int)));

}

void qtCMBManualProfilePointFunctionModifier
::setUp()
{
  manualFunction = NULL;
  manualModifierParams = NULL;
  wedgeFunction = NULL;
  wedgeModifierParams = NULL;

  this->UI->useDefaults->setChecked(true);
  this->UI->useDefaults->hide();
  this->UI->parameters->hide();
  this->UI->wedgeParam->hide();

  manualFunction = NULL;
  wedgeFunction = NULL;

  if(wrapper.getFunction() == NULL)
  {
    return;
  }

  switch(wrapper.getFunction()->getType())
  {
    case cmbProfileFunction::MANUAL:
      manualFunction = dynamic_cast< cmbManualProfileFunction const* >(wrapper.getFunction());
      manualModifierParams =
              dynamic_cast< cmbManualProfileFunctionParameters * >(manualFunction->getParameters());
      break;
    case cmbProfileFunction::WEDGE:
      wedgeFunction = dynamic_cast< cmbProfileWedgeFunction const* >(wrapper.getFunction());
      wedgeModifierParams =
                dynamic_cast< cmbProfileWedgeFunctionParameters * >(wedgeFunction->getParameters());
      break;
  }

}

void qtCMBManualProfilePointFunctionModifier
::setUseDefaults(bool v)
{
  setUp();
}

void qtCMBManualProfilePointFunctionModifier
::setLeftDist(double d)
{
  if(!manualFunction->isRelative())
  {
    manualModifierParams->setDistanceRange(pqCMBModifierArc::MIN, d);
  }
}

void qtCMBManualProfilePointFunctionModifier
::setRightDist(double d)
{
  manualModifierParams->setDistanceRange(pqCMBModifierArc::MAX, d);
  if(manualFunction->isRelative())
  {
    this->UI->leftDist->setValue(-d);
  }
}

void qtCMBManualProfilePointFunctionModifier
::setMinDepth(double d)
{
  manualModifierParams->setDepthRange(pqCMBModifierArc::MIN, d);
}

void qtCMBManualProfilePointFunctionModifier
::setMaxDepth(double d)
{
  manualModifierParams->setDepthRange(pqCMBModifierArc::MAX, d);
}

void qtCMBManualProfilePointFunctionModifier
::functionIndexChange(int in)
{
  if(in != cindex)
  {
    cindex = in;
    if(in == 0)
    {
      wrapper.setFunction(NULL);
    }
    else
    {
      wrapper.setFunction(functions[in-1]);
    }
    setUp();
  }
}

void qtCMBManualProfilePointFunctionModifier::setRightSlope(double d)
{
  wedgeModifierParams->setSlopeRight(d);
  if(wedgeFunction->isRelative())
  {
    this->UI->leftSlope->setValue(d);
  }
}

void qtCMBManualProfilePointFunctionModifier::setLeftSlope(double d)
{
  wedgeModifierParams->setSlopeLeft(d);
}

void qtCMBManualProfilePointFunctionModifier::setBaseDistance(double d)
{
  wedgeModifierParams->setBaseWidth(d);
}

void qtCMBManualProfilePointFunctionModifier::setDepth(double d)
{
  wedgeModifierParams->setDepth(d);
}
