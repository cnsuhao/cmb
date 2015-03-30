/*=========================================================================

  Program:   CMB
  Module:    pqCMBPreviewDialog.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "pqCMBPreviewDialog.h"
#include "ui_qtPreviewDialog.h"

#include "pqCMBEnumPropertyWidget.h"
#include "pqRenderView.h"
#include "pqDataRepresentation.h"

#include <QFrame>
#include <QVBoxLayout>
#include <pqSetName.h>

//-----------------------------------------------------------------------------
pqCMBPreviewDialog::pqCMBPreviewDialog(QWidget* Parent) :
  QDialog(Parent), Ui(new Ui::qtPreviewDialog())
{
  this->Ui->setupUi(this);
  this->setObjectName("pqCMBPreviewDialog");
  this->setModal(false);
  this->RepresentationWidget = NULL;
}

//-----------------------------------------------------------------------------
pqCMBPreviewDialog::~pqCMBPreviewDialog()
{
  delete this->Ui;
  if(this->RepresentationWidget)
    {
    delete this->RepresentationWidget;
    }
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void pqCMBPreviewDialog::setRepresentationAndView(
  pqDataRepresentation* dataRep, pqRenderView* renderView)
  {
  if(!dataRep || !renderView)
    {
    return;
    }
  if(this->RepresentationWidget)
    {
    delete this->RepresentationWidget;
    this->RepresentationWidget = NULL;
    }

  this->RepresentationWidget =
    new pqCMBEnumPropertyWidget(this->Ui->toolbarFrame)
    << pqSetName("previewRepresentation");
  this->RepresentationWidget->setPropertyName("Representation");
  this->RepresentationWidget->setLabelText("Change representation style");
  this->RepresentationWidget->setRepresentation(dataRep);

  if(this->Ui->toolbarFrame->layout())
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
  if(this->Ui->RenderViewFrame->layout())
    {
    delete this->Ui->RenderViewFrame->layout();
    }

  QVBoxLayout* vboxlayout = new QVBoxLayout(this->Ui->RenderViewFrame);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(renderView->widget());

  renderView->resetCamera();
  renderView->render();
}
