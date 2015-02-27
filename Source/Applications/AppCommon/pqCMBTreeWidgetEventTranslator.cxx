/*=========================================================================

Program:   CMB
Module:    pqCMBSceneObjectBase.cxx

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

#include "pqCMBTreeWidgetEventTranslator.h"

#include <QTreeWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QAbstractItemModel>

static QString toIndexStr(QModelIndex index)
{
  QString result;
  for(QModelIndex i = index; i.isValid(); i = i.parent())
    {
    result = "/" + QString("%1:%2").arg(i.row()).arg(i.column()) + result;
    }
  return result;
}

pqCMBTreeWidgetEventTranslator::pqCMBTreeWidgetEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p)
{
}

bool pqCMBTreeWidgetEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QTreeWidget* object = qobject_cast<QTreeWidget*>(Object);
  if(!object)
    {
    // mouse events go to the viewport widget
    object = qobject_cast<QTreeWidget*>(Object->parent());
    }
  if(!object)
    return false;

  switch(Event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      {
      QKeyEvent* ke = static_cast<QKeyEvent*>(Event);
      QString data =QString("%1,%2,%3,%4,%5,%6")
        .arg(ke->type())
        .arg(ke->key())
        .arg(static_cast<int>(ke->modifiers()))
        .arg(ke->text())
        .arg(ke->isAutoRepeat())
        .arg(ke->count());
      emit recordEvent(object, "keyEvent", data);
      return true;
      }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
      {
      if(Object == object)
        {
        return false;
        }
      QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(Event);
      if(Event->type() != QEvent::MouseButtonRelease)
        {
        this->LastPos = mouseEvent->pos();
        }
      QString idxStr;
      QPoint relPt = QPoint(0,0);
      QModelIndex idx = object->indexAt(mouseEvent->pos());
      idxStr = toIndexStr(idx);
      relPt = mouseEvent->pos() - object->visualItemRect(
        object->itemAt(mouseEvent->pos())).topLeft();

      QString info = QString("%1,%2,%3,%4,%5,%6")
        .arg(mouseEvent->button())
        .arg(mouseEvent->buttons())
        .arg(mouseEvent->modifiers())
        .arg(relPt.x())
        .arg(relPt.y())
        .arg(idxStr);
      if(Event->type() == QEvent::MouseButtonPress)
        {
        emit recordEvent(object, "mousePress", info);
        }
      else if(Event->type() == QEvent::MouseButtonDblClick)
        {
        emit recordEvent(object, "mouseDblClick", info);
        }
      else if(Event->type() == QEvent::MouseButtonRelease)
        {
        if(this->LastPos != mouseEvent->pos())
          {
          emit recordEvent(object, "mouseMove", info);
          }
        emit recordEvent(object, "mouseRelease", info);
        }
      }
    default:
      break;
    }

  return true;
}
