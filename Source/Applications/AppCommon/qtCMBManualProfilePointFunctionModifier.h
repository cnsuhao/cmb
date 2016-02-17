#ifndef qtCMBManualProfilePointFunctionModifier_h_
#define qtCMBManualProfilePointFunctionModifier_h_

#include <QWidget>
#include <vector>

#include "pqCMBModifierArc.h"

class Ui_qtCMBFunctionEditor;
class cmbManualProfileFunction;
class cmbProfileWedgeFunction;
class cmbManualProfileFunctionParameters;
class cmbProfileWedgeFunctionParameters;

class qtCMBManualProfilePointFunctionModifier: public QWidget
{
  Q_OBJECT
public:
  qtCMBManualProfilePointFunctionModifier(QWidget * parent,
                                          std::vector<cmbProfileFunction*> funs,
                                          pqCMBModifierArc::modifierParams & params);
protected slots:
  void setUseDefaults(bool);
  void setLeftDist(double);
  void setRightDist(double);
  void setMinDepth(double);
  void setMaxDepth(double);
  void functionIndexChange(int);

  void setRightSlope(double);
  void setLeftSlope(double);
  void setBaseDistance(double);
  void setDepth(double);
protected:
  Ui_qtCMBFunctionEditor * UI;
  pqCMBModifierArc::modifierParams & modifier;
  cmbManualProfileFunction const* manualFunction;
  cmbProfileWedgeFunction const* wedgeFunction;
  cmbManualProfileFunctionParameters * manualModifierParams;
  cmbProfileWedgeFunctionParameters * wedgeModifierParams;
  std::vector<cmbProfileFunction*> functions;
  size_t cindex;

  void setUp();
};

#endif
