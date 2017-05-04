//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqModelTreeViewEventTranslator - The event translator for smtk::extension::qtModelView
// .SECTION Description
// .SECTION Caveats

#include "pqModelTreeViewEventPlayer.h"

#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtDebug>

#include "pqEventDispatcher.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtEntityItemModel.h"
#include "smtk/extension/qt/qtModelView.h"

static QModelIndex GetIndex(smtk::extension::qtModelView* View, const QString& Name)
{
  QStringList idxs = Name.split('/', QString::SkipEmptyParts);

  QModelIndex index;
  for (int i = 0; i != idxs.size(); ++i)
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

bool pqModelTreeViewEventPlayer::playEvent(
  QObject* Object, const QString& command, const QString& arguments, bool& /*Error*/)
{
  smtk::extension::qtModelView* const object = qobject_cast<smtk::extension::qtModelView*>(Object);
  if (!object)
  {
    return false;
  }

  if (command != "toggleVisibility" && command != "changeColor" && command != "showContextMenu")
  {
    return false;
  }

  QStringList args = arguments.split(',');
  if (args.size() == 1)
  {
    QModelIndex idx = GetIndex(object, args[0]);
    if (command == "toggleVisibility")
    {
      object->toggleEntityVisibility(idx);
    }
    else if (command == "changeColor")
    {
      object->changeEntityColor(idx);
    }
    else if (command == "showContextMenu")
    {
      object->showContextMenu(idx);
    }
    pqEventDispatcher::processEventsAndWait(1);
  }

  return true;
}
