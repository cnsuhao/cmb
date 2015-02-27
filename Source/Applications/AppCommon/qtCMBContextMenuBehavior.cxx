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
#include "qtCMBContextMenuBehavior.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqEditColorMapReaction.h"
#include "pqPipelineRepresentation.h"
#include "pqRenderView.h"
#include "pqScalarsToColors.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqUndoStack.h"
#include "vtkSMProxy.h"

#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QRegExp>

//-----------------------------------------------------------------------------
qtCMBContextMenuBehavior::qtCMBContextMenuBehavior(QObject* parentObject)
  : Superclass(parentObject)
{
  QObject::connect(
    pqApplicationCore::instance()->getServerManagerModel(),
    SIGNAL(viewAdded(pqView*)),
    this, SLOT(onViewAdded(pqView*)));
  this->Menu = new QMenu();
  this->Menu << pqSetName("PipelineContextMenu");
}

//-----------------------------------------------------------------------------
qtCMBContextMenuBehavior::~qtCMBContextMenuBehavior()
{
  delete this->Menu;
}

//-----------------------------------------------------------------------------
void qtCMBContextMenuBehavior::onViewAdded(pqView* view)
{
  if (view && view->getProxy()->IsA("vtkSMRenderViewProxy"))
    {
    // add a link view menu
    view->getWidget()->installEventFilter(this);
    }
}

//-----------------------------------------------------------------------------
bool qtCMBContextMenuBehavior::eventFilter(QObject* caller, QEvent* e)
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
            this->Menu->clear();
            this->buildMenu(picked_repr);
            this->Menu->popup(senderWidget->mapToGlobal(newPos));
            }
          }
        }
      this->Position = QPoint();
      }
    }

  return Superclass::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void qtCMBContextMenuBehavior::buildMenu(pqDataRepresentation* repr)
{
  pqPipelineRepresentation* pipelineRepr =
    qobject_cast<pqPipelineRepresentation*>(repr);

  /*
  QAction* action;
  action = this->Menu->addAction("Hide");
  QObject::connect(action, SIGNAL(triggered()), this, SLOT(hide()));
  */

  QMenu* reprMenu = this->Menu->addMenu("Representation")
    << pqSetName("Representation");

  // populate the representation types menu.
  QList<QVariant> rTypes = pqSMAdaptor::getEnumerationPropertyDomain(
    repr->getProxy()->GetProperty("Representation"));
  QVariant curRType = pqSMAdaptor::getEnumerationProperty(
    repr->getProxy()->GetProperty("Representation"));
  foreach (QVariant rtype, rTypes)
    {
    QAction* raction = reprMenu->addAction(rtype.toString());
    raction->setCheckable(true);
    raction->setChecked(rtype == curRType);
    }
  QObject::connect(reprMenu, SIGNAL(triggered(QAction*)),
    this, SLOT(reprTypeChanged(QAction*)));
}

//-----------------------------------------------------------------------------
void qtCMBContextMenuBehavior::reprTypeChanged(QAction* action)
{
  pqDataRepresentation* repr = this->PickedRepresentation;
  if (repr)
    {
    BEGIN_UNDO_SET("Representation Type Changed");
    pqSMAdaptor::setEnumerationProperty(
      repr->getProxy()->GetProperty("Representation"),
      action->text());
    repr->getProxy()->UpdateVTKObjects();
    repr->renderViewEventually();
    END_UNDO_SET();
    }
}

//-----------------------------------------------------------------------------
void qtCMBContextMenuBehavior::hide()
{
  pqDataRepresentation* repr = this->PickedRepresentation;
  if (repr)
    {
    BEGIN_UNDO_SET("Visibility Changed");
    repr->setVisible(false);
    repr->renderViewEventually();
    END_UNDO_SET();
    }
}
