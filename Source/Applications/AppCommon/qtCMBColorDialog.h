//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBColorDialog - a simple class that only has a static method
// The purpose of this class is to hide QColorDialog so that we can do
// some custom stiff - such as forcing non-native mode when dealing
// with tests

// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBColorDialog_h
#define __qtCMBColorDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QColor>

class QWidget;

class CMBAPPCOMMON_EXPORT qtCMBColorDialog
{
public:
  static QColor getColor(const QColor& initial = Qt::white, QWidget* parent = 0);
};

#endif /* qtCMBColorDialog_h */
