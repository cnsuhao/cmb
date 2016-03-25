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
pqCMBModifierArc::pointFunctionWrapper::pointFunctionWrapper(pointFunctionWrapper const& other)
: function(other.function)
{
}

pqCMBModifierArc::pointFunctionWrapper::~pointFunctionWrapper()
{ }

void pqCMBModifierArc
::pointFunctionWrapper::operator=(pqCMBModifierArc::pointFunctionWrapper const& other)
{
  function = other.function;
}

pqCMBModifierArc::pointFunctionWrapper::pointFunctionWrapper(cmbProfileFunction const* fun)
:function(fun)
{
}

cmbProfileFunction const* pqCMBModifierArc::pointFunctionWrapper::getFunction()
{
  return function;
}

void
pqCMBModifierArc::pointFunctionWrapper::setFunction(cmbProfileFunction const* f)
{
  function = f;
}

std::string pqCMBModifierArc::pointFunctionWrapper::getName()
{
  if(function) return function->getName();
  return "NULL";
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
    std::map< cmbProfileFunction const*, unsigned int> function_to_id;
    std::map<std::string, cmbProfileFunction * >::const_iterator iter = functions.begin();
    for(unsigned int i = 0;
        iter != functions.end(); ++i,++iter)
    {
      iter->second->sendDataToProxy(Id, i, source);
      function_to_id[iter->second] = i;
    }
    for(unsigned int i = 0; i < static_cast<unsigned int>(info->GetNumberOfPoints());
        ++i)
    {
      vtkIdType id;
      info->GetPointID(i, id);
      pointFunctionWrapper & wrapper = pointsFunctions[id];
      if(pointsFunctions[i].getFunction() != NULL)
      {
        QList< QVariant > v;
        v.clear();
        v << Id << i << function_to_id[pointsFunctions[i].getFunction()];
        pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
      }
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

pqCMBModifierArc::pointFunctionWrapper * pqCMBModifierArc::getPointFunction(vtkIdType i)
{
  std::map<vtkIdType, pointFunctionWrapper>::iterator it = pointsFunctions.find(i);
  return (it == pointsFunctions.end())?NULL:&(it->second);
}

void pqCMBModifierArc::setUpFunction()
{
  vtkPVArcInfo* info = CmbArc->getArcInfo();
  if(info == NULL)
  {
    pointsFunctions.clear();
    return;
  }
  std::set<vtkIdType> usedIds;
  for(unsigned int i = 0; i < static_cast<unsigned int>(info->GetNumberOfPoints()); ++i)
  {
    vtkIdType id;
    info->GetPointID(i, id);
    std::map<vtkIdType, pointFunctionWrapper>::iterator it = pointsFunctions.find(id);
    usedIds.insert(id);
  }
  for(std::map<vtkIdType, pointFunctionWrapper>::iterator it = pointsFunctions.begin();
      it != pointsFunctions.end();/*in the loop*/)
  {
    if(usedIds.find(it->first) == usedIds.end())
    {
      pointsFunctions.erase(it++);
    }
    else
    {
      ++it;
    }
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
    for( std::map<vtkIdType, pointFunctionWrapper>::iterator it = pointsFunctions.begin();
        it != pointsFunctions.end(); ++it)
    {
      if(it->second.getName() == name)
      {
        it->second.setFunction(fun);
      }
    }
    delete i->second;
    i->second = fun;
  }
}
