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

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqEditColorMapReaction.h"
#include "pqMultiBlockInspectorPanel.h"
#include "pqPVApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "pqRenderView.h"
#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqUndoStack.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqCMBModelManager.h"
#include "pqSMTKModelPanel.h"
#include "pqSMTKModelInfo.h"

#include "vtkDataObject.h"
#include "vtkNew.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVGeneralSettings.h"
#include "vtkPVSelectionInformation.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSMArrayListDomain.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkSMViewProxy.h"
#include "vtkUnsignedIntArray.h"

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPair>
#include <QWidget>

#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Volume.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include "smtk/extension/vtk/vtkModelMultiBlockSource.h"

namespace
{
static bool _internal_getBlockIndex(const smtk::model::EntityRef& eref,
                             unsigned int& flatIndex)
  {
  const smtk::model::IntegerList& prop(eref.integerProperty("block_index"));
  if(!prop.empty() && prop[0] >=0)
    {
    flatIndex = prop[0]+1;
    return true;
    }
  return false;
  }

/// Fetch children for volum and group entities.
static void _internal_AccumulateChildGeometricEntities(
  QSet<unsigned int>& blockIds,
  const smtk::model::EntityRef& toplevel)
  {
  unsigned int bidx = -1;
  if (toplevel.isVolume())
    { // Add free cells
    smtk::model::Faces faces = toplevel.as<smtk::model::Volume>().faces();
    // Find all boundaries of all free cells
    smtk::model::Faces::iterator fcit;
    for (fcit = faces.begin(); fcit != faces.end(); ++fcit)
      {
      if(fcit->hasIntegerProperty("block_index") &&
         _internal_getBlockIndex(*fcit, bidx))
        blockIds.insert(bidx);

      // for its edges and vertices
      smtk::model::EntityRefs bdys = fcit->lowerDimensionalBoundaries(-1); // Get *all* boundaries.
      smtk::model::EntityRefs::const_iterator evit;
      for (evit = bdys.begin(); evit != bdys.end(); ++evit)
        if(evit->hasIntegerProperty("block_index") &&
           _internal_getBlockIndex(*evit, bidx))
          blockIds.insert(bidx);
      }
    }
  else if (toplevel.isGroup())
    { // Add group members, but not their boundaries
    smtk::model::EntityRefs members =
      toplevel.as<smtk::model::Group>().members<smtk::model::EntityRefs>();
    for (smtk::model::EntityRefs::const_iterator it = members.begin();
       it != members.end(); ++it)
      // Do this recursively since a group may contain other groups
      _internal_AccumulateChildGeometricEntities(blockIds, *it);
    }
  else if(toplevel.hasIntegerProperty("block_index") &&
         _internal_getBlockIndex(toplevel, bidx))
    blockIds.insert(bidx);
  }

// only use valid color, the rest will be colored
// randomly with CTF
static bool _internal_getValidEntityColor(QColor& color,
  const smtk::model::EntityRef& entref)
  {
  smtk::model::FloatList rgba = entref.color();
  if ((rgba.size() == 3 || rgba.size() ==4) &&
     !(rgba[0]+rgba[1]+rgba[2] == 0))
    {
    color.setRgbF(rgba[0], rgba[1], rgba[2]);
    return true;
    }
  return false;
  }

// return total number of blocks selected
//-----------------------------------------------------------------------------
static int _internal_getSelectedRepBlocks(
  const QList<pqSMTKModelInfo*> &selModels,
  QMap<pqSMTKModelInfo*, QList<unsigned int> >& result,
  bool& hasAnalysisMesh, bool& analysisMeshShown)
  {
  int totalBlocks = 0;
  hasAnalysisMesh = false;
  analysisMeshShown = false;
  foreach(pqSMTKModelInfo* modinfo, selModels)
    {
    pqPipelineSource *source = modinfo->RepSource;
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
    vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
    selSource->UpdatePipeline();
    vtkNew<vtkPVSelectionInformation> selInfo;
    selSource->GatherInformation(selInfo.GetPointer());
    if(selInfo->GetSelection() &&
      selInfo->GetSelection()->GetNumberOfNodes())
      {
      unsigned int block_id;
      vtkUnsignedIntArray* blockIds = vtkUnsignedIntArray::SafeDownCast(
        selInfo->GetSelection()->GetNode(0)->GetSelectionList());
      if(blockIds)
        {
        for(vtkIdType ui=0;ui<blockIds->GetNumberOfTuples();ui++)
          {
          block_id = blockIds->GetValue(ui);
          result[modinfo].push_back(block_id);
          totalBlocks++;
          }
        }
      // this may be the a ID selection
      else
        {
        vtkSMPropertyHelper selIDs(selSource, "IDs");
        unsigned int count = selIDs.GetNumberOfElements();
        // [composite_index, process_id, index]
        for (unsigned int cc=0; cc < (count/3); cc++)
          {
          block_id = selIDs.GetAsInt(3*cc);
          result[modinfo].push_back(block_id);
          totalBlocks++;
          }
        }
      }
    // if any model has analysis mesh, set hasAnalysisMesh to true
    if(!hasAnalysisMesh)
      {
      hasAnalysisMesh = modinfo->hasAnalysisMesh();
      }
    if(!analysisMeshShown)
      {
      analysisMeshShown = modinfo->ShowMesh;
      }
    }
  return totalBlocks;
  }

static bool _internal_hasSessionOp(const smtk::model::SessionPtr& brSession,
  const std::string& opname)
  {
  if(brSession)
    {
    smtk::model::StringList opNames = brSession->operatorNames();
    return std::find(opNames.begin(), opNames.end(), opname) != opNames.end();
    }
  return false;
  }

static const std::string s_internal_groupOpName("entity group");

static bool _internal_startGroupOp(
 pqCMBModelManager* modMgr,
 pqSMTKModelInfo* minfo,
 const std::string& optype,
 const QList<unsigned int>& addblocks,
 const smtk::model::Group& modifyGroup)
  {
  if (_internal_hasSessionOp(minfo->Session, s_internal_groupOpName))
    {
    // create the group operator, if the session has one
    smtk::model::OperatorPtr grpOp = minfo->Session->op(s_internal_groupOpName);
    if (!grpOp)
      {
      std::cout
        << "Could not create operator: \"" << s_internal_groupOpName << "\" for session"
        << " \"" << minfo->Session->name() << "\""
        << " (" << minfo->Session->sessionId() << ")\n";
      return false;
      }
    // set up group operator with selected entities. Currently, only
    // the discrete session has the "entity group" operator
    smtk::attribute::AttributePtr attrib = grpOp->specification();
    smtk::attribute::ModelEntityItemPtr modelItem =
      attrib->findModelEntity("model");
    smtk::attribute::StringItem::Ptr optypeItem =
      attrib->findString("Operation");
    smtk::attribute::ModelEntityItemPtr grpItem =
      attrib->findAs<smtk::attribute::ModelEntityItem>(
        "modify cell group", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::ModelEntityItemPtr addItem =
      attrib->findAs<smtk::attribute::ModelEntityItem>(
        "cell to add", smtk::attribute::ALL_CHILDREN);
    smtk::attribute::IntItem::Ptr grptypeItem =
      attrib->findInt("group type");
    smtk::attribute::StringItem::Ptr grpnameItem =
      attrib->findString("group name");
    if(!modelItem || !optypeItem || !grpItem || !addItem || !grptypeItem || !grpnameItem)
      {
      std::cout << "The entity group operator's specification is missing items!\n"
                << "For reference, checkout smtk/bridge/discrete/operators/EntityGroupOperator.sbt.\n";
      return false;  
      }

    smtk::common::UUID modelId = minfo->Info->GetModelUUID();
    smtk::model::ManagerPtr mgr = modMgr->managerProxy()->modelManager();
    smtk::model::Model activeModel(mgr, modelId);
    if(!activeModel.isValid())
      {
      std::cout
        << "Could not find model with UUID: "
        << modelId << "\n";
      return false;
      }
    modelItem->setValue(activeModel);
    optypeItem->setValue(optype.c_str());

    // Due to limitations in the underlying group operation in "discrete" modeling kernel:
    // For 2D model boundary group, only edges are allowed; 2D model faces are only allowed for domain group.
    // For 3D model boundary group, only faces are allowed; domain group can only contained volumes (regions).
    // Therefore, based on the highest dimension selected, we will have above contraints to set up the group op.
    const smtk::attribute::ModelEntityItemDefinition *addItemDef =
      dynamic_cast<const smtk::attribute::ModelEntityItemDefinition*>(
      addItem->definition().get());

    int dim = activeModel.dimension();
    bool hasFace = false, hasVol = false;
    smtk::model::EntityRefArray selEntRefs;
    smtk::common::UUID uid;
    foreach(unsigned int bid, addblocks)
      {
      uid = minfo->Info->GetModelEntityId(bid-1);
      smtk::model::EntityRef entref(mgr, uid);
      if(addItemDef->isValueValid(entref))
        {
        selEntRefs.push_back(entref);
        if(!hasFace)
          {
          hasFace = entref.isFace();
          }
        if(!hasVol)
          {
          hasVol = entref.isVolume();
          }
        }
      }

    if(selEntRefs.size() > 0 &&
       !addItem->setValues(selEntRefs.begin(), selEntRefs.end()))
      {
      std::cout << "setNumberOfValues failed for \"cell to add\" item!\n";
      return false;
      }

    if( optype == "Create" )
      {
      grpnameItem->setValue("boundary group");
      // by default, group type is set to 0 (boundary) for new group
      if( (dim == 2 && hasFace) ||
          (dim == 3 && hasVol) )
        {
        grptypeItem->setValue(1); // domain
        grpnameItem->setValue("domain group");
        }
      }
    else if(optype == "Modify")
      {
      if(!modifyGroup.isValid() || addItem->numberOfValues() == 0)
        {
        std::cout << "Either the input group is not valid, or no entities is set to add to the group.\n";
        return false;
        }
      grpItem->setValue(modifyGroup);
      }

    // lanuch the group operation
    if (!modMgr->startOperation(grpOp))
      {
      std::cout
        << "Could not start operator: \"" << s_internal_groupOpName << "\" for session"
        << " \"" << minfo->Session->name() << "\""
        << " (" << minfo->Session->sessionId() << ")\n";
      return false;
      }
    return true;
    }
  return false;
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
  if(this->m_colormapReaction)
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
void pqModelBuilderViewContextMenuBehavior::syncBlockVisibility(
    pqDataRepresentation* rep,
    const QList<unsigned int>& visBlocks, bool visible, vtkIdType numBlocks)
{
  pqMultiBlockInspectorPanel *panel = this->m_dataInspector;
  if (panel && rep)
    {
    pqOutputPort* prevOutport = pqActiveObjects::instance().activePort();
    pqDataRepresentation* prevRep = pqActiveObjects::instance().activeRepresentation();

    pqOutputPort* outport = rep->getOutputPortFromInput();
    panel->onPortChanged(outport);
    panel->onRepresentationChanged(rep);

    if(visible && visBlocks.count() && rep)
      {
      // if one block is visible, the rep has to be visible
      rep->setVisible(visible);
      }

    panel->setBlockVisibility(visBlocks, visible);

   if(!visible)
      {
      if(outport)
        {
        outport->setSelectionInput(0, 0);
        }
       // if all blocks are off, the rep should be invisible 
       if(rep && outport)
        {
        vtkSMProxy *proxy = rep->getProxy();
        vtkSMProperty *blockVisibilityProperty = proxy->GetProperty("BlockVisibility");
        proxy->UpdatePropertyInformation(blockVisibilityProperty);
        vtkSMIntVectorProperty *ivp = vtkSMIntVectorProperty::SafeDownCast(blockVisibilityProperty);

        if(ivp)
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
          if(nbElems/2 != numBlocks)
            return;

          bool repVisible = false;
          for(vtkIdType i = 0; i + 1 < nbElems; i += 2)
            {
            if(ivp->GetElement(i+1))
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
void pqModelBuilderViewContextMenuBehavior::colorByEntity(
  const QString & colorMode)
{
  if(!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  // active rep
  pqDataRepresentation* activeRep = pqActiveObjects::instance().activeRepresentation();
  pqMultiBlockInspectorPanel *datapanel = this->m_dataInspector;
  if (!datapanel || !activeRep)
    return;
  pqSMTKModelInfo* minfo = this->m_modelPanel->modelManager()->modelInfo(activeRep);
  if(!minfo)
    return;
  if(minfo->ColorMode == colorMode)
    return;
  QStringList list;
  this->m_modelPanel->modelManager()->supportedColorByModes(list);
  if(!list.contains(colorMode))
    return;

  smtk::common::UUID modelId = minfo->Info->GetModelUUID();
  smtk::model::Model activeModel(
    this->m_modelPanel->modelManager()->managerProxy()->modelManager(), modelId);
  if(!activeModel.isValid())
    return;

  // turn off the current scalar bar before switch to the new array
  vtkSMProxy* ctfProxy = activeRep->getLookupTableProxy();
  vtkSMProxy* sb = vtkSMTransferFunctionProxy::FindScalarBarRepresentation(
    ctfProxy, pqActiveObjects::instance().activeView()->getProxy());
  if(sb)
    {
    vtkSMPropertyHelper(sb, "Visibility").Set(0);
    sb->UpdateVTKObjects();
    }

  // clear all colors
  QList<unsigned int> indices;
  std::map<smtk::common::UUID, unsigned int>::const_iterator uit =
    minfo->Info->GetUUID2BlockIdMap().begin();
  for(; uit != minfo->Info->GetUUID2BlockIdMap().end(); ++uit)
    {
    indices.append(uit->second + 1);
    }
  datapanel->clearBlockColor(indices);

  QColor color;
  QMap<smtk::model::EntityRef, QColor > colorEntities;
  if(colorMode ==
    vtkModelMultiBlockSource::GetVolumeTagName())
    {
    // if colorby-volume, get volumes' color,
    smtk::model::CellEntities modVols = activeModel.cells();
    for(smtk::model::CellEntities::iterator it = modVols.begin();
       it != modVols.end(); ++it)
      {
      if(it->isVolume() && it->hasColor()
        && _internal_getValidEntityColor(color, *it))
        {
        colorEntities[*it] = color;
        }
      }
    }
 else if( colorMode ==
      vtkModelMultiBlockSource::GetGroupTagName())
   {
    // if colorby-group, get groups' color,
    smtk::model::Groups modGroups = activeModel.groups();
    for(smtk::model::Groups::iterator it = modGroups.begin();
       it != modGroups.end(); ++it)
      {
      if(it->hasColor()
        && _internal_getValidEntityColor(color, *it))
        {
        colorEntities[*it] = color;
        }
      }
    }
  else if (colorMode ==
      vtkModelMultiBlockSource::GetEntityTagName())
    {
    // if colorby-entity, get entities' color, 
    for(uit = minfo->Info->GetUUID2BlockIdMap().begin();
      uit != minfo->Info->GetUUID2BlockIdMap().end(); ++uit)
      {
      smtk::model::EntityRef eref(activeModel.manager(), uit->first);
      if(eref.hasColor()
        && _internal_getValidEntityColor(color, eref))
        {
        colorEntities[eref] = color;
        }
      }
    }

  if(colorEntities.size() > 0)
    {
    this->updateColorForEntities(activeRep, colorMode, colorEntities);
    this->m_modelPanel->modelManager()->updateEntityColorTable(
      activeRep, colorEntities, colorMode);
    }
  this->m_modelPanel->modelManager()->colorRepresentationByEntity(
    activeRep, colorMode);

}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::colorByAttribute(
    smtk::attribute::SystemPtr attSys,
    const QString& attdeftype, const QString& itemname)
{
  if(!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  // active rep
  pqDataRepresentation* activeRep = pqActiveObjects::instance().activeRepresentation();
  pqMultiBlockInspectorPanel *datapanel = this->m_dataInspector;
  if (!datapanel || !activeRep)
    return;
  pqSMTKModelInfo* minfo = this->m_modelPanel->modelManager()->modelInfo(activeRep);
  if(!minfo)
    return;

  smtk::common::UUID modelId = minfo->Info->GetModelUUID();
  smtk::model::Model activeModel(
    this->m_modelPanel->modelManager()->managerProxy()->modelManager(), modelId);
  if(!activeModel.isValid())
    return;

  // turn off the current scalar bar before switch to the new array
  vtkSMProxy* ctfProxy = activeRep->getLookupTableProxy();
  vtkSMProxy* sb = vtkSMTransferFunctionProxy::FindScalarBarRepresentation(
    ctfProxy, pqActiveObjects::instance().activeView()->getProxy());
  if(sb)
    {
    vtkSMPropertyHelper(sb, "Visibility").Set(0);
    sb->UpdateVTKObjects();
    }

  // clear all colors
  QList<unsigned int> indices;
  std::map<smtk::common::UUID, unsigned int>::const_iterator uit =
    minfo->Info->GetUUID2BlockIdMap().begin();
  for(; uit != minfo->Info->GetUUID2BlockIdMap().end(); ++uit)
    {
    indices.append(uit->second + 1);
    }
  datapanel->clearBlockColor(indices);

  this->m_modelPanel->modelManager()->colorRepresentationByAttribute(
    activeRep, attSys, attdeftype, itemname);

}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::updateColorForEntities(
    pqDataRepresentation* rep, const QString& colorMode,
    const QMap<smtk::model::EntityRef, QColor >& colorEntities)
{
  if(colorMode == "None")
    return;

  foreach(const smtk::model::EntityRef& entref, colorEntities.keys())
    {
    QSet<unsigned int> blockIds;
    if((entref.isVolume() && colorMode ==
      vtkModelMultiBlockSource::GetVolumeTagName()) ||
      (entref.isGroup() && colorMode ==
      vtkModelMultiBlockSource::GetGroupTagName()) ||
      (entref.hasIntegerProperty("block_index") && colorMode ==
      vtkModelMultiBlockSource::GetEntityTagName()))
      {
      _internal_AccumulateChildGeometricEntities(blockIds, entref);
      }
    if(blockIds.size() > 0)
      this->syncBlockColor(rep,
        QList<unsigned int>::fromSet(blockIds), colorEntities[entref]);
    }
}

//----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::updateColorForMeshes(
    pqDataRepresentation* rep,
    const QString& colorMode,
    const QMap<smtk::mesh::MeshSet, QColor >& colorEntities)
{
  if(colorMode == "None")
    return;

  foreach(const smtk::mesh::MeshSet& mesh, colorEntities.keys())
    {
    smtk::mesh::CollectionPtr c = mesh.collection();
    if(!c->hasIntegerProperty(mesh, "block_index"))
      continue;
    smtk::model::EntityRefArray meshEntRefs = mesh.modelEntities();
    if(meshEntRefs.size() == 0)
      continue;

    smtk::model::EntityRef entref = meshEntRefs[0];
    QSet<unsigned int> blockIds;
    if((entref.isVolume() && colorMode ==
      vtkModelMultiBlockSource::GetVolumeTagName()) ||
      (entref.isGroup() && colorMode ==
      vtkModelMultiBlockSource::GetGroupTagName()) ||
      (colorMode ==
      vtkModelMultiBlockSource::GetEntityTagName()))
      {
      const smtk::model::IntegerList& prop(c->integerProperty(mesh, "block_index"));
      if(!prop.empty() && prop[0] >=0)
        {
        unsigned int bidx = prop[0]+1;
        blockIds.insert(bidx);
        }
      }
    if(blockIds.size() > 0)
      this->syncBlockColor(rep,
        QList<unsigned int>::fromSet(blockIds), colorEntities[mesh]);
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::syncBlockColor(
    pqDataRepresentation* rep,
    const QList<unsigned int>& colorBlocks, const QColor& color)
{
  if(!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  pqMultiBlockInspectorPanel *panel = this->m_dataInspector;
  if (panel && rep)
    {
    pqOutputPort* prevOutport = pqActiveObjects::instance().activePort();
    pqDataRepresentation* prevRep = pqActiveObjects::instance().activeRepresentation();

    panel->onRepresentationChanged(rep);
    panel->onPortChanged(rep->getOutputPortFromInput());
    if(color.isValid())
      panel->setBlockColor(colorBlocks, color);
    else
      panel->clearBlockColor(colorBlocks);

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
  if (e->type() == QEvent::MouseButtonPress)
    {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() & Qt::RightButton)
      {
      this->m_clickPosition = me->pos();
      }
    }
  else if (e->type() == QEvent::MouseButtonRelease)
    {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() & Qt::RightButton && !this->m_clickPosition.isNull())
      {
      QPoint newPos = static_cast<QMouseEvent*>(e)->pos();
      QPoint delta = newPos - this->m_clickPosition;
      QWidget* senderWidget = qobject_cast<QWidget*>(caller);
      if (delta.manhattanLength() < 3 && senderWidget != NULL)
        {
        pqRenderView* view = qobject_cast<pqRenderView*>(
          pqActiveObjects::instance().activeView());
        if (view)
          {
          // If we already have selection in representation(s) in the render view,
          // do not do picking, just use the existing selection to build the context menu.
          QList<pqSMTKModelInfo*> selModels = this->m_modelPanel->modelManager()->selectedModels();
          if(selModels.count() == 0) // pick a block from click
            {
            int pos[2] = { newPos.x(), newPos.y() } ;
            // we need to flip Y.
            int height = senderWidget->size().height();
            pos[1] = height - pos[1];
            unsigned int blockIndex = 0;
            
            pqDataRepresentation* pickedRepresentation = view->pickBlock(pos, blockIndex);

//            this->buildMenu(this->PickedRepresentation, blockIndex);

            // we want to select this block.
            if(pickedRepresentation)
              {
              emit this->representationBlockPicked(pickedRepresentation, blockIndex);
              }
            }
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
void pqModelBuilderViewContextMenuBehavior::buildMenuFromSelections()
{
  // get currently selected <representation, block ids>
  this->m_selModelBlocks.clear();
  bool hasAnalysisMesh = false;
  bool analysisMeshShown = false;
  int selNumBlocks = _internal_getSelectedRepBlocks(
    this->m_modelPanel->modelManager()->selectedModels(),
    this->m_selModelBlocks, hasAnalysisMesh, analysisMeshShown);

  if(this->m_colormapReaction)
    {
    this->m_colormapReaction->deleteLater();
    this->m_colormapReaction = NULL;
    }

  this->m_contextMenu->clear();
  if (selNumBlocks > 0 && this->m_selModelBlocks.count() > 0)
    {
    QMap<pqSMTKModelInfo*, QList<unsigned int> >::const_iterator rbit =
      this->m_selModelBlocks.begin();

    bool multipleBlocks = selNumBlocks > 1;
    bool multipleModels = this->m_selModelBlocks.count() > 1;
    if(multipleBlocks)
      {
      this->m_contextMenu->addAction(QString("%1 Entities").arg(selNumBlocks));
      }
    else // only one block is selected
      {
      QString blockName = this->lookupBlockName(rbit.value().value(0), rbit.key());
      this->m_contextMenu->addAction(QString("%1").arg(blockName));
      }
    this->m_contextMenu->addSeparator();

    // Add actions to (if the underlying model support these operations) :
    // 1. Create group(s) with selected entities. If selections are from multiple models,
    //    each model will create their own group;
    // 2. Add selected entities to existing groups, which means the related groups will be shown as sub-menus.
    //    Currently this action will only available if the selections are from a single model.
    QAction *newGroupAction =
      this->m_contextMenu->addAction(QString("New Group%1")
        .arg(multipleModels ? "s" : ""));
    this->connect(newGroupAction, SIGNAL(triggered()),
                  this, SLOT(createGroup()));
    if(!multipleModels && rbit.key()->grp_annotations.size() > 2) // skip the "no group" entry
      {
      QMenu* groupMenu = this->m_contextMenu->addMenu("Add to Group")
        << pqSetName("addToGroupMenu");

      // populate this menu with available groups types menu.
      std::string str_uuid;
      std::vector<std::string>::const_iterator git;
      for(git = rbit.key()->grp_annotations.begin();
          git != rbit.key()->grp_annotations.end(); ++git)
        {
        if(*git != "no group")
          {
          str_uuid = *git; // UUID
          // the next in the array is group name, which will be used for action text
          QAction* gaction = groupMenu->addAction((++git)->c_str());
          gaction->setData(str_uuid.c_str());
          }
        }

      QObject::connect(groupMenu, SIGNAL(triggered(QAction*)),
        this, SLOT(addToGroup(QAction*)));
      }

    this->m_contextMenu->addSeparator();

    if(hasAnalysisMesh)
      {
      QAction* meshaction = this->m_contextMenu->addAction("Show Analysis Mesh");
      meshaction->setCheckable(true);
      meshaction->setChecked(analysisMeshShown);
      this->connect(meshaction, SIGNAL(triggered()),
                    this, SLOT(switchModelTessellation()));

      this->m_contextMenu->addSeparator();
      }

    QAction *hideBlockAction =
      this->m_contextMenu->addAction("Hide Selected");
    this->connect(hideBlockAction, SIGNAL(triggered()),
                  this, SLOT(hideBlock()));

    QAction *showOnlyBlockAction =
      this->m_contextMenu->addAction("Hide Others");
    this->connect(showOnlyBlockAction, SIGNAL(triggered()),
                  this, SLOT(showOnlyBlock()));

    QAction *showAllBlocksAction =
      this->m_contextMenu->addAction("Show Whole Model");
    this->connect(showAllBlocksAction, SIGNAL(triggered()),
                  this, SLOT(showAllBlocks()));

    this->m_contextMenu->addSeparator();

    QAction *setBlockColorAction =
      this->m_contextMenu->addAction("Set Color");
    this->connect(setBlockColorAction, SIGNAL(triggered()),
                  this, SLOT(setBlockColor()));

    QAction *unsetBlockColorAction =
      this->m_contextMenu->addAction("Unset Color");
    this->connect(unsetBlockColorAction, SIGNAL(triggered()),
                  this, SLOT(unsetBlockColor()));

    QAction* action = this->m_contextMenu->addAction("Edit Color");
    this->m_colormapReaction = new pqEditColorMapReaction(action);

    this->m_contextMenu->addSeparator();

    QMenu* reprMenu = this->m_contextMenu->addMenu("Representation")
      << pqSetName("Representation");

    // populate the representation types menu.
    QList<QVariant> rTypes = pqSMAdaptor::getEnumerationPropertyDomain(
      rbit.key()->Representation->getProxy()->GetProperty("Representation"));
    QVariant curRType = pqSMAdaptor::getEnumerationProperty(
      rbit.key()->Representation->getProxy()->GetProperty("Representation"));
    foreach (QVariant rtype, rTypes)
      {
      QAction* raction = reprMenu->addAction(rtype.toString());
      raction->setCheckable(true);
      raction->setChecked(rtype == curRType);
      }

    QObject::connect(reprMenu, SIGNAL(triggered(QAction*)),
      this, SLOT(reprTypeChanged(QAction*)));

    this->m_contextMenu->addSeparator();
    }

  // when nothing was picked we show the "link camera" menu.
  this->m_contextMenu->addAction("Show All Models",
    this, SLOT(showAllRepresentations()));
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::switchModelTessellation()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  bool analysisMeshShown = action->isChecked();
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    if(minfo && minfo->hasAnalysisMesh() && minfo->ShowMesh == analysisMeshShown)
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
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    pqDataRepresentation* repr = minfo->Representation;
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
}

/// This is triggered from context menu, which will set off
/// a SetProperty op in smtk, then the application will
/// process the op result to set visibilities through
/// pqModelBuilderViewContextMenuBehavior::setBlockVisibility()
//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::hideBlock()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    if (minfo->Representation)
      {
      this->m_modelPanel->setBlockVisibility(
        minfo->Representation, this->m_selModelBlocks[minfo], false);
      }
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showOnlyBlock()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    if (minfo->Representation)
      {
      this->m_modelPanel->showOnlyBlocks(
        minfo->Representation, this->m_selModelBlocks[minfo]);
      }
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllBlocks()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    if (minfo->Representation)
      {
      this->m_modelPanel->showAllBlocks(minfo->Representation);
      }
    }
/*
  pqMultiBlockInspectorPanel *panel = this->m_dataInspector;
  if (panel)
    {
    panel->showAllBlocks();
    }
*/
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::showAllRepresentations()
{
  if(!this->m_modelPanel || !this->m_modelPanel->modelManager())
    return;

  foreach(pqDataRepresentation* repr,
          this->m_modelPanel->modelManager()->modelRepresentations())
    {
    this->m_modelPanel->showAllBlocks(repr);
    }
  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());
  if (view)
    {
    view->resetCamera();
    view->render();
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::setBlockColor()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  QColor color = QColorDialog::getColor(QColor(),
    this->m_dataInspector, "Choose Block Color",
    QColorDialog::DontUseNativeDialog);
  if(color.isValid())
    {
    foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
      {
      if (minfo->Representation)
        {
        this->m_modelPanel->setBlockColor(
          minfo->Representation, this->m_selModelBlocks[minfo], color);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::unsetBlockColor()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if(!action || !this->m_modelPanel)
    {
    return;
    }
  QColor invalidColor;
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    if (minfo->Representation)
      {
      this->m_modelPanel->setBlockColor(
        minfo->Representation, this->m_selModelBlocks[minfo], invalidColor);
      }
    }
}

//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::createGroup()
{
  pqCMBModelManager* modMgr = this->m_modelPanel->modelManager();
  foreach(pqSMTKModelInfo* minfo, this->m_selModelBlocks.keys())
    {
    _internal_startGroupOp(modMgr, minfo, "Create",
      this->m_selModelBlocks[minfo], smtk::model::Group());
    }
}

// This slot will only be available when there is only one model is selected
//-----------------------------------------------------------------------------
void pqModelBuilderViewContextMenuBehavior::addToGroup(QAction* action)
{
  if(this->m_selModelBlocks.count() != 1)
    {
    std::cout << "addToGroup Failed, because this operation requires one and only one model being selected!\n";
    return;  
    }

  QMap<pqSMTKModelInfo*, QList<unsigned int> >::const_iterator rbit =
    this->m_selModelBlocks.begin();
  smtk::common::UUID entId(action->data().toString().toStdString());
  pqCMBModelManager* modMgr = this->m_modelPanel->modelManager();
  pqSMTKModelInfo* minfo = rbit.key();
  _internal_startGroupOp(modMgr, minfo, "Modify",
    this->m_selModelBlocks[minfo],
    smtk::model::Group(modMgr->managerProxy()->modelManager(), entId));
}

//-----------------------------------------------------------------------------
QString pqModelBuilderViewContextMenuBehavior::lookupBlockName(
  unsigned int blockIdx, pqSMTKModelInfo* minfo) const
{
  // if there is an entity name in smtk, use that
  if(blockIdx > 0 && minfo)
    {
    smtk::common::UUID entId = minfo->Info->GetModelEntityId(blockIdx - 1);
    const smtk::model::EntityRef entity(this->m_modelPanel->modelManager()->
      managerProxy()->modelManager(), entId);
    if(entity.isValid())
      {
      return entity.name().c_str();
      }
    }

  // else fall back to multiblock data representation
  pqMultiBlockInspectorPanel *panel = this->m_dataInspector;
  if(panel)
    {
    return panel->lookupBlockName(blockIdx);
    }
  else
    {
    return QString();
    }
}
