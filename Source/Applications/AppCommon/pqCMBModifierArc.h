//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqCMBModifierArc_h
#define __pqCMBModifierArc_h

#include <QObject>
#include <QAbstractItemView>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "vtkDataObject.h"

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class qtCMBArcWidget;
class qtCMBArcEditWidget;
class qtCMBArcWidgetManager;
class pqCMBArc;
class pqPipelineSource;
class vtkPiecewiseFunction;
class vtkSMSourceProxy;
class cmbManualProfileFunction;
class cmbProfileFunction;
class cmbProfileFunctionParameters;

class CMBAPPCOMMON_EXPORT pqCMBModifierArc :  public QObject
{
  Q_OBJECT

public:
  enum FunctionMode{Single = 0, EndPoints = 1, PointAssignment = 2};
  struct pointFunctionWrapper
  {
  private:
    cmbProfileFunction const* function;
  public:
    pointFunctionWrapper(pointFunctionWrapper const& other);
    void operator=(pointFunctionWrapper const& other);
    pointFunctionWrapper(cmbProfileFunction const* fun = NULL);
    ~pointFunctionWrapper();
    void setFunction(cmbProfileFunction const* f);
    cmbProfileFunction const* getFunction();
    std::string getName();
  };
  enum RangeLable{ MIN = 0, MAX = 1};
  pqCMBModifierArc();
  pqCMBModifierArc(vtkSMSourceProxy *proxy);
  ~pqCMBModifierArc();

  pqCMBArc * GetCmbArc()
  { return CmbArc; }

  void setId(int i)
  { Id = i; }

  int getId() const { return Id; }

  void setVisablity(bool vis);

  void writeFunction(std::ofstream & f);
  void readFunction(std::ifstream & f);

  void write(std::ofstream & f);
  void read(std::ifstream & f);

  bool updateLabel(std::string str, cmbProfileFunction * fun);

  cmbProfileFunction * getStartFun()
  {
    return startFunction;
  }
  cmbProfileFunction * getEndFun()
  {
    return endFunction;
  }
  bool setStartFun(std::string const& name);
  bool setEndFun(std::string const& name);

  void getFunctions(std::vector<cmbProfileFunction*> & funs) const;

  cmbProfileFunction * createFunction();

  bool deleteFunction(std::string const& name);

  void setFunction(std::string const& name, cmbProfileFunction* fun);

  cmbProfileFunction * cloneFunction(std::string const& name);

  pointFunctionWrapper * getPointFunction(vtkIdType i);

  FunctionMode getFunctionMode() const;
  void setFunctionMode(FunctionMode fm);

public slots:
  void sendChangeSignals();
  void updateArc(vtkSMSourceProxy* source);
  void switchToNotEditable();
  void switchToEditable();
  void removeFromServer(vtkSMSourceProxy* source);
  bool setCMBArc(pqCMBArc *);
  void setRelative(bool b);

signals:
  void functionChanged(int);
  void updateDisplacementProfile(/*Displacent info*/);
  void updateWeightProfile(/*Weight info*/);
  void finishCreating();
  void requestRender();

protected:
  //Varable for the path
  pqCMBArc * CmbArc;
  std::map<vtkIdType, pointFunctionWrapper> pointsFunctions;
  bool IsExternalArc;

  FunctionMode functionMode;

  std::map<std::string, cmbProfileFunction * > functions;
  cmbProfileFunction * startFunction;
  cmbProfileFunction * endFunction;
  
  qtCMBArcEditWidget* Modifier;
  int Id;
  
  bool IsVisible;

  bool IsRelative;

  void setUpFunction();
};

#endif
