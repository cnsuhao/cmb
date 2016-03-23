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

  cmbProfileFunction * getDefaultFun()
  {
    return defaultFun;
  }
  bool setDefaultFun(std::string const& name);

  void getFunctions(std::vector<cmbProfileFunction*> & funs) const;

  cmbProfileFunction * createFunction();

  bool deleteFunction(std::string const& name);

  void setFunction(std::string const& name, cmbProfileFunction* fun);

  cmbProfileFunction * cloneFunction(std::string const& name);

  pointFunctionWrapper * getPointFunction(vtkIdType i);

public slots:
  void sendChangeSignals();
  void updateArc(vtkSMSourceProxy* source);
  void switchToNotEditable();
  void switchToEditable();
  void removeFromServer(vtkSMSourceProxy* source);
  bool setCMBArc(pqCMBArc *);

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

  std::map<std::string, cmbProfileFunction * > functions;
  cmbProfileFunction * defaultFun;
  
  qtCMBArcEditWidget* Modifier;
  int Id;
  
  bool IsVisible;

  void setUpFunction();
};

#endif
