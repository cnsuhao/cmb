#ifndef __dlgObjectImport_H_
#define __dlgObjectImport_H_

#include <QDialog>

class QLabel;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QDoubleSpinBox;
class QSpinBox;

class dlgObjectImport : public QDialog
{
  Q_OBJECT

public:
  dlgObjectImport(QWidget *parent=0);
  bool getResults( QString &ofname, double &scale, double &px, double &py, double &pz, int &matid, bool &rotate);
  void setTitle( QString ttl );
  void setFileAttributes( QString hd, QString ft );
  void setGlobalUnits( int gunits );

private slots:
  void importFileName();

private:
  QString title;
  QString head;
  QString ftypes;
  int GlobalUnits;

  QGroupBox *gbFName;
  QLabel *lblFName;
  QLabel *FName;
  QPushButton *pbFName;

  QGroupBox *gbScale;
  QGroupBox *gbInRatio;
  QButtonGroup *sclIn;
  QRadioButton *sclInKM;
  QRadioButton *sclInM;
  QRadioButton *sclInCM;
  QRadioButton *sclInMM;
  QRadioButton *sclInFT;
  QRadioButton *sclInIN;
  QGroupBox *gbOutRatio;
  QButtonGroup *sclOut;
  QRadioButton *sclOutKM;
  QRadioButton *sclOutM;
  QRadioButton *sclOutCM;
  QRadioButton *sclOutMM;
  QRadioButton *sclOutFT;
  QRadioButton *sclOutIN;

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

  QCheckBox *cbYtoZUp;

  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
