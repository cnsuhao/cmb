#ifndef __BaseMatIDDialog_H_
#define __BaseMatIDDialog_H_

#include <QDialog>

class QLabel;
class QSpinBox;
class QPushButton;

class BaseMatIDDialog : public QDialog
{
  Q_OBJECT

public:
  BaseMatIDDialog(QWidget *parent=0);
  int getBaseMatID();

private:
  QLabel *Lbl;
  QSpinBox *BaseMatVal;
  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
