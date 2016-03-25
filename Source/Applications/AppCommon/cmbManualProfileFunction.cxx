#include "cmbManualProfileFunction.h"

#include "vtkPiecewiseFunction.h"
#include "vtkSMPropertyHelper.h"
#include "pqSMAdaptor.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"

////////////////////////////////////////////////////
cmbManualProfileFunctionParameters
::cmbManualProfileFunctionParameters()
{
  this->DistanceRange[pqCMBModifierArc::MIN] = 0.0;
  this->DistanceRange[pqCMBModifierArc::MAX] = 1.0;
  this->DisplacementDepthRange[pqCMBModifierArc::MIN] = -8.0;
  this->DisplacementDepthRange[pqCMBModifierArc::MAX] = -3.0;
}

cmbManualProfileFunctionParameters
::cmbManualProfileFunctionParameters(cmbManualProfileFunctionParameters const* other)
{
  this->DistanceRange[0] = other->DistanceRange[0];
  this->DistanceRange[1] = other->DistanceRange[1];
  this->DisplacementDepthRange[0] = other->DisplacementDepthRange[0];
  this->DisplacementDepthRange[1] = other->DisplacementDepthRange[1];
}

cmbProfileFunctionParameters * cmbManualProfileFunctionParameters
::clone()
{
  return new cmbManualProfileFunctionParameters(this);
}

cmbManualProfileFunctionParameters
::~cmbManualProfileFunctionParameters()
{}

double cmbManualProfileFunctionParameters
::getDistanceRange(pqCMBModifierArc::RangeLable i)
{
  return DistanceRange[i];
}

double cmbManualProfileFunctionParameters
::getDepthRange(pqCMBModifierArc::RangeLable i)
{
  return DisplacementDepthRange[i];
}

void cmbManualProfileFunctionParameters
::setDistanceRange(pqCMBModifierArc::RangeLable i, double v)
{
  this->DistanceRange[i] = v;
}

void cmbManualProfileFunctionParameters
::setDepthRange(pqCMBModifierArc::RangeLable i, double v)
{
  this->DisplacementDepthRange[i] = v;
}

/////////////////////////////////////////////////////

cmbManualProfileFunction::cmbManualProfileFunction()
: DisplacementProfile(vtkPiecewiseFunction::New()), WeightingFunction(vtkPiecewiseFunction::New()),
  parameters(new cmbManualProfileFunctionParameters()), Symmetric(true), Relative(true),
  DispUseSpline(false), WeightUseSpline(false)
{
  this->DisplacementProfile->SetAllowDuplicateScalars(1);
  this->WeightingFunction->SetAllowDuplicateScalars(1);
  this->DisplacementProfile->AddPoint(1, 1);
  this->DisplacementProfile->AddPoint(0, 0);
  this->WeightingFunction->AddPoint(1, 0);
  this->WeightingFunction->AddPoint(0.75, 1-(0.75*0.75));
  this->WeightingFunction->AddPoint(0.5, 1-(0.5*0.5));
  this->WeightingFunction->AddPoint(0.25, 1-(0.25*0.25));
  this->WeightingFunction->AddPoint(0, 1);
}

cmbManualProfileFunction::cmbManualProfileFunction(cmbManualProfileFunction const* other)
: DisplacementProfile(vtkPiecewiseFunction::New()),
  WeightingFunction(vtkPiecewiseFunction::New()),
  parameters(new cmbManualProfileFunctionParameters(other->parameters)),
  Symmetric(other->Symmetric), Relative(other->Relative),
  DispUseSpline(other->DispUseSpline), WeightUseSpline(other->WeightUseSpline)
{
  this->DisplacementProfile->SetAllowDuplicateScalars(1);
  this->WeightingFunction->SetAllowDuplicateScalars(1);
  this->DisplacementProfile->DeepCopy(other->DisplacementProfile);
  this->WeightingFunction->DeepCopy(other->WeightingFunction);
}

cmbManualProfileFunction::~cmbManualProfileFunction()
{
  DisplacementProfile->Delete();
  WeightingFunction->Delete();
  delete parameters;
}

cmbProfileFunctionParameters * cmbManualProfileFunction::getParameters() const
{
  return parameters;
}

cmbProfileFunction::FunctionType cmbManualProfileFunction::getType() const
{ return cmbProfileFunction::MANUAL; }

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
  tdisp->SetAllowDuplicateScalars(1);
  tdisp->DeepCopy(DisplacementProfile);
  vtkPiecewiseFunction * tw = vtkPiecewiseFunction::New();
  tw->SetAllowDuplicateScalars(1);
  tw->DeepCopy(WeightingFunction);
  WeightingFunction->Initialize();
  DisplacementProfile->Initialize();
  if(p)
  {
    parameters->setDistanceRange(pqCMBModifierArc::MIN,
                                 -parameters->getDistanceRange(pqCMBModifierArc::MAX));
    double v[4];
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
    for(int i = 0; i < tdisp->GetSize(); ++i)
    {
      tdisp->GetNodeValue(i, v);
      DisplacementProfile->AddPoint(v[0], v[1], v[2], v[3]);
    }
    for(int i = tdisp->GetSize()-1; i >= 0; --i)
    {
      tdisp->GetNodeValue(i, v);
      if(v[0]!=0)
      {
        DisplacementProfile->AddPoint(-v[0], v[1], v[2], v[3]);
      }
    }
  }
  else
  {
    parameters->setDistanceRange(pqCMBModifierArc::MIN, 0);
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
  return parameters->getDistanceRange(i);
}

double cmbManualProfileFunction::getDepthRange(pqCMBModifierArc::RangeLable i)
{
  return parameters->getDepthRange(i);
}

void  cmbManualProfileFunction::setDistanceRange(pqCMBModifierArc::RangeLable i, double v)
{
  parameters->setDistanceRange(i,v);
}
void  cmbManualProfileFunction::setDepthRange(pqCMBModifierArc::RangeLable i, double v)
{
  parameters->setDepthRange(i,v);
}

void cmbManualProfileFunction::setDistanceRange(double min, double max)
{
  parameters->setDistanceRange(pqCMBModifierArc::MIN,min);
  parameters->setDistanceRange(pqCMBModifierArc::MAX,max);
}

void cmbManualProfileFunction::setDepthRange(double min, double max)
{
  parameters->setDistanceRange(pqCMBModifierArc::MIN,min);
  parameters->setDistanceRange(pqCMBModifierArc::MAX,max);
}


bool cmbManualProfileFunction::readData(std::ifstream & in, int version)
{
  int sub_version = 0;
  if(version != 1)
  {
    in >> sub_version;
  }
  double junk[3];
  double tmp[2];
  in >> tmp[pqCMBModifierArc::MIN] >> tmp[pqCMBModifierArc::MAX];
  this->setDepthRange(tmp[pqCMBModifierArc::MIN],
                      tmp[pqCMBModifierArc::MAX]);
  in >> tmp[pqCMBModifierArc::MIN] >> tmp[pqCMBModifierArc::MAX];
  this->setDistanceRange(tmp[pqCMBModifierArc::MIN],
                         tmp[pqCMBModifierArc::MAX]);
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

void cmbManualProfileFunction::sendDataToProxy(int arc_ID, int funID,
                                               vtkSMSourceProxy* source) const
{
  QList< QVariant > v;
  v << arc_ID << funID
    << ((this->WeightUseSpline)?1:0) << ((this->DispUseSpline)?1:0)
    << Relative << Symmetric ;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("CreateManualFunction"), v);
  source->UpdateVTKObjects();
  v.clear();
  //Send function parameters
  cmbManualProfileFunctionParameters * p = parameters;
  v << arc_ID << funID
    << p->getDepthRange(pqCMBModifierArc::MIN) << p->getDepthRange(pqCMBModifierArc::MAX)
    << p->getDistanceRange(pqCMBModifierArc::MIN) << p->getDistanceRange(pqCMBModifierArc::MAX);
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetManualControlVars"), v);
  v.clear();
  source->UpdateVTKObjects();
  //send function points
  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    WeightingFunction->GetNodeValue(i, d);
    v << arc_ID << funID << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
    source->UpdateVTKObjects();
  }
  for(int i = 0; i < DisplacementProfile->GetSize(); ++i)
  {
    double d[4];
    v.clear();
    DisplacementProfile->GetNodeValue(i, d);
    v << arc_ID << funID << d[0] << d[1] << d[2] << d[3];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddManualDespPoint"), v);
    source->UpdateVTKObjects();
  }
  //TODO, this can be shared
  //TODO MOVE THIS
  //v.clear();
  //v << arc_ID << pointID << pointID;
  //pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
}

cmbProfileFunction * cmbManualProfileFunction::clone(std::string const& name) const
{
  cmbManualProfileFunction * fun = new cmbManualProfileFunction(this);
  fun->setName(name);
  return fun;
}
