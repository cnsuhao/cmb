/*=========================================================================

  Program:   CMB
  Module:    qtCMBGroundPlaneDialog.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADiVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME Represents a dialog editing a GroundPlane.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBGroundPlaneDialog.h"

#include "ui_qtDefineGroundPlane.h"
#include "pqCMBPlane.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

#include <QLineEdit>
#include <QDoubleValidator>

//-----------------------------------------------------------------------------
int qtCMBGroundPlaneDialog::manageGroundPlane(pqCMBSceneNode *node)
{
  if (node->isTypeNode() ||
      (dynamic_cast<pqCMBPlane*>(node->getDataObject()) == NULL))
    {
    return 0;
    }

  qtCMBGroundPlaneDialog editor(node);
  return editor.exec();
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
qtCMBGroundPlaneDialog::qtCMBGroundPlaneDialog(pqCMBSceneNode *n) :
  Status(-1), Node(n)
{
  if (n)
    {
    pqCMBPlane *object = dynamic_cast<pqCMBPlane*>(n->getDataObject());
    object->getPlaneInfo(this->Point1, this->Point2);
    }
  else
    {
    this->Point1[0] = this->Point1[1] = this->Point1[2] = this->Point2[2]= 0.0;
    this->Point2[0] = this->Point2[1]  = 1.0;
    }
  this->MainDialog = new QDialog();
  QDoubleValidator *validator = new QDoubleValidator(this->MainDialog);
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
  this->GroundPlaneDialog->XLength->
    setText(QString::number(this->Point2[0] - this->Point1[0]));
  this->GroundPlaneDialog->YLength->setValidator(validator);
  this->GroundPlaneDialog->YLength->
    setText(QString::number(this->Point2[1] - this->Point1[1]));
   //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
int qtCMBGroundPlaneDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
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
    pqCMBPlane *object =
      dynamic_cast<pqCMBPlane*>(this->Node->getDataObject());
    object->setPlaneInfo(this->Point1, this->Point2);
    }
}
//-----------------------------------------------------------------------------
void qtCMBGroundPlaneDialog::cancel()
{
  this->Status = 0;
}


//-----------------------------------------------------------------------------
