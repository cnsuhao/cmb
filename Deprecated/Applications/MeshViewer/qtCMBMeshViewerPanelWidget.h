/*=========================================================================

  Program:   CMB
  Module:    qtCMBMeshViewerPanelWidget.h

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
// .NAME qtCMBMeshViewerPanelWidget - The MeshViewer control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the MeshViewer.
//  The main component of the panel is a tree structure to represent the MeshViewer.
// .SECTION Caveats

#ifndef __qtCMBMeshViewerPanelWidget_h
#define __qtCMBMeshViewerPanelWidget_h

#include <QWidget>
#include <QPoint>
#include "cmbSystemConfig.h"
class QIcon;
class QAction;

namespace Ui { class qtMeshViewerPanel; }

class qtCMBMeshViewerPanelWidgetInternal;

class qtCMBMeshViewerPanelWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBMeshViewerPanelWidget(QWidget* parent=0);
  virtual ~qtCMBMeshViewerPanelWidget();

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
  void showContextMenu(const QPoint &);
private slots:

private:
  qtCMBMeshViewerPanelWidgetInternal* Internal;

};
#endif
