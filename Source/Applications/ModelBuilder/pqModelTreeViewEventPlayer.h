//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqModelTreeViewEventPlayer - The event player for smtk::attribute::qtModelView
// .SECTION Description
// .SECTION Caveats

#ifndef __pqModelTreeViewEventPlayer_h
#define __pqModelTreeViewEventPlayer_h

#include "pqWidgetEventPlayer.h"
#include "cmbSystemConfig.h"

/**
Concrete implementation of pqWidgetEventPlayer that translates high-level cmb events into low-level Qt events.

\sa pqEventPlayer
*/

class pqModelTreeViewEventPlayer : public pqWidgetEventPlayer
{
    Q_OBJECT
public:
  pqModelTreeViewEventPlayer(QObject* p=0);

  bool playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& Error);

private:
  pqModelTreeViewEventPlayer(const pqModelTreeViewEventPlayer&);
  pqModelTreeViewEventPlayer& operator=(const pqModelTreeViewEventPlayer&);
};

#endif // !_pqModelTreeViewEventPlayer_h

