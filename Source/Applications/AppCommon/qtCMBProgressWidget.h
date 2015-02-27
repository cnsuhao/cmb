/*=========================================================================

  Program:   CMB
  Module:    qtCMBProgressWidget.h

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
