/*=========================================================================

  Program:   CMB
  Module:    qtCMBProgressWidget.cxx

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

#include "qtCMBProgressWidget.h"

#include <QApplication>
#include <QGridLayout>
#include <QProgressBar>
#include <QToolButton>


//-----------------------------------------------------------------------------
qtCMBProgressWidget::qtCMBProgressWidget(QWidget* _parent/*=0*/)
  : QWidget(_parent)
{
  QGridLayout *gridLayout = new QGridLayout(this);
  gridLayout->setSpacing(4);
  gridLayout->setMargin(0);
  gridLayout->setObjectName("gridLayout");

  this->ProgressBar = new QProgressBar(this);
  this->ProgressBar->setObjectName("ProgressBar");
  this->ProgressBar->setOrientation(Qt::Horizontal);
  gridLayout->addWidget(this->ProgressBar, 0, 1, 1, 1);

  this->AbortButton = new QToolButton(this);
  this->AbortButton->setObjectName("AbortButton");
  this->AbortButton->setIcon(
    QIcon(QString::fromUtf8(":/cmb/pqDelete16.png")));
  this->AbortButton->setIconSize(QSize(12, 12));
  this->AbortButton->setToolTip(
    QApplication::translate("Form", "Abort", 0, QApplication::UnicodeUTF8));

  this->AbortButton->setEnabled(false);
  QObject::connect(this->AbortButton, SIGNAL(pressed()),
    this, SIGNAL(abortPressed()));

  gridLayout->addWidget(this->AbortButton, 0, 0, 1, 1);

}

//-----------------------------------------------------------------------------
qtCMBProgressWidget::~qtCMBProgressWidget()
{
  delete this->ProgressBar;
  delete this->AbortButton;
}

//-----------------------------------------------------------------------------
void qtCMBProgressWidget::setProgress(const QString& message, int value)
{
  this->ProgressBar->setFormat(QString("%1: %p").arg(message));
  this->ProgressBar->setValue(value);
}

//-----------------------------------------------------------------------------
void qtCMBProgressWidget::enableProgress(bool enabled)
{
  if(this->ProgressBar->isEnabled() != enabled)
    {
    this->ProgressBar->setEnabled(enabled);
    this->ProgressBar->setTextVisible(enabled);
    if(!enabled)
      {
      this->ProgressBar->reset();
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBProgressWidget::enableAbort(bool enabled)
{
  this->AbortButton->setEnabled(enabled);
}
