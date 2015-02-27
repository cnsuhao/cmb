/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneObjectFilterDialog.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME Represents a dialog for importing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBSceneObjectFilterDialog.h"

#include "ui_qtCMBSceneObjectFilterDialog.h"
#include "pqCMBSceneObjectBase.h"
#include <QMessageBox>
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include <QDoubleValidator>
#include "vtkBoundingBox.h"

//-----------------------------------------------------------------------------
qtCMBSceneObjectFilterDialog::qtCMBSceneObjectFilterDialog(QWidget* /*parent*/)
{
  this->FilterDialog = new Ui::qtqtCMBSceneObjectFilterDialog();
  this->FilterDialog->setupUi(this);

  this->BoundsValidator = new QDoubleValidator(
    this->FilterDialog->groupBox_Bounds);
  this->FilterDialog->X1->setValidator(this->BoundsValidator);
  this->FilterDialog->X2->setValidator(this->BoundsValidator);
  this->FilterDialog->Y1->setValidator(this->BoundsValidator);
  this->FilterDialog->Y2->setValidator(this->BoundsValidator);
  this->FilterDialog->Z1->setValidator(this->BoundsValidator);
  this->FilterDialog->Z2->setValidator(this->BoundsValidator);

}

//-----------------------------------------------------------------------------
qtCMBSceneObjectFilterDialog::~qtCMBSceneObjectFilterDialog()
{
  if (this->FilterDialog)
    {
    delete FilterDialog;
    }
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::accept()
{
  if(this->FilterDialog->listObjectTypes->selectedItems().count()==0)
    {
    QMessageBox::warning(this,
      "No Object Types Selected!",
      "At least one object type has to be selected!");
    return;
    }
  double bounds[6];
  this->getBounds(bounds);
  if(this->getUseBoundsConstraint() && !vtkBoundingBox::IsValid(bounds))
    {
    QMessageBox::warning(this,
      "Bounds are not valid!",
      "The Bounds have to set properly!");
    return;
    }
  this->Superclass::accept();
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::setBounds(double bounds[6])
{
  this->FilterDialog->X1->setText(QString::number(bounds[0]));
  this->FilterDialog->X2->setText(QString::number(bounds[1]));
  this->FilterDialog->Y1->setText(QString::number(bounds[2]));
  this->FilterDialog->Y2->setText(QString::number(bounds[3]));
  this->FilterDialog->Z1->setText(QString::number(bounds[4]));
  this->FilterDialog->Z2->setText(QString::number(bounds[5]));
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::getBounds(double bounds[6])
{
  bounds[0] = this->FilterDialog->X1->text().toDouble();
  bounds[1] = this->FilterDialog->X2->text().toDouble();
  bounds[2] = this->FilterDialog->Y1->text().toDouble();
  bounds[3] = this->FilterDialog->Y2->text().toDouble();
  bounds[4] = this->FilterDialog->Z1->text().toDouble();
  bounds[5] = this->FilterDialog->Z2->text().toDouble();
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::setObjectTypes(QStringList& objTypes)
{
  this->FilterDialog->listObjectTypes->addItems(objTypes);
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::getSelectedObjectTypes(QStringList& objTypes)
{
  objTypes.clear();
  QList<QListWidgetItem*> idxs = this->FilterDialog->
    listObjectTypes->selectedItems();
  for (int i = 0; i < idxs.count(); i++)
    {
    objTypes.append(idxs.value(i)->text());
    }
}

//-----------------------------------------------------------------------------
const char* qtCMBSceneObjectFilterDialog::getSceneFile()
{
  return this->FilterDialog->labelFile->text().toAscii().constData();
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::setSceneFile(const char* filename)
{
  this->FilterDialog->labelFile->setText(QString(filename));
}
//-----------------------------------------------------------------------------
bool qtCMBSceneObjectFilterDialog::getUseBoundsConstraint()
{
  return this->FilterDialog->groupBox_Bounds->isChecked();
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectFilterDialog::setUseBoundsConstraint(bool use)
{
  this->FilterDialog->groupBox_Bounds->setChecked(use);
}
