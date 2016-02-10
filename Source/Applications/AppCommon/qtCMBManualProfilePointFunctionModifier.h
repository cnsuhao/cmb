#ifndef qtCMBManualProfilePointFunctionModifier_h_
#define qtCMBManualProfilePointFunctionModifier_h_

#include <QWidget>
#include <vector>

#include "pqCMBModifierArc.h"

class Ui_qtCMBFunctionEditor;
class cmbManualProfileFunction;

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
protected:
  Ui_qtCMBFunctionEditor * UI;
  pqCMBModifierArc::modifierParams & modifier;
  cmbManualProfileFunction const* function;
  std::vector<cmbProfileFunction*> functions;
  size_t cindex;

  void setUp();
};

#endif
