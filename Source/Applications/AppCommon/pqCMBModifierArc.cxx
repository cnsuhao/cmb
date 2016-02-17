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
#include <sstream>

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
#include "cmbManualProfileFunction.h"
#include "cmbProfileFunction.h"

////////////////////////////////////////
pqCMBModifierArc::modifierParams::modifierParams(modifierParams const& other)
: function(other.function), params(NULL), useDefault(other.useDefault)
{
  if(other.params != NULL)
  {
    params = other.params->clone();
  }
}

pqCMBModifierArc::modifierParams::~modifierParams()
{ delete params; }

void pqCMBModifierArc::modifierParams::operator=(pqCMBModifierArc::modifierParams const& other)
{
  function = other.function;
  useDefault = other.useDefault;
  delete params;
  params = NULL;
  if(other.params != NULL)
  {
    params = other.params->clone();
  }
}

pqCMBModifierArc::modifierParams::modifierParams(cmbProfileFunction const* fun)
:function(fun), params(NULL), useDefault(true)
{
  if(function)
  {
    params = function->getParameters()->clone();
  }
}

bool pqCMBModifierArc::modifierParams::getUseDefault()
{
  return useDefault;
}

void pqCMBModifierArc::modifierParams::setUseDefault(bool b)
{
  useDefault = b;
}

cmbProfileFunctionParameters * pqCMBModifierArc::modifierParams::getParams()
{
  if((params == NULL || useDefault) && function)
  {
    return function->getParameters();
  }
  return params;
}

cmbProfileFunction const* pqCMBModifierArc::modifierParams::getFunction()
{
  return function;
}

void
pqCMBModifierArc::modifierParams::setFunction(cmbProfileFunction const* f)
{
  if(f == NULL)
  {
    delete params;
    params = NULL;
    function = NULL;
  }
  else if(f != function)
  {
    function = f;
    delete params;
    params = function->getParameters()->clone();
  }
}

////////////////////////////////////////

pqCMBModifierArc::pqCMBModifierArc()
:CmbArc(new pqCMBArc()), IsExternalArc(false), Modifier(NULL), IsVisible(true)
{
  defaultFun = new cmbManualProfileFunction();
  defaultFun->setName("FUN0");
  functions[defaultFun->getName()] = defaultFun;
  setUpFunction();
}

pqCMBModifierArc::pqCMBModifierArc(vtkSMSourceProxy *proxy)
:CmbArc(new pqCMBArc(proxy)),
 IsExternalArc(false), Modifier(NULL), IsVisible(true)
{
  IsVisible = true;
  defaultFun = new cmbManualProfileFunction();
  defaultFun->setName("FUN0");
  functions[defaultFun->getName()] = defaultFun;
}

pqCMBModifierArc::~pqCMBModifierArc()
{
  if(!IsExternalArc) delete CmbArc;
  for(std::map<std::string, cmbProfileFunction * >::iterator i = functions.begin();
      i != functions.end(); ++i)
  {
    delete i->second;
    i->second = NULL;
  }
  functions.clear();
}


void pqCMBModifierArc::switchToNotEditable()
{
  double rgba[4] ={0.2, 0.0, 0.8, 1.0};
  this->CmbArc->pqCMBSceneObjectBase::setColor(rgba, this->IsVisible);
  CmbArc->arcIsModified();
  CmbArc->updateRepresentation();
  setUpFunction();
  emit requestRender();
}

void pqCMBModifierArc::switchToEditable()
{
  double rgba[4] ={0.2, 0.8, 0.2, 1.0};
  this->CmbArc->pqCMBSceneObjectBase::setColor(rgba, true);
  CmbArc->arcIsModified();
  CmbArc->updateRepresentation();
  setUpFunction();
  emit requestRender();
}

bool pqCMBModifierArc::setCMBArc(pqCMBArc * arc)
{
  if(arc == NULL) return false;
  if(!IsExternalArc) delete CmbArc;
  CmbArc = arc;
  setUpFunction();
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
  setUpFunction();
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
    for(unsigned int i = 0; i < static_cast<unsigned int>(info->GetNumberOfPoints());
        ++i)
    {
      pointsParams[i].getFunction()->sendDataToPoint(Id, i, pointsParams[i], source);
    }
    {
    //clear functions
    QList< QVariant > v;
    v << -1 << 0;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ClearFunctions"), v);
    source->UpdateVTKObjects();
    }
    {
    QList< QVariant > v;
    v << -1 << -1 << 0 << 1;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionModes"), v);
    source->UpdateVTKObjects();
    }
    {
    QList< QVariant > v;
    v << -1 << -1 << 0 << 1;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SelectFunctionType"), v);
    source->UpdateVTKObjects();
      v.clear();
      v << -1 << -1 << 0 << 0 << 0 << 0;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetControlVars"), v);
      source->UpdateVTKObjects();
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

void pqCMBModifierArc::sendChangeSignals()
{
}

void pqCMBModifierArc::writeFunction(std::ofstream & f)
{
  //TODO
}

void pqCMBModifierArc::readFunction(std::ifstream & f)
{
  //TODO
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
  setUpFunction();
}

pqCMBModifierArc::modifierParams * pqCMBModifierArc::getPointModifer(size_t i)
{
  if(i < pointsParams.size())
  {
    return &(pointsParams[i]);
  }
  return NULL;
}

void pqCMBModifierArc::setUpFunction()
{
  vtkPVArcInfo* info = CmbArc->getArcInfo();
  if(info == NULL) return;
  if(pointsParams.size() == info->GetNumberOfPoints())
  {
    //DO Nothing
  }
  else if(pointsParams.size() > info->GetNumberOfPoints())
  {
    pointsParams.resize(info->GetNumberOfPoints());
  }
  else for(size_t i = pointsParams.size();
           i < static_cast<size_t>(info->GetNumberOfPoints()); ++i)
  {
    modifierParams mp = defaultFun->getDefault();
    pointsParams.push_back(mp);
  }
}

void pqCMBModifierArc::getFunctions(std::vector<cmbProfileFunction*> & funs) const
{
  funs.clear();
  funs.reserve(functions.size());
  for(std::map<std::string, cmbProfileFunction * >::const_iterator i = functions.begin();
      i != functions.end(); ++i)
  {
    funs.push_back(i->second);
  }
}

bool pqCMBModifierArc::updateLabel(std::string str, cmbProfileFunction * fun)
{
  if(str == fun->getName()) return true;
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(str);
  if(i == functions.end())
  {
    functions.erase(functions.find(fun->getName()));
    functions[str] = fun;
    fun->setName(str);
    return true;
  }
  return false;
}

bool pqCMBModifierArc::setDefaultFun(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return false;
  }
  defaultFun = i->second;
  return true;
}

cmbProfileFunction * pqCMBModifierArc::createFunction()
{
  int count = 1;
  std::string name = "FUN0";
  while(functions.find(name) != functions.end())
  {
    std::stringstream ss;
    ss << "FUN" << count++;
    ss >> name;
  }
  cmbProfileFunction * result = new cmbManualProfileFunction();
  result->setName(name);
  functions[name] = result;
  return result;
}

bool pqCMBModifierArc::deleteFunction(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return false;
  }
  cmbProfileFunction * fun = i->second;
  if(fun == defaultFun) return false;
  functions.erase(i);
  delete fun;
  return true;
}

cmbProfileFunction * pqCMBModifierArc::cloneFunction(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return NULL;
  }
  int count = 1;
  std::string newname = i->second->getName() + "Clone0";
  while(functions.find(newname) != functions.end())
  {
    std::stringstream ss;
    ss << i->second->getName() + "Clone" << count++;
    ss >> newname;
  }
  functions[newname] = i->second->clone(newname);
  return functions[newname];
}

void pqCMBModifierArc::setFunction(std::string const& name, cmbProfileFunction* fun)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    functions[name] = fun;
  }
  else
  {
    if(defaultFun == i->second) defaultFun = fun;
    fun->setName(name);
    for (size_t ind = 0; ind < pointsParams.size(); ++ind)
    {
      if(pointsParams[ind].getFunction()->getName() == name)
      {
        pointsParams[ind].setFunction(fun);
      }
    }
    delete i->second;
    i->second = fun;
  }
}
