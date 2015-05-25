//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for creating stack of TINS in SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBStackedTINDialog.h"

#include "ui_qtCMBStackedTINDialog.h"
#include "pqCMBSceneObjectBase.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

#include <QLineEdit>
#include <QDoubleValidator>

//-----------------------------------------------------------------------------
int qtCMBStackedTINDialog::processTIN(pqCMBSceneNode *node)
{
  if (node->isTypeNode())
    {
    return 0;
    }

  qtCMBStackedTINDialog importer(node);
  return importer.exec();
}

//-----------------------------------------------------------------------------
qtCMBStackedTINDialog::qtCMBStackedTINDialog(pqCMBSceneNode *n) :
  Status(-1), Node(n)
{
  pqCMBSceneObjectBase *object = n->getDataObject();

  this->MainDialog = new QDialog();
  QDoubleValidator *validator = new QDoubleValidator(this->MainDialog);
  this->StackDialog = new Ui::qtSceneGenqtCMBStackedTINDialog;
  this->StackDialog->setupUi(MainDialog);

  // Set up connections
  QObject::connect(this->StackDialog->NumberOfLayers, SIGNAL(valueChanged(int)),
                   this, SLOT(setNumberOfLayers(int)));

  QObject::connect(this->StackDialog->TotalThickness, SIGNAL(valueChanged(double)),
                   this, SLOT(setTotalThickness(double)));

  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));

  this->StackDialog->LayerInformation->setRowCount(1);
  QLineEdit *entry = new QLineEdit(this->StackDialog->LayerInformation);
  entry->setValidator(validator);
  QObject::connect(entry, SIGNAL(editingFinished()), this, SLOT(offsetChanged()));

  this->StackDialog->LayerInformation->setCellWidget(0, 0, entry);
  this->StackDialog->TotalThickness->setValue(1.0);
}

//-----------------------------------------------------------------------------
qtCMBStackedTINDialog::~qtCMBStackedTINDialog()
{
  if (this->StackDialog)
    {
    delete StackDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
void qtCMBStackedTINDialog::setTotalThickness(double thickness)
{
  // Distribute this equally amoung the copies
  int n = this->StackDialog->LayerInformation->rowCount();
  int i;
  double delta = thickness / static_cast<double>(n);
  QLineEdit *entry;
  QString val;
  val.setNum(delta);
  for (i = 0; i < n; i++)
    {
    entry =
      dynamic_cast<QLineEdit*>(this->StackDialog->LayerInformation->cellWidget(i, 0));
    entry->blockSignals(true);
    entry->setText(val);
    entry->blockSignals(false);
    }
}
//-----------------------------------------------------------------------------
void qtCMBStackedTINDialog::setNumberOfLayers(int n)
{
  double delta = this->StackDialog->TotalThickness->value() / static_cast<double>(n);
  this->StackDialog->LayerInformation->setRowCount(n);
  // Grab the validator from the first cell
  QLineEdit *entry =
    dynamic_cast<QLineEdit*>(this->StackDialog->LayerInformation->cellWidget(0, 0));
  const QValidator *validator = entry->validator();

  // Set the Column label and the LineEdit entry
  // Distribute this equally amoung the copies
  int i;
  QString val;
  val.setNum(delta);
  for (i = 0; i < n; i++)
    {
    entry = new QLineEdit(this->StackDialog->LayerInformation);
    entry->setText(val);
    entry->setValidator(validator);
    QObject::connect(entry, SIGNAL(editingFinished()), this, SLOT(offsetChanged()));
    this->StackDialog->LayerInformation->setCellWidget(i, 0, entry);
    }
}
//-----------------------------------------------------------------------------
int qtCMBStackedTINDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
void qtCMBStackedTINDialog::accept()
{
  this->Status = 1;
  int n = this->StackDialog->LayerInformation->rowCount();
  int i;
  double totalDisplacement = 0.0, delta;
  QLineEdit *entry;
  QString val;
  pqCMBSceneTree *tree = this->Node->getTree();
  pqCMBSceneNode *newNode;
  for (i = 0; i < n; i++)
    {
    entry =
      dynamic_cast<QLineEdit*>(this->StackDialog->LayerInformation->cellWidget(i, 0));
    val = entry->text();
    delta = val.toDouble();
    totalDisplacement += delta;
    newNode = tree->duplicateNode(this->Node, totalDisplacement);
    newNode->select();
    }
}
//-----------------------------------------------------------------------------
void qtCMBStackedTINDialog::offsetChanged()
{
  int n = this->StackDialog->LayerInformation->rowCount();
  int i;
  double totalDisplacement = 0.0, delta;
  QLineEdit *entry;
  QString val;
  for (i = 0; i < n; i++)
    {
    entry =
      dynamic_cast<QLineEdit*>(this->StackDialog->LayerInformation->cellWidget(i, 0));
    val = entry->text();
    delta = val.toDouble();
    totalDisplacement += delta;
    }
  this->StackDialog->TotalThickness->blockSignals(true);
  this->StackDialog->TotalThickness->setValue(totalDisplacement);
  this->StackDialog->TotalThickness->blockSignals(false);
}
//-----------------------------------------------------------------------------
void qtCMBStackedTINDialog::cancel()
{
  this->Status = 0;
}

//-----------------------------------------------------------------------------
