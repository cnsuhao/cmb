//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "cmbProfileWedgeFunction.h"
#include "vtkSMPropertyHelper.h"
#include "pqSMAdaptor.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"
#include "vtkPiecewiseFunction.h"

#include <cmath>

cmbProfileWedgeFunction::cmbProfileWedgeFunction()
: depth(-8), baseWidth(0), slopeLeft(1), slopeRight(1),
  WeightingFunction(vtkPiecewiseFunction::New()), Relative(true),
  Symmetry(true), WeightUseSpline(false), clamp(true), dispMode(Dig)
{
  this->WeightingFunction->SetAllowDuplicateScalars(1);
  this->WeightingFunction->AddPoint(0, 1);
  this->WeightingFunction->AddPoint(1, 1);
}

cmbProfileWedgeFunction
::cmbProfileWedgeFunction(cmbProfileWedgeFunction const* other)
: depth(other->depth), baseWidth(other->baseWidth),
  slopeLeft(other->slopeLeft), slopeRight(other->slopeRight),
  WeightingFunction(vtkPiecewiseFunction::New()), Relative(other->Relative),
  Symmetry(other->Symmetry), WeightUseSpline(other->WeightUseSpline),
  clamp(other->clamp), dispMode(other->dispMode)
{
  this->WeightingFunction->DeepCopy(other->WeightingFunction);
}

cmbProfileWedgeFunction::~cmbProfileWedgeFunction()
{
  WeightingFunction->Delete();
}

cmbProfileFunction::FunctionType cmbProfileWedgeFunction::getType() const
{
  return cmbProfileFunction::WEDGE;
}

cmbProfileFunction * cmbProfileWedgeFunction::clone(std::string const& name) const
{
  cmbProfileWedgeFunction * result = new cmbProfileWedgeFunction(this);
  result->setName(name);
  return result;
}

void cmbProfileWedgeFunction::sendDataToProxy(int arc_ID, int funId,
                                              vtkBoundingBox bbox,
                                              vtkSMSourceProxy* source) const
{
  double slopeLeft = getSlopeLeft();
  double slopeRight = getSlopeRight();
  double baseWidth = getBaseWidth();
  double depth = getDepth();

  QList< QVariant > v;
  double widthLeft = widthLeft = baseWidth * 0.5;
  if(slopeLeft  != 0)
  {
    slopeLeft = 1.0/slopeLeft;
  }
  if(slopeRight != 0)
  {
    slopeRight = 1.0/slopeRight;
  }
  v << arc_ID << funId << ((this->WeightUseSpline)?1:0)
    << Relative << this->getMode()
    << this->clamp
    << getBaseWidth() << getDepth() << slopeLeft << slopeRight;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("CreateWedgeFunction"), v);
  source->UpdateVTKObjects();
  v.clear();
  v << -1 << -1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("CreateWedgeFunction"), v);
  source->UpdateVTKObjects();

  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    WeightingFunction->GetNodeValue(i, d);
    v << arc_ID << funId << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
    source->UpdateVTKObjects();
  }
  v.clear();
  v << -1 << -1 << 0 << 0 << 0 << 0;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
  source->UpdateVTKObjects();
}

double cmbProfileWedgeFunction::getDepth() const
{
  return this->depth;
}

double cmbProfileWedgeFunction::getBaseWidth() const
{
  return this->baseWidth;
}

double cmbProfileWedgeFunction::getSlopeLeft() const
{
  return this->slopeLeft;
}

double cmbProfileWedgeFunction::getSlopeRight() const
{
  return this->slopeRight;
}

void cmbProfileWedgeFunction::setDepth(double d)
{
  this->depth = d;
}

void cmbProfileWedgeFunction::setBaseWidth(double d)
{
  this->baseWidth = d;
}

void cmbProfileWedgeFunction::setSlopeLeft(double d)
{
  this->slopeLeft = d;
}

void cmbProfileWedgeFunction::setSlopeRight(double d)
{
  this->slopeRight = d;
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
  return this->Symmetry;
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
    this->setSlopeRight(this->getSlopeLeft());
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
  int subv, tmpMode;
  in >> subv
     >> this->depth >> this->baseWidth >> this->slopeLeft >> this->slopeRight
     >> this->Relative >> this->Symmetry >> this->WeightUseSpline
     >> this->clamp >> tmpMode;
  this->dispMode = static_cast<DisplacmentMode>(tmpMode);

  int n;
  in >> n;
  this->WeightingFunction->Initialize();
  for(int i = 0; i < n; ++i)
  {
    double d[4];
    in >> d[0] >> d[1] >> d[2] >> d[3];
    this->WeightingFunction->AddPoint(d[0], d[1], d[2], d[3]);
  }
  return true;
}

bool cmbProfileWedgeFunction::writeData(std::ofstream & out) const
{
  out << '\n' << 1 << '\n' //version
      << this->depth << ' ' << this->baseWidth << ' ' << this->slopeLeft << ' '
      << this->slopeRight << '\n' << this->Relative << ' ' << this->Symmetry << ' '
      << this->WeightUseSpline << ' ' << this->clamp << ' ' << this->dispMode << '\n';

  out << this->WeightingFunction->GetSize() << "\n";
  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    this->WeightingFunction->GetNodeValue(i, d);
    out << d[0] << ' ' << d[1] << ' ' << d[2] << ' ' << d[3] << '\n';
  }

  return true;
}

bool cmbProfileWedgeFunction::isWeightSpline() const
{
  return WeightUseSpline;
}

void cmbProfileWedgeFunction::setWeightSpline(bool w)
{
  WeightUseSpline = w;
}

bool cmbProfileWedgeFunction::isClamped() const
{
  return clamp;
}
void cmbProfileWedgeFunction::setClamped(bool w)
{
  clamp = w;
}

void cmbProfileWedgeFunction::setMode(DisplacmentMode d)
{
  if(Relative)
  {
    if(d == Level) return;
    if(depth < 0) this->dispMode = Dig;
    else this->dispMode = Raise;
  }
  else
  {
    this->dispMode = d;
  }
}

cmbProfileWedgeFunction::DisplacmentMode cmbProfileWedgeFunction::getMode() const
{
  if(Relative)
  {
    if(depth<0) return Dig;
    else return Raise;
  }
  return this->dispMode;
}
