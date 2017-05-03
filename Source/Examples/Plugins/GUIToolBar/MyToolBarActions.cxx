//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "MyToolBarActions.h"

#include <QApplication>
#include <QMessageBox>
#include <QStyle>

MyToolBarActions::MyToolBarActions(QObject* p)
  : QActionGroup(p)
{
  QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical);

  QAction* a = this->addAction(new QAction(icon, "MyAction", this));
  QObject::connect(a, SIGNAL(triggered(bool)), this, SLOT(onAction()));
}

MyToolBarActions::~MyToolBarActions()
{
}

void MyToolBarActions::onAction()
{
  QMessageBox::information(NULL, "MyAction", "MyAction was invoked\n");
}
