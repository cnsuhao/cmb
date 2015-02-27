/*=========================================================================

  Program:   CMB
  Module:    qtCMBConeNodeDialog.cxx

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
// .NAME Represents a dialog editing a VOI.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBConeNodeDialog.h"

#include "ui_qtCMBConicalSourceDialog.h"
#include "pqCMBConicalRegion.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include <QLineEdit>
#include <QDoubleValidator>
#include <QIntValidator>

//-----------------------------------------------------------------------------
int qtCMBConeNodeDialog::manageCone(pqCMBSceneNode *node)
{
  if (node->isTypeNode() ||
      (node->getDataObject()->getType() != pqCMBSceneObjectBase::GeneralCone))
    {
    return 0;
    }

  qtCMBConeNodeDialog editor(node);
  return editor.exec();
}

//-----------------------------------------------------------------------------
qtCMBConeNodeDialog::qtCMBConeNodeDialog(pqCMBSceneNode *n) :
  Status(-1), Node(n)
{
  pqCMBConicalRegion *object =
    dynamic_cast<pqCMBConicalRegion*>(n->getDataObject());

  this->MainDialog = new QDialog();
  QDoubleValidator *validator = new QDoubleValidator(this->MainDialog);
  QIntValidator *ivalidator = new QIntValidator(this->MainDialog);
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

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
int qtCMBConeNodeDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
void qtCMBConeNodeDialog::accept()
{
  this->Status = 1;
  double p1[3], a;
  int b;
  // Get the point infomation
  p1[0] = this->ConeDialog->XPos->text().toDouble();
  p1[1] = this->ConeDialog->YPos->text().toDouble();
  p1[2] = this->ConeDialog->ZPos->text().toDouble();

  pqCMBConicalRegion *object =
    dynamic_cast<pqCMBConicalRegion*>(this->Node->getDataObject());
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
//-----------------------------------------------------------------------------
void qtCMBConeNodeDialog::cancel()
{
  this->Status = 0;
}


//-----------------------------------------------------------------------------
