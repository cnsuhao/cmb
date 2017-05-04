//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBTreeWidget.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QLayout>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QStyle>
#include <QTimer>

// enum for different pixmap types
enum qtCMBTreeWidgetPixmap
{
  pqCheck = 0,
  pqPartialCheck = 1,
  pqUnCheck = 2,

  // All active states in lower half
  pqCheck_Active = 3,
  pqPartialCheck_Active = 4,
  pqUnCheck_Active = 5,

  pqMaxCheck = 6
};

//-----------------------------------------------------------------------------
// array of style corresponding with the qtCMBTreeWidgetPixmap enum
static const QStyle::State qtCMBTreeWidgetPixmapStyle[] = { QStyle::State_On |
    QStyle::State_Enabled,
  QStyle::State_NoChange | QStyle::State_Enabled, QStyle::State_Off | QStyle::State_Enabled,
  QStyle::State_On | QStyle::State_Enabled | QStyle::State_Active,
  QStyle::State_NoChange | QStyle::State_Enabled | QStyle::State_Active,
  QStyle::State_Off | QStyle::State_Enabled | QStyle::State_Active };

//-----------------------------------------------------------------------------
QPixmap qtCMBTreeWidget::pixmap(Qt::CheckState cs, bool active)
{
  int offset = active ? pqMaxCheck / 2 : 0;
  switch (cs)
  {
    case Qt::Checked:
      return *this->CheckPixmaps[offset + pqCheck];
    case Qt::Unchecked:
      return *this->CheckPixmaps[offset + pqUnCheck];
    case Qt::PartiallyChecked:
      return *this->CheckPixmaps[offset + pqPartialCheck];
  }
  return QPixmap();
}

//-----------------------------------------------------------------------------
qtCMBTreeWidget::qtCMBTreeWidget(QWidget* p)
  : QTreeWidget(p)
{

  QStyleOptionButton option;
  QRect r = this->style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
  option.rect = QRect(QPoint(0, 0), r.size());

  this->CheckPixmaps = new QPixmap*[6];
  for (int i = 0; i < pqMaxCheck; i++)
  {
    this->CheckPixmaps[i] = new QPixmap(r.size());
    this->CheckPixmaps[i]->fill(QColor(0, 0, 0, 0));
    QPainter painter(this->CheckPixmaps[i]);
    option.state = qtCMBTreeWidgetPixmapStyle[i];

    this->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, &painter, this);
  }

  /*
  QObject::connect(this->header(), SIGNAL(sectionClicked(int)),
                   this, SLOT(doToggle(int)),
                   Qt::QueuedConnection);

  this->header()->setClickable(true);

  QObject::connect(this->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                   this, SLOT(updateCheckState()));
  QObject::connect(this->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                   this, SLOT(updateCheckState()));

  QObject::connect(this->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                   this, SLOT(invalidateLayout()));
  QObject::connect(this->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
                   this, SLOT(invalidateLayout()));
  QObject::connect(this->model(), SIGNAL(modelReset()),
                   this, SLOT(invalidateLayout()));
*/
  this->Timer = new QTimer(this);
  this->Timer->setSingleShot(true);
  this->Timer->setInterval(10);
  //QObject::connect(this->Timer, SIGNAL(timeout()),
  //  this, SLOT(updateCheckStateInternal()));
}

//-----------------------------------------------------------------------------
qtCMBTreeWidget::~qtCMBTreeWidget()
{
  delete this->Timer;
  for (int i = 0; i < pqMaxCheck; i++)
  {
    delete this->CheckPixmaps[i];
  }
  delete[] this->CheckPixmaps;
}

//-----------------------------------------------------------------------------
bool qtCMBTreeWidget::event(QEvent* e)
{
  /*
  if(e->type() == QEvent::FocusIn ||
     e->type() == QEvent::FocusOut)
    {
    bool convert = false;
    int cs = this->headerItem()->data(0, Qt::CheckStateRole).toInt(&convert);
    if(convert)
      {
      bool active = e->type() == QEvent::FocusIn;
      this->headerItem()->setData(0, Qt::DecorationRole,
                            pixmap(static_cast<Qt::CheckState>(cs), active));
      }
    }
*/
  return Superclass::event(e);
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::updateCheckState()
{
  this->Timer->start();
  // updateCheckStateInternal needs to call rowIndex() which forces the tree to
  // sort. Hence when multiple items are being added/updated the tree is sorted
  // after every insert. To avoid that we use this timer.
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::updateCheckStateInternal()
{
  Qt::CheckState newState = Qt::Checked;
  int numChecked = 0;
  int numPartial = 0;
  int numUnchecked = 0;
  QAbstractItemModel* m = this->model();
  int numRows = m->rowCount(QModelIndex());
  for (int i = 0; i < numRows; i++)
  {
    QModelIndex idx = m->index(i, 0);
    bool convert = 0;
    int v = m->data(idx, Qt::CheckStateRole).toInt(&convert);
    if (convert)
    {
      if (v == Qt::Checked)
      {
        numChecked++;
      }
      else if (v == Qt::PartiallyChecked)
      {
        numPartial++;
      }
      else
      {
        numUnchecked++;
      }
    }
  }

  // if there are no check boxes at all
  if (0 == (numUnchecked + numPartial + numChecked))
  {
    return;
  }

  if (numChecked != numRows)
  {
    newState = ((numChecked == 0) && (numPartial == 0)) ? Qt::Unchecked : Qt::PartiallyChecked;
  }

  this->headerItem()->setCheckState(0, newState);
  this->headerItem()->setData(0, Qt::DecorationRole, pixmap(newState, this->hasFocus()));
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::allOn()
{
  QTreeWidgetItem* item;
  int i, end = this->topLevelItemCount();
  for (i = 0; i < end; i++)
  {
    item = this->topLevelItem(i);
    item->setCheckState(0, Qt::Checked);
  }
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::allOff()
{
  QTreeWidgetItem* item;
  int i, end = this->topLevelItemCount();
  for (i = 0; i < end; i++)
  {
    item = this->topLevelItem(i);
    item->setCheckState(0, Qt::Unchecked);
  }
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::doToggle(int column)
{
  if (column == 0)
  {
    bool convert = false;
    int cs = this->headerItem()->data(0, Qt::CheckStateRole).toInt(&convert);
    if (convert)
    {
      if (cs == Qt::Checked)
      {
        this->allOff();
      }
      else
      {
        // both unchecked and partial checked go here
        this->allOn();
      }
    }
  }
}

//-----------------------------------------------------------------------------
QSize qtCMBTreeWidget::sizeHint() const
{
  // lets show X items before we get a scrollbar
  // probably want to make this a member variable
  // that a caller has access to
  int maxItemHint = 10;
  // for no items, let's give a space of X pixels
  int minItemHeight = 20;

  int num = this->topLevelItemCount() + 1; /* extra room for scroll bar */
  num = qMin(num, maxItemHint);

  int pix = minItemHeight;

  if (num)
  {
    pix = qMax(pix, this->sizeHintForRow(0) * num);
  }

  int margin[4];
  this->getContentsMargins(margin, margin + 1, margin + 2, margin + 3);
  int h = pix + margin[1] + margin[3] + this->header()->frameSize().height();
  return QSize(156, h);
}

//-----------------------------------------------------------------------------
QSize qtCMBTreeWidget::minimumSizeHint() const
{
  return this->sizeHint();
}

//-----------------------------------------------------------------------------
QModelIndex qtCMBTreeWidget::getItemIndex(QTreeWidgetItem* item)
{
  if (item)
  {
    return this->indexFromItem(item);
  }
  return QModelIndex();
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::invalidateLayout()
{
  // sizeHint is dynamic, so we need to invalidate parent layouts
  // when items are added or removed
  for (QWidget* w = this->parentWidget(); w && w->layout(); w = w->parentWidget())
  {
    w->layout()->invalidate();
  }
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::dropEvent(QDropEvent* dEvent)
{
  /*
  QModelIndex topIndex;
  int col = -1;
  int row = -1;
  if (this->items->dropOn(event, &row, &col, &topIndex)) {
      QList<QModelIndex> idxs = selectedIndexes();
      QList<QPersistentModelIndex> indexes;
      for (int i = 0; i < idxs.count(); i++)
          indexes.append(idxs.at(i));

      if (indexes.contains(topIndex))
          return;

      // When removing items the drop location could shift
      QPersistentModelIndex dropRow = model()->index(row, col, topIndex);
*/
  if (dEvent->proposedAction() == Qt::MoveAction)
  {
    //move events break the way we handle drops, convert it to a copy
    dEvent->setDropAction(Qt::CopyAction);
  }

  QTreeWidgetItem* item = this->itemAt(dEvent->pos());

  if (item == NULL)
  {
    emit this->itemsDroppedOnItem(this->invisibleRootItem(), dEvent);
  }
  else
  {
    emit this->itemsDroppedOnItem(item, dEvent);
  }
  dEvent->accept();
}
//-----------------------------------------------------------------------------
Qt::DropActions qtCMBTreeWidget::supportedDropActions() const
{
  // returns what actions are supported when dropping
  return Qt::CopyAction;
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::startDrag(Qt::DropActions supportedActions)
{
  emit this->dragStarted(this);
  this->Superclass::startDrag(supportedActions);
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
  QStringList mType = this->mimeTypes();
  const QMimeData* mData = event->mimeData();
  foreach (QString type, mType)
  {
    if (mData->hasFormat(type))
    {
      event->accept();
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void qtCMBTreeWidget::dragMoveEvent(QDragMoveEvent* event)
{
  if (event->proposedAction() & this->supportedDropActions())
  {
    event->accept();
  }
}

//-----------------------------------------------------------------------------
QModelIndex qtCMBTreeWidget::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  QModelIndex suggestedIndex = this->Superclass::moveCursor(cursorAction, modifiers);

  int max_rows = this->topLevelItemCount();
  int max_colums = this->columnCount();
  QTreeWidgetItem* curItem = this->currentItem();
  int cur_col = this->currentColumn();
  if (!curItem || cur_col < 0 || cur_col >= max_colums)
  {
    return suggestedIndex;
  }

  int cur_row = this->indexOfTopLevelItem(curItem);
  if (cursorAction == QAbstractItemView::MoveNext && modifiers == Qt::NoModifier)
  {
    int next_column = cur_col + 1;
    while (next_column < max_colums && this->isColumnHidden(next_column))
    {
      // skip hidden columns.
      next_column++;
    }
    if (next_column < max_colums)
    {
      return this->indexFromItem(curItem, next_column);
    }
    else if ((cur_row + 1) == max_rows)
    {
      // User is at last row, we need to add a new row before moving to that
      // row.
      emit this->navigatedPastEnd();
      // if the table grows, the index may change.
      suggestedIndex = this->Superclass::moveCursor(cursorAction, modifiers);
    }
    // otherwise default behaviour takes it to the first column in the next
    // row, which is what is expected.
  }
  else if (cursorAction == QAbstractItemView::MovePrevious && modifiers == Qt::NoModifier)
  {
    int prev_column = cur_col - 1;
    while (prev_column >= 0 && this->isColumnHidden(prev_column))
    {
      // skip hidden columns.
      prev_column--;
    }
    if (prev_column >= 0)
    {
      return this->indexFromItem(curItem, prev_column);
    }
    else
    {
      // we need to go to the last column in the previous row.
      if (cur_row > 0)
      {
        prev_column = max_colums - 1;
        while (prev_column >= 0 && this->isColumnHidden(prev_column))
        {
          // skip hidden columns.
          prev_column--;
        }
        if (prev_column >= 0)
        {
          return this->indexFromItem(this->topLevelItem(cur_row - 1), max_colums - 1);
        }
      }
    }
  }

  return suggestedIndex;
}
//-----------------------------------------------------------------------------
void qtCMBTreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
  QPoint pos = event->pos();
  QPersistentModelIndex index = indexAt(pos);

  if (index.isValid())
  {
    if (event->button() & Qt::LeftButton)
    {
      emit this->itemLeftButtonClicked(this->itemAt(pos), index.column());
    }
    else if (event->button() & Qt::RightButton)
    {
      emit this->itemRightButtonClicked(this->itemAt(pos), index.column());
    }
    else if (event->button() & Qt::MidButton)
    {
      emit this->itemMiddleButtonClicked(this->itemAt(pos), index.column());
    }
  }
  this->Superclass::mouseReleaseEvent(event);
}
