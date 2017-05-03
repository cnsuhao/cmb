//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBTabWidgetTranslator.h"

#include <QEvent>
#include <QString>
#include <QTabBar>

pqCMBTabWidgetTranslator::pqCMBTabWidgetTranslator(QObject* p)
  : pqWidgetEventTranslator(p)
  , CurrentObject(0)
{
}

bool pqCMBTabWidgetTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QTabBar* const object = qobject_cast<QTabBar*>(Object);
  if (!object)
    return false;

  switch (Event->type())
  {
    case QEvent::Enter:
      if (this->CurrentObject != Object)
      {
        if (this->CurrentObject)
        {
          disconnect(this->CurrentObject, 0, this, 0);
        }

        this->CurrentObject = object;
        connect(object, SIGNAL(currentChanged(int)), this, SLOT(indexChanged(int)));
      }
      break;
    default:
      break;
  }

  return true;
}

void pqCMBTabWidgetTranslator::indexChanged(int which)
{
  QString text = this->CurrentObject->tabText(which);
  if (text.isEmpty())
  { // If tab text is empty, use index instead
    int index = this->CurrentObject->currentIndex();
    emit recordEvent(this->CurrentObject, "set_tab", QString::number(index));
  }
  else
  {
    emit recordEvent(this->CurrentObject, "set_tab_with_text", text);
  }
}
