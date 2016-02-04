#include "cmbManualProfileFunction.h"

#include "vtkPiecewiseFunction.h"
#include "vtkSMPropertyHelper.h"
#include "pqSMAdaptor.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"

cmbManualProfileFunction::cmbManualProfileFunction()
: DisplacementProfile(vtkPiecewiseFunction::New()),
  WeightingFunction(vtkPiecewiseFunction::New()),
  Symmetric(true),
  Relative(true)
{
  DispUseSpline = false;
  WeightUseSpline = false;
  this->DistanceRange[pqCMBModifierArc::MIN] = 0.0;
  this->DistanceRange[pqCMBModifierArc::MAX] = 1.0;
  this->DisplacementDepthRange[pqCMBModifierArc::MIN] = -8.0;
  this->DisplacementDepthRange[pqCMBModifierArc::MAX] = -3.0;
  this->DisplacementProfile->AddPoint(1, 1);
  this->DisplacementProfile->AddPoint(0, 0);
  this->WeightingFunction->AddPoint(1, 0);
  this->WeightingFunction->AddPoint(0.75, 1-(0.75*0.75));
  this->WeightingFunction->AddPoint(0.5, 1-(0.5*0.5));
  this->WeightingFunction->AddPoint(0.25, 1-(0.25*0.25));
  this->WeightingFunction->AddPoint(0, 1);
}

cmbManualProfileFunction::~cmbManualProfileFunction()
{
  DisplacementProfile->Delete();
  WeightingFunction->Delete();
}

cmbProfileFunction::FunctionType cmbManualProfileFunction::getType() const
{ return cmbProfileFunction::MANUAL; }

pqCMBModifierArc::modifierParams cmbManualProfileFunction::getDefault() const
{
  pqCMBModifierArc::modifierParams result;
  result.DistanceRange[0] = this->DistanceRange[0];
  result.DistanceRange[1] = this->DistanceRange[1];
  result.DisplacementDepthRange[0] = this->DisplacementDepthRange[0];
  result.DisplacementDepthRange[1] = this->DisplacementDepthRange[1];
  result.function = this;
  return result;
}

vtkPiecewiseFunction * cmbManualProfileFunction::getDisplacementProfile() const
{
  return this->DisplacementProfile;
}

vtkPiecewiseFunction * cmbManualProfileFunction::getWeightingFunction() const
{
  return this->WeightingFunction;
}

bool cmbManualProfileFunction::isSymmetric() const
{
  return Symmetric;
}

bool cmbManualProfileFunction::isRelative() const
{
  return Relative;
}

void cmbManualProfileFunction::setSymmetric(bool is)
{
  if(is == Symmetric)
  {
    return;
  }
  bool p = Symmetric;
  Symmetric = is;
  vtkPiecewiseFunction * tdisp = vtkPiecewiseFunction::New();
  tdisp->DeepCopy(DisplacementProfile);
  vtkPiecewiseFunction * tw = vtkPiecewiseFunction::New();
  tw->DeepCopy(WeightingFunction);
  WeightingFunction->Initialize();
  DisplacementProfile->Initialize();
  if(p)
  {
    DistanceRange[pqCMBModifierArc::MIN] = -DistanceRange[pqCMBModifierArc::MAX];
    double v[4];
    for(int i = 0; i < tw->GetSize(); ++i)
    {
      tw->GetNodeValue(i, v);
      WeightingFunction->AddPoint(v[0], v[1], v[2], v[3]);
      if(v[0]!=0)
      {
        WeightingFunction->AddPoint(-v[0], v[1], v[2], v[3]);
      }
    }
    for(int i = 0; i < tdisp->GetSize(); ++i)
    {
      tdisp->GetNodeValue(i, v);
      DisplacementProfile->AddPoint(v[0], v[1], v[2], v[3]);
      if(v[0]!=0)
      {
        DisplacementProfile->AddPoint(-v[0], v[1], v[2], v[3]);
      }
    }
  }
  else
  {
    DistanceRange[pqCMBModifierArc::MIN] = 0;
    double v[4];
    for(int i = 0; i < tw->GetSize(); ++i)
    {
      tw->GetNodeValue(i, v);
      if(v[0]>0)
      {
        WeightingFunction->AddPoint(v[0], v[1], v[2], v[3]);
      }
    }
    WeightingFunction->AddPoint(0, tw->GetValue(0));
    for(int i = 0; i < tdisp->GetSize(); ++i)
    {
      tdisp->GetNodeValue(i, v);
      if(v[0]>0)
      {
        DisplacementProfile->AddPoint(v[0], v[1], v[2], v[3]);
      }
    }
    DisplacementProfile->AddPoint(0, tdisp->GetValue(0));
  }
  tdisp->Delete();
  tw->Delete();
}

void cmbManualProfileFunction::setRelative(bool ir)
{
  Relative = ir;
}

bool cmbManualProfileFunction::isDispSpline() const
{
  return DispUseSpline;
}

bool cmbManualProfileFunction::isWeightSpline() const
{
  return WeightUseSpline;
}

void cmbManualProfileFunction::setDispSpline(bool s)
{
  DispUseSpline = s;
}

void cmbManualProfileFunction::setWeightSpline(bool w)
{
  WeightUseSpline = w;
}

double cmbManualProfileFunction::getDistanceRange(pqCMBModifierArc::RangeLable i)
{
  return DistanceRange[i];
}

double cmbManualProfileFunction::getDepthRange(pqCMBModifierArc::RangeLable i)
{
  return DisplacementDepthRange[i];
}

void  cmbManualProfileFunction::setDistanceRange(pqCMBModifierArc::RangeLable i, double v)
{
  this->DistanceRange[i] = v;
}
void  cmbManualProfileFunction::setDepthRange(pqCMBModifierArc::RangeLable i, double v)
{
  this->DisplacementDepthRange[i] = v;
}

void cmbManualProfileFunction::setDistanceRange(double min, double max)
{
  this->DistanceRange[pqCMBModifierArc::MIN] = min;
  this->DistanceRange[pqCMBModifierArc::MAX] = max;
}

void cmbManualProfileFunction::setDepthRange(double min, double max)
{
  this->DisplacementDepthRange[pqCMBModifierArc::MIN] = min;
  this->DisplacementDepthRange[pqCMBModifierArc::MAX] = max;
}


bool cmbManualProfileFunction::readData(std::ifstream & in, int version)
{
  int sub_version = 0;
  if(version != 1)
  {
    in >> sub_version;
  }
  double junk[0];
  in >> DistanceRange[0] >> DistanceRange[1];
  in >> DisplacementDepthRange[0] >> DisplacementDepthRange[1];
  if(sub_version == 0)
  {
    in >> junk[0] >> junk[1] >> junk[2];
    in >> junk[0] >> junk[1] >> junk[2];
  }
  in >> Symmetric >> Relative >> DispUseSpline >> WeightUseSpline;
  int n;
  in >> n;
  WeightingFunction->Initialize();
  for(int i = 0; i < n; ++i)
  {
    double d[4];
    in >> d[0] >> d[1] >> d[2] >> d[3];
    WeightingFunction->AddPoint(d[0], d[1], d[2], d[3]);
  }
  in >> n;
  DisplacementProfile->Initialize();
  for(int i = 0; i < n; ++i)
  {
    double d[4];
    in >> d[0] >> d[1] >> d[2] >> d[3];
    DisplacementProfile->AddPoint(d[0], d[1], d[2], d[3]);
  }
  return true;
}

bool cmbManualProfileFunction::writeData(std::ofstream & out) const
{
  return true;
  //TODO
}

void cmbManualProfileFunction::sendDataToPoint(int arc_ID, int pointID,
                                               pqCMBModifierArc::modifierParams & mp,
                                               vtkSMSourceProxy* source) const
{
  QList< QVariant > v;
  v << arc_ID << pointID;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearFunctions"), v);
  source->UpdateVTKObjects();
  v.clear();
  v << arc_ID << pointID
    << mp.DisplacementDepthRange[pqCMBModifierArc::MIN] << mp.DisplacementDepthRange[pqCMBModifierArc::MAX]
    << mp.DistanceRange[pqCMBModifierArc::MIN] << mp.DistanceRange[pqCMBModifierArc::MAX];
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetControlVars"), v);
  v.clear();
  source->UpdateVTKObjects();
  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    WeightingFunction->GetNodeValue(i, d);
    v << arc_ID << pointID << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
    source->UpdateVTKObjects();
  }
  for(int i = 0; i < DisplacementProfile->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    DisplacementProfile->GetNodeValue(i, d);
    v << arc_ID << pointID << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddDespPoint"), v);
    source->UpdateVTKObjects();
  }
  {
    v.clear();
    v << arc_ID << pointID << Relative << Symmetric;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionModes"), v);
    source->UpdateVTKObjects();
  }
  {
    v.clear();
    v << arc_ID << pointID << ((this->WeightUseSpline)?1:0)
      << ((this->DispUseSpline)?1:0);
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SelectFunctionType"), v);
  }
  //TODO, this can be shared
  v.clear();
  v << arc_ID << pointID << pointID;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
}
