//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCheckableComboPopupEventTranslator.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include "smtk/extension/qt/qtCheckItemComboBox.h"

static QString toIndexStr(QModelIndex index)
{
  QString result;
  for(QModelIndex i = index; i.isValid(); i = i.parent())
    {
    result = "/" + QString("%1:%2").arg(i.row()).arg(i.column()) + result;
    }
  return result;
}

pqCheckableComboPopupEventTranslator::pqCheckableComboPopupEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p)
{
}

bool pqCheckableComboPopupEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QAbstractItemView* object = qobject_cast<QAbstractItemView*>(Object);
  if(!object)
    {
    // mouse events go to the viewport widget
    object = qobject_cast<QAbstractItemView*>(Object->parent());
    }
  if(!object)
    return false;

  // only record the list view event if it's from smtk::attribute::qtCheckItemComboBox
  smtk::attribute::qtCheckItemComboBox* checkCombo =
    qobject_cast<smtk::attribute::qtCheckItemComboBox*>(object->parent());

  if(!checkCombo)
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
      QHeaderView* header = qobject_cast<QHeaderView*>(object);
      if(header)
        {
        int idx = header->logicalIndexAt(mouseEvent->pos());
        idxStr = QString("%1").arg(idx);
        }
      else
        {
        QModelIndex idx = object->indexAt(mouseEvent->pos());
        idxStr = toIndexStr(idx);
        QRect r = object->visualRect(idx);
        relPt = mouseEvent->pos() - r.topLeft();
        }

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
    case QEvent::Wheel:
      {
      if(Object == object)
        {
        return false;
        }
      QPoint relPt = QPoint(0,0);
      QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(Event);
      if(wheelEvent)
        {
        QString idxStr;
        QModelIndex idx = object->indexAt(wheelEvent->pos());
        idxStr = toIndexStr(idx);
        QRect r = object->visualRect(idx);
        relPt = wheelEvent->pos() - r.topLeft();
        int numStep = wheelEvent->delta() > 0 ? 120 : -120;
        int buttons = wheelEvent->buttons();
        int modifiers = wheelEvent->modifiers();
        emit emit recordEvent(Object, "mouseWheel", QString("%1,%2,%3,%4,%5")
                              .arg(numStep)
                              .arg(buttons)
                              .arg(modifiers)
                              .arg(relPt.x())
                              .arg(relPt.y())
                              .arg(idxStr));
        }
      }
      break;
    default:
      break;
    }

  return true;
}


