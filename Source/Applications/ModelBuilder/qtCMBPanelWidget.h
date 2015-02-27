/*=========================================================================

  Program:   CMB
  Module:    qtCMBPanelWidget.h

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
// .NAME qtCMBPanelWidget - The CMB GUI control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the CMB model.
//  The main components of the panel are tree structures to represent the model.
// .SECTION Caveats

#ifndef __qtCMBPanelWidget_h
#define __qtCMBPanelWidget_h

#include <QWidget>
#include "cmbSystemConfig.h"

namespace Ui { class qtCMBPanel; }

class qtCMBPanelWidgetInternal;

class qtCMBPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBPanelWidget(QWidget* parent=0);
  virtual ~qtCMBPanelWidget();

  Ui::qtCMBPanel* getGUIPanel();

signals:

public slots:

private slots:

private:
  qtCMBPanelWidgetInternal* Internal;

};
#endif
