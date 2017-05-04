//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBProcessWidget.h"

#include "pqOutputWindow.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>

//-----------------------------------------------------------------------------
pqCMBProcessWidget::pqCMBProcessWidget(QWidget* _parent /*=0*/)
  : QWidget(_parent)
{
  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setSpacing(4);
  gridLayout->setMargin(0);
  gridLayout->setObjectName("gridLayout");

  this->OutputButton = new QToolButton(this);
  this->OutputButton->setObjectName("OutputButton");
  this->OutputButton->setText("Process Output");
  QObject::connect(this->OutputButton, SIGNAL(pressed()), this, SLOT(showOutputWindow()));

  this->Message = new QLabel;
  this->Message->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  this->AbortButton = new QToolButton(this);
  this->AbortButton->setObjectName("AbortButton");
  this->AbortButton->setIcon(QIcon(QString::fromUtf8(":/QtWidgets/Icons/pqDelete16.png")));
  this->AbortButton->setIconSize(QSize(12, 12));
  this->AbortButton->setToolTip(QApplication::translate("Form", "Abort", 0
#if QT_VERSION < 0x050000
    ,
    QApplication::UnicodeUTF8
#endif
    ));

  this->AbortButton->setEnabled(false);
  QObject::connect(this->AbortButton, SIGNAL(pressed()), this, SIGNAL(abortPressed()));

  gridLayout->addWidget(this->Message, 0, 0);
  gridLayout->addWidget(this->AbortButton, 0, 1);
  gridLayout->addWidget(this->OutputButton, 0, 2);

  // hmmm... set minimum width or setFixedWidth on the message?
  gridLayout->setColumnMinimumWidth(0, 300);

  this->OutputWindow = new pqOutputWindow(0);
  this->OutputWindow->setAttribute(Qt::WA_QuitOnClose, false);
  this->OutputWindow->setObjectName("processOutputDialog");
  this->OutputWindow->setWindowTitle(tr("Process Output"));
  //  this->OutputWindow->setShowOutput(false);
}

//-----------------------------------------------------------------------------
pqCMBProcessWidget::~pqCMBProcessWidget()
{
  delete this->Message;
  delete this->AbortButton;
  delete this->OutputButton;
  delete this->OutputWindow;
}

//-----------------------------------------------------------------------------
void pqCMBProcessWidget::setMessage(const QString& message)
{
  this->Message->setText(message);
}

//-----------------------------------------------------------------------------
void pqCMBProcessWidget::appendToOutput(const QString& message)
{
  this->OutputWindow->onDisplayText(message);
}

//-----------------------------------------------------------------------------
void pqCMBProcessWidget::showOutputWindow()
{
  this->OutputWindow->show();
}

//-----------------------------------------------------------------------------
void pqCMBProcessWidget::enableAbort(bool enabled)
{
  this->AbortButton->setEnabled(enabled);
}
