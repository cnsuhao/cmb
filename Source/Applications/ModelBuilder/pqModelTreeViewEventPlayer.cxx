//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqModelTreeViewEventTranslator - The event translator for smtk::attribute::qtModelView
// .SECTION Description
// .SECTION Caveats

#include "pqModelTreeViewEventPlayer.h"

#include <QHeaderView>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "pqEventDispatcher.h"

/// Converts a string representation of a model index into the real thing
static QModelIndex OldGetIndex(smtk::model::qtModelView& View, const QString& Name)
{
    QStringList rows = Name.split('/', QString::SkipEmptyParts);
    QString column;
    
    if(rows.size())
      {
      column = rows.back().split('|').at(1);
      rows.back() = rows.back().split('|').at(0);
      }
    
    QModelIndex index;
    for(int i = 0; i < rows.size() - 1; ++i)
      {
      index = View.getModel()->index(rows[i].toInt(), 0, index);
      }

    if(rows.size() > 0)
      {
      index = View.getModel()->index(rows[rows.size() - 1].toInt(), column.toInt(), index);
      }
      
    return index;
}

static QModelIndex GetIndex(smtk::model::qtModelView* View, const QString& Name)
{
  QStringList idxs = Name.split('/', QString::SkipEmptyParts);
  
  QModelIndex index;
  for(int i = 0; i != idxs.size(); ++i)
    {
    QStringList rowCol = idxs[i].split(':');
    index = View->getModel()->index(rowCol[0].toInt(), rowCol[1].toInt(), index);
    }
    
  return index;
}

///////////////////////////////////////////////////////////////////////////////
// pqModelTreeViewEventPlayer

pqModelTreeViewEventPlayer::pqModelTreeViewEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

bool pqModelTreeViewEventPlayer::playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& Error)
{
  smtk::model::qtModelView* const object = qobject_cast<smtk::model::qtModelView*>(Object);
  if(!object)
    {
    return false;
    }
    
  if(Command == "currentChanged")  // left to support old recordings
    {
    const QModelIndex index = OldGetIndex(*object, Arguments);
    if(!index.isValid())
      return false;
      
    object->setCurrentIndex(index);
    return true;
    }
  else if(Command == "keyEvent")
    {
    QStringList data = Arguments.split(',');
    if(data.size() == 6)
      {
      QKeyEvent ke(static_cast<QEvent::Type>(data[0].toInt()),
                   data[1].toInt(),
                   static_cast<Qt::KeyboardModifiers>(data[2].toInt()),
                   data[3],
                   !!data[4].toInt(),
                   data[5].toInt());
      qApp->notify(object, &ke);
      return true;
      }
    }
  else if(Command.startsWith("mouse"))
    {
    QStringList args = Arguments.split(',');
    if(args.size() == 6)
      {
      Qt::MouseButton button = static_cast<Qt::MouseButton>(args[0].toInt());
      Qt::MouseButtons buttons = static_cast<Qt::MouseButton>(args[1].toInt());
      Qt::KeyboardModifiers keym = static_cast<Qt::KeyboardModifier>(args[2].toInt());
      int x = args[3].toInt();
      int y = args[4].toInt();
      QPoint pt;
      QModelIndex idx = GetIndex(object, args[5]);
      QRect r = object->visualRect(idx);
      pt = r.topLeft() + QPoint(x,y);

      QEvent::Type type = QEvent::MouseButtonPress;
      type = Command == "mouseMove" ? QEvent::MouseMove : type;
      type = Command == "mouseRelease" ? QEvent::MouseButtonRelease : type;
      type = Command == "mouseDblClick" ? QEvent::MouseButtonDblClick : type;
      QMouseEvent e(type, pt, button, buttons, keym);
      qApp->notify(object->viewport(), &e);

        std::cout << "mouse button: " << button << "\n";
 /*
        if(type == QEvent::MouseButtonPress && button == Qt::RightButton)
        {
        std::cout << "trigger context menu for model ops\n";
        qApp->notify(object, &e);
        }
      else
        qApp->notify(object->viewport(), &e);
*/
      pqEventDispatcher::processEventsAndWait(1);
      return true;
      }
    }
    
  qCritical() << "Unknown abstract item command: " << Command << "\n";
  Error = true;
  return true;
}
