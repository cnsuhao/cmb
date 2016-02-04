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

class CMBAPPCOMMON_EXPORT pqCMBModifierArc :  public QObject
{
  Q_OBJECT

public:
  struct modifierParams
  {
    double DistanceRange[2];
    double DisplacementDepthRange[2];
    cmbProfileFunction const* function;
    modifierParams()
    :function(NULL)
    {}
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

  void getDisplacementParams(size_t pt, double & min, double & max) const;
  void setDisplacementParams(size_t pt, double min, double max);

  void getDepthParams(size_t pt, double & min, double & max) const;
  void setDepthParams(size_t pt, double min, double max);

  bool updateLabel(std::string str, cmbProfileFunction * fun);

  cmbProfileFunction * getDefaultFun()
  {
    return defaultFun;
  }
  bool setDefaultFun(std::string const& name);

  void getFunctions(std::vector<cmbProfileFunction*> & funs) const;

  cmbProfileFunction * createFunction();

  bool deleteFunction(std::string const& name);

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
  std::vector<modifierParams> pointsParams;
  bool IsExternalArc;

  std::map<std::string, cmbProfileFunction * > functions;
  cmbProfileFunction * defaultFun;
  
  qtCMBArcEditWidget* Modifier;
  int Id;
  
  bool IsVisible;

  void setUpFunction();
};

#endif
