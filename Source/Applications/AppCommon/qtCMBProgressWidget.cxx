//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBProgressWidget.h"

#include <QApplication>
#include <QGridLayout>
#include <QProgressBar>
#include <QToolButton>

qtCMBProgressWidget::qtCMBProgressWidget(QWidget* _parent /*=0*/)
  : QWidget(_parent)
{
  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setSpacing(4);
  gridLayout->setMargin(0);
  gridLayout->setObjectName("gridLayout");

  this->ProgressBar = new QProgressBar(this);
  this->ProgressBar->setObjectName("ProgressBar");
  this->ProgressBar->setOrientation(Qt::Horizontal);
  gridLayout->addWidget(this->ProgressBar, 0, 1, 1, 1);

  this->AbortButton = new QToolButton(this);
  this->AbortButton->setObjectName("AbortButton");
  this->AbortButton->setIcon(QIcon(QString::fromUtf8(":/cmb/pqDelete16.png")));
  this->AbortButton->setIconSize(QSize(12, 12));
  this->AbortButton->setToolTip(QApplication::translate("Form", "Abort", 0
#if QT_VERSION < 0x050000
    ,
    QApplication::UnicodeUTF8
#endif
    ));

  this->AbortButton->setEnabled(false);
  QObject::connect(this->AbortButton, SIGNAL(pressed()), this, SIGNAL(abortPressed()));

  gridLayout->addWidget(this->AbortButton, 0, 0, 1, 1);
}

qtCMBProgressWidget::~qtCMBProgressWidget()
{
  delete this->ProgressBar;
  delete this->AbortButton;
}

void qtCMBProgressWidget::setProgress(const QString& message, int value)
{
  this->ProgressBar->setFormat(QString("%1: %p").arg(message));
  this->ProgressBar->setValue(value);
}

void qtCMBProgressWidget::enableProgress(bool enabled)
{
  if (this->ProgressBar->isEnabled() != enabled)
  {
    this->ProgressBar->setEnabled(enabled);
    this->ProgressBar->setTextVisible(enabled);
    if (!enabled)
    {
      this->ProgressBar->reset();
    }
  }
}

void qtCMBProgressWidget::enableAbort(bool enabled)
{
  this->AbortButton->setEnabled(enabled);
}
