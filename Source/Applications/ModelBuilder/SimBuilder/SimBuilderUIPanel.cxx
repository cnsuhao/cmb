/*=========================================================================

  Module:    SimBuilderUIPanel.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "SimBuilderUIPanel.h"

#include "smtkUIManager.h"

#include "smtk/extension/qt/qtRootView.h"
#include "smtk/view/Root.h"
#include "vtkSMProxy.h"

#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

//----------------------------------------------------------------------------
SimBuilderUIPanel::SimBuilderUIPanel(QWidget* pW) : QDockWidget(pW)
{
  this->ContainerWidget = NULL;
  this->setObjectName("SimBuilderDockWidget");
}

//----------------------------------------------------------------------------
SimBuilderUIPanel::~SimBuilderUIPanel()
{
}

//----------------------------------------------------------------------------
QWidget* SimBuilderUIPanel::panelWidget()
{  
  return this->ContainerWidget;
}

//----------------------------------------------------------------------------
void SimBuilderUIPanel::initialize()
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
