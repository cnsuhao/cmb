//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCheckableComboPopupEventPlayer - The event player for smtk::attribute::qtCheckItemComboBox
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCheckableComboPopupEventPlayer_h
#define __pqCheckableComboPopupEventPlayer_h

#include "cmbSystemConfig.h"
#include "pqWidgetEventPlayer.h"

/**
Concrete implementation of pqWidgetEventPlayer that translates high-level cmb events into low-level Qt events.

\sa pqEventPlayer
*/

class pqCheckableComboPopupEventPlayer : public pqWidgetEventPlayer
{
  Q_OBJECT
public:
  pqCheckableComboPopupEventPlayer(QObject* p = 0);

  bool playEvent(
    QObject* Object, const QString& Command, const QString& Arguments, bool& Error) override;

private:
  pqCheckableComboPopupEventPlayer(const pqCheckableComboPopupEventPlayer&);
  pqCheckableComboPopupEventPlayer& operator=(const pqCheckableComboPopupEventPlayer&);
};

#endif // !_pqCheckableComboPopupEventPlayer_h
