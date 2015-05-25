//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBGeologyBuilderPanelWidget.h"
#include "ui_qtCMBGeologyBuilderPanel.h"

class qtCMBGeologyBuilderPanelWidgetInternal :
  public Ui::qtCMBGeologyBuilderPanel
{
public:

};

//-----------------------------------------------------------------------------
qtCMBGeologyBuilderPanelWidget::qtCMBGeologyBuilderPanelWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internal = new qtCMBGeologyBuilderPanelWidgetInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qtCMBGeologyBuilderPanelWidget::~qtCMBGeologyBuilderPanelWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
Ui::qtCMBGeologyBuilderPanel* qtCMBGeologyBuilderPanelWidget::getGUIPanel()
{
  return this->Internal;
}

//----------------------------------------------------------------------------
void qtCMBGeologyBuilderPanelWidget::focusOnSceneTab( )
{
  this->Internal->tabWidget->setCurrentIndex(0);
}


//----------------------------------------------------------------------------
void qtCMBGeologyBuilderPanelWidget::focusOnDisplayTab( )
{
  this->Internal->tabWidget->setCurrentIndex(1);
}
