#ifndef __dlgNew_H_
#define __dlgNew_H_

#include <QDialog>

class QLabel;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QDoubleSpinBox;
class QSpinBox;

class dlgNew : public QDialog
{
  Q_OBJECT

public:
  dlgNew(QWidget *parent=0);
  bool getResults( QString &ofname, int &units, double &px, double &py, double &pz, int &matid);
  void setTitle( QString title );

private slots:
  void importFileName();

private:

  QGroupBox *gbFName;
  QLabel *lblFName;
  QLabel *FName;
  QPushButton *pbFName;

  QGroupBox *gbScale;
  QGroupBox *gbUnits;
  QButtonGroup *sclIn;
  QRadioButton *sclInKM;
  QRadioButton *sclInM;
  QRadioButton *sclInCM;
  QRadioButton *sclInMM;
  QRadioButton *sclInFT;
  QRadioButton *sclInIN;

  QGroupBox *gbLoc;
  QLabel *lblX;
  QDoubleSpinBox *valX;
  QLabel *lblY;
  QDoubleSpinBox *valY;
  QLabel *lblZ;
  QDoubleSpinBox *valZ;

  QGroupBox *gbMatID;
  QLabel *lblMatID;
  QSpinBox *valMatID;

  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
