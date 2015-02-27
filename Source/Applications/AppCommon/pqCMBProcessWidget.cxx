/*=========================================================================

   Program: ParaView
   Module:    pqCMBProcessWidget.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1.

   See License_v1.1.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "pqCMBProcessWidget.h"

#include "pqOutputWindow.h"

#include <QApplication>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>


//-----------------------------------------------------------------------------
pqCMBProcessWidget::pqCMBProcessWidget(QWidget* _parent/*=0*/)
  : QWidget(_parent)
{
  QGridLayout *gridLayout = new QGridLayout(this);
  gridLayout->setSpacing(4);
  gridLayout->setMargin(0);
  gridLayout->setObjectName("gridLayout");

  this->OutputButton = new QToolButton(this);
  this->OutputButton->setObjectName("OutputButton");
  this->OutputButton->setText("Process Output");
  QObject::connect(this->OutputButton, SIGNAL(pressed()),
    this, SLOT(showOutputWindow()));

  this->Message = new QLabel;
  this->Message->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  this->AbortButton = new QToolButton(this);
  this->AbortButton->setObjectName("AbortButton");
  this->AbortButton->setIcon(
    QIcon(QString::fromUtf8(":/QtWidgets/Icons/pqDelete16.png")));
  this->AbortButton->setIconSize(QSize(12, 12));
  this->AbortButton->setToolTip(
    QApplication::translate("Form", "Abort", 0, QApplication::UnicodeUTF8));

  this->AbortButton->setEnabled(false);
  QObject::connect(this->AbortButton, SIGNAL(pressed()),
    this, SIGNAL(abortPressed()));

  gridLayout->addWidget(this->Message, 0, 0);
  gridLayout->addWidget(this->AbortButton, 0, 1);
  gridLayout->addWidget(this->OutputButton, 0, 2);

  // hmmm... set minimum width or setFixedWidth on the message?
  gridLayout->setColumnMinimumWidth(0, 300);

  this->OutputWindow = new pqOutputWindow(0);
  this->OutputWindow->setAttribute(Qt::WA_QuitOnClose, false);
  this->OutputWindow->setObjectName("processOutputDialog");
  this->OutputWindow->setWindowTitle(tr("Process Output"));
  this->OutputWindow->setShowOutput(false);
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
