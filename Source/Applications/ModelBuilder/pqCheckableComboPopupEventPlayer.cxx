//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqModelTreeViewEventTranslator - The event translator for smtk::attribute::qtCheckItemComboBox
// .SECTION Description
// .SECTION Caveats

#include "pqCheckableComboPopupEventPlayer.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "pqEventDispatcher.h"

///////////////////////////////////////////////////////////////////////////////
// pqCheckableComboPopupEventPlayer

pqCheckableComboPopupEventPlayer::pqCheckableComboPopupEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

bool pqCheckableComboPopupEventPlayer::playEvent(QObject* Object, const QString& command,
 const QString& /* arguments*/, bool& /*Error*/)
{
  smtk::attribute::qtCheckItemComboBox* const object = qobject_cast<smtk::attribute::qtCheckItemComboBox*>(Object);
  if(!object)
    {
    return false;
    }
  bool played = ( command == "showPopup" || command == "hidePopup" );
  if ( command == "showPopup" )
    {
    object->showPopup();
    }
  else if ( command == "hidePopup" )
    {
    object->hidePopup();
    }

  if(played)
    {
    pqEventDispatcher::processEventsAndWait(1);
    }

  return played;
}
