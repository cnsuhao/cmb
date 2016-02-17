#include "qtCMBManualProfilePointFunctionModifier.h"

#include "ui_qtCMBFunctionEditor.h"

#include "cmbManualProfileFunction.h"
#include "cmbProfileWedgeFunction.h"

qtCMBManualProfilePointFunctionModifier
::qtCMBManualProfilePointFunctionModifier(QWidget * parent,
                                          std::vector<cmbProfileFunction*> funs,
                                          pqCMBModifierArc::modifierParams & params)
:QWidget(parent), UI(new Ui_qtCMBFunctionEditor), modifier(params), functions(funs)
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

  //TODO remove the function selection form here, it is here
  //     for now because we only have manal
  for( unsigned int i = 0; i < functions.size(); ++i)
  {
    if(functions[i] == params.getFunction())
    {
      cindex = i;
    }
    QVariant vdata;
    vdata.setValue(static_cast<void*>(functions[i]));
    this->UI->function->addItem(functions[i]->getName().c_str(), vdata);
  }
  this->UI->function->setCurrentIndex(cindex);
  connect(this->UI->function, SIGNAL(currentIndexChanged(int)),
          this, SLOT(functionIndexChange(int)));
  //done todo

}

void qtCMBManualProfilePointFunctionModifier
::setUp()
{
  manualFunction = NULL;
  manualModifierParams = NULL;
  wedgeFunction = NULL;
  wedgeModifierParams = NULL;

  this->UI->useDefaults->setChecked(modifier.getUseDefault());
  this->UI->parameters->hide();
  this->UI->wedgeParam->hide();

  switch(modifier.getFunction()->getType())
  {
    case cmbProfileFunction::MANUAL:
      manualFunction = dynamic_cast< cmbManualProfileFunction const* >(modifier.getFunction());
      manualModifierParams =
                        dynamic_cast< cmbManualProfileFunctionParameters * >(modifier.getParams());
      if(!modifier.getUseDefault())
      {
        this->UI->parameters->show();
        this->UI->leftDist->setValue(manualModifierParams->getDistanceRange(pqCMBModifierArc::MIN));
        this->UI->rightDist->setValue(manualModifierParams->getDistanceRange(pqCMBModifierArc::MAX));
        this->UI->minDepth->setValue(manualModifierParams->getDepthRange(pqCMBModifierArc::MIN));
        this->UI->maxDepth->setValue(manualModifierParams->getDepthRange(pqCMBModifierArc::MAX));

        if(manualFunction->isRelative())
        {
          this->UI->leftDist->setDisabled(true);
          this->UI->leftDist->setValue(-manualModifierParams->getDistanceRange(pqCMBModifierArc::MAX));
        }
      }
      break;
    case cmbProfileFunction::WEDGE:
      wedgeFunction = dynamic_cast< cmbProfileWedgeFunction const* >(modifier.getFunction());
      wedgeModifierParams =
                          dynamic_cast< cmbProfileWedgeFunctionParameters * >(modifier.getParams());
      if (!modifier.getUseDefault())
      {
        this->UI->wedgeParam->show();
        this->UI->wedgeDepth->setValue(wedgeModifierParams->getDepth());
        this->UI->baseWidth->setValue(wedgeModifierParams->getBaseWidth());
        this->UI->leftSlope->setValue(wedgeModifierParams->getSlopeLeft());
        this->UI->rightSlope->setValue(wedgeModifierParams->getSlopeRight());
        if(wedgeFunction->isRelative())
        {
          this->UI->leftSlope->setDisabled(true);
          this->UI->leftSlope->setValue(wedgeModifierParams->getSlopeRight());
        }
      }
      break;
  }

}

void qtCMBManualProfilePointFunctionModifier
::setUseDefaults(bool v)
{
  modifier.setUseDefault(v);
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
    modifier.setFunction(functions[in]);
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
