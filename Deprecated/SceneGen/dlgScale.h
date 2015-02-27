#ifndef __dlgScale_H_
#define __dlgScale_H_

#include <QDialog>

class QLabel;
class QDoubleSpinBox;
class QPushButton;

class dlgScale : public QDialog
{
  Q_OBJECT

public:
  dlgScale(QWidget *parent=0);
  double getResults();

private:
  QLabel *Lbl;
  QDoubleSpinBox *ScaleVal;
  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
