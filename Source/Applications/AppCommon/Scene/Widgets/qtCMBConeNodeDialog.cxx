//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog editing a VOI.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBConeNodeDialog.h"

#include "pqCMBConicalRegion.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneTree.h"
#include "ui_qtCMBConicalSourceDialog.h"
#include <QDoubleValidator>
#include <QIntValidator>
#include <QLineEdit>

int qtCMBConeNodeDialog::manageCone(pqCMBSceneNode* node)
{
  if (node->isTypeNode() || (node->getDataObject()->getType() != pqCMBSceneObjectBase::GeneralCone))
  {
    return 0;
  }

  qtCMBConeNodeDialog editor(node);
  return editor.exec();
}

qtCMBConeNodeDialog::qtCMBConeNodeDialog(pqCMBSceneNode* n)
  : Status(-1)
  , Node(n)
{
  pqCMBConicalRegion* object = dynamic_cast<pqCMBConicalRegion*>(n->getDataObject());

  this->MainDialog = new QDialog();
  QDoubleValidator* validator = new QDoubleValidator(this->MainDialog);
  QIntValidator* ivalidator = new QIntValidator(this->MainDialog);
  ivalidator->setBottom(3);
  this->ConeDialog = new Ui::qtCMBConicalSourceDialog;
  this->ConeDialog->setupUi(MainDialog);

  double p1[3];
  double a;
  int b;
  object->getBaseCenter(p1);

  // Prep the line edit widgets
  this->ConeDialog->XPos->setValidator(validator);
  this->ConeDialog->XPos->setText(QString::number(p1[0]));
  this->ConeDialog->YPos->setValidator(validator);
  this->ConeDialog->YPos->setText(QString::number(p1[1]));
  this->ConeDialog->ZPos->setValidator(validator);
  this->ConeDialog->ZPos->setText(QString::number(p1[2]));

  a = object->getHeight();
  this->ConeDialog->Depth->setValidator(validator);
  this->ConeDialog->Depth->setText(QString::number(a));

  a = object->getBaseRadius();
  this->ConeDialog->BaseRadius->setValidator(validator);
  this->ConeDialog->BaseRadius->setText(QString::number(a));

  a = object->getTopRadius();
  this->ConeDialog->DepthRadius->setValidator(validator);
  this->ConeDialog->DepthRadius->setText(QString::number(a));

  b = object->getResolution();
  this->ConeDialog->Resolution->setValidator(ivalidator);
  this->ConeDialog->Resolution->setText(QString::number(b));

  //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

qtCMBConeNodeDialog::~qtCMBConeNodeDialog()
{
  if (this->ConeDialog)
  {
    delete ConeDialog;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}

int qtCMBConeNodeDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void qtCMBConeNodeDialog::accept()
{
  this->Status = 1;
  double p1[3], a;
  int b;
  // Get the point infomation
  p1[0] = this->ConeDialog->XPos->text().toDouble();
  p1[1] = this->ConeDialog->YPos->text().toDouble();
  p1[2] = this->ConeDialog->ZPos->text().toDouble();

  pqCMBConicalRegion* object = dynamic_cast<pqCMBConicalRegion*>(this->Node->getDataObject());
  object->setBaseCenter(p1);
  a = this->ConeDialog->Depth->text().toDouble();
  object->setHeight(a);
  a = this->ConeDialog->DepthRadius->text().toDouble();
  object->setTopRadius(a);
  a = this->ConeDialog->BaseRadius->text().toDouble();
  object->setBaseRadius(a);
  b = this->ConeDialog->Resolution->text().toInt();
  object->setResolution(b);
}

void qtCMBConeNodeDialog::cancel()
{
  this->Status = 0;
}
