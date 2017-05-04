//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBTableWidget.h"

#include <QHeaderView>
#include <QKeyEvent>

qtCMBTableWidget::qtCMBTableWidget(QWidget* p)
  : QTableWidget(p)
{
  //we want the table to always fill the frame
  this->horizontalHeader()->setStretchLastSection(true);
}

qtCMBTableWidget::~qtCMBTableWidget()
{
}

void qtCMBTableWidget::keyPressEvent(QKeyEvent* e)
{
  emit this->keyPressed(e);
}
