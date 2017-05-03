//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqCMBProcessWidget_h
#define __pqCMBProcessWidget_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QWidget>

class QLabel;
class QToolButton;
class pqOutputWindow;

class CMBAPPCOMMON_EXPORT pqCMBProcessWidget : public QWidget
{
  Q_OBJECT
public:
  pqCMBProcessWidget(QWidget* parent = 0);
  ~pqCMBProcessWidget() override;

  QToolButton* getAbortButton() const { return this->AbortButton; }
  QToolButton* getOutputButton() const { return this->OutputButton; }

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
  pqOutputWindow* OutputWindow;

private:
  pqCMBProcessWidget(const pqCMBProcessWidget&); // Not implemented.
  void operator=(const pqCMBProcessWidget&);     // Not implemented.
};

#endif
