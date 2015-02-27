/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBProjectServerManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __qtCMBGeologyBuilderContextMenuBehavior_h
#define __qtCMBGeologyBuilderContextMenuBehavior_h

#include <QObject>
#include <QPoint> // needed for QPoint.
#include <QPointer>
#include "cmbSystemConfig.h"

class pqDataRepresentation;
class pqPipelineRepresentation;
class pqView;
class QAction;
class pqCMBSceneTree;

/// @ingroup Behaviors
///
/// This behavior manages showing up of a context menu with sensible pipeline
/// related actions for changing scene builder items
class qtCMBGeologyBuilderContextMenuBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
public:
  qtCMBGeologyBuilderContextMenuBehavior(pqCMBSceneTree *tree, QObject* parent=0);
  virtual ~qtCMBGeologyBuilderContextMenuBehavior();
  void setTree(pqCMBSceneTree *tree){Tree=tree;}
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
  virtual bool eventFilter(QObject* caller, QEvent* e);

  pqCMBSceneTree* Tree;
  QPoint Position;
  QPointer<pqDataRepresentation> PickedRepresentation;
private:
  Q_DISABLE_COPY(qtCMBGeologyBuilderContextMenuBehavior)

};

#endif
