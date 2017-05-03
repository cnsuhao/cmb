//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBLIDARPanelWidget.h"
#include "ui_qtLIDARPanel.h"

class qtCMBLIDARPanelWidgetInternal : public Ui::qtLIDARPanel
{
public:
};

//-----------------------------------------------------------------------------
qtCMBLIDARPanelWidget::qtCMBLIDARPanelWidget(QWidget* _p)
  : QWidget(_p)
{
  this->Internal = new qtCMBLIDARPanelWidgetInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qtCMBLIDARPanelWidget::~qtCMBLIDARPanelWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
Ui::qtLIDARPanel* qtCMBLIDARPanelWidget::getGUIPanel()
{
  return this->Internal;
}
