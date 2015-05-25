//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBTreeWidget - a CMB Tree widget.
// .SECTION Description
//  A convenience QTreeWidget with extra features:
//  1.  Automatic size hints based on contents
//  2.  A check box added in a header if items have check boxes
//  3.  Navigation through columns of top level items on Tab.
//  4.  Signal emitted when user navigates beyond end of the table giving an
//      opportunity to the lister to grow the table.
//  5.  Customized Drag-n-Drop
// .SECTION Caveats

#ifndef _qtCMBTreeWidget_h
#define _qtCMBTreeWidget_h

#include "cmbAppCommonExport.h"
#include <QTreeWidget>
#include "cmbSystemConfig.h"

class QDropEvent;
class QMouseEvent;

class CMBAPPCOMMON_EXPORT qtCMBTreeWidget : public QTreeWidget
{
  typedef QTreeWidget Superclass;
  Q_OBJECT

public:

  qtCMBTreeWidget(QWidget* p = NULL);
  ~qtCMBTreeWidget();

  // Description:
  // Handle tree widget events
  bool event(QEvent* e);

  // Description:
  // give a hint on the size
  QSize sizeHint() const;
  QSize minimumSizeHint() const;

  // Description:
  // Get the item index of the tree item
  QModelIndex getItemIndex(QTreeWidgetItem*);

public slots:
  // Description:
  // Slots to update the check states of all tree items
  void allOn();
  void allOff();

signals:
  // Description:
  // Fired when moveCursor takes the cursor beyond the last row.
  void navigatedPastEnd();

  // Description:
  // Fired when drag-n-drop happpens
  void dragStarted(QTreeWidget*);
  void itemsDroppedOnItem(QTreeWidgetItem* pItem, QDropEvent* dEvent);

  void itemLeftButtonClicked(QTreeWidgetItem*, int);
  void itemRightButtonClicked(QTreeWidgetItem*, int);
  void itemMiddleButtonClicked(QTreeWidgetItem*, int);

protected slots:
  void doToggle(int col);
  void updateCheckState();
  void invalidateLayout();

protected:
  // Description:
  // Support for customized drag-n-drop events
  virtual Qt::DropActions supportedDropActions() const;
  void dragEnterEvent( QDragEnterEvent * event );
  void dragMoveEvent( QDragMoveEvent * event );
  virtual void startDrag ( Qt::DropActions supportedActions );

  virtual void mouseReleaseEvent( QMouseEvent * );

private slots:
  void updateCheckStateInternal();

protected:

  // Description:
  // Customize drop-event
  virtual void dropEvent(QDropEvent* event);

  // Description:
  // Move the cursor in the way described by cursorAction,
  // using the information provided by the button modifiers.
  virtual QModelIndex moveCursor(CursorAction cursorAction,
    Qt::KeyboardModifiers modifiers);

  // Description:
  // ivars
  QPixmap** CheckPixmaps;
  QPixmap pixmap(Qt::CheckState state, bool active);
  QTimer* Timer;
};

#endif // !_qtCMBTreeWidget_h
