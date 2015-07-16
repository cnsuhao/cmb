//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqContextMenuEventTranslator.h"

#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <iostream>

pqContextMenuEventTranslator::pqContextMenuEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p)
{
}

pqContextMenuEventTranslator::~pqContextMenuEventTranslator()
{
}

bool pqContextMenuEventTranslator::translateEvent(QObject* Object, QEvent* Event,
                                           bool& /*Error*/)
{
  QMenu* const menu = qobject_cast<QMenu*>(Object);
  if(!menu)
    {
    return false;
    }

  if(Event->type() == QEvent::KeyPress)
    {
    QKeyEvent* e = static_cast<QKeyEvent*>(Event);
    if(e->key() == Qt::Key_Enter)
      {
      QAction* action = menu->activeAction();
      if(action)
        {
        QString which = action->objectName();
        if(which == QString::null)
          {
          which = action->text();
          }
        emit recordEvent(menu, "activate", which);
        }
      }
    }
  
  if(Event->type() == QEvent::MouseButtonRelease)
    {
    QMouseEvent* e = static_cast<QMouseEvent*>(Event);
    if(e->button() == Qt::LeftButton)
      {
      QAction* action = menu->actionAt(e->pos());
      if (action && !action->menu())
        {
        QString which = action->objectName();

        if(which == QString::null)
          {
          which = action->text();
          }
        emit recordEvent(menu, "activate", which);
        }
      }
    }
    
  return true;
}

