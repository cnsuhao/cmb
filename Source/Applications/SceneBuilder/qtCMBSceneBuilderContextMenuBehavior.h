//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __qtCMBSceneBuilderContextMenuBehavior_h
#define __qtCMBSceneBuilderContextMenuBehavior_h

#include "cmbSystemConfig.h"
#include <QObject>
#include <QPoint> // needed for QPoint.
#include <QPointer>

class pqDataRepresentation;
class pqPipelineRepresentation;
class pqView;
class QAction;
class pqCMBSceneTree;

/// @ingroup Behaviors
///
/// This behavior manages showing up of a context menu with sensible pipeline
/// related actions for changing scene builder items
class qtCMBSceneBuilderContextMenuBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  qtCMBSceneBuilderContextMenuBehavior(pqCMBSceneTree* tree, QObject* parent = 0);
  ~qtCMBSceneBuilderContextMenuBehavior() override;
  void setTree(pqCMBSceneTree* tree) { Tree = tree; }
protected slots:
  /// Called when a new view is added. We add actions to the widget for context
  /// menu if the view is a render-view.
  void onViewAdded(pqView*);

protected:
  /// called to build the context menu for the given representation.
  virtual void buildMenu(QPoint position);

  /// event filter to capture the right-click. We don't directly use mechanisms
  /// from QWidget to popup the context menu since all of those mechanism seem
  /// to eat away the right button release, leaving the render window in a
  /// dragging state.
  bool eventFilter(QObject* caller, QEvent* e) override;

  pqCMBSceneTree* Tree;
  QPoint Position;
  QPointer<pqDataRepresentation> PickedRepresentation;

private:
  Q_DISABLE_COPY(qtCMBSceneBuilderContextMenuBehavior)
};

#endif
