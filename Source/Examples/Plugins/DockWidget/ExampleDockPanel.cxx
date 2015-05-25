//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "ExampleDockPanel.h"
#include "ui_ExampleDockPanel.h"

void ExampleDockPanel::constructor()
{
  this->setWindowTitle("Example Dock Panel");
  QWidget* t_widget = new QWidget(this);
  Ui::ExampleDockPanel ui;
  ui.setupUi(t_widget);
  this->setWidget(t_widget);
}
