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

#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/MeshSet.h"
#include "vtkType.h"
#include <QColor>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QPoint>
#include <QPointer>

class pqDataRepresentation;
class pqPipelineRepresentation;
class pqView;
class QAction;
class QMenu;
class pqCMBModelManager;
class pqSMTKModelPanel;
class pqEditColorMapReaction;
class pqSMTKModelInfo;
class pqSMTKMeshInfo;
class pqRenderView;

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
  pqModelBuilderViewContextMenuBehavior(QObject* parent = 0);
  ~pqModelBuilderViewContextMenuBehavior() override;

  void setModelPanel(pqSMTKModelPanel*);

  void syncBlockVisibility(pqDataRepresentation* rep, const QList<vtkIdType>& visBlocks,
    bool visible, vtkIdType numBlocks);
  void syncBlockColor(
    pqDataRepresentation* rep, const QList<vtkIdType>& colorBlocks, const QColor&);
  virtual void colorByEntity(const QString&);
  virtual void colorByAttribute(
    smtk::attribute::SystemPtr attSys, const QString& attdeftype, const QString& itemname);
  virtual void updateColorForEntities(pqDataRepresentation* rep, const QString& colorMode,
    const QMap<smtk::model::EntityRef, QColor>& colorEntities);
  virtual void updateColorForMeshes(pqDataRepresentation* rep, const QString& colorMode,
    const QMap<smtk::mesh::MeshSet, QColor>& colorEntities);

signals:
  void representationBlockPicked(pqDataRepresentation*, vtkIdType, bool ctrlKey);

protected slots:
  /// Called when a new view is added. We add actions to the widget for context
  /// menu if the view is a render-view.
  void onViewAdded(pqView*);

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

  /// called to change the representation type.
  void reprTypeChanged(QAction* action);

  /// called to switch model and mesh geometry
  void switchModelTessellation();

  /// called to create group or add to an existing group with selected entities
  void createGroup();
  void addToGroup(QAction*);

protected:
  /// called to build the context menu for the given representation. If the
  /// picked representation was a composite data set the block index of the
  /// selected block will be passed in blockIndex.
  //virtual void buildMenu(pqDataRepresentation* repr, vtkIdType blockIndex);

  /// build the context menu based on model selections.
  virtual void buildMenuFromSelections();

  /// event filter to capture the right-click. We don't directly use mechanisms
  /// from QWidget to popup the context menu since all of those mechanism seem
  /// to eat away the right button release, leaving the render window in a
  /// dragging state.
  bool eventFilter(QObject* caller, QEvent* e) override;

  virtual void showAllEntitiesAndMeshes(
    const QList<pqSMTKModelInfo*>&, const QList<pqSMTKMeshInfo*>&);
  // \a sessionBlocks is map of <sessionId, < Entities, Meshes> >
  virtual void getSelectedEntitiesAndMeshes(
    QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> >& sessionBlocks);
  virtual void setSelectedBlocksColor(const QColor& color);

  void pickRepresentationBlock(
    pqRenderView* view, const QPoint& newPos, QWidget* senderWidget, bool ctrl);

  QMenu* m_contextMenu;
  QPoint m_clickPosition;
  QPointer<pqSMTKModelPanel> m_modelPanel;
  QPointer<pqEditColorMapReaction> m_colormapReaction;
  QMap<pqSMTKModelInfo*, QList<vtkIdType> > m_selModelBlocks;
  QMap<pqSMTKMeshInfo*, QList<vtkIdType> > m_selMeshBlocks;

private:
  Q_DISABLE_COPY(pqModelBuilderViewContextMenuBehavior)
};

#endif
