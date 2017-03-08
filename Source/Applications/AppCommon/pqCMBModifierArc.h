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
#include "vtkBoundingBox.h"

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "cmbProfileFunction.h"

class qtCMBArcEditWidget;
class qtCMBArcWidgetManager;
class pqCMBArc;
class pqPipelineSource;
class vtkPiecewiseFunction;
class vtkSMSourceProxy;

class CMBAPPCOMMON_EXPORT pqCMBModifierArc :  public QObject
{
  Q_OBJECT

private:
  struct profileFunctionWrapper;

public:
  enum FunctionMode{Single = 0, EndPoints = 1, PointAssignment = 2};
  struct pointFunctionWrapper
  {
    friend class pqCMBModifierArc;
  private:
    profileFunctionWrapper const* function;
    vtkIdType ptId;
    vtkIdType pointIndex;
    void setFunction(profileFunctionWrapper const* f);
  public:
    pointFunctionWrapper(pointFunctionWrapper const& other);
    void operator=(pointFunctionWrapper const& other);
    pointFunctionWrapper(profileFunctionWrapper const* fun = NULL);
    ~pointFunctionWrapper();
    cmbProfileFunction const* getFunction() const;
    std::string getName() const;
    vtkIdType getPointId() const;
    vtkIdType getPointIndex() const;
  };
  enum RangeLable{ MIN = 0, MAX = 1};
  pqCMBModifierArc();
  pqCMBModifierArc(vtkSMSourceProxy *proxy);
  ~pqCMBModifierArc() override;

  pqCMBArc * GetCmbArc()
  { return CmbArc; }

  void setId(int i)
  { Id = i; }

  int getId() const { return Id; }

  void setVisablity(bool vis);

  void writeFunction(std::ofstream & f);
  void readFunction(std::ifstream & f, bool import_functions = false);

  void write(std::ofstream & f);
  void read(std::ifstream & f, bool import_functions = false);

  bool updateLabel(std::string str, cmbProfileFunction * fun);

  cmbProfileFunction * getStartFun()
  {
    return const_cast<cmbProfileFunction *>(startFunction->getFunction());
  }
  cmbProfileFunction * getEndFun()
  {
    return const_cast<cmbProfileFunction *>(endFunction->getFunction());
  }
  bool setStartFun(std::string const& name);
  bool setEndFun(std::string const& name);

  void getFunctions(std::vector<cmbProfileFunction*> & funs) const;

  cmbProfileFunction * createFunction();

  bool deleteFunction(std::string const& name);

  cmbProfileFunction * setFunction(std::string const& name, cmbProfileFunction::FunctionType mode);

  cmbProfileFunction * cloneFunction(std::string const& name);

  pointFunctionWrapper * getPointFunction(vtkIdType i);
  pqCMBModifierArc::pointFunctionWrapper const* addFunctionAtPoint(vtkIdType i,
                                                                   cmbProfileFunction * fun);
  void removeFunctionAtPoint(vtkIdType i);
  bool pointHasFunction(vtkIdType i) const;

  FunctionMode getFunctionMode() const;
  void setFunctionMode(FunctionMode fm);

  void getPointFunctions(std::vector<pointFunctionWrapper const*>& result) const;

  bool isRelative() const;

public slots:
  void sendChangeSignals();
  void updateArc(vtkSMSourceProxy* source, vtkBoundingBox bbox);
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
  pqCMBModifierArc::pointFunctionWrapper const* addFunctionAtPoint(vtkIdType i,
                                                                   profileFunctionWrapper * fun);
  pqCMBArc * CmbArc;
  std::map<vtkIdType, pointFunctionWrapper *> pointsFunctions;
  bool IsExternalArc;

  FunctionMode functionMode;

  std::map<std::string, profileFunctionWrapper * > functions;
  pointFunctionWrapper * startFunction;
  pointFunctionWrapper * endFunction;
  
  int Id;
  
  bool IsVisible;

  bool IsRelative;

  void setUpFunction();
};

#endif
