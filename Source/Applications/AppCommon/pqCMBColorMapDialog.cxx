//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBColorMapDialog.h"

#include "pqDataRepresentation.h"
#include "pqProxyWidget.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

/// constructor
pqCMBColorMapDialog::pqCMBColorMapDialog(pqDataRepresentation* repr, QWidget* p)
  : Status(0)
{
  this->MainDialog = new QDialog(p);
  this->MainDialog->setObjectName("pqCMBColorMapDialog");

  QVBoxLayout* layout = new QVBoxLayout(this->MainDialog);
  if (repr && vtkSMPVRepresentationProxy::GetUsingScalarColoring(repr->getProxy()))
  {
    vtkSMProxy* ctf = vtkSMPropertyHelper(repr->getProxy(), "LookupTable", true).GetAsProxy();
    this->ColorEditor = new pqProxyWidget(ctf, this->MainDialog);
    this->ColorEditor->setObjectName("cmbColorEditor");
    this->ColorEditor->setApplyChangesImmediately(true);
    this->ColorEditor->filterWidgets();
    layout->addWidget(this->ColorEditor);
  }

  QDialogButtonBox* buttonBox = new QDialogButtonBox(this->MainDialog);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  layout->addWidget(buttonBox);
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

/// destructor
pqCMBColorMapDialog::~pqCMBColorMapDialog()
{
  delete this->ColorEditor;
  delete this->MainDialog;
}

int pqCMBColorMapDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void pqCMBColorMapDialog::accept()
{
  this->MainDialog->hide();
  this->Status = 1;
}
