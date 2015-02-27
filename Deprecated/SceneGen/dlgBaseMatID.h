#ifndef __dlgBaseMatID_H_
#define __dlgBaseMatID_H_

#include <QDialog>

class QLabel;
class QSpinBox;
class QPushButton;

class dlgBaseMatID : public QDialog
{
  Q_OBJECT

public:
  dlgBaseMatID(QWidget *parent=0);
  int getBaseMatID();

private:
  QLabel *Lbl;
  QSpinBox *BaseMatVal;
  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
