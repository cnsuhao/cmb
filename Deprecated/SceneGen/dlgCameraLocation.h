#ifndef __dlgCameraLocation_H_
#define __dlgCameraLocation_H_

#include <QDialog>

class QLabel;
class QDoubleSpinBox;
class QPushButton;

class dlgCameraLocation : public QDialog
{
  Q_OBJECT

public:
  dlgCameraLocation(QWidget *parent=0);
  void getResults( double &X, double &Y, double &Z, double &Heading, double &Pitch, double &Yaw );

private:
  QLabel *lblX;
  QDoubleSpinBox *locX;

  QLabel *lblY;
  QDoubleSpinBox *locY;

  QLabel *lblZ;
  QDoubleSpinBox *locZ;

  QLabel *lblHead;
  QDoubleSpinBox *angHead;

  QLabel *lblPitch;
  QDoubleSpinBox *angPitch;

  QLabel *lblYaw;
  QDoubleSpinBox *angYaw;

  QPushButton *AcceptButton;
  QPushButton *CancelButton;
};
#endif
