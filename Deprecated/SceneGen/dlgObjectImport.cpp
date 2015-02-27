#include <QtGui>

#include "dlgObjectImport.h"

dlgObjectImport::dlgObjectImport(QWidget *parent)
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
  gbInRatio = new QGroupBox(tr("Input"));
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
  // Output factor
  gbOutRatio = new QGroupBox(tr("Output"));
  sclOut = new QButtonGroup();
  sclOutKM = new QRadioButton(tr("km"));
  sclOutM = new QRadioButton(tr("m"));
  sclOutCM = new QRadioButton(tr("cm"));
  sclOutMM = new QRadioButton(tr("mm"));
  sclOutFT = new QRadioButton(tr("ft"));
  sclOutIN = new QRadioButton(tr("in"));
  sclOut->addButton(sclOutKM,0);
  sclOut->addButton(sclOutM,1);
  sclOut->addButton(sclOutCM,2);
  sclOut->addButton(sclOutMM,3);
  sclOut->addButton(sclOutFT,4);
  sclOut->addButton(sclOutIN,5);
  sclOut->button(1)->setChecked(true);

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

  //*** Orientation (rhr+y to rhr+z)
  cbYtoZUp = new QCheckBox(tr("Convert from Y to Z as up"));
  cbYtoZUp->setChecked(false);

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
  gbInRatio->setLayout(InLayout);

  QVBoxLayout *OutLayout = new QVBoxLayout;
  OutLayout->addWidget(sclOutKM);
  OutLayout->addWidget(sclOutM);
  OutLayout->addWidget(sclOutCM);
  OutLayout->addWidget(sclOutMM);
  OutLayout->addWidget(sclOutFT);
  OutLayout->addWidget(sclOutIN);
  gbOutRatio->setLayout(OutLayout);

  QHBoxLayout *sclLayout = new QHBoxLayout;
  sclLayout->addWidget(gbInRatio);
  sclLayout->addWidget(gbOutRatio);
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
  yzLayout->addWidget(cbYtoZUp);
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

  setWindowTitle(tr("Import Entity"));
  setFileAttributes("Import Entity", "Entity Files (*.2dm, *.3dm, *.obj, *.dat, *.pts, *_bin.pts)");
  setFixedHeight(sizeHint().height());
  setGlobalUnits(1);
}

void dlgObjectImport::setTitle( QString ttl )
{
  title = ttl;
  setWindowTitle(title);
}

void dlgObjectImport::setFileAttributes( QString hd, QString ft )
{
  head = hd;
  ftypes = ft;
}

void dlgObjectImport::setGlobalUnits( int gu )
{
  GlobalUnits = gu;
}

void dlgObjectImport::importFileName()
{
  QString nfname = QFileDialog::getOpenFileName(this,head, "*.*", ftypes);
  if (!nfname.isEmpty())
  {
    FName->setText(nfname);
  }
}

bool dlgObjectImport::getResults( QString &ofname, double &scale, double &px, double &py, double &pz, int &matid ,bool &rotate)
{
  int iid, oid;

  ofname = FName->text();
  if (ofname.isEmpty()) return false;

  px = valX->value();
  py = valY->value();
  pz = valZ->value();

  matid = valMatID->value();

  //compute scale
  iid = sclIn->checkedId();
  oid = sclOut->checkedId();
  scale = 1.0;
  switch (iid)
  {
  case 0:  //km
    switch (oid)
    {
    case 0:  //km
      scale = 1.0;
      break;
    case 1:  //m
      scale = 1000.0;
      break;
    case 2:  //cm
      scale = 100000.0;
      break;
    case 3:  //mm
      scale = 1000000.0;
      break;
    case 4:  //ft
      scale = 3280.8399;
      break;
    case 5:  //in
      scale = 39370.0787;
      break;
    }
    break;
  case 1:  //m
    switch (oid)
    {
    case 0:  //km
      scale = 0.001;
      break;
    case 1:  //m
      scale = 1.0;
      break;
    case 2:  //cm
      scale = 100.0;
      break;
    case 3:  //mm
      scale = 1000.0;
      break;
    case 4:  //ft
      scale = 3.2808399;
      break;
    case 5:  //in
      scale = 39.3700787;
      break;
    }
    break;
  case 2:  //cm
    switch (oid)
    {
    case 0:  //km
      scale = 0.00001;
      break;
    case 1:  //m
      scale = 0.01;
      break;
    case 2:  //cm
      scale = 1.0;
      break;
    case 3:  //mm
      scale = 10.0;
      break;
    case 4:  //ft
      scale = 0.032808399;
      break;
    case 5:  //in
      scale = 0.393700787;
      break;
    }
    break;
  case 3:  //mm
    switch (oid)
    {
    case 0:  //km
      scale = 0.000001;
      break;
    case 1:  //m
      scale = 0.001;
      break;
    case 2:  //cm
      scale = 0.1;
      break;
    case 3:  //mm
      scale = 1.0;
      break;
    case 4:  //ft
      scale = 0.0032808399;
      break;
    case 5:  //in
      scale = 0.0393700787;
      break;
    }
    break;
  case 4:  //ft
    switch (oid)
    {
    case 0:  //km
      scale = 0.0003048;
      break;
    case 1:  //m
      scale = 0.3048;
      break;
    case 2:  //cm
      scale = 30.48;
      break;
    case 3:  //mm
      scale = 304.8;
      break;
    case 4:  //ft
      scale = 1.0;
      break;
    case 5:  //in
      scale = 12.0;
      break;
    }
    break;
  case 5:  //in
    switch (oid)
    {
    case 0:  //km
      scale = 0.0000254;
      break;
    case 1:  //m
      scale = 0.0254;
      break;
    case 2:  //cm
      scale = 2.54;
      break;
    case 3:  //mm
      scale = 25.4;
      break;
    case 4:  //ft
      scale = 0.08333333333;
      break;
    case 5:  //in
      scale = 1.0;
      break;
    }
    break;
  }

  rotate = (cbYtoZUp->checkState() == Qt::Checked);

  return true;
}
