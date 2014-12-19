/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelBuilderMainWindow.h

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

#ifndef __pqModelBuilderViewContextMenuBehavior_h
#define __pqModelBuilderViewContextMenuBehavior_h

#include <QObject>
#include <QPoint> // needed for QPoint.
#include <QPointer>
#include <QList> // needed for QList.
#include "vtkType.h"

class pqDataRepresentation;
class pqPipelineRepresentation;
class pqView;
class QAction;
class QMenu;
class pqMultiBlockInspectorPanel;
class ModelManager;

/// @ingroup Behaviors
///
/// This behavior manages showing up of a context menu with sensible pipeline
/// related actions for changing color/visibility etc. when the user
/// right-clicks on top of an object in the 3D view. Currently, it only supports
/// views with proxies that vtkSMRenderViewProxy subclasses.
class pqModelBuilderViewContextMenuBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
public:
  pqModelBuilderViewContextMenuBehavior(QObject* parent=0);
  virtual ~pqModelBuilderViewContextMenuBehavior();

  void setModelManager(ModelManager*);
  pqMultiBlockInspectorPanel* mbPanel();

signals:
  void representationBlockPicked(pqDataRepresentation*, unsigned int);

protected slots:
  /// Called when a new view is added. We add actions to the widget for context
  /// menu if the view is a render-view.
  void onViewAdded(pqView*);

  /// called to hide the representation.
  void hide();

  /// called to hide the block. the action which emits the signal will
  /// contain the block index in its data().
  void hideBlock();

  /// called to show only the selected block. the action which emits the
  /// signal will contain the block index in its data().
  void showOnlyBlock();

  /// called to show all blocks.
  void showAllBlocks();
  void showAllRepresentations();

  /// called to unset the visibility flag for the block. after this call the
  /// block will inherit the visibility from its parent. the action which
  /// emits the signal will contain the block index in its data()
  void unsetBlockVisibility();

  /// called to set the color for the block. the action which emits the
  /// signal will contain the block index in its data()
  void setBlockColor();

  /// called to unset the color for the block. the action which emits the
  /// signal will contain the block index in its data()
  void unsetBlockColor();

  /// called to set the opacity for the block. the action which emits the
  /// signal will contain the block index in its data()
  void setBlockOpacity();

  /// called to unset the opacity for the block. the action which emits the
  /// signal will contain the block index in its data()
  void unsetBlockOpacity();

  /// called to change the representation type.
  void reprTypeChanged(QAction* action);

  /// called to change the coloring mode.
  void colorMenuTriggered(QAction* action);

protected:
  /// called to build the context menu for the given representation. If the
  /// picked representation was a composite data set the block index of the
  /// selected block will be passed in blockIndex.
  virtual void buildMenu(pqDataRepresentation* repr, unsigned int blockIndex);

  /// called to build the color arrays submenu.
  virtual void buildColorFieldsMenu(
    pqPipelineRepresentation* pipelineRepr, QMenu* menu);

  /// event filter to capture the right-click. We don't directly use mechanisms
  /// from QWidget to popup the context menu since all of those mechanism seem
  /// to eat away the right button release, leaving the render window in a
  /// dragging state.
  virtual bool eventFilter(QObject* caller, QEvent* e);

  /// return the name of the block from its flat index
  QString lookupBlockName(unsigned int flatIndex) const;

  QMenu* Menu;
  QPoint Position;
  QPointer<pqDataRepresentation> PickedRepresentation;
  QList<unsigned int> PickedBlocks;
  pqMultiBlockInspectorPanel* m_MBPanel;
  QPointer<ModelManager> m_ModelManager;

private:
  Q_DISABLE_COPY(pqModelBuilderViewContextMenuBehavior)

};

#endif
