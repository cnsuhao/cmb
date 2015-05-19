//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBModifierArc.h"
#include "pqCMBArc.h"

#include "qtCMBArcWidget.h"

#include <iomanip>

#include <vtkIdTypeArray.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkPiecewiseFunction.h>

#include "pqDataRepresentation.h"
#include "vtkPVArcInfo.h"
#include "vtkSMPropertyHelper.h"
#include "pqSMAdaptor.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"
#include "qtCMBArcEditWidget.h"
#include "qtCMBArcWidgetManager.h"

pqCMBModifierArc::pqCMBModifierArc()
:CmbArc(new pqCMBArc()),
 IsExternalArc(false),
 DisplacementProfile(vtkPiecewiseFunction::New()),
 WeightingFunction(vtkPiecewiseFunction::New()),
 Modifier(NULL),
 Symmetric(true),
 Relative(true)
{
  IsVisible = true;
  DispUseSpline = false;
  WeightUseSpline = false;
  this->DistanceRange[MIN] = 0.0;
  this->DistanceRange[MAX] = 1.0;
  this->DisplacementDepthRange[MIN] = -8.0;
  this->DisplacementDepthRange[MAX] = -3.0;
  this->DisplacementProfile->AddPoint(1, 1);
  this->DisplacementProfile->AddPoint(0, 0);
  this->WeightingFunction->AddPoint(1, 0);
  this->WeightingFunction->AddPoint(0.75, 1-(0.75*0.75));
  this->WeightingFunction->AddPoint(0.5, 1-(0.5*0.5));
  this->WeightingFunction->AddPoint(0.25, 1-(0.25*0.25));
  this->WeightingFunction->AddPoint(0, 1);

  DispSplineControl[0] = 0;
  DispSplineControl[1] = 0;
  DispSplineControl[2] = 0;

  WeightSplineControl[0] = 0;
  WeightSplineControl[1] = 0;
  WeightSplineControl[2] = 0;
}

pqCMBModifierArc::pqCMBModifierArc(vtkSMSourceProxy *proxy)
:CmbArc(new pqCMBArc(proxy)),
 IsExternalArc(false),
 DisplacementProfile(vtkPiecewiseFunction::New()),
 WeightingFunction(vtkPiecewiseFunction::New()),
 Modifier(NULL),
 Symmetric(true),
 Relative(true)
{
  IsVisible = true;
  DispUseSpline = false;
  WeightUseSpline = false;
  this->DistanceRange[MIN] = 0.0;
  this->DistanceRange[MAX] = 1.0;
  this->DisplacementDepthRange[MIN] = -8.0;
  this->DisplacementDepthRange[MAX] = -3.0;
  this->DisplacementProfile->AddPoint(1, 1);
  this->DisplacementProfile->AddPoint(0, 0);
  this->WeightingFunction->AddPoint(1, 0);
  this->WeightingFunction->AddPoint(0.75, 1-(0.75*0.75));
  this->WeightingFunction->AddPoint(0.5, 1-(0.5*0.5));
  this->WeightingFunction->AddPoint(0.25, 1-(0.25*0.25));
  this->WeightingFunction->AddPoint(0, 1);

  DispSplineControl[0] = 0;
  DispSplineControl[1] = 0;
  DispSplineControl[2] = 0;

  WeightSplineControl[0] = 0;
  WeightSplineControl[1] = 0;
  WeightSplineControl[2] = 0;
}

pqCMBModifierArc::~pqCMBModifierArc()
{
  if(!IsExternalArc) delete CmbArc;
  DisplacementProfile->Delete();
  WeightingFunction->Delete();
}


void pqCMBModifierArc::switchToNotEditable()
{
  double rgba[4] ={0.2, 0.0, 0.8, 1.0};
  this->CmbArc->pqCMBSceneObjectBase::setColor(rgba, this->IsVisible);
  CmbArc->arcIsModified();
  CmbArc->updateRepresentation();
  emit requestRender();
}

void pqCMBModifierArc::switchToEditable()
{
  double rgba[4] ={0.2, 0.8, 0.2, 1.0};
  this->CmbArc->pqCMBSceneObjectBase::setColor(rgba, true);
  CmbArc->arcIsModified();
  CmbArc->updateRepresentation();
  emit requestRender();
}

bool pqCMBModifierArc::setCMBArc(pqCMBArc * arc)
{
  if(arc == NULL) return false;
  if(!IsExternalArc) delete CmbArc;
  CmbArc = arc;
  IsExternalArc = true;
  return true;
}

void pqCMBModifierArc::removeFromServer(vtkSMSourceProxy* source)
{
  QList< QVariant > v;
  v <<Id;
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("RemoveArc"), v);
  source->UpdateVTKObjects();
  source->UpdatePipeline();
  source->UpdatePropertyInformation();
}

void pqCMBModifierArc::updateArc(vtkSMSourceProxy* source)
{
  vtkPVArcInfo* info = CmbArc->getArcInfo();
  if(info != NULL)
    {
    CmbArc->arcIsModified();
    CmbArc->updateRepresentation();
      {
      //clear functions
      QList< QVariant > v;
      v <<Id;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearArc"), v);
      source->UpdateVTKObjects();
      v.clear();
      v << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearArc"), v);
      source->UpdateVTKObjects();
      }
    for(unsigned int i = 0; i < static_cast<unsigned int>(info->GetNumberOfPoints()); ++i)
      {
      double tmp[3];
      info->GetPointLocation(i, tmp);
      QList< QVariant > v;
      v <<Id << tmp[0] << tmp[1];
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddPoint"), v);
      source->UpdateVTKObjects();
      }
    if(info->IsClosedLoop())
      {
      QList< QVariant > v;
      v <<Id;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetArcAsClosed"), v);
      source->UpdateVTKObjects();
      v.clear();
      v << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetArcAsClosed"), v);
      source->UpdateVTKObjects();
      }
    }
    {
    //clear functions
    QList< QVariant > v;
    v <<Id;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearFunctions"), v);
    source->UpdateVTKObjects();
    v.clear();
    v << -1;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearFunctions"), v);
    source->UpdateVTKObjects();
    }
    {
    for(int i = 0; i < WeightingFunction->GetSize(); ++i)
      {
      double d[4];
      WeightingFunction->GetNodeValue(i, d);
      QList< QVariant > v;
      v <<Id << d[0] << d[1] << d[2] << d[3];
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddWeightPoint"), v);
      source->UpdateVTKObjects();
      }
    for(int i = 0; i < DisplacementProfile->GetSize(); ++i)
      {
      double d[4];
      DisplacementProfile->GetNodeValue(i, d);
      QList< QVariant > v;
      v <<Id << d[0] << d[1] << d[2] << d[3];
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddDespPoint"), v);
      source->UpdateVTKObjects();
      }
    sendRanges(source);
    }
    {
    QList< QVariant > v;
    v <<Id << Relative << Symmetric;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionModes"), v);
    source->UpdateVTKObjects();
    }
    {
    QList< QVariant > v;
    v << Id << ((this->WeightUseSpline)?1:0) << ((this->DispUseSpline)?1:0);
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SelectFunctionType"), v);
    }
  source->UpdateVTKObjects();
  source->MarkAllPropertiesAsModified();
  source->UpdatePipeline();
}

void pqCMBModifierArc::setVisablity(bool vis)
{
  this->IsVisible=vis;
  CmbArc->setVisibility(vis);
  CmbArc->arcIsModified();
  CmbArc->updateRepresentation();
  emit requestRender();
}

void pqCMBModifierArc::setLeftDistance(double dist)
{
  DistanceRange[MIN] = dist;
  emit(functionChanged(Id));
}

void pqCMBModifierArc::setRightDistance(double dist)
{
  DistanceRange[MAX] = dist;
  emit(functionChanged(Id));
}

void pqCMBModifierArc::setMinDisplacementDepth(double d)
{
  DisplacementDepthRange[MIN] = d;
  emit(functionChanged(Id));
}

void pqCMBModifierArc::setMaxDisplacementDepth(double d)
{
  DisplacementDepthRange[MAX] = d;
  emit(functionChanged(Id));
}

void
pqCMBModifierArc::setSymmetry(bool b)
{
  if(b == Symmetric)
    {
    return;
    }
  bool p = Symmetric;
  Symmetric = b;
  vtkPiecewiseFunction * tdisp = vtkPiecewiseFunction::New();
  tdisp->DeepCopy(DisplacementProfile);
  vtkPiecewiseFunction * tw = vtkPiecewiseFunction::New();
  tw->DeepCopy(WeightingFunction);
  WeightingFunction->Initialize();
  DisplacementProfile->Initialize();
  if(p)
    {
    DistanceRange[MIN] = -DistanceRange[MAX];
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
    DistanceRange[MIN] = 0;
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
  emit(functionChanged(Id));
}

void
pqCMBModifierArc::setRelative(bool b)
{
  Relative = b;
  emit(functionChanged(Id));
}

void
pqCMBModifierArc::sendRanges(vtkSMSourceProxy* source)
{
  QList< QVariant > v;
  v << Id << DisplacementDepthRange[MIN] << DisplacementDepthRange[MAX]
          << DistanceRange[MIN] << DistanceRange[MAX];
  pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetControlVars"), v);
  source->UpdateVTKObjects();
}

void pqCMBModifierArc::sendChangeSignals()
{
}

vtkPiecewiseFunction * pqCMBModifierArc::getDisplacementProfile()
{
  return DisplacementProfile;
}

vtkPiecewiseFunction * pqCMBModifierArc::getWeightingFunction()
{
  return WeightingFunction;
}

void
pqCMBModifierArc::setDisplacementFunctionType(bool v)
{
  DispUseSpline = v;
  emit(functionChanged(Id));
}

void
pqCMBModifierArc::setWeightingFunctionType(bool v)
{
  WeightUseSpline = v;
}

void pqCMBModifierArc::getDisplacementSplineControl(double& tension, double& continuity, double& bias)
{
  tension = DispSplineControl[0];
  continuity = DispSplineControl[1];
  bias = DispSplineControl[2];
}

void pqCMBModifierArc::getWeightingSplineControl(double& tension, double& continuity, double& bias)
{
  tension = WeightSplineControl[0];
  continuity = WeightSplineControl[1];
  bias = WeightSplineControl[2];
}

void pqCMBModifierArc::setDisplacementSplineControl(double tension, double continuity, double bias)
{
  DispSplineControl[0] = tension;
  DispSplineControl[1] = continuity;
  DispSplineControl[2] = bias;
}

void pqCMBModifierArc::setWeightingSplineControl(double tension, double continuity, double bias)
{
  WeightSplineControl[0] = tension;
  WeightSplineControl[1] = continuity;
  WeightSplineControl[2] = bias;
}

void pqCMBModifierArc::writeFunction(std::ofstream & f)
{
  f << 1 << "\n";
  f << std::setprecision(10) << DistanceRange[0] << " " << DistanceRange[1] << "\n";
  f << std::setprecision(10) << DisplacementDepthRange[0] << " " << DisplacementDepthRange[1] << "\n";
  f << std::setprecision(10) << DispSplineControl[0] << " " << DispSplineControl[1] << " " << DispSplineControl[2] << "\n";
  f << std::setprecision(10) << WeightSplineControl[0] << " " << WeightSplineControl[1] << " " << WeightSplineControl[2] << "\n";
  f << Symmetric << " " << Relative << " " << DispUseSpline << " " << WeightUseSpline << "\n";
  f << WeightingFunction->GetSize() << "\n";
  for(int i = 0; i < WeightingFunction->GetSize(); ++i)
  {
    double d[4];
    WeightingFunction->GetNodeValue(i, d);
    f << std::setprecision(10) << d[0] << " " << d[1] << " " << d[2] << " " << d[3] << "\n";
  }
  f << DisplacementProfile->GetSize() << "\n";
  for(int i = 0; i < DisplacementProfile->GetSize(); ++i)
  {
    double d[4];
    DisplacementProfile->GetNodeValue(i, d);
    f << std::setprecision(10) << d[0] << " " << d[1] << " " << d[2] << " " << d[3] << "\n";
  }
}

void pqCMBModifierArc::readFunction(std::ifstream & f)
{
  int version;
  f >> version; //nothing for now
  assert(version == 1);
  f >> DistanceRange[0] >> DistanceRange[1];
  f >> DisplacementDepthRange[0] >> DisplacementDepthRange[1];
  f >> DispSplineControl[0] >> DispSplineControl[1] >> DispSplineControl[2];
  f >> WeightSplineControl[0] >> WeightSplineControl[1] >> WeightSplineControl[2];
  f >> Symmetric >> Relative >> DispUseSpline >> WeightUseSpline;
  int n;
  f >> n;
  WeightingFunction->Initialize();
  for(int i = 0; i < n; ++i)
  {
    double d[4];
    f >> d[0] >> d[1] >> d[2] >> d[3];
    WeightingFunction->AddPoint(d[0], d[1], d[2], d[3]);
  }
  f >> n;
  DisplacementProfile->Initialize();
  for(int i = 0; i < n; ++i)
  {
    double d[4];
    f >> d[0] >> d[1] >> d[2] >> d[3];
    DisplacementProfile->AddPoint(d[0], d[1], d[2], d[3]);
  }
}

void pqCMBModifierArc::write(std::ofstream & f)
{
  f << 1 << "\n";
  if(CmbArc->getArcInfo() == NULL)
  {
    f << 0 << "\n";
    return;
  }
  f << 1 << "\n";
  f << /*this->Id << " " <<*/ IsVisible << "\n";
  f << CmbArc->getPlaneProjectionNormal() << " " << CmbArc->getPlaneProjectionPosition() << "\n";
  writeFunction(f);
}

void pqCMBModifierArc::read(std::ifstream & f)
{
  int version;
  f >> version;
  assert(version == 1);
  int hasInfo;
  f >> hasInfo;
  if(!hasInfo) return;
  f >> /*this->Id >>*/ IsVisible;
  int norm;
  double pos;
  f >> norm >> pos;
  readFunction(f);
  CmbArc->setPlaneProjectionNormal(norm);
  CmbArc->setPlaneProjectionPosition(pos);
}
