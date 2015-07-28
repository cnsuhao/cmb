//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __pqModelBuilderViewContextMenuBehavior_h
#define __pqModelBuilderViewContextMenuBehavior_h

#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QList>
#include <QColor>
#include "vtkType.h"
#include "smtk/PublicPointerDefs.h"

class pqDataRepresentation;
class pqPipelineRepresentation;
class pqView;
class QAction;
class QMenu;
class pqMultiBlockInspectorPanel;
class pqCMBModelManager;
class pqSMTKModelPanel;
class pqEditColorMapReaction;
class cmbSMTKModelInfo;

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

  void setModelPanel(pqSMTKModelPanel*);

  void syncBlockVisibility(pqDataRepresentation* rep,
    const QList<unsigned int>& visBlocks, bool visible, vtkIdType numBlocks);
  void syncBlockColor(pqDataRepresentation* rep,
    const QList<unsigned int>& colorBlocks, const QColor&);
  virtual void colorByEntity(const QString &);
  virtual void colorByAttribute(smtk::attribute::SystemPtr attSys,
    const QString& attdeftype, const QString& itemname);
  virtual void updateColorForEntities(pqDataRepresentation* rep,
    const QString& colorMode,
    const QMap<smtk::model::EntityRef, QColor >& colorEntities);

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

  /// called to switch model and mesh geometry
  void switchModelTessellation();

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
  QString lookupBlockName(unsigned int flatIndex, cmbSMTKModelInfo* minfo) const;

  QMenu* Menu;
  QPoint Position;
  QPointer<pqDataRepresentation> PickedRepresentation;
  QList<unsigned int> PickedBlocks;
  QPointer<pqSMTKModelPanel> m_ModelPanel;
  pqMultiBlockInspectorPanel* m_DataInspector;
  QPointer<pqEditColorMapReaction> m_colormapReaction;

private:
  Q_DISABLE_COPY(pqModelBuilderViewContextMenuBehavior)

};

#endif
