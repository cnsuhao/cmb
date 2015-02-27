#include <QtGui>

#include "dlgScale.h"

dlgScale::dlgScale(QWidget *parent)
{
  Lbl = new QLabel(tr("Scale Factor:"));

  ScaleVal = new QDoubleSpinBox();
  ScaleVal->setDecimals(4);
  ScaleVal->setRange(-10000,10000);
  ScaleVal->setSingleStep(0.001);
  ScaleVal->setValue(1.0);

  AcceptButton = new QPushButton(tr("&Accept"));
  AcceptButton->setDefault(true);

  CancelButton = new QPushButton(tr("&Cancel"));

  connect(AcceptButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  //layout information
  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->addWidget(Lbl);
  topLayout->addWidget(ScaleVal);

  QHBoxLayout *btmLayout = new QHBoxLayout;
  btmLayout->addStretch();
  btmLayout->addWidget(AcceptButton);
  btmLayout->addWidget(CancelButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(topLayout);
  mainLayout->addLayout(btmLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Change Relative Scale Factor"));
  setFixedHeight(sizeHint().height());
}

double dlgScale::getResults()
{
  double ret;

  ret = ScaleVal->value();
  if (ret==0.0) ret = 0.0001;

  return (ret);
}
