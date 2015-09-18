//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBSceneBuilderContextMenuBehavior.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "vtkSMProxy.h"

#include "pqCMBSceneNode.h"
#include "pqCMBSceneTree.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QRegExp>

//-----------------------------------------------------------------------------
qtCMBSceneBuilderContextMenuBehavior::qtCMBSceneBuilderContextMenuBehavior(pqCMBSceneTree *tree,
                                                   QObject* parentObject):
Superclass(parentObject)
{
  QObject::connect(
    pqApplicationCore::instance()->getServerManagerModel(),
    SIGNAL(viewAdded(pqView*)),
    this, SLOT(onViewAdded(pqView*)));
  this->Tree = tree;
}

//-----------------------------------------------------------------------------
qtCMBSceneBuilderContextMenuBehavior::~qtCMBSceneBuilderContextMenuBehavior()
{
  this->Tree = NULL;
}

//-----------------------------------------------------------------------------
void qtCMBSceneBuilderContextMenuBehavior::onViewAdded(pqView* view)
{
  if (view && view->getProxy()->IsA("vtkSMRenderViewProxy"))
    {
    // add a link view menu
    view->widget()->installEventFilter(this);
    }
}

//-----------------------------------------------------------------------------
bool qtCMBSceneBuilderContextMenuBehavior::eventFilter(QObject* caller, QEvent* e)
{
  if (e->type() == QEvent::MouseButtonPress)
    {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() & Qt::RightButton)
      {
      this->Position = me->pos();
      }
    }
  else if (e->type() == QEvent::MouseButtonRelease)
    {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() & Qt::RightButton && !this->Position.isNull())
      {
      QPoint newPos = static_cast<QMouseEvent*>(e)->pos();
      QPoint delta = newPos - this->Position;
      QWidget* senderWidget = qobject_cast<QWidget*>(caller);
      if (delta.manhattanLength() < 3 && senderWidget != NULL)
        {
        pqRenderView* view = qobject_cast<pqRenderView*>(
          pqActiveObjects::instance().activeView());
        if (view)
          {
          int pos[2] = { newPos.x(), newPos.y() } ;
          // we need to flip Y.
          int height = senderWidget->size().height();
          pos[1] = height - pos[1];
          pqDataRepresentation* picked_repr = view->pick(pos);

          this->PickedRepresentation = picked_repr;
          if (picked_repr)
            {
            this->buildMenu(senderWidget->mapToGlobal(newPos));
            }
          }
        }
      this->Position = QPoint();
      }
    }

  return Superclass::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void qtCMBSceneBuilderContextMenuBehavior::buildMenu(QPoint position)
{
  pqPipelineRepresentation* pipelineRepr =
    qobject_cast<pqPipelineRepresentation*>(this->PickedRepresentation);

  if (!pipelineRepr)
    {
    return;
    }

  pqPipelineSource *source = pipelineRepr->getInput();
  if (!source)
    {
    return;
    }
  pqCMBSceneNode *node = this->Tree->findNode(source);
  if (!node)
    {
    return;
    }

  if (!this->Tree)
    {
    return;
    }

  //remove the current selection
  this->Tree->clearSelection();

  //make the node the current selection
  node->select();

  //convert the position to the tree widget
  QPoint pos = this->Tree->getWidget()->mapFromGlobal(position);

  //show the tree widget
  this->Tree->showContextMenu(pos);
}
