//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog editing a GroundPlane.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBGroundPlaneDialog.h"

#include "pqCMBPlane.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneTree.h"
#include "ui_qtDefineGroundPlane.h"

#include <QDoubleValidator>
#include <QLineEdit>

int qtCMBGroundPlaneDialog::manageGroundPlane(pqCMBSceneNode* node)
{
  if (node->isTypeNode() || (dynamic_cast<pqCMBPlane*>(node->getDataObject()) == NULL))
  {
    return 0;
  }

  qtCMBGroundPlaneDialog editor(node);
  return editor.exec();
}

int qtCMBGroundPlaneDialog::defineGroundPlane(double p1[3], double p2[3])
{
  int status;
  qtCMBGroundPlaneDialog editor(NULL);
  status = editor.exec();
  int i;
  for (i = 0; i < 3; i++)
  {
    p1[i] = editor.Point1[i];
    p2[i] = editor.Point2[i];
  }
  return status;
}

qtCMBGroundPlaneDialog::qtCMBGroundPlaneDialog(pqCMBSceneNode* n)
  : Status(-1)
  , Node(n)
{
  if (n)
  {
    pqCMBPlane* object = dynamic_cast<pqCMBPlane*>(n->getDataObject());
    object->getPlaneInfo(this->Point1, this->Point2);
  }
  else
  {
    this->Point1[0] = this->Point1[1] = this->Point1[2] = this->Point2[2] = 0.0;
    this->Point2[0] = this->Point2[1] = 1.0;
  }
  this->MainDialog = new QDialog();
  QDoubleValidator* validator = new QDoubleValidator(this->MainDialog);
  this->GroundPlaneDialog = new Ui::qtDefineGroundPlane;
  this->GroundPlaneDialog->setupUi(MainDialog);

  // Prep the line edit widgets
  this->GroundPlaneDialog->X1->setValidator(validator);
  this->GroundPlaneDialog->X1->setText(QString::number(this->Point1[0]));
  this->GroundPlaneDialog->Y1->setValidator(validator);
  this->GroundPlaneDialog->Y1->setText(QString::number(this->Point1[1]));
  this->GroundPlaneDialog->Z1->setValidator(validator);
  this->GroundPlaneDialog->Z1->setText(QString::number(this->Point1[2]));

  this->GroundPlaneDialog->XLength->setValidator(validator);
  this->GroundPlaneDialog->XLength->setText(QString::number(this->Point2[0] - this->Point1[0]));
  this->GroundPlaneDialog->YLength->setValidator(validator);
  this->GroundPlaneDialog->YLength->setText(QString::number(this->Point2[1] - this->Point1[1]));
  //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

qtCMBGroundPlaneDialog::~qtCMBGroundPlaneDialog()
{
  if (this->GroundPlaneDialog)
  {
    delete GroundPlaneDialog;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}

int qtCMBGroundPlaneDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void qtCMBGroundPlaneDialog::accept()
{
  this->Status = 1;
  // Get the point infomation
  this->Point1[0] = this->GroundPlaneDialog->X1->text().toDouble();
  this->Point1[1] = this->GroundPlaneDialog->Y1->text().toDouble();
  this->Point1[2] = this->GroundPlaneDialog->Z1->text().toDouble();

  this->Point2[0] = this->GroundPlaneDialog->XLength->text().toDouble() + this->Point1[0];
  this->Point2[1] = this->GroundPlaneDialog->YLength->text().toDouble() + this->Point1[1];
  this->Point2[2] = this->Point1[2];

  if (this->Node)
  {
    pqCMBPlane* object = dynamic_cast<pqCMBPlane*>(this->Node->getDataObject());
    object->setPlaneInfo(this->Point1, this->Point2);
  }
}

void qtCMBGroundPlaneDialog::cancel()
{
  this->Status = 0;
}
