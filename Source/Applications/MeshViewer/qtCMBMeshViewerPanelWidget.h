//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBMeshViewerPanelWidget - The MeshViewer control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the MeshViewer.
//  The main component of the panel is a tree structure to represent the MeshViewer.
// .SECTION Caveats

#ifndef __qtCMBMeshViewerPanelWidget_h
#define __qtCMBMeshViewerPanelWidget_h

#include "cmbSystemConfig.h"
#include <QPoint>
#include <QWidget>
class QIcon;
class QAction;

namespace Ui
{
class qtMeshViewerPanel;
}

class qtCMBMeshViewerPanelWidgetInternal;

class qtCMBMeshViewerPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBMeshViewerPanelWidget(QWidget* parent = 0);
  ~qtCMBMeshViewerPanelWidget() override;

  Ui::qtMeshViewerPanel* getGUIPanel();
  QIcon* iconVisible();
  QIcon* iconInvisible();
  QIcon* iconActive();
  QIcon* iconInactive();

  QAction* deleteInputAction();
  QAction* activeInputAction();
  QAction* exportSubsetAction();

signals:

public slots:
  void showContextMenu(const QPoint&);
private slots:

private:
  qtCMBMeshViewerPanelWidgetInternal* Internal;
};
#endif
