//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBColorDialog.h"
#include <QColorDialog>
#include <stdlib.h>

QColor qtCMBColorDialog::getColor(const QColor& icolor, QWidget* parent)
{
  /*
  // See if we are running in a test mode or creating a test on a Mac
  if (getenv("DASHBOARD_TEST_FROM_CTEST") ||
      getenv("DART_TEST_FROM_DART") ||
      getenv("QT_MAC_NO_NATIVE_MENUBAR"))
    {
    return QColorDialog::getColor(icolor, parent, "Select Color",
                                  QColorDialog::DontUseNativeDialog |
                                  QColorDialog::ShowAlphaChannel);
    }
*/
  return QColorDialog::getColor(icolor, parent, "Select Color", QColorDialog::DontUseNativeDialog);
}
