//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBGeologyBuilderPanelWidget - The scene control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the scene.
//  The main component of the panel is a tree structure to represent the scene.
// .SECTION Caveats

#ifndef __qtCMBGeologyBuilderPanelWidget_h
#define __qtCMBGeologyBuilderPanelWidget_h

#include <QWidget>
#include "cmbSystemConfig.h"

namespace Ui { class qtCMBGeologyBuilderPanel; }

class qtCMBGeologyBuilderPanelWidgetInternal;

class qtCMBGeologyBuilderPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBGeologyBuilderPanelWidget(QWidget* parent=0);
  virtual ~qtCMBGeologyBuilderPanelWidget();

  Ui::qtCMBGeologyBuilderPanel* getGUIPanel();

signals:

public slots:
  void focusOnDisplayTab();
  void focusOnSceneTab();

private slots:

private:
  qtCMBGeologyBuilderPanelWidgetInternal* Internal;

};
#endif
