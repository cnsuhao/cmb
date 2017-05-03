//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBSceneBuilderPanelWidget.h"
#include "ui_qtCMBSceneBuilderPanel.h"

class qtCMBSceneBuilderPanelWidgetInternal : public Ui::qtCMBSceneBuilderPanel
{
public:
};

//-----------------------------------------------------------------------------
qtCMBSceneBuilderPanelWidget::qtCMBSceneBuilderPanelWidget(QWidget* _p)
  : QWidget(_p)
{
  this->Internal = new qtCMBSceneBuilderPanelWidgetInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qtCMBSceneBuilderPanelWidget::~qtCMBSceneBuilderPanelWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
Ui::qtCMBSceneBuilderPanel* qtCMBSceneBuilderPanelWidget::getGUIPanel()
{
  return this->Internal;
}

//----------------------------------------------------------------------------
void qtCMBSceneBuilderPanelWidget::focusOnSceneTab()
{
  this->Internal->tabWidget->setCurrentIndex(0);
}

//----------------------------------------------------------------------------
void qtCMBSceneBuilderPanelWidget::focusOnDisplayTab()
{
  this->Internal->tabWidget->setCurrentIndex(1);
}
