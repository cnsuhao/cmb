#include <QtGui>

#include "BaseMatIDDialog.h"

BaseMatIDDialog::BaseMatIDDialog(QWidget *parent)
{
  Lbl = new QLabel(tr("Base material ID:"));

  BaseMatVal = new QSpinBox();
  BaseMatVal->setRange(0,10000);

  AcceptButton = new QPushButton(tr("&Accept"));
  AcceptButton->setDefault(true);

  CancelButton = new QPushButton(tr("&Cancel"));

  connect(AcceptButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  //layout information
  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->addWidget(Lbl);
  topLayout->addWidget(BaseMatVal);

  QHBoxLayout *btmLayout = new QHBoxLayout;
  btmLayout->addStretch();
  btmLayout->addWidget(AcceptButton);
  btmLayout->addWidget(CancelButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(topLayout);
  mainLayout->addLayout(btmLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Set Base Material ID"));
  setFixedHeight(sizeHint().height());
}

int BaseMatIDDialog::getBaseMatID()
{
  return BaseMatVal->value();
}
