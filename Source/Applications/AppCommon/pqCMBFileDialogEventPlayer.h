//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef _pqCMBFileDialogEventPlayer_h
#define _pqCMBFileDialogEventPlayer_h

#include "cmbAppCommonExport.h"
#include <pqWidgetEventPlayer.h>

/**
Concrete implementation of pqWidgetEventPlayer that handles playback of recorded file dialog user input.

\sa pqEventPlayer
*/
class CMBAPPCOMMON_EXPORT pqCMBFileDialogEventPlayer : public pqWidgetEventPlayer
{
  Q_OBJECT
public:
  pqCMBFileDialogEventPlayer(QObject* p = 0);

  bool playEvent(
    QObject* Object, const QString& Command, const QString& Arguments, bool& Error) override;

private:
  pqCMBFileDialogEventPlayer(const pqCMBFileDialogEventPlayer&);
  pqCMBFileDialogEventPlayer& operator=(const pqCMBFileDialogEventPlayer&);
};

#endif // !_pqCMBFileDialogEventPlayer_h
