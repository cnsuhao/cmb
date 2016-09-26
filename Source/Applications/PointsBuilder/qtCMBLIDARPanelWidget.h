//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBLIDARPanelWidget - The CMB GUI control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the CMB model.
//  The main components of the panel are tree structures to represent the model.
// .SECTION Caveats

#ifndef __qtCMBLIDARPanelWidget_h
#define __qtCMBLIDARPanelWidget_h

#include <QWidget>
#include "cmbSystemConfig.h"

namespace Ui { class qtLIDARPanel; }

class qtCMBLIDARPanelWidgetInternal;

class qtCMBLIDARPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBLIDARPanelWidget(QWidget* parent=0);
  ~qtCMBLIDARPanelWidget() override;

  Ui::qtLIDARPanel* getGUIPanel();

signals:

public slots:

private slots:

private:
  qtCMBLIDARPanelWidgetInternal* Internal;

};
#endif
