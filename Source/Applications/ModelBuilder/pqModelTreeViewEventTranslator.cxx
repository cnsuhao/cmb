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

#include "pqModelTreeViewEventTranslator.h"

#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtModelView.h"
#include <QAbstractItemModel>
#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

static QString toIndexStr(QModelIndex index)
{
  QString result;
  for (QModelIndex i = index; i.isValid(); i = i.parent())
  {
    result = "/" + QString("%1:%2").arg(i.row()).arg(i.column()) + result;
  }
  return result;
}

pqModelTreeViewEventTranslator::pqModelTreeViewEventTranslator(QObject* p)
  : pqTreeViewEventTranslator(p)
{
}

bool pqModelTreeViewEventTranslator::translateEvent(
  QObject* senderObject, QEvent* tr_event, bool& Error)
{

  smtk::extension::qtModelView* treeWidget =
    qobject_cast<smtk::extension::qtModelView*>(senderObject);
  if (!treeWidget)
  {
    // mouse events go to the viewport widget
    treeWidget = qobject_cast<smtk::extension::qtModelView*>(senderObject->parent());
  }
  if (!treeWidget)
  {
    return false;
  }

  if (tr_event->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(tr_event);
    // record visibility or color change on left button release
    if (mouseEvent->button() == Qt::LeftButton)
    {
      std::string actionString = treeWidget->determineAction(mouseEvent->pos());
      std::string recordCommand;
      if (actionString == "visible")
      {
        recordCommand = "toggleVisibility";
      }
      else if (actionString == "color")
      {
        recordCommand = "changeColor";
      }

      if (!recordCommand.empty())
      {
        QString str_index = toIndexStr(treeWidget->indexAt(mouseEvent->pos()));
        emit this->recordEvent(treeWidget, recordCommand.c_str(), str_index);
      }
    }
    // show context menu on right button release
    else if (mouseEvent->button() == Qt::RightButton)
    {
      QModelIndex idx = treeWidget->indexAt(mouseEvent->pos());
      if (idx.isValid())
      {
        emit this->recordEvent(treeWidget, "showContextMenu", toIndexStr(idx));
      }
    }
  }

  // always return false so that its super class can still do its event recording
  return pqWidgetEventTranslator::translateEvent(senderObject, tr_event, Error);
}
