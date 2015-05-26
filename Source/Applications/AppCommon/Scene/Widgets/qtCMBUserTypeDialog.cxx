//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for importing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBUserTypeDialog.h"

#include "ui_qtObjectTypeDialog.h"
#include "pqCMBSceneObjectBase.h"
#include <QInputDialog>
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

//-----------------------------------------------------------------------------
void qtCMBUserTypeDialog::updateUserType(pqCMBSceneNode *node)
{
  if ((!node) || node->isTypeNode())
    {
    return;
    }
  qtCMBUserTypeDialog dialog(node);
  dialog.exec();
}

//-----------------------------------------------------------------------------
qtCMBUserTypeDialog::qtCMBUserTypeDialog(pqCMBSceneNode *node)
{
  this->Node = node;
  this->MainDialog = new QDialog();

  this->TypeDialog = new Ui::qtObjectTypeDialog();
  this->TypeDialog->setupUi(this->MainDialog);

  this->TypeDialog->ObjectTypes->
    addItems(this->Node->getTree()->getUserDefinedObjectTypes());
  this->TypeDialog->ObjectTypes->addItem("Specify New Type");
  int index = this->Node->getTree()->
    getUserDefinedObjectTypes().indexOf(this->Node->
                                        getDataObject()->
                                        getUserDefinedType().c_str());
  this->TypeDialog->ObjectTypes->setCurrentIndex(index);
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(this->TypeDialog->ObjectTypes, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(changeObjectType()));
}

//-----------------------------------------------------------------------------
qtCMBUserTypeDialog::~qtCMBUserTypeDialog()
{
  if (this->TypeDialog)
    {
    delete TypeDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
void qtCMBUserTypeDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
}
//-----------------------------------------------------------------------------
void qtCMBUserTypeDialog::accept()
{
  this->Node->getDataObject()->
    setUserDefinedType(this->TypeDialog->ObjectTypes->currentText().toAscii());
}
//-----------------------------------------------------------------------------
void qtCMBUserTypeDialog::cancel()
{
}

//-----------------------------------------------------------------------------
void qtCMBUserTypeDialog::changeObjectType()
{
  if (this->TypeDialog->ObjectTypes->currentIndex() ==
      (this->TypeDialog->ObjectTypes->count()-1))
    {
    // User has asked to add a new type
    this->TypeDialog->ObjectTypes->blockSignals(true);
    QString newType = QInputDialog::getText(this->MainDialog,
                                            "SceneBuilder - New Object Type",
                                            "Enter New Object Type:");
    int index;
    if (!newType.isEmpty())
      {
      // See if the type is already in list
      index = this->Node->getTree()->
        getUserDefinedObjectTypes().indexOf(newType);
      if (index == -1)
        {
        this->Node->getTree()->addUserDefinedType(newType.toAscii());
        index = this->Node->getTree()->
          getUserDefinedObjectTypes().indexOf(newType);
        this->TypeDialog->ObjectTypes->insertItem(index, newType);
        }
      this->TypeDialog->ObjectTypes->setCurrentIndex(index);
      }
    else
      {
      // Set the type to be the original name
      index = this->Node->getTree()->
        getUserDefinedObjectTypes().indexOf(this->Node->
                                            getDataObject()->
                                            getUserDefinedType().c_str());
      this->TypeDialog->ObjectTypes->setCurrentIndex(index);
      }
    this->TypeDialog->ObjectTypes->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
