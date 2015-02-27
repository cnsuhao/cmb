/*=========================================================================

  Program:   CMB
  Module:    qtCMBColorDialog.h

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
// .NAME qtCMBColorDialog - a simple class that only has a static method
// The purpose of this class is to hide QColorDialog so that we can do
// some custom stiff - such as forcing non-native mode when dealing
// with tests

// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBColorDialog_h
#define __qtCMBColorDialog_h

#include <QColor>
#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class QWidget;

class  CMBAPPCOMMON_EXPORT qtCMBColorDialog
{
public:
  static QColor getColor ( const QColor & initial = Qt::white, QWidget * parent = 0 );
};

#endif /* qtCMBColorDialog_h */
