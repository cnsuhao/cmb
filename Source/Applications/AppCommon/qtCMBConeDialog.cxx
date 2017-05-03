//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog editing a cone source.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBConeDialog.h"

#include "pqPipelineSource.h"
#include "pqProxyWidget.h"
#include "pqRenderView.h"
#include "vtkSMSourceProxy.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
//-----------------------------------------------------------------------------
qtCMBConeDialog::qtCMBConeDialog(pqPipelineSource* coneSource, pqRenderView* view)
  : Status(0)
  , ConeSourcePanel(0)
{
  if (coneSource)
  {
    this->MainDialog = new QDialog();
    QVBoxLayout* layout = new QVBoxLayout(this->MainDialog);
    this->ConeSourcePanel = new pqProxyWidget(coneSource->getProxy(), this->MainDialog);

    this->ConeSourcePanel->setObjectName("ConeSourcePanel");
    layout->addWidget(ConeSourcePanel);
    this->ConeSourcePanel->setView(view);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this->MainDialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    layout->addWidget(buttonBox);
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
  }
}

//-----------------------------------------------------------------------------
qtCMBConeDialog::~qtCMBConeDialog()
{
  if (this->ConeSourcePanel)
  {
    delete this->ConeSourcePanel;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}
//-----------------------------------------------------------------------------
int qtCMBConeDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}
//-----------------------------------------------------------------------------
void qtCMBConeDialog::accept()
{
  this->MainDialog->hide();
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(this->ConeSourcePanel->proxy());
  smSource->MarkModified(NULL);
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();
  this->ConeSourcePanel->apply();
  this->Status = 1;
}
//-----------------------------------------------------------------------------
void qtCMBConeDialog::cancel()
{
  this->MainDialog->hide();
  this->ConeSourcePanel->reset();
  this->Status = 0;
}
//-----------------------------------------------------------------------------
