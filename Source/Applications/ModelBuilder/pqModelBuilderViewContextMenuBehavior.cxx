/*=========================================================================

  Program:   CMB
  Module:    pqModelBuilderViewContextMenuBehavior.h

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

#include "pqModelBuilderViewContextMenuBehavior.h"

#include "pqActiveObjects.h"
#include "pqPVApplicationCore.h"
#include "pqEditColorMapReaction.h"
#include "pqMultiBlockInspectorPanel.h"
#include "pqPipelineRepresentation.h"
#include "pqApplicationCore.h"
#include "pqRenderView.h"
#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"

#include "pqUndoStack.h"
#include "vtkDataObject.h"
#include "vtkNew.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVGeneralSettings.h"
#include "vtkSMArrayListDomain.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMViewProxy.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPair>
#include <QWidget>
#include "ModelManager.h"
#include "qtSMTKModelPanel.h"

namespace
{
  // converts array association/name pair to QVariant.
  QVariant convert(const QPair<int, QString>& array)
    {
    if (!array.second.isEmpty())
      {
      QStringList val;
      val << QString::number(array.first)
          << array.second;
      return val;
      }
    return QVariant();
    }

  // converts QVariant to array association/name pair.
  QPair <int, QString> convert(const QVariant& val)
    {
    QPair<int, QString> result;
    if (val.canConvert<QStringList>())
      {
      QStringList list = val.toStringList();
      Q_ASSERT(list.size() == 2);
      result.first = list[0].toInt();
      result.second = list[1];
      }
    return result;
    }
}

//-----------------------------------------------------------------------------
pqModelBuilderViewContextMenuBehavior::pqModelBuilderViewContextMenuBehavior(QObject* parentObject)
  : Superclass(parentObject)
{
  QObject::connect(
    pqApplicationCore::instance()->getServerManagerModel(),
    SIGNAL(viewAdded(pqView*)),
    this, SLOT(onViewAdded(pqView*)));
  this->Menu = new QMenu();
  this->Menu << pqSetName("PipelineContextMenu");
  this->m_DataInspector = new pqMultiBlockInspectorPanel(NULL);
  this->m_DataInspector->setVisible(false);
}

//-----------------------------------------------------------------------------
pqModelBuilderViewContextMenuBehavior::~pqModelBuilderViewContextMenuBehavior()
{
  delete this->Menu;
  delete this->m_DataInspector;
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setModelPanel(qtSMTKModelPanel* panel)
{
  this->m_ModelPanel = panel;
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockVisibility(
    const QList<unsigned int>& visBlocks, bool visible)
{
  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if (panel)
    {
      if(!visible)
      {
      pqOutputPort* outport = panel->getOutputPort();
      if(outport)
        {
        outport->setSelectionInput(0, 0);
        }     
      }
    panel->setBlockVisibility(visBlocks, visible);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockColor(
    const QList<unsigned int>& colorBlocks, const QColor& color)
{
  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if (panel)
    {
    if(color.isValid())
      panel->setBlockColor(colorBlocks, color);
    else
      panel->clearBlockColor(colorBlocks);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::onViewAdded(pqView* view)
{
  if (view && view->getProxy()->IsA("vtkSMRenderViewProxy"))
    {
    // add a link view menu
    view->getWidget()->installEventFilter(this);
    }
}

//-----------------------------------------------------------------------------
bool pqModelBuilderViewContextMenuBehavior::eventFilter(QObject* caller, QEvent* e)
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
          unsigned int blockIndex = 0;
          this->PickedRepresentation = view->pickBlock(pos, blockIndex);

          // we want to select this block.
          if(this->PickedRepresentation)
            {
            emit this->representationBlockPicked(this->PickedRepresentation, blockIndex);
            }

          this->buildMenu(this->PickedRepresentation, blockIndex);
          this->Menu->popup(senderWidget->mapToGlobal(newPos));
          }
        }
      this->Position = QPoint();
      }
    }

  return Superclass::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::buildMenu(pqDataRepresentation* repr,
                                              unsigned int blockIndex)
{
  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());  

  // get currently selected block ids
  this->PickedBlocks.clear();

  bool picked_block_in_selected_blocks = false;
  pqSelectionManager *selectionManager =
    pqPVApplicationCore::instance()->selectionManager();
  if(selectionManager)
    {
    pqOutputPort *port = selectionManager->getSelectedPort();
    if(port)
      {
      vtkSMSourceProxy *activeSelection = port->getSelectionInput();
      if(activeSelection &&
         strcmp(activeSelection->GetXMLName(), "BlockSelectionSource") == 0)
        {
        vtkSMPropertyHelper blocksProp(activeSelection, "Blocks");
        QVector <vtkIdType> vblocks;
        vblocks.resize(blocksProp.GetNumberOfElements());
        blocksProp.Get(&vblocks[0], blocksProp.GetNumberOfElements());
        foreach (const vtkIdType &index, vblocks)
          {
          if (index >= 0)
            {
            if (static_cast<unsigned int>(index) == blockIndex)
              {
              picked_block_in_selected_blocks = true;
              }
            this->PickedBlocks.push_back(static_cast<unsigned int>(index));
            }
          }
        }
      }
    }

  if (!picked_block_in_selected_blocks)
    {
    // the block that was clicked on is not one of the currently selected
    // block so actions should only affect that block
    this->PickedBlocks.clear();
    this->PickedBlocks.append(static_cast<unsigned int>(blockIndex));
    }

  this->Menu->clear();
  if (repr)
    {
    QAction* action;

    vtkPVDataInformation *info = repr->getInputDataInformation();
    vtkPVCompositeDataInformation *compositeInfo = info->GetCompositeDataInformation();
    if(compositeInfo && compositeInfo->GetDataIsComposite())
      {
      bool multipleBlocks = this->PickedBlocks.size() > 1;

      if(multipleBlocks)
        {
        this->Menu->addAction(QString("%1 Entities").arg(this->PickedBlocks.size()));
        }
      else
        {
        QString blockName = this->lookupBlockName(blockIndex);
        this->Menu->addAction(QString("Entity '%1'").arg(blockName));
        }
      this->Menu->addSeparator();

      QAction *hideBlockAction =
        this->Menu->addAction(QString("Hide Entit%1").arg(multipleBlocks ? "ies" : "y"));
      this->connect(hideBlockAction, SIGNAL(triggered()),
                    this, SLOT(hideBlock()));

//      action = this->Menu->addAction("Hide All Entities");
//      QObject::connect(action, SIGNAL(triggered()), this, SLOT(hide()));

      QAction *showOnlyBlockAction =
        this->Menu->addAction(QString("Show Only Entit%1").arg(multipleBlocks ? "ies" : "y"));
      this->connect(showOnlyBlockAction, SIGNAL(triggered()),
                    this, SLOT(showOnlyBlock()));

      QAction *showAllBlocksAction =
        this->Menu->addAction("Show All Entities");
      this->connect(showAllBlocksAction, SIGNAL(triggered()),
                    this, SLOT(showAllBlocks()));
/*
      QAction *unsetVisibilityAction =
        this->Menu->addAction(QString("Unset Entity %1")
            .arg(multipleBlocks ? "Visibilities" : "Visibility"));
      this->connect(unsetVisibilityAction, SIGNAL(triggered()),
                    this, SLOT(unsetBlockVisibility()));
*/
      this->Menu->addSeparator();

      QAction *setBlockColorAction =
        this->Menu->addAction(QString("Set Entity Color%1")
          .arg(multipleBlocks ? "s" : ""));
      this->connect(setBlockColorAction, SIGNAL(triggered()),
                    this, SLOT(setBlockColor()));

      QAction *unsetBlockColorAction =
        this->Menu->addAction(QString("Unset Entity Color%1")
          .arg(multipleBlocks ? "s" : ""));
      this->connect(unsetBlockColorAction, SIGNAL(triggered()),
                    this, SLOT(unsetBlockColor()));

      this->Menu->addSeparator();

      QAction *setBlockOpacityAction =
        this->Menu->addAction(QString("Set Entity %1")
          .arg(multipleBlocks ? "Opacities" : "Opacity"));
      this->connect(setBlockOpacityAction, SIGNAL(triggered()),
                    this, SLOT(setBlockOpacity()));

      QAction *unsetBlockOpacityAction =
        this->Menu->addAction(QString("Unset Entity %1")
            .arg(multipleBlocks ? "Opacities" : "Opacity"));
      this->connect(unsetBlockOpacityAction, SIGNAL(triggered()),
                    this, SLOT(unsetBlockOpacity()));

      this->Menu->addSeparator();
      }


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

    this->Menu->addSeparator();

    pqPipelineRepresentation* pipelineRepr =
      qobject_cast<pqPipelineRepresentation*>(repr);

    if (pipelineRepr)
      {
      QMenu* colorFieldsMenu = this->Menu->addMenu("Color By")
        << pqSetName("ColorBy");
      this->buildColorFieldsMenu(pipelineRepr, colorFieldsMenu);
      }

    action = this->Menu->addAction("Edit Color");
    new pqEditColorMapReaction(action);

    this->Menu->addSeparator();
    }

  // when nothing was picked we show the "link camera" menu.
  this->Menu->addAction("Show All Models",
    this, SLOT(showAllRepresentations()));
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::buildColorFieldsMenu(
  pqPipelineRepresentation* pipelineRepr, QMenu* menu)
{
  QObject::connect(menu, SIGNAL(triggered(QAction*)),
    this, SLOT(colorMenuTriggered(QAction*)), Qt::QueuedConnection);

  QIcon cellDataIcon(":/pqWidgets/Icons/pqCellData16.png");
  QIcon pointDataIcon(":/pqWidgets/Icons/pqPointData16.png");
  QIcon solidColorIcon(":/pqWidgets/Icons/pqSolidColor16.png");

  menu->addAction(solidColorIcon, "Solid Color")->setData(
    convert(QPair<int, QString>()));
  vtkSMProperty* prop = pipelineRepr->getProxy()->GetProperty("ColorArrayName");
  vtkSMArrayListDomain* domain = prop?
    vtkSMArrayListDomain::SafeDownCast(prop->FindDomain("vtkSMArrayListDomain")) : NULL;
  if (!domain)
    {
    return;
    }

  // We are only showing array names here without worrying about components since that
  // keeps the menu simple and code even simpler :).
  for (unsigned int cc=0, max = domain->GetNumberOfStrings(); cc < max; cc++)
    {
    int association = domain->GetFieldAssociation(cc);
    int icon_association = domain->GetDomainAssociation(cc);
    QString name = domain->GetString(cc);

    QIcon& icon = (icon_association == vtkDataObject::CELL)?
      cellDataIcon : pointDataIcon;

    QVariant data = convert(QPair<int, QString>(association, name));
    menu->addAction(icon, name)->setData(data);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::colorMenuTriggered(QAction* action)
{
  QPair<int, QString> array = convert(action->data());
  if (this->PickedRepresentation)
    {
    BEGIN_UNDO_SET("Change coloring");
    vtkSMViewProxy* view = pqActiveObjects::instance().activeView()->getViewProxy();
    vtkSMProxy* reprProxy = this->PickedRepresentation->getProxy();

    vtkSMProxy* oldLutProxy = vtkSMPropertyHelper(reprProxy, "LookupTable", true).GetAsProxy();

    vtkSMPVRepresentationProxy::SetScalarColoring(
      reprProxy, array.second.toLatin1().data(), array.first);

    vtkNew<vtkSMTransferFunctionManager> tmgr;

    // Hide unused scalar bars, if applicable.
    vtkPVGeneralSettings* gsettings = vtkPVGeneralSettings::GetInstance();
    switch (gsettings->GetScalarBarMode())
      {
    case vtkPVGeneralSettings::AUTOMATICALLY_HIDE_SCALAR_BARS:
    case vtkPVGeneralSettings::AUTOMATICALLY_SHOW_AND_HIDE_SCALAR_BARS:
      tmgr->HideScalarBarIfNotNeeded(oldLutProxy, view);
      break;
      }

    if (!array.second.isEmpty())
      {
      // we could now respect some application setting to determine if the LUT is
      // to be reset.
      vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(reprProxy, true);

      /// BUG #0011858. Users often do silly things!
      bool reprVisibility =
        vtkSMPropertyHelper(reprProxy, "Visibility", /*quiet*/true).GetAsInt() == 1;

      // now show used scalar bars if applicable.
      if (reprVisibility &&
        gsettings->GetScalarBarMode() ==
        vtkPVGeneralSettings::AUTOMATICALLY_SHOW_AND_HIDE_SCALAR_BARS)
        {
        vtkSMPVRepresentationProxy::SetScalarBarVisibility(reprProxy, view, true);
        }
      }

    this->PickedRepresentation->renderViewEventually();
    END_UNDO_SET();
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::reprTypeChanged(QAction* action)
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
void pqModelBuilderViewContextMenuBehavior::hide()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  pqDataRepresentation* repr = this->PickedRepresentation;
  QList<unsigned int> emptyList;
  this->m_ModelPanel->showOnlyBlocks(
    repr, emptyList);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::hideBlock()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  this->m_ModelPanel->setBlockVisibility(
    this->PickedRepresentation, this->PickedBlocks, false);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showOnlyBlock()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  this->m_ModelPanel->showOnlyBlocks(
    this->PickedRepresentation, this->PickedBlocks);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllBlocks()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  this->m_ModelPanel->showAllBlocks(
    this->PickedRepresentation);
/*
  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if (panel)
    {
    panel->showAllBlocks();
    }
*/
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllRepresentations()
{
  if(!this->m_ModelPanel || !this->m_ModelPanel->modelManager())
    return;

  foreach(pqDataRepresentation* repr,
          this->m_ModelPanel->modelManager()->modelRepresentations())
    {
    this->m_ModelPanel->showAllBlocks(repr);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockColor()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  QColor color = QColorDialog::getColor(QColor(),
    this->m_DataInspector, "Choose Block Color",
    QColorDialog::DontUseNativeDialog);
  if(color.isValid())
    {
    this->m_ModelPanel->setBlockColor(
      this->PickedRepresentation, this->PickedBlocks, color);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::unsetBlockColor()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_ModelPanel)
    {
    return;
    }
  QColor invalidColor;
  this->m_ModelPanel->setBlockColor(
    this->PickedRepresentation, this->PickedBlocks, invalidColor);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockOpacity()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action)
    {
    return;
    }

  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if(panel)
    {
    panel->promptAndSetBlockOpacity(this->PickedBlocks);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::unsetBlockOpacity()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action)
    {
    return;
    }

  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if(panel)
    {
    panel->clearBlockOpacity(this->PickedBlocks);
    }
}

//-----------------------------------------------------------------------------
QString pqModelBuilderViewContextMenuBehavior::lookupBlockName(unsigned int flatIndex) const
{
  pqMultiBlockInspectorPanel *panel = this->m_DataInspector;
  if(panel)
    {
    return panel->lookupBlockName(flatIndex);
    }
  else
    {
    return QString();
    }
}
