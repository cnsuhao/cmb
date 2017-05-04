//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBPreviewDialog.h"
#include "ui_qtPreviewDialog.h"

#include "pqCMBEnumPropertyWidget.h"
#include "pqDataRepresentation.h"
#include "pqRenderView.h"

#include <QFrame>
#include <QVBoxLayout>
#include <pqSetName.h>

pqCMBPreviewDialog::pqCMBPreviewDialog(QWidget* Parent)
  : QDialog(Parent)
  , Ui(new Ui::qtPreviewDialog())
{
  this->Ui->setupUi(this);
  this->setObjectName("pqCMBPreviewDialog");
  this->setModal(false);
  this->RepresentationWidget = NULL;
}

pqCMBPreviewDialog::~pqCMBPreviewDialog()
{
  delete this->Ui;
  if (this->RepresentationWidget)
  {
    delete this->RepresentationWidget;
  }
}

void pqCMBPreviewDialog::enableErrorView(bool state)
{
  if (state)
  {
    this->Ui->OkButton->hide();
    this->Ui->CancelButton->setText("Close");
  }
  else
  {
    this->Ui->OkButton->show();
    this->Ui->CancelButton->setText("Reject");
  }
}

void pqCMBPreviewDialog::setRepresentationAndView(
  pqDataRepresentation* dataRep, pqRenderView* renderView)
{
  if (!dataRep || !renderView)
  {
    return;
  }
  if (this->RepresentationWidget)
  {
    delete this->RepresentationWidget;
    this->RepresentationWidget = NULL;
  }

  this->RepresentationWidget = new pqCMBEnumPropertyWidget(this->Ui->toolbarFrame)
    << pqSetName("previewRepresentation");
  this->RepresentationWidget->setPropertyName("Representation");
  this->RepresentationWidget->setLabelText("Change representation style");
  this->RepresentationWidget->setRepresentation(dataRep);

  if (this->Ui->toolbarFrame->layout())
  {
    delete this->Ui->toolbarFrame->layout();
  }
  QVBoxLayout* toollayout = new QVBoxLayout(this->Ui->toolbarFrame);
  toollayout->setMargin(0);
  toollayout->addWidget(this->RepresentationWidget);

  //pqCMBEnumPropertyWidget* repWidget = this->Ui->toolbarFrame->
  //  findChild<pqCMBEnumPropertyWidget*>("previewRepresentation");
  //if(repWidget)
  //  {
  //  }
  if (this->Ui->RenderViewFrame->layout())
  {
    delete this->Ui->RenderViewFrame->layout();
  }

  QVBoxLayout* vboxlayout = new QVBoxLayout(this->Ui->RenderViewFrame);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(renderView->widget());

  renderView->resetCamera();
  renderView->render();
}
