/*=========================================================================

  Module:    qtSimBuilderUIPanel.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "qtSimBuilderUIPanel.h"

#include "smtk/extension/qt/qtRootView.h"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

//----------------------------------------------------------------------------
qtSimBuilderUIPanel::qtSimBuilderUIPanel(QWidget* pW) : QDockWidget(pW)
{
  this->ContainerWidget = NULL;
  this->setObjectName("SimBuilderDockWidget");
}

//----------------------------------------------------------------------------
qtSimBuilderUIPanel::~qtSimBuilderUIPanel()
{
}

//----------------------------------------------------------------------------
QWidget* qtSimBuilderUIPanel::panelWidget()
{  
  return this->ContainerWidget;
}

//----------------------------------------------------------------------------
void qtSimBuilderUIPanel::initialize()
{
  if(this->ContainerWidget)
    {
    delete this->ContainerWidget;
    }

  this->ContainerWidget = new QWidget();
  this->ContainerWidget->setObjectName("attScrollWidget");
  this->ContainerWidget->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(this);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(this->ContainerWidget);

  QVBoxLayout* vboxlayout = new QVBoxLayout(this->ContainerWidget);
  vboxlayout->setMargin(0);
  this->setWidget(s);
}
