//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBOptionsContainer.h"

qtCMBOptionsContainer::qtCMBOptionsContainer(QWidget* widgetParent)
  : qtCMBOptionsPage(widgetParent)
{
  this->Prefix = new QString();
}

qtCMBOptionsContainer::~qtCMBOptionsContainer()
{
  delete this->Prefix;
}

const QString& qtCMBOptionsContainer::getPagePrefix() const
{
  return *this->Prefix;
}

void qtCMBOptionsContainer::setPagePrefix(const QString& prefix)
{
  *this->Prefix = prefix;
}
