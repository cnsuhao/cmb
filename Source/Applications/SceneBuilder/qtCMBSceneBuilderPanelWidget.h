//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBSceneBuilderPanelWidget - The scene control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the scene.
//  The main component of the panel is a tree structure to represent the scene.
// .SECTION Caveats

#ifndef __qtCMBSceneBuilderPanelWidget_h
#define __qtCMBSceneBuilderPanelWidget_h

#include <QWidget>
#include "cmbSystemConfig.h"

namespace Ui { class qtCMBSceneBuilderPanel; }

class qtCMBSceneBuilderPanelWidgetInternal;

class qtCMBSceneBuilderPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBSceneBuilderPanelWidget(QWidget* parent=0);
  ~qtCMBSceneBuilderPanelWidget() override;

  Ui::qtCMBSceneBuilderPanel* getGUIPanel();

signals:

public slots:
  void focusOnDisplayTab();
  void focusOnSceneTab();

private slots:

private:
  qtCMBSceneBuilderPanelWidgetInternal* Internal;

};
#endif
