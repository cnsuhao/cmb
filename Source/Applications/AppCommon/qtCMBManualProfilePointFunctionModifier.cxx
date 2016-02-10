#include "qtCMBManualProfilePointFunctionModifier.h"

#include "ui_qtCMBFunctionEditor.h"

#include "cmbManualProfileFunction.h"

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

  //TODO remove the function selection form here, it is here
  //     for now because we only have manal
  for( unsigned int i = 0; i < functions.size(); ++i)
  {
    if(functions[i] == params.function)
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

  if(function->isRelative())
  {
    this->UI->leftDist->setDisabled(true);
    this->UI->leftDist->setValue(-modifier.DistanceRange[1]);
  }

}

void qtCMBManualProfilePointFunctionModifier
::setUp()
{
  function = dynamic_cast< cmbManualProfileFunction const* >(modifier.function);
  this->UI->useDefaults->setChecked(modifier.useDefault);
  this->UI->parameters->setVisible(!modifier.useDefault);
  this->UI->leftDist->setValue(modifier.DistanceRange[0]);
  this->UI->rightDist->setValue(modifier.DistanceRange[1]);
  this->UI->minDepth->setValue(modifier.DisplacementDepthRange[0]);
  this->UI->maxDepth->setValue(modifier.DisplacementDepthRange[1]);
}

void qtCMBManualProfilePointFunctionModifier
::setUseDefaults(bool v)
{
  this->UI->parameters->setVisible(!v);
  modifier.useDefault = v;
}

void qtCMBManualProfilePointFunctionModifier
::setLeftDist(double d)
{
  if(!function->isRelative())
  {
    modifier.DistanceRange[0] = d;
  }
}

void qtCMBManualProfilePointFunctionModifier
::setRightDist(double d)
{
  modifier.DistanceRange[1] = d;
  if(function->isRelative())
  {
    this->UI->leftDist->setValue(-d);
  }
}

void qtCMBManualProfilePointFunctionModifier
::setMinDepth(double d)
{
  modifier.DisplacementDepthRange[0] = d;
}

void qtCMBManualProfilePointFunctionModifier
::setMaxDepth(double d)
{
  modifier.DisplacementDepthRange[1] = d;
}

void qtCMBManualProfilePointFunctionModifier
::functionIndexChange(int in)
{
  if(in != cindex)
  {
    cindex = in;
    modifier.function = functions[in];
    setUp();
  }
}
