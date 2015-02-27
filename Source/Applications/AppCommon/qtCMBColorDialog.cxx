/*=========================================================================

  Program:   CMB
  Module:    pqCMBSceneTree.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "qtCMBColorDialog.h"
#include <QColorDialog>
#include <stdlib.h>

QColor qtCMBColorDialog::getColor ( const QColor &icolor, QWidget * parent)
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
  return QColorDialog::getColor(icolor, parent, "Select Color",
                                QColorDialog::DontUseNativeDialog);
}
