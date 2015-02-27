/*=========================================================================

   Program: ParaView
   Module:    pqCMBProcessWidget.h

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
#ifndef __pqCMBProcessWidget_h
#define __pqCMBProcessWidget_h

#include "cmbAppCommonExport.h"
#include <QWidget>
#include "cmbSystemConfig.h"

class QLabel;
class QToolButton;
class pqOutputWindow;

class CMBAPPCOMMON_EXPORT pqCMBProcessWidget : public QWidget
{
  Q_OBJECT
public:
  pqCMBProcessWidget(QWidget* parent=0);
  virtual ~pqCMBProcessWidget();

  QToolButton* getAbortButton() const
    {
    return this->AbortButton;
    }
  QToolButton* getOutputButton() const
    {
    return this->OutputButton;
    }

  void appendToOutput(const QString& message);

public slots:
  /// Set the message.
  void setMessage(const QString& message);

  /// Enable/Disable the abort button.
  void enableAbort(bool enabled);

  /// Enable/Disable the abort button.
  void showOutputWindow();

signals:
  void abortPressed();
  void outputPressed();

protected:
  QLabel* Message;
  QToolButton* AbortButton;
  QToolButton* OutputButton;
  pqOutputWindow *OutputWindow;

private:
  pqCMBProcessWidget(const pqCMBProcessWidget&); // Not implemented.
  void operator=(const pqCMBProcessWidget&); // Not implemented.
};

#endif
