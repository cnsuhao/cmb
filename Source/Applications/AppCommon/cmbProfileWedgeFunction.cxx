#include "cmbProfileWedgeFunction.h"
#include "vtkSMPropertyHelper.h"
#include "pqSMAdaptor.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"
#include "vtkPiecewiseFunction.h"

#include <cmath>

cmbProfileWedgeFunctionParameters::cmbProfileWedgeFunctionParameters()
: depth(-8), baseWidth(0), slopeLeft(1), slopeRight(1)
{
}

cmbProfileWedgeFunctionParameters::~cmbProfileWedgeFunctionParameters()
{
}

cmbProfileWedgeFunctionParameters
::cmbProfileWedgeFunctionParameters(cmbProfileWedgeFunctionParameters const* other)
: depth(other->depth), baseWidth(other->baseWidth),
  slopeLeft(other->slopeLeft), slopeRight(other->slopeRight)
{
}

cmbProfileWedgeFunctionParameters * cmbProfileWedgeFunctionParameters::clone()
{
  return new cmbProfileWedgeFunctionParameters(this);
}

double cmbProfileWedgeFunctionParameters::getDepth() const
{
  return this->depth;
}

double cmbProfileWedgeFunctionParameters::getBaseWidth() const
{
  return this->baseWidth;
}

double cmbProfileWedgeFunctionParameters::getSlopeLeft() const
{
  return this->slopeLeft;
}

double cmbProfileWedgeFunctionParameters::getSlopeRight() const
{
  return this->slopeRight;
}

void cmbProfileWedgeFunctionParameters::setDepth(double d)
{
  this->depth = d;
}

void cmbProfileWedgeFunctionParameters::setBaseWidth(double d)
{
  this->baseWidth = d;
}

void cmbProfileWedgeFunctionParameters::setSlopeLeft(double d)
{
  this->slopeLeft = d;
}

void cmbProfileWedgeFunctionParameters::setSlopeRight(double d)
{
  this->slopeRight = d;
}
/////////////////////////////////////////////////////////////

cmbProfileWedgeFunction::cmbProfileWedgeFunction()
: parameters(new cmbProfileWedgeFunctionParameters()),
  WeightingFunction(vtkPiecewiseFunction::New()), Relative(true),
  Symmetry(true), WeightUseSpline(false)
{
  this->WeightingFunction->SetAllowDuplicateScalars(1);
  this->WeightingFunction->AddPoint(0, 1);
  this->WeightingFunction->AddPoint(1, 1);
}

cmbProfileWedgeFunction
::cmbProfileWedgeFunction(cmbProfileWedgeFunction const* other)
: parameters(new cmbProfileWedgeFunctionParameters(other->parameters)),
  WeightingFunction(vtkPiecewiseFunction::New()), Relative(other->Relative),
  Symmetry(other->Symmetry)
{
  this->WeightingFunction->DeepCopy(other->WeightingFunction);
}

cmbProfileWedgeFunction::~cmbProfileWedgeFunction()
{
  WeightingFunction->Delete();
  delete parameters;
}

cmbProfileFunction::FunctionType cmbProfileWedgeFunction::getType() const
{
  return cmbProfileFunction::WEDGE;
}

pqCMBModifierArc::modifierParams cmbProfileWedgeFunction::getDefault() const
{
  return pqCMBModifierArc::modifierParams(this);
}

cmbProfileFunction * cmbProfileWedgeFunction::clone(std::string const& name) const
{
  cmbProfileWedgeFunction * result = new cmbProfileWedgeFunction(this);
  result->setName(name);
  return result;
}

void cmbProfileWedgeFunction::sendDataToPoint(int arc_ID, int pointID,
                                              pqCMBModifierArc::modifierParams & mp,
                                              vtkSMSourceProxy* source) const
{
  QList< QVariant > v;
  v << arc_ID << pointID;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearFunctions"), v);
  source->UpdateVTKObjects();
  {
    v.clear();
    v << arc_ID << pointID << Relative << Symmetry;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionModes"), v);
    source->UpdateVTKObjects();
  }
  cmbProfileWedgeFunctionParameters * p =
                                dynamic_cast<cmbProfileWedgeFunctionParameters *>(mp.getParams());
  double slopeLeft = p->getSlopeLeft();
  double slopeRight = p->getSlopeRight();
  double baseWidth = p->getBaseWidth();
  double depth = p->getDepth();

  std::vector<double> points_x;
  std::vector<double> points_y;
  double widthRight = baseWidth * 0.5;
  if(slopeRight != 0)
  {
    widthRight = std::abs(widthRight) + std::abs(depth/slopeRight);
  }
  double widthLeft = 0;
  if(!Symmetry)
  {
    widthLeft = baseWidth * 0.5;
    if(slopeLeft != 0)
    {
      widthLeft = std::abs(widthLeft) + std::abs(depth/slopeLeft);
    }
    points_x.push_back(-1);
    points_y.push_back(0);
    if(baseWidth != 0)
    {
      points_x.push_back(-(baseWidth*0.5)/widthLeft);
      points_y.push_back(1);
    }
  }

  points_x.push_back(0);
  points_y.push_back(1);
  if(baseWidth != 0)
  {
    points_x.push_back((baseWidth*0.5)/widthRight);
    points_y.push_back(1);
  }
  points_x.push_back(1);
  points_y.push_back(0);

  v.clear();
  v << arc_ID << pointID << 0 << depth << -widthLeft << widthRight;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetControlVars"), v);
  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    WeightingFunction->GetNodeValue(i, d);
    v << arc_ID << pointID << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
    source->UpdateVTKObjects();
  }

  for(int i = 0; i < points_x.size(); ++i)
  {
    v.clear();
    v << arc_ID << pointID << points_x[i] << points_y[i] << 0.5 << 0;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddDespPoint"), v);
    source->UpdateVTKObjects();
  }
  {
    v.clear();
    v << arc_ID << pointID << 0 << WeightUseSpline;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SelectFunctionType"), v);
  }
  //TODO, this can be shared
  v.clear();
  v << arc_ID << pointID << pointID;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
}

cmbProfileFunctionParameters * cmbProfileWedgeFunction::getParameters() const
{
  return this->parameters;
}

double cmbProfileWedgeFunction::getDepth() const
{
  return parameters->getDepth();
}

double cmbProfileWedgeFunction::getBaseWidth() const
{
  return parameters->getBaseWidth();
}

double cmbProfileWedgeFunction::getSlopeLeft() const
{
  return parameters->getSlopeLeft();
}

double cmbProfileWedgeFunction::getSlopeRight() const
{
  return parameters->getSlopeRight();
}

void cmbProfileWedgeFunction::setDepth(double d)
{
  this->parameters->setDepth(d);
}

void cmbProfileWedgeFunction::setBaseWidth(double d)
{
  this->parameters->setBaseWidth(d);
}

void cmbProfileWedgeFunction::setSlopeLeft(double d)
{
  this->parameters->setSlopeLeft(d);
}

void cmbProfileWedgeFunction::setSlopeRight(double d)
{
  this->parameters->setSlopeRight(d);
}

bool cmbProfileWedgeFunction::isRelative() const
{
  return this->Relative;
}

void cmbProfileWedgeFunction::setRelative(bool ir)
{
  this->Relative = ir;
}

bool cmbProfileWedgeFunction::isSymmetric() const
{
  return Symmetry;
}

void cmbProfileWedgeFunction::setSymmetry(bool d)
{
  if(d == Symmetry) return;
  Symmetry = d;
  double v[4];
  vtkPiecewiseFunction * tw = vtkPiecewiseFunction::New();
  tw->SetAllowDuplicateScalars(1);
  tw->DeepCopy(WeightingFunction);
  WeightingFunction->Initialize();
  if(!d)
  {
    this->parameters->setSlopeRight(this->parameters->getSlopeLeft());
    for(int i = 0; i < tw->GetSize(); ++i)
    {
      tw->GetNodeValue(i, v);
      WeightingFunction->AddPoint(v[0], v[1], v[2], v[3]);
    }
    for(int i = tw->GetSize()-1; i >= 0; --i)
    {
      tw->GetNodeValue(i, v);
      if(v[0]!=0)
      {
        WeightingFunction->AddPoint(-v[0], v[1], v[2], v[3]);
      }
    }
  }
  else
  {
    for(int i = 0; i < tw->GetSize(); ++i)
    {
      tw->GetNodeValue(i, v);
      if(v[0]>0)
      {
        WeightingFunction->AddPoint(v[0], v[1], v[2], v[3]);
      }
    }
    WeightingFunction->AddPoint(0, tw->GetValue(0));
  }
}

bool cmbProfileWedgeFunction::readData(std::ifstream & in, int version)
{
  //TODO
  return false;
}

bool cmbProfileWedgeFunction::writeData(std::ofstream & out) const
{
  //todo
  return false;
}

bool cmbProfileWedgeFunction::isWeightSpline() const
{
  return WeightUseSpline;
}

void cmbProfileWedgeFunction::setWeightSpline(bool w)
{
  WeightUseSpline = w;
}
