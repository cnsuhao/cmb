/*=========================================================================

  Program:   CMB
  Module:    qtCMBVOIDialog.cxx

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

#include "qtCMBVOIDialog.h"

#include "ui_qtDefineVOI.h"
#include "pqCMBVOI.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

#include <QLineEdit>
#include <QDoubleValidator>

//-----------------------------------------------------------------------------
int qtCMBVOIDialog::manageVOI(pqCMBSceneNode *node)
{
  if (node->isTypeNode() ||
      (node->getDataObject()->getType() != pqCMBSceneObjectBase::VOI))
    {
    return 0;
    }

  qtCMBVOIDialog editor(node);
  return editor.exec();
}

//-----------------------------------------------------------------------------
qtCMBVOIDialog::qtCMBVOIDialog(pqCMBSceneNode *n) :
  Status(-1), Node(n)
{
  pqCMBVOI *object = dynamic_cast<pqCMBVOI*>(n->getDataObject());

  this->MainDialog = new QDialog();
  QDoubleValidator *validator = new QDoubleValidator(this->MainDialog);
  this->VOIDialog = new Ui::qtDefineVOI;
  this->VOIDialog->setupUi(MainDialog);

  double p1[3], p2[3];
  object->getVOI(p1, p2);

  // Prep the line edit widgets
  this->VOIDialog->X1->setValidator(validator);
  this->VOIDialog->X1->setText(QString::number(p1[0]));
  this->VOIDialog->Y1->setValidator(validator);
  this->VOIDialog->Y1->setText(QString::number(p1[1]));
  this->VOIDialog->Z1->setValidator(validator);
  this->VOIDialog->Z1->setText(QString::number(p1[2]));

  this->VOIDialog->X2->setValidator(validator);
  this->VOIDialog->X2->setText(QString::number(p2[0]));
  this->VOIDialog->Y2->setValidator(validator);
  this->VOIDialog->Y2->setText(QString::number(p2[1]));
  this->VOIDialog->Z2->setValidator(validator);
  this->VOIDialog->Z2->setText(QString::number(p2[2]));
   //
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

//-----------------------------------------------------------------------------
qtCMBVOIDialog::~qtCMBVOIDialog()
{
  if (this->VOIDialog)
    {
    delete VOIDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
int qtCMBVOIDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
void qtCMBVOIDialog::accept()
{
  this->Status = 1;
  double p1[3], p2[3];
  // Get the point infomation
  p1[0] = this->VOIDialog->X1->text().toDouble();
  p1[1] = this->VOIDialog->Y1->text().toDouble();
  p1[2] = this->VOIDialog->Z1->text().toDouble();

  p2[0] = this->VOIDialog->X2->text().toDouble();
  p2[1] = this->VOIDialog->Y2->text().toDouble();
  p2[2] = this->VOIDialog->Z2->text().toDouble();

  int i;
  double a;
  // Make sure p1 is the min pnt and p2 is the max
  for (i = 0; i < 3; i++)
    {
    if (p1[i] > p2[i])
      {
      a = p1[i];
      p1[i] = p2[i];
      p2[i] = a;
      }
    }
  pqCMBVOI *object = dynamic_cast<pqCMBVOI*>(this->Node->getDataObject());
  object->setVOI(p1, p2);
}
//-----------------------------------------------------------------------------
void qtCMBVOIDialog::cancel()
{
  this->Status = 0;
}


//-----------------------------------------------------------------------------
