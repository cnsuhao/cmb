#include <QtGui>

#include "dlgCameraLocation.h"

dlgCameraLocation::dlgCameraLocation(QWidget *parent)
{
  lblX = new QLabel(tr("X location:"));
  lblY = new QLabel(tr("Y location:"));
  lblZ = new QLabel(tr("Z location:"));
  lblHead = new QLabel(tr("Heading:"));
  lblPitch = new QLabel(tr("Pitch:"));
  lblYaw = new QLabel(tr("Yaw:"));

  locX = new QDoubleSpinBox();
  locX->setDecimals(2);
  locX->setRange(-10000.00, 10000.00);

  locY = new QDoubleSpinBox();
  locY->setDecimals(2);
  locY->setRange(-10000.00, 10000.00);

  locZ = new QDoubleSpinBox();
  locZ->setDecimals(2);
  locZ->setRange(-10000.00, 10000.00);

  angHead = new QDoubleSpinBox();
  angHead->setDecimals(2);
  angHead->setRange(0.00, 360.00);

  angPitch = new QDoubleSpinBox();
  angPitch->setDecimals(2);
  angPitch->setRange(-89.00, 89.00);

  angYaw = new QDoubleSpinBox();
  angYaw->setDecimals(2);
  angYaw->setRange(-90.00, 90.00);

  AcceptButton = new QPushButton(tr("&Accept"));
  AcceptButton->setDefault(true);

  CancelButton = new QPushButton(tr("&Cancel"));

  connect(AcceptButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  //layout information
  QHBoxLayout *XLayout = new QHBoxLayout;
  XLayout->addWidget(lblX);
  XLayout->addWidget(locX);

  QHBoxLayout *YLayout = new QHBoxLayout;
  YLayout->addWidget(lblY);
  YLayout->addWidget(locY);

  QHBoxLayout *ZLayout = new QHBoxLayout;
  ZLayout->addWidget(lblZ);
  ZLayout->addWidget(locZ);

  QHBoxLayout *HeLayout = new QHBoxLayout;
  HeLayout->addWidget(lblHead);
  HeLayout->addWidget(angHead);

  QHBoxLayout *PiLayout = new QHBoxLayout;
  PiLayout->addWidget(lblPitch);
  PiLayout->addWidget(angPitch);

  QHBoxLayout *YaLayout = new QHBoxLayout;
  YaLayout->addWidget(lblYaw);
  YaLayout->addWidget(angYaw);

  QHBoxLayout *btmLayout = new QHBoxLayout;
  btmLayout->addStretch();
  btmLayout->addWidget(AcceptButton);
  btmLayout->addWidget(CancelButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(XLayout);
  mainLayout->addLayout(YLayout);
  mainLayout->addLayout(ZLayout);
  mainLayout->addLayout(HeLayout);
  mainLayout->addLayout(PiLayout);
  mainLayout->addLayout(YaLayout);
  mainLayout->addLayout(btmLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Set Camera View"));
  setFixedHeight(sizeHint().height());
}

void dlgCameraLocation::getResults( double &X, double &Y, double &Z, double &Heading, double &Pitch, double &Yaw )
{
  X = locX->value();
  Y = locY->value();
  Z = locZ->value();
  Heading = angHead->value();
  Pitch = angPitch->value();
  Yaw = angYaw->value();
}
