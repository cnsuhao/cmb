//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBProgressWidget Represents a class for a progress bar and an
//       abort button.
// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBProgressWidget_h
#define __qtCMBProgressWidget_h

#include "cmbAppCommonExport.h"
#include <QWidget>
#include "cmbSystemConfig.h"

class QProgressBar;
class QToolButton;

class CMBAPPCOMMON_EXPORT qtCMBProgressWidget : public QWidget
{
  Q_OBJECT
public:
  qtCMBProgressWidget(QWidget* parent=0);
  virtual ~qtCMBProgressWidget();

  QToolButton* getAbortButton() const
    {
    return this->AbortButton;
    }

public slots:
  /// Set the progress.
  void setProgress(const QString& message, int value);

  /// Enabled/Disable the progress. This is different from
  /// enabling/disabling the widget itself. This shows/hides
  /// the progress part of the widget.
  void enableProgress(bool enabled);

  /// Enable/Disable the abort button.
  void enableAbort(bool enabled);

signals:
  void abortPressed();

protected:
  QProgressBar* ProgressBar;
  QToolButton* AbortButton;

private:
  qtCMBProgressWidget(const qtCMBProgressWidget&); // Not implemented.
  void operator=(const qtCMBProgressWidget&); // Not implemented.
};

#endif
