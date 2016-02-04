#ifndef qtCMBManualFunctionWidget_h_
#define qtCMBManualFunctionWidget_h_

#include <QWidget>
#include <QPointer>
#include "pqGeneralTransferFunctionWidget.h"

class Ui_qtCMBManualFunctionWidget;
class cmbManualProfileFunction;
class QGridLayout;

class qtCMBManualFunctionWidget: public QWidget
{
  Q_OBJECT
public:
  qtCMBManualFunctionWidget(cmbManualProfileFunction * fun,
                            QWidget * parent);
  ~qtCMBManualFunctionWidget();

public slots:
  void setSemetricMode(bool);
  void updateDepthMax(double);
  void updateDepthMin(double);
  void updateDistMax(double);
  void updateDistMin(double);
  void dispSplineBox(bool);
  void weightSplineBox(bool);
  void relativeChanged(bool);
  void render();

signals:
  void changeDisplacementFunctionType(bool);
protected:
  Ui_qtCMBManualFunctionWidget * Ui;
  cmbManualProfileFunction * function;
  QPointer<pqGeneralTransferFunctionWidget> DisplacementProfile;
  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;
};

#endif
