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

//#include "qtCMBArcWidget.h"

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
#include "cmbProfileWedgeFunction.h"
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

cmbProfileFunction const* pqCMBModifierArc::pointFunctionWrapper::getFunction() const
{
  return function;
}

void
pqCMBModifierArc::pointFunctionWrapper::setFunction(cmbProfileFunction const* f)
{
  function = f;
}

std::string pqCMBModifierArc::pointFunctionWrapper::getName() const
{
  if(function) return function->getName();
  return "NULL";
}

vtkIdType pqCMBModifierArc::pointFunctionWrapper::getPointId() const
{
  return ptId;
}

vtkIdType pqCMBModifierArc::pointFunctionWrapper::getPointIndex() const
{
  return pointIndex;
}

////////////////////////////////////////

pqCMBModifierArc::pqCMBModifierArc()
:CmbArc(new pqCMBArc()), IsExternalArc(false), IsVisible(true), IsRelative(true),
 functionMode(Single)
{
  cmbProfileWedgeFunction * fun = new cmbProfileWedgeFunction();
  startFunction = new pointFunctionWrapper(fun);
  endFunction = new pointFunctionWrapper(fun);
  fun->setName("FUN0");
  fun->setRelative(true);
  functions[startFunction->getName()] = fun;
  setUpFunction();
}

pqCMBModifierArc::pqCMBModifierArc(vtkSMSourceProxy *proxy)
:CmbArc(new pqCMBArc(proxy)),
 IsExternalArc(false), IsVisible(true), IsRelative( true )
{
  IsVisible = true;
  cmbProfileWedgeFunction * fun = new cmbProfileWedgeFunction();
  startFunction = new pointFunctionWrapper(fun);
  endFunction = new pointFunctionWrapper(fun);
  fun->setName("FUN0");
  fun->setRelative(true);
  functions[startFunction->getName()] = fun;
}

pqCMBModifierArc::~pqCMBModifierArc()
{
  delete startFunction;
  delete endFunction;
  if(!IsExternalArc) delete CmbArc;
  for(std::map<std::string, cmbProfileFunction * >::iterator i = functions.begin();
      i != functions.end(); ++i)
  {
    delete i->second;
    i->second = NULL;
  }
  functions.clear();

  for( std::map<vtkIdType, pointFunctionWrapper*>::iterator it = pointsFunctions.begin();
      it != pointsFunctions.end(); ++it)
  {
    delete it->second;
  }
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
  if(info == NULL) return;
  setUpFunction();
  CmbArc->arcIsModified();
  QList< QVariant > v;
  CmbArc->updateRepresentation();
    {
    //clear functions
    v.clear();
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
    v.clear();
    v <<Id << tmp[0] << tmp[1];
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddPoint"), v);
    source->UpdateVTKObjects();
    }
  if(info->IsClosedLoop())
    {
    v.clear();
    v <<Id;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetArcAsClosed"), v);
    source->UpdateVTKObjects();
    v.clear();
    v << -1;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetArcAsClosed"), v);
    source->UpdateVTKObjects();
    }

  switch(functionMode)
  {
    case EndPoints:
      if(endFunction->getFunction() != startFunction->getFunction())
      {
        v.clear();
        endFunction->getFunction()->sendDataToProxy(Id, 1, source);
        v << Id << info->GetNumberOfPoints()-1 << 1;
        pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
        source->UpdateVTKObjects();
      }
    case Single:
      v.clear();
      startFunction->getFunction()->sendDataToProxy(Id, 0, source);
      v << Id << 0 << 0;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
      source->UpdateVTKObjects();
      break;
    case PointAssignment:
    {
      std::map< cmbProfileFunction const*, unsigned int> function_to_id;
      std::map<std::string, cmbProfileFunction * >::const_iterator iter = functions.begin();
      for(unsigned int i = 0; iter != functions.end(); ++i,++iter)
      {
        iter->second->sendDataToProxy(Id, i, source);
        function_to_id[iter->second] = i;
      }
      for(unsigned int i = 0; i < static_cast<unsigned int>(info->GetNumberOfPoints());++i)
      {
        vtkIdType id;
        info->GetPointID(i, id);
        pointFunctionWrapper * wrapper = pointsFunctions[id];
        if(wrapper != NULL && wrapper->getFunction() != NULL)
        {
          v.clear();
          v << Id << i << function_to_id[wrapper->getFunction()];
          pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
          source->UpdateVTKObjects();
        }
      }
    }
  }
  {
    v.clear();
    v << -1 << -1 << -1;
    pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetFunctionToPoint"), v);
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
  f << 2 << '\n'
    << functions.size() << '\n';
  for(std::map<std::string, cmbProfileFunction * >::const_iterator iter = functions.begin();
      iter != functions.end(); ++iter)
  {
    iter->second->write(f);
  }
  switch( functionMode )
  {
    case Single:
      f << startFunction->getName() << '\n';
      break;
    case EndPoints:
      f << startFunction->getName() << '\n';
      f << endFunction->getName() << '\n';
      break;
    case PointAssignment:
    {
      std::vector<pqCMBModifierArc::pointFunctionWrapper const*> functions;
      this->getPointFunctions(functions);
      f << functions.size() << std::endl;
      for(unsigned int i = 0; i < functions.size(); ++i)
      {
        f << functions[i]->getPointId() << " " << functions[i]->getName() << '\n';
      }
    }
  }
}

void pqCMBModifierArc::readFunction(std::ifstream & f, bool import_functions)
{
  int version;
  f >> version;
  int size;
  f >> size;
  std::map<std::string, cmbProfileFunction * > tmp_fun;
  for( int i = 0; i < size; ++i )
  {
    cmbProfileFunction * cpf = cmbProfileFunction::read(f, i);
    tmp_fun[cpf->getName()] = cpf;
  }

  if(!import_functions)
  {
    this->functions.clear();
  }

  for(std::map<std::string, cmbProfileFunction * >::const_iterator iter = tmp_fun.begin();
      iter != tmp_fun.end(); ++iter)
  {
    std::string str = iter->first;
    int i = 0;
    while(functions.find(str) != functions.end())
    {
      std::stringstream ss;
      ss << iter->first << i++;
      ss >> str;
    }
    iter->second->setName(str);
    this->setFunction(str, iter->second);
  }

  std::string name[2];
  vtkIdType id;
  switch( functionMode )
  {
    case Single:
      f >> name[0];
      if(!import_functions)
      {
        cmbProfileFunction * f = tmp_fun[name[0]];
        setStartFun(f->getName());
        setEndFun(f->getName());
      }
      break;
    case EndPoints:
      f >> name[0] >> name[1];
      if(!import_functions)
      {
        cmbProfileFunction * f = tmp_fun[name[0]];
        setStartFun(f->getName());
        f = tmp_fun[name[1]];
        setEndFun(f->getName());
      }
      break;
    case PointAssignment:
    {
      int count;
      f >> count;
      for(int i = 0; i < count; ++i)
      {
        f >> id >> name[0];
        if(!import_functions)
        {
          cmbProfileFunction * f = tmp_fun[name[0]];
          this->addFunctionAtPoint(id, f);
        }
      }
    }
  }
}

void pqCMBModifierArc::write(std::ofstream & f)
{
  f << 2 << "\n";
  if(CmbArc->getArcInfo() == NULL)
  {
    f << 0 << "\n";
    return;
  }
  f << 1 << "\n";
  f << IsVisible << "\n";
  f << static_cast<int>(functionMode) << std::endl;
  f << CmbArc->getPlaneProjectionNormal() << " " << CmbArc->getPlaneProjectionPosition() << "\n";
  writeFunction(f);
}

void pqCMBModifierArc::read(std::ifstream & f, bool import_function)
{
  if(!import_function)
  {
    setUpFunction();
  }
  int version;
  f >> version;
  assert(version == 2);
  int hasInfo;
  f >> hasInfo;
  if(!hasInfo) return;
  if(import_function)
  {
    bool v;
    f >> IsVisible;
    int t;
    f >> t;
  }
  else
  {
    f >> IsVisible;
    int mode;
    f >> mode;
    functionMode = static_cast<FunctionMode>(mode);
  }
  int norm;
  double pos;
  f >> norm >> pos;
  readFunction(f, import_function);
  if(!import_function)
  {
    CmbArc->setPlaneProjectionNormal(norm);
    CmbArc->setPlaneProjectionPosition(pos);
    setUpFunction();
  }
}

pqCMBModifierArc::pointFunctionWrapper * pqCMBModifierArc::getPointFunction(vtkIdType i)
{
  std::map<vtkIdType, pointFunctionWrapper *>::iterator it = pointsFunctions.find(i);
  return (it == pointsFunctions.end())?NULL:it->second;
}

bool pqCMBModifierArc::pointHasFunction(vtkIdType i) const
{
  std::map<vtkIdType, pointFunctionWrapper *>::const_iterator it =pointsFunctions.find(i);
  return (it != pointsFunctions.end()) && it->second != NULL && it->second->getFunction() != NULL;
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
    std::map<vtkIdType, pointFunctionWrapper*>::iterator it = pointsFunctions.find(id);
    if(it == pointsFunctions.end())
    {
      pointsFunctions[id] = new pointFunctionWrapper(NULL);
      pointsFunctions[id]->ptId = id;
      pointsFunctions[id]->pointIndex = i;
    }
    else
    {
      it->second->pointIndex = i;
    }
    usedIds.insert(id);
  }

  //clean out cruff
  for(std::map<vtkIdType, pointFunctionWrapper*>::iterator it = pointsFunctions.begin();
      it != pointsFunctions.end();/*in the loop*/)
  {
    if(usedIds.find(it->first) == usedIds.end())
    {
      delete it->second;
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

bool pqCMBModifierArc::setStartFun(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return false;
  }
  startFunction->setFunction(i->second);
  return true;
}

bool pqCMBModifierArc::setEndFun(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return false;
  }
  endFunction->setFunction(i->second);
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
  cmbProfileFunction * result = new cmbProfileWedgeFunction();
  result->setName(name);
  functions[name] = result;
  result->setRelative(this->IsRelative);
  return result;
}

bool pqCMBModifierArc::deleteFunction(std::string const& name)
{
  std::map<std::string, cmbProfileFunction * >::iterator i = functions.find(name);
  if(i == functions.end())
  {
    return false;
  }
  if(functions.size() == 1) return false;
  cmbProfileFunction * fun = i->second;
  functions.erase(i);
  if(fun == startFunction->getFunction()) startFunction->setFunction( functions.begin()->second );
  if(fun == endFunction->getFunction()) endFunction->setFunction( functions.begin()->second );
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
  fun->setRelative(IsRelative);
  if(i == functions.end())
  {
    functions[name] = fun;
  }
  else
  {
    if(startFunction->getFunction() == i->second) startFunction->setFunction(fun);
    fun->setName(name);
    for( std::map<vtkIdType, pointFunctionWrapper*>::iterator it = pointsFunctions.begin();
        it != pointsFunctions.end(); ++it)
    {
      if(it->second != NULL && it->second->getName() == name)
      {
        it->second->setFunction(fun);
      }
    }
    if(i->second == startFunction->getFunction()) startFunction->setFunction( fun );
    if(i->second == endFunction->getFunction())   endFunction->setFunction( fun );
    delete i->second;
    i->second = fun;
  }
}

pqCMBModifierArc::FunctionMode pqCMBModifierArc::getFunctionMode() const
{
  return this->functionMode;
}

void pqCMBModifierArc::setFunctionMode(pqCMBModifierArc::FunctionMode fm)
{
  this->functionMode = fm;
}

void pqCMBModifierArc::setRelative(bool b)
{
  this->IsRelative = b;
  for(std::map<std::string, cmbProfileFunction * >::iterator i = functions.begin();
      i != functions.end(); ++i)
  {
    i->second->setRelative(b);
  }
}

pqCMBModifierArc::pointFunctionWrapper const*
pqCMBModifierArc::addFunctionAtPoint(vtkIdType i, cmbProfileFunction * fun)
{
  std::map<vtkIdType, pointFunctionWrapper *>::iterator it = pointsFunctions.find(i);
  if (it == pointsFunctions.end() && fun == NULL) return NULL;
  if (fun == NULL)
  {
    this->removeFunctionAtPoint(i);
    return NULL;
  }
  if (it != pointsFunctions.end() && it->second != NULL)
  {
    it->second->function = fun;
    return it->second;
  }
  else
  {
    assert(false && "the wrapper should always exist");
    return pointsFunctions[i];
  }
}

void pqCMBModifierArc::removeFunctionAtPoint(vtkIdType i)
{
  std::map<vtkIdType, pointFunctionWrapper *>::iterator it = pointsFunctions.find(i);
  if (it == pointsFunctions.end()) return;
  it->second->function = NULL;
}

void pqCMBModifierArc
::getPointFunctions(std::vector<pqCMBModifierArc::pointFunctionWrapper const*>& result) const
{
  result.clear();
  vtkPVArcInfo* info = CmbArc->getArcInfo();
  if(info == NULL) return;
  vtkIdType id;
  switch(functionMode)
  {
    case PointAssignment:
      for(unsigned int i = 0; i < info->GetNumberOfPoints(); ++i)
      {
        info->GetPointID(i, id);
        pointFunctionWrapper const* wrapper = this->pointsFunctions.find(id)->second;
        if(wrapper != NULL && wrapper->getFunction() != NULL)
        {
          result.push_back(wrapper);
        }
      }
      return;
    case EndPoints:
      result.reserve(2);
      info->GetPointID(info->GetNumberOfPoints()-1, id);
      this->endFunction->ptId = id;
      this->endFunction->pointIndex = info->GetNumberOfPoints()-1;
      result.push_back(this->endFunction);
    case Single:
      info->GetPointID(0, id);
      this->startFunction->ptId = id;
      this->startFunction->pointIndex = 0;
      result.insert(result.begin(), this->startFunction); //add to front
  }
}

bool pqCMBModifierArc
::isRelative() const
{
  return IsRelative;
}
