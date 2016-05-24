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
  if(!Object)
    {
    return false;
    }
  smtk::extension::qtCheckItemComboBox* checkComboSelf =
    qobject_cast<smtk::extension::qtCheckItemComboBox*>(Object);
  // if this is the click on combobox, and not its popup menu, show popUp
  if(checkComboSelf && Event->type() == QEvent::MouseButtonPress)
    {
    emit recordEvent(checkComboSelf, "showPopup", "");
    return true;
    }

  QAbstractItemView* popView = qobject_cast<QAbstractItemView*>(Object);
  if(!popView)
    {
    // mouse events go to the viewport widget
    popView = qobject_cast<QAbstractItemView*>(Object->parent());
    }

  // only record the list view event if it's the popup menu from smtk::extension::qtCheckItemComboBox
  checkComboSelf = NULL;
  for(QObject* test = Object; checkComboSelf == NULL && test != NULL; test = test->parent())
    {
    checkComboSelf = qobject_cast<smtk::extension::qtCheckItemComboBox*>(test);
    }

  if(!popView || !checkComboSelf)
    {
    return false;
    }

  switch(Event->type())
    {
    case QEvent::FocusOut:
      {
      emit recordEvent(checkComboSelf, "hidePopup", "");
      return true;
      }
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
      emit recordEvent(popView, "keyEvent", data);
      return true;
      }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      {
      if(Object == popView)
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
      QHeaderView* header = qobject_cast<QHeaderView*>(popView);
      if(header)
        {
        int idx = header->logicalIndexAt(mouseEvent->pos());
        idxStr = QString("%1").arg(idx);
        }
      else
        {
        QModelIndex idx = popView->indexAt(mouseEvent->pos());
        idxStr = toIndexStr(idx);
        QRect r = popView->visualRect(idx);
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
        emit recordEvent(popView, "mousePress", info);
        }
      else if(Event->type() == QEvent::MouseButtonRelease)
        {
        emit recordEvent(popView, "mouseRelease", info);
        }
      }
    default:
      break;
    }

  return true;
}


