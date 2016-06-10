#ifndef qtCMBProfilePointFunctionModifier_h_
#define qtCMBProfilePointFunctionModifier_h_

#include <QWidget>
#include <QPointer>
#include <vector>

#include "pqCMBModifierArc.h"
#include "pqGeneralTransferFunctionWidget.h"

class Ui_qtCMBProfileWedgeFunction;
class cmbProfileWedgeFunction;
class cmbProfileWedgeFunctionParameters;

class qtCMBProfileWedgeFunctionWidget: public QWidget
{
  Q_OBJECT
public:
  qtCMBProfileWedgeFunctionWidget(QWidget * parent, cmbProfileWedgeFunction * function);
  ~qtCMBProfileWedgeFunctionWidget();
  void setRelative(bool);
protected slots:
  void setLeftSlope(double);
  void setRightSlope(double);
  void setBaseWidth(double);
  void setDepth(double);
  void setSymmetry(bool);
  void setClamp(bool);
  void setMode(int);
  void weightSplineBox(bool);
  void render();
protected:
  Ui_qtCMBProfileWedgeFunction * UI;
  cmbProfileWedgeFunction * function;
  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;

  void setUp();
  void setWeightFunction();
};

#endif