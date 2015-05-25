//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBPanelWidget.h"
#include "ui_qtCMBPanel.h"

class qtCMBPanelWidgetInternal :
  public Ui::qtCMBPanel
{
public:

};

//-----------------------------------------------------------------------------
qtCMBPanelWidget::qtCMBPanelWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internal = new qtCMBPanelWidgetInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qtCMBPanelWidget::~qtCMBPanelWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
Ui::qtCMBPanel* qtCMBPanelWidget::getGUIPanel()
{
  return this->Internal;
}
