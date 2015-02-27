#include <QtGui>

#include "dlgNew.h"

dlgNew::dlgNew(QWidget *parent)
{
  //*** File name input ***
  gbFName = new QGroupBox(tr("Import File Name"));
  lblFName = new QLabel(tr("File Name:"));
  FName = new QLabel();
  FName->setFixedWidth(320);
  FName->setFrameStyle(QFrame::Box);
  pbFName = new QPushButton(tr("Browse..."));
  connect(pbFName, SIGNAL(clicked()), this, SLOT(importFileName()));

  //*** Scale factor input ***
  gbScale = new QGroupBox(tr("Import Scale Factor"));
  // Input factor
  gbUnits = new QGroupBox(tr("Units"));
  sclIn = new QButtonGroup();
  sclInKM = new QRadioButton(tr("km"));
  sclInM = new QRadioButton(tr("m"));
  sclInCM = new QRadioButton(tr("cm"));
  sclInMM = new QRadioButton(tr("mm"));
  sclInFT = new QRadioButton(tr("ft"));
  sclInIN = new QRadioButton(tr("in"));
  sclIn->addButton(sclInKM,0);
  sclIn->addButton(sclInM,1);
  sclIn->addButton(sclInCM,2);
  sclIn->addButton(sclInMM,3);
  sclIn->addButton(sclInFT,4);
  sclIn->addButton(sclInIN,5);
  sclIn->button(1)->setChecked(true);

  //*** Object Location
  gbLoc = new QGroupBox(tr("Position"));
  lblX = new QLabel(tr("X:"));
  valX = new QDoubleSpinBox();
  valX->setRange(-10000.00, 10000.00);
  valX->setDecimals(4);
  lblY = new QLabel(tr("Y:"));
  valY = new QDoubleSpinBox();
  valY->setRange(-10000.00, 10000.00);
  valY->setDecimals(4);
  lblZ = new QLabel(tr("Z:"));
  valZ = new QDoubleSpinBox();
  valZ->setRange(-10000.00, 10000.00);
  valZ->setDecimals(4);

  //*** Object Material
  gbMatID = new QGroupBox(tr("Base Material"));
  lblMatID = new QLabel(tr("Material ID:"));
  valMatID = new QSpinBox();
  valMatID->setRange(0, 10000);

  //*** Acceptance ***
  AcceptButton = new QPushButton(tr("&Accept"));
  AcceptButton->setDefault(true);
  CancelButton = new QPushButton(tr("&Cancel"));
  connect(AcceptButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  //layout information
  QHBoxLayout *FNLayout = new QHBoxLayout;
  FNLayout->addWidget(lblFName);
  FNLayout->addWidget(FName);
  FNLayout->addWidget(pbFName);

  QVBoxLayout *InLayout = new QVBoxLayout;
  InLayout->addWidget(sclInKM);
  InLayout->addWidget(sclInM);
  InLayout->addWidget(sclInCM);
  InLayout->addWidget(sclInMM);
  InLayout->addWidget(sclInFT);
  InLayout->addWidget(sclInIN);
  gbUnits->setLayout(InLayout);

  QHBoxLayout *sclLayout = new QHBoxLayout;
  sclLayout->addWidget(gbUnits);
  gbScale->setLayout(sclLayout);

  QHBoxLayout *posxLayout = new QHBoxLayout;
  posxLayout->addWidget(lblX);
  posxLayout->addWidget(valX);
  QHBoxLayout *posyLayout = new QHBoxLayout;
  posyLayout->addWidget(lblY);
  posyLayout->addWidget(valY);
  QHBoxLayout *poszLayout = new QHBoxLayout;
  poszLayout->addWidget(lblZ);
  poszLayout->addWidget(valZ);
  QVBoxLayout *posLayout = new QVBoxLayout;
  posLayout->addLayout(posxLayout);
  posLayout->addLayout(posyLayout);
  posLayout->addLayout(poszLayout);
  gbLoc->setLayout(posLayout);

  QHBoxLayout *matLayout = new QHBoxLayout;
  matLayout->addWidget(lblMatID);
  matLayout->addWidget(valMatID);
  gbMatID->setLayout(matLayout);

  QVBoxLayout *yzLayout = new QVBoxLayout;
  yzLayout->addWidget(gbLoc);
  yzLayout->addWidget(gbMatID);
  yzLayout->addStretch();

  QHBoxLayout *MidLayout =  new QHBoxLayout;
  MidLayout->addWidget(gbScale);
  MidLayout->addLayout(yzLayout);

  QHBoxLayout *btmLayout = new QHBoxLayout;
  btmLayout->addStretch();
  btmLayout->addWidget(AcceptButton);
  btmLayout->addWidget(CancelButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(FNLayout);
  mainLayout->addLayout(MidLayout);
  mainLayout->addLayout(btmLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("New Scene"));
  setFixedHeight(sizeHint().height());
}

void dlgNew::setTitle( QString title )
{
  setWindowTitle(title);
}

void dlgNew::importFileName()
{
  QString nfname = QFileDialog::getOpenFileName(this,tr("Import Surface Information"), "*.*", tr("Surface Files (*.2dm *.3dm *.obj *.pts)"));
  if (!nfname.isEmpty())
  {
    FName->setText(nfname);
  }
}

bool dlgNew::getResults( QString &ofname, int &units, double &px, double &py, double &pz, int &matid)
{
  ofname = FName->text();
  if (ofname.isEmpty()) return false;

  px = valX->value();
  py = valY->value();
  pz = valZ->value();

  matid = valMatID->value();

  //compute scale
  units = sclIn->checkedId();

  return true;
}
