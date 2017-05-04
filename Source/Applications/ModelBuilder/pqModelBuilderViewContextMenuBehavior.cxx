//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqModelBuilderViewContextMenuBehavior.h"

#include "SimBuilder/pqSMTKUIHelper.h"
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCMBContextMenuHelper.h"
#include "pqCMBModelManager.h"
#include "pqEditColorMapReaction.h"
#include "pqMultiBlockInspectorPanel.h"
#include "pqPVApplicationCore.h"
#include "pqPipelineRepresentation.h"
#include "pqRenderView.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqSMAdaptor.h"
#include "pqSMTKMeshInfo.h"
#include "pqSMTKModelInfo.h"
#include "pqSMTKModelPanel.h"
#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqUndoStack.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/mesh/Collection.h"
#include "smtk/model/Group.h"
#include "vtkNew.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVGeneralSettings.h"
#include "vtkPVSMTKMeshInformation.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMTransferFunctionProxy.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QWidget>

//-----------------------------------------------------------------------------
pqModelBuilderViewContextMenuBehavior::pqModelBuilderViewContextMenuBehavior(QObject* parentObject)
  : Superclass(parentObject)
{
  QObject::connect(pqApplicationCore::instance()->getServerManagerModel(),
    SIGNAL(viewAdded(pqView*)), this, SLOT(onViewAdded(pqView*)));
  this->m_contextMenu = new QMenu();
  this->m_contextMenu << pqSetName("PipelineContextMenu");
  this->m_dataInspector = new pqMultiBlockInspectorPanel(NULL);
  this->m_dataInspector->setVisible(false);
}

//-----------------------------------------------------------------------------
pqModelBuilderViewContextMenuBehavior::~pqModelBuilderViewContextMenuBehavior()
{
  delete this->m_contextMenu;
  delete this->m_dataInspector;
  if (this->m_colormapReaction)
  {
    delete this->m_colormapReaction;
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setModelPanel(pqSMTKModelPanel* panel)
{
  this->m_modelPanel = panel;
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::syncBlockVisibility(pqDataRepresentation* rep,
  const QList<unsigned int>& visBlocks, bool visible, vtkIdType numBlocks)
{
  pqMultiBlockInspectorPanel* panel = this->m_dataInspector;
  if (panel && rep)
  {
    pqOutputPort* prevOutport = pqActiveObjects::instance().activePort();
    pqDataRepresentation* prevRep = pqActiveObjects::instance().activeRepresentation();

    pqOutputPort* outport = rep->getOutputPortFromInput();
    panel->onPortChanged(outport);
    panel->onRepresentationChanged(rep);

    if (visible && visBlocks.count() && rep)
    {
      // if one block is visible, the rep has to be visible
      rep->setVisible(visible);
    }

    panel->setBlockVisibility(visBlocks, visible);

    if (!visible)
    {
      if (outport)
      {
        outport->setSelectionInput(0, 0);
      }
      // if all blocks are off, the rep should be invisible
      if (rep && outport)
      {
        vtkSMProxy* proxy = rep->getProxy();
        vtkSMProperty* blockVisibilityProperty = proxy->GetProperty("BlockVisibility");
        proxy->UpdatePropertyInformation(blockVisibilityProperty);
        vtkSMIntVectorProperty* ivp = vtkSMIntVectorProperty::SafeDownCast(blockVisibilityProperty);

        if (ivp)
        {
          /*
          vtkPVDataInformation *info = outport->getDataInformation();
          if(!info)
            return;
          vtkPVCompositeDataInformation *compositeInfo =
            info->GetCompositeDataInformation();
          if(!compositeInfo || !compositeInfo->GetDataIsComposite())
            return;
*/
          vtkIdType nbElems = static_cast<vtkIdType>(ivp->GetNumberOfElements());
          if (nbElems / 2 != numBlocks)
            return;

          bool repVisible = false;
          for (vtkIdType i = 0; i + 1 < nbElems; i += 2)
          {
            if (ivp->GetElement(i + 1))
            {
              repVisible = true;
              break;
            }
          }
          rep->setVisible(repVisible);
        }
      }
    }

    panel->onRepresentationChanged(prevRep);
    panel->onPortChanged(prevOutport);
  }
}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::colorByEntity(const QString& colorMode)
{
  if (!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  pqMultiBlockInspectorPanel* datapanel = this->m_dataInspector;
  if (!datapanel)
    return;
  // active rep
  pqDataRepresentation* activeRep = pqActiveObjects::instance().activeRepresentation();
  pqSMTKModelInfo* modinfo = NULL;
  pqSMTKMeshInfo* meshinfo = NULL;
  if (!activeRep)
  {
    // if no active rep, we try to activate a model rep
    modinfo = this->m_modelPanel->modelManager()->activateModelRepresentation();
    if (modinfo)
      activeRep = modinfo->Representation;
  }
  else
  {
    modinfo = this->m_modelPanel->modelManager()->modelInfo(activeRep);
    meshinfo = this->m_modelPanel->modelManager()->meshInfo(activeRep);
  }

  if (!modinfo && !meshinfo)
    return;
  if ((modinfo && modinfo->ColorMode == colorMode) ||
    (meshinfo && meshinfo->ColorMode == colorMode))
    return;
  QStringList list;
  this->m_modelPanel->modelManager()->supportedColorByModes(list);
  if (!list.contains(colorMode))
    return;

  // turn off the current scalar bar before switch to the new array
  vtkSMProxy* ctfProxy = activeRep->getLookupTableProxy();
  vtkSMProxy* sb = vtkSMTransferFunctionProxy::FindScalarBarRepresentation(
    ctfProxy, pqActiveObjects::instance().activeView()->getProxy());
  if (sb)
  {
    vtkSMPropertyHelper(sb, "Visibility").Set(0);
    sb->UpdateVTKObjects();
  }

  QList<unsigned int> indices;
  QColor color;
  if (modinfo)
  {
    smtk::common::UUID modelId = modinfo->Info->GetModelUUID();
    smtk::model::Model activeModel(
      this->m_modelPanel->modelManager()->managerProxy()->modelManager(), modelId);
    if (!activeModel.isValid())
      return;

    QMap<smtk::model::EntityRef, QColor> colorEntities;
    std::map<smtk::common::UUID, unsigned int>::const_iterator uit =
      modinfo->Info->GetUUID2BlockIdMap().begin();
    for (; uit != modinfo->Info->GetUUID2BlockIdMap().end(); ++uit)
    {
      indices.append(uit->second);
    }
    // clear all colors
    datapanel->clearBlockColor(indices);

    if (colorMode == vtkModelMultiBlockSource::GetVolumeTagName())
    {
      // if colorby-volume, get volumes' color,
      smtk::model::CellEntities modVols = activeModel.cells();
      for (smtk::model::CellEntities::iterator it = modVols.begin(); it != modVols.end(); ++it)
      {
        if (it->isVolume() && it->hasColor() &&
          pqCMBContextMenuHelper::getValidEntityColor(color, *it))
        {
          colorEntities[*it] = color;
        }
      }
    }
    else if (colorMode == vtkModelMultiBlockSource::GetGroupTagName())
    {
      // if colorby-group, get groups' color,
      smtk::model::Groups modGroups = activeModel.groups();
      for (smtk::model::Groups::iterator it = modGroups.begin(); it != modGroups.end(); ++it)
      {
        if (it->hasColor() && pqCMBContextMenuHelper::getValidEntityColor(color, *it))
        {
          colorEntities[*it] = color;
        }
      }
    }
    else if (colorMode == vtkModelMultiBlockSource::GetEntityTagName())
    {
      // if colorby-entity, get entities' color,
      for (uit = modinfo->Info->GetUUID2BlockIdMap().begin();
           uit != modinfo->Info->GetUUID2BlockIdMap().end(); ++uit)
      {
        smtk::model::EntityRef eref(activeModel.manager(), uit->first);
        if (eref.hasColor() && pqCMBContextMenuHelper::getValidEntityColor(color, eref))
        {
          colorEntities[eref] = color;
        }
      }
    }

    if (colorEntities.size() > 0)
    {
      this->updateColorForEntities(activeRep, colorMode, colorEntities);
      // EntityColorTable is no longer used to avoid random color
      //this->m_modelPanel->modelManager()->updateEntityColorTable(
      //  activeRep, colorEntities, colorMode);
    }
    modinfo->ColorMode = colorMode;
  }
  else if (meshinfo)
  {
    QMap<smtk::mesh::MeshSet, QColor> colorMeshes;
    std::map<smtk::mesh::MeshSet, unsigned int>::const_iterator mit =
      meshinfo->Info->GetMesh2BlockIdMap().begin();
    for (; mit != meshinfo->Info->GetMesh2BlockIdMap().end(); ++mit)
      indices.append(mit->second);
    // clear all colors
    datapanel->clearBlockColor(indices);

    // if colorby-entity, get meshes' color,
    for (mit = meshinfo->Info->GetMesh2BlockIdMap().begin();
         mit != meshinfo->Info->GetMesh2BlockIdMap().end(); ++mit)
    {
      smtk::mesh::CollectionPtr c = mit->first.collection();
      if (pqCMBContextMenuHelper::validMeshColorMode(colorMode, mit->first) &&
        pqCMBContextMenuHelper::getValidMeshColor(color, mit->first))
      {
        colorMeshes[mit->first] = color;
      }
    }

    if (colorMeshes.size() > 0)
    {
      this->updateColorForMeshes(activeRep, colorMode, colorMeshes);
    }
    meshinfo->ColorMode = colorMode;
    // still need to know modinfo for LUTs
    modinfo = meshinfo->ModelInfo;
  }

  // EntityColorTable is no longer used to avoid random color
  // check git history e826a54cbb325016130b19b0d325e4b71e0526d4
  // for volume, group LUT

  activeRep->renderViewEventually();
}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::colorByAttribute(
  smtk::attribute::SystemPtr attSys, const QString& attdeftype, const QString& itemname)
{
  if (!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  pqMultiBlockInspectorPanel* datapanel = this->m_dataInspector;
  if (!datapanel)
    return;

  // active rep
  pqSMTKModelInfo* minfo = this->m_modelPanel->modelManager()->activateModelRepresentation();
  if (!minfo)
  {
    qDebug("There is no model to be colored by!");
    return;
  }

  pqDataRepresentation* activeRep = minfo->Representation;

  smtk::common::UUID modelId = minfo->Info->GetModelUUID();
  smtk::model::Model activeModel(
    this->m_modelPanel->modelManager()->managerProxy()->modelManager(), modelId);
  if (!activeModel.isValid())
    return;

  // turn off the current scalar bar before switch to the new array
  vtkSMProxy* ctfProxy = activeRep->getLookupTableProxy();
  vtkSMProxy* sb = vtkSMTransferFunctionProxy::FindScalarBarRepresentation(
    ctfProxy, pqActiveObjects::instance().activeView()->getProxy());
  if (sb)
  {
    vtkSMPropertyHelper(sb, "Visibility").Set(0);
    sb->UpdateVTKObjects();
  }

  // clear all colors
  QList<unsigned int> indices;
  std::map<smtk::common::UUID, unsigned int>::const_iterator uit =
    minfo->Info->GetUUID2BlockIdMap().begin();
  for (; uit != minfo->Info->GetUUID2BlockIdMap().end(); ++uit)
  {
    indices.append(uit->second);
  }
  datapanel->clearBlockColor(indices);

  this->m_modelPanel->modelManager()->colorRepresentationByAttribute(
    activeRep, attSys, attdeftype, itemname);
}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::updateColorForEntities(pqDataRepresentation* rep,
  const QString& colorMode, const QMap<smtk::model::EntityRef, QColor>& colorEntities)
{
  if (colorMode == "None")
    return;

  foreach (const smtk::model::EntityRef& entref, colorEntities.keys())
  {
    QSet<unsigned int> blockIds;
    if ((entref.isVolume() && colorMode == vtkModelMultiBlockSource::GetVolumeTagName()) ||
      (entref.isGroup() && colorMode == vtkModelMultiBlockSource::GetGroupTagName()) ||
      (entref.hasIntegerProperty("block_index") &&
          colorMode == vtkModelMultiBlockSource::GetEntityTagName()))
    {
      pqCMBContextMenuHelper::accumulateChildGeometricEntities(blockIds, entref);
    }
    if (blockIds.size() > 0)
      this->syncBlockColor(rep, QList<unsigned int>::fromSet(blockIds), colorEntities[entref]);
  }
}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::updateColorForMeshes(pqDataRepresentation* rep,
  const QString& colorMode, const QMap<smtk::mesh::MeshSet, QColor>& colorEntities)
{
  if (colorMode == "None")
    return;

  foreach (const smtk::mesh::MeshSet& mesh, colorEntities.keys())
  {
    smtk::mesh::CollectionPtr c = mesh.collection();
    if (!c->hasIntegerProperty(mesh, "block_index"))
    {
      continue;
    }
    smtk::model::EntityRefArray meshEntRefs;
    mesh.modelEntities(meshEntRefs);
    if (meshEntRefs.size() == 0)
    {
      continue;
    }

    smtk::model::EntityRef entref = meshEntRefs[0];
    QSet<unsigned int> blockIds;
    if ((entref.isVolume() && colorMode == vtkModelMultiBlockSource::GetVolumeTagName()) ||
      (entref.isGroup() && colorMode == vtkModelMultiBlockSource::GetGroupTagName()) ||
      (colorMode == vtkModelMultiBlockSource::GetEntityTagName()))
    {
      const smtk::model::IntegerList& prop(c->integerProperty(mesh, "block_index"));
      if (!prop.empty() && prop[0] >= 0)
      {
        unsigned int bidx = prop[0] + 1;
        blockIds.insert(bidx);
      }
    }
    if (blockIds.size() > 0)
    {
      this->syncBlockColor(rep, QList<unsigned int>::fromSet(blockIds), colorEntities[mesh]);
    }
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::syncBlockColor(
  pqDataRepresentation* rep, const QList<unsigned int>& colorBlocks, const QColor& color)
{
  if (!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  pqMultiBlockInspectorPanel* panel = this->m_dataInspector;
  if (panel && rep)
  {
    pqOutputPort* prevOutport = pqActiveObjects::instance().activePort();
    pqDataRepresentation* prevRep = pqActiveObjects::instance().activeRepresentation();

    panel->onRepresentationChanged(rep);
    panel->onPortChanged(rep->getOutputPortFromInput());
    if (color.isValid())
    {
      panel->setBlockColor(colorBlocks, color);
      panel->setBlockOpacity(colorBlocks, color.alphaF());
    }
    else
    {
      panel->clearBlockColor(colorBlocks);
      panel->clearBlockOpacity(colorBlocks);
    }

    panel->onRepresentationChanged(prevRep);
    panel->onPortChanged(prevOutport);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::onViewAdded(pqView* view)
{
  if (view && view->getProxy()->IsA("vtkSMRenderViewProxy"))
  {
    // add a link view menu
    view->widget()->installEventFilter(this);
  }
}

//-----------------------------------------------------------------------------
bool pqModelBuilderViewContextMenuBehavior::eventFilter(QObject* caller, QEvent* e)
{
  if (!this->m_modelPanel || !this->m_modelPanel->modelManager())
  {
    return Superclass::eventFilter(caller, e);
  }

  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  vtkSMRenderViewProxy* rmp = view->getRenderViewProxy();
  if (!rmp)
    return Superclass::eventFilter(caller, e);
  QWidget* senderWidget = qobject_cast<QWidget*>(caller);

  bool ctrl = (rmp->GetInteractor()->GetControlKey() == 1);
  // Left-mouse double clicked will trigger a pick of the underneath representation, and
  // if Ctrl-key is down the same time, the picked block will be toggle its selection.
  if (e->type() == QEvent::MouseButtonDblClick)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if ((me->button() & Qt::LeftButton) && view && senderWidget) // left mouse double-clicked
    {
      QPoint newPos = me->pos();
      this->pickRepresentationBlock(view, newPos, senderWidget, false);
    }
  }
  // Right-mouse single click, if there is no selection yet, it will trigger a pick first,
  // then builds context menu with the newly selected (if any) block, otherwise
  // it will just use existing selection to build context menu.
  else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() & Qt::RightButton || (me->button() & Qt::LeftButton && ctrl))
    {
      this->m_clickPosition = me->pos();
    }
  }
  else if (e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    bool rightB = me->button() & Qt::RightButton;
    bool leftB = me->button() & Qt::LeftButton;
    if ((rightB || (leftB && ctrl)) && !this->m_clickPosition.isNull())
    {
      QPoint newPos = me->pos();
      QPoint delta = newPos - this->m_clickPosition;
      if (delta.manhattanLength() < 3 && senderWidget != NULL && view)
      {
        // For right-click, if we already have selection in representation(s) in the render view,
        // do not do picking, just use the existing selection to build the context menu.
        if ((rightB && this->m_modelPanel->modelManager()->selectedModels().count() == 0 &&
              this->m_modelPanel->modelManager()->selectedMeshes().count() == 0) ||
          (leftB && ctrl)) // pick a block from click
        {
          this->pickRepresentationBlock(view, newPos, senderWidget, rightB ? false : ctrl);
        }
        if (rightB)
        {
          this->buildMenuFromSelections();
          this->m_contextMenu->popup(senderWidget->mapToGlobal(newPos));
        }
      }
      this->m_clickPosition = QPoint();
    }
  }

  return Superclass::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::pickRepresentationBlock(
  pqRenderView* view, const QPoint& newPos, QWidget* senderWidget, bool ctrl)
{
  int pos[2] = { newPos.x(), newPos.y() };
  // we need to flip Y.
  int height = senderWidget->size().height();
  pos[1] = height - pos[1];
  unsigned int blockIndex = 0;

  pqDataRepresentation* pickedRepresentation = view->pickBlock(pos, blockIndex);
  // we want to select this block.
  if (pickedRepresentation)
  {
    emit this->representationBlockPicked(pickedRepresentation, blockIndex, ctrl);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::buildMenuFromSelections()
{
  // get currently selected <representation, block ids>
  this->m_selModelBlocks.clear();
  this->m_selMeshBlocks.clear();

  int selNumBlocks = pqCMBContextMenuHelper::getSelectedRepBlocks(
    this->m_modelPanel->modelManager()->selectedModels(),
    this->m_modelPanel->modelManager()->selectedMeshes(), this->m_selModelBlocks,
    this->m_selMeshBlocks);

  if (this->m_colormapReaction)
  {
    this->m_colormapReaction->deleteLater();
    this->m_colormapReaction = NULL;
  }

  this->m_contextMenu->clear();
  if (selNumBlocks > 0)
  {
    QMap<pqSMTKModelInfo*, QList<unsigned int> >::const_iterator modit =
      this->m_selModelBlocks.begin();
    QMap<pqSMTKMeshInfo*, QList<unsigned int> >::const_iterator meshit =
      this->m_selMeshBlocks.begin();

    if (selNumBlocks > 1)
    {
      this->m_contextMenu->addAction(QString("%1 Entities").arg(selNumBlocks));
      this->m_contextMenu->addSeparator();
    }
    /*
    else // only one block is selected
      {
      QString blockName = this->lookupBlockName(modit.value().value(0), modit.key());
      this->m_contextMenu->addAction(QString("%1").arg(blockName));
      }
*/

    // Add actions to (if no meshes are selected, and the underlying model support these operations) :
    // 1. Create group(s) with selected entities. If selections are from multiple models,
    //    each model will create their own group;
    // 2. Add selected entities to existing groups, which means the related groups will be shown as sub-menus.
    //    Currently this action will only available if the selections are from a single model.
    bool hasAnalysisMesh = this->m_selMeshBlocks.count() > 0;
    if (!hasAnalysisMesh)
    {
      bool multipleModels = this->m_selModelBlocks.count() > 1;
      QAction* newGroupAction =
        this->m_contextMenu->addAction(QString("New Group%1").arg(multipleModels ? "s" : ""));
      this->connect(newGroupAction, SIGNAL(triggered()), this, SLOT(createGroup()));
      if (!multipleModels && modit.key()->grp_annotations.size() > 2) // skip the "no group" entry
      {
        QMenu* groupMenu = this->m_contextMenu->addMenu("Add to Group")
          << pqSetName("addToGroupMenu");

        // populate this menu with available groups types menu.
        std::string str_uuid;
        std::vector<std::string>::const_iterator git;
        for (git = modit.key()->grp_annotations.begin(); git != modit.key()->grp_annotations.end();
             ++git)
        {
          if (*git != "no group")
          {
            str_uuid = *git; // UUID
            // the next in the array is group name, which will be used for action text
            QAction* gaction = groupMenu->addAction((++git)->c_str());
            gaction->setData(str_uuid.c_str());
          }
        }

        QObject::connect(groupMenu, SIGNAL(triggered(QAction*)), this, SLOT(addToGroup(QAction*)));
      }

      this->m_contextMenu->addSeparator();
    }
    /*
    if(hasAnalysisMesh)
      {
      QAction* meshaction = this->m_contextMenu->addAction("Show Analysis Mesh");
      meshaction->setCheckable(true);
      meshaction->setChecked(analysisMeshShown);
      this->connect(meshaction, SIGNAL(triggered()),
                    this, SLOT(switchModelTessellation()));

      this->m_contextMenu->addSeparator();
      }
*/
    QAction* hideBlockAction = this->m_contextMenu->addAction("Hide Selected");
    this->connect(hideBlockAction, SIGNAL(triggered()), this, SLOT(hideBlock()));

    QAction* showOnlyBlockAction = this->m_contextMenu->addAction("Hide Others");
    this->connect(showOnlyBlockAction, SIGNAL(triggered()), this, SLOT(showOnlyBlock()));

    QAction* showAllBlocksAction = this->m_contextMenu->addAction("Show Whole Model");
    this->connect(showAllBlocksAction, SIGNAL(triggered()), this, SLOT(showAllBlocks()));

    this->m_contextMenu->addSeparator();

    QAction* setBlockColorAction = this->m_contextMenu->addAction("Set Color");
    this->connect(setBlockColorAction, SIGNAL(triggered()), this, SLOT(setBlockColor()));

    QAction* unsetBlockColorAction = this->m_contextMenu->addAction("Unset Color");
    this->connect(unsetBlockColorAction, SIGNAL(triggered()), this, SLOT(unsetBlockColor()));

    QAction* action = this->m_contextMenu->addAction("Edit Color");
    this->m_colormapReaction = new pqEditColorMapReaction(action);

    this->m_contextMenu->addSeparator();

    QMenu* reprMenu = this->m_contextMenu->addMenu("Representation") << pqSetName("Representation");

    // populate the representation types menu.

    pqDataRepresentation* rep =
      hasAnalysisMesh ? meshit.key()->Representation : modit.key()->Representation;
    QList<QVariant> rTypes =
      pqSMAdaptor::getEnumerationPropertyDomain(rep->getProxy()->GetProperty("Representation"));
    QVariant curRType =
      pqSMAdaptor::getEnumerationProperty(rep->getProxy()->GetProperty("Representation"));
    QString strRepType;
    foreach (QVariant rtype, rTypes)
    {
      strRepType = rtype.toString();
      if (strRepType == "Outline" || strRepType == "3D Glyphs")
        continue;

      QAction* raction = reprMenu->addAction(strRepType);
      raction->setCheckable(true);
      raction->setChecked(rtype == curRType);
    }

    QObject::connect(reprMenu, SIGNAL(triggered(QAction*)), this, SLOT(reprTypeChanged(QAction*)));

    this->m_contextMenu->addSeparator();
  }

  // when nothing was picked we show the "Show All Models" menu.
  this->m_contextMenu->addAction("Show All Models", this, SLOT(showAllRepresentations()));
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::switchModelTessellation()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }
  bool analysisMeshShown = action->isChecked();
  foreach (pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
  {
    if (minfo && minfo->hasAnalysisMesh() && minfo->ShowMesh == analysisMeshShown)
    {
      minfo->ShowMesh = !analysisMeshShown;
      this->m_modelPanel->modelManager()->updateModelRepresentation(minfo);
    }
  }
  action->setChecked(!analysisMeshShown);
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::reprTypeChanged(QAction* action)
{
  foreach (pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
  {
    pqCMBContextMenuHelper::setRepresentationType(minfo->Representation, action->text());
  }
  foreach (pqSMTKMeshInfo* minfo, this->m_selMeshBlocks.keys())
  {
    pqCMBContextMenuHelper::setRepresentationType(minfo->Representation, action->text());
  }
}
//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::getSelectedEntitiesAndMeshes(
  QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> >& sessionBlocks)
{
  foreach (pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
  {
    foreach (unsigned int idx, this->m_selModelBlocks[minfo])
      sessionBlocks[minfo->SessionId].first.insert(minfo->Info->GetModelEntityId(idx));
  }

  foreach (pqSMTKMeshInfo* minfo, this->m_selMeshBlocks.keys())
  {
    foreach (unsigned int idx, this->m_selMeshBlocks[minfo])
      sessionBlocks[minfo->ModelInfo->SessionId].second.insert(minfo->Info->GetMeshSet(idx - 1));
  }
}

/// This is triggered from context menu, which will set off
/// a SetProperty op in smtk, then the application will
/// process the op result to set visibilities through
/// pqModelBuilderViewContextMenuBehavior::setBlockVisibility()
//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::hideBlock()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }

  QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> > blocksVis;
  this->getSelectedEntitiesAndMeshes(blocksVis);
  foreach (smtk::common::UUID sessid, blocksVis.keys())
  {
    this->m_modelPanel->setBlockVisibility(
      sessid, blocksVis[sessid].first, blocksVis[sessid].second, false);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showOnlyBlock()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }

  QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> > blocksVis;
  foreach (pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
  {
    QList<unsigned int> tmpList(this->m_selModelBlocks[minfo]);
    std::map<smtk::common::UUID, unsigned int>::const_iterator it =
      minfo->Info->GetUUID2BlockIdMap().begin();
    for (; it != minfo->Info->GetUUID2BlockIdMap().end(); ++it)
    {
      if (!tmpList.count() || !tmpList.contains(it->second))
        blocksVis[minfo->SessionId].first.insert(it->first);
    }
  }

  foreach (pqSMTKMeshInfo* minfo, this->m_selMeshBlocks.keys())
  {
    QList<unsigned int> tmpList(this->m_selMeshBlocks[minfo]);
    std::map<smtk::mesh::MeshSet, unsigned int>::const_iterator it =
      minfo->Info->GetMesh2BlockIdMap().begin();
    for (; it != minfo->Info->GetMesh2BlockIdMap().end(); ++it)
    {
      if (!tmpList.count() || !tmpList.contains(it->second + 1))
        blocksVis[minfo->ModelInfo->SessionId].second.insert(it->first);
    }
  }

  foreach (smtk::common::UUID sessid, blocksVis.keys())
  {
    this->m_modelPanel->setBlockVisibility(
      sessid, blocksVis[sessid].first, blocksVis[sessid].second, false);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllBlocks()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }
  this->showAllEntitiesAndMeshes(this->m_selModelBlocks.keys(), this->m_selMeshBlocks.keys());
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllEntitiesAndMeshes(
  const QList<pqSMTKModelInfo*>& ents, const QList<pqSMTKMeshInfo*>& meshes)
{
  smtk::model::ManagerPtr modelMan =
    this->m_modelPanel->modelManager()->managerProxy()->modelManager();
  QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> > blocksVis;
  foreach (pqSMTKModelInfo* minfo, ents)
  {
    minfo->Representation->setVisible(true);
    pqCMBContextMenuHelper::getAllEntityIds(minfo, modelMan, blocksVis[minfo->SessionId].first);
  }

  smtk::mesh::ManagerPtr meshMgr = modelMan->meshes();
  foreach (pqSMTKMeshInfo* minfo, meshes)
  {
    minfo->Representation->setVisible(true);
    pqCMBContextMenuHelper::getAllMeshSets(
      minfo, meshMgr, blocksVis[minfo->ModelInfo->SessionId].second);
  }

  foreach (smtk::common::UUID sessid, blocksVis.keys())
  {
    this->m_modelPanel->setBlockVisibility(
      sessid, blocksVis[sessid].first, blocksVis[sessid].second, true);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllRepresentations()
{
  if (!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  this->showAllEntitiesAndMeshes(this->m_modelPanel->modelManager()->allModels(),
    this->m_modelPanel->modelManager()->allMeshes());

  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  if (view)
  {
    view->resetCamera();
    view->render();
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockColor()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }
  // Trying to figure out the current color of the first selected entity or mesh
  QColor currentColor = Qt::white;
  pqCMBModelManager* modMgr = this->m_modelPanel->modelManager();
  QColor tmpcolor;
  if (m_selModelBlocks.size())
  {
    QMap<pqSMTKModelInfo*, QList<unsigned int> >::const_iterator it =
      this->m_selModelBlocks.begin();
    if (it.value().count())
    {
      smtk::common::UUID selID = it.key()->Info->GetModelEntityId(it.value()[0]);
      smtk::model::EntityRef selEnt(modMgr->managerProxy()->modelManager(), selID);
      if (selEnt.isValid() && selEnt.hasColor() &&
        pqCMBContextMenuHelper::getValidEntityColor(tmpcolor, selEnt))
      {
        currentColor = tmpcolor;
      }
    }
  }
  else if (m_selMeshBlocks.size())
  {
    QMap<pqSMTKMeshInfo*, QList<unsigned int> >::const_iterator it = this->m_selMeshBlocks.begin();
    if (it.key() && it.key()->Info->GetMesh2BlockIdMap().size() && it.value().count())
    {
      std::map<smtk::mesh::MeshSet, unsigned int>::const_iterator mit =
        it.key()->Info->GetMesh2BlockIdMap().begin();
      if (pqCMBContextMenuHelper::getValidMeshColor(tmpcolor, mit->first))
      {
        currentColor = tmpcolor;
      }
    }
  }

  QColor color = QColorDialog::getColor(currentColor, this->m_dataInspector, "Choose Block Color",
    QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
  if (color.isValid())
  {
    this->setSelectedBlocksColor(color);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::unsetBlockColor()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action || !this->m_modelPanel)
  {
    return;
  }
  this->setSelectedBlocksColor(QColor());
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setSelectedBlocksColor(const QColor& color)
{
  // map of <sessionId, < Entities, Meshes> >
  QMap<smtk::common::UUID, QPair<smtk::common::UUIDs, smtk::mesh::MeshSets> > blocksColor;
  this->getSelectedEntitiesAndMeshes(blocksColor);
  foreach (smtk::common::UUID sessid, blocksColor.keys())
  {
    this->m_modelPanel->setBlockColor(
      sessid, blocksColor[sessid].first, blocksColor[sessid].second, color);
  }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::createGroup()
{
  pqCMBModelManager* modMgr = this->m_modelPanel->modelManager();
  foreach (pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
  {
    pqCMBContextMenuHelper::startGroupOp(
      modMgr, minfo, "Create", this->m_selModelBlocks[minfo], smtk::model::Group());
  }
}

// This slot will only be available when there is only one model is selected
//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::addToGroup(QAction* action)
{
  if (this->m_selModelBlocks.count() != 1)
  {
    std::cout << "addToGroup Failed, because this operation requires one and only one model being "
                 "selected!\n";
    return;
  }

  QMap<pqSMTKModelInfo*, QList<unsigned int> >::const_iterator rbit =
    this->m_selModelBlocks.begin();
  smtk::common::UUID entId(action->data().toString().toStdString());
  pqCMBModelManager* modMgr = this->m_modelPanel->modelManager();
  pqSMTKModelInfo* minfo = rbit.key();
  pqCMBContextMenuHelper::startGroupOp(modMgr, minfo, "Modify", this->m_selModelBlocks[minfo],
    smtk::model::Group(modMgr->managerProxy()->modelManager(), entId));
}

//-----------------------------------------------------------------------------
QString pqModelBuilderViewContextMenuBehavior::lookupBlockName(
  unsigned int blockIdx, pqSMTKModelInfo* minfo) const
{
  // if there is an entity name in smtk, use that
  if (blockIdx > 0 && minfo)
  {
    smtk::common::UUID entId = minfo->Info->GetModelEntityId(blockIdx - 1);
    const smtk::model::EntityRef entity(
      this->m_modelPanel->modelManager()->managerProxy()->modelManager(), entId);
    if (entity.isValid())
    {
      return entity.name().c_str();
    }
  }

  // else fall back to multiblock data representation
  pqMultiBlockInspectorPanel* panel = this->m_dataInspector;
  if (panel)
  {
    return panel->lookupBlockName(blockIdx);
  }
  else
  {
    return QString();
  }
}
