//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqSMTKModelInfo.h"

#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqServer.h"

#include "vtkPVSMTKModelInformation.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

/// Get whether the model has an analysis mesh.
/// For different modelers, meshing could have happened on the submodels
/// of the model, and the parent model was set with the property, but
/// not the submodel itself, so we need to check the parent model (if exists)
/// for analysis mesh property.
bool pqSMTKModelInfo::hasAnalysisMesh() const
{
  return this->MeshInfos.size() > 0;
  /*
  smtk::common::UUID uid = this->Info->GetModelUUID();
  if(this->Session->manager()->hasIntegerProperty(uid, SMTK_MESH_GEN_PROP))
    {
    const smtk::model::IntegerList& gen(
      this->Session->manager()->integerProperty(uid, SMTK_MESH_GEN_PROP));
    if(!gen.empty())
      {
      return true;
      }
    }
  // look up the property in parent model(s)
  smtk::model::Model subModel(this->Session->manager(), uid);
  while(subModel.parent().isModel())
    {
    if(this->Session->manager()->hasIntegerProperty(subModel.parent().entity(), SMTK_MESH_GEN_PROP))
      {
      const smtk::model::IntegerList& gen(
        this->Session->manager()->integerProperty(subModel.parent().entity(), SMTK_MESH_GEN_PROP));
      if(!gen.empty())
        {
        return true;
        }
      }
    subModel = subModel.parent();
    }

  return false;
*/
}

void pqSMTKModelInfo::init(pqPipelineSource* modelsource, pqPipelineSource* repsource,
  pqDataRepresentation* rep, const std::string& filename, smtk::model::ManagerPtr mgr)
{
  this->ModelSource = modelsource;
  this->RepSource = repsource;
  this->FileName = filename;
  this->Representation = rep;
  this->Info = vtkSmartPointer<vtkPVSMTKModelInformation>::New();

  // create block selection source proxy
  vtkSMSessionProxyManager* proxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  this->BlockSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "BlockSelectionSource"));
  // [composite_index, process_id, cell_id]
  this->CompositeDataIdSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "CompositeDataIDSelectionSource"));

  this->EntityLUT.TakeReference(proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->EntityLUT, "IndexedLookup", true).Set(1);
  this->GroupLUT.TakeReference(proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->GroupLUT, "IndexedLookup", true).Set(1);
  this->VolumeLUT.TakeReference(proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->VolumeLUT, "IndexedLookup", true).Set(1);
  this->AttributeLUT.TakeReference(proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->AttributeLUT, "IndexedLookup", true).Set(1);

  this->ColorMode = "Entity";
  this->updateBlockInfo(mgr);
  smtk::model::Model modelEntity(mgr, this->Info->GetModelUUID());
  if (modelEntity.isValid())
    this->SessionId = modelEntity.session().entity();
  /*
  vtkSMProxy* scalarBarProxy =
    proxyManager->NewProxy("representations", "ScalarBarWidgetRepresentation");
  scalarBarProxy->SetPrototype(true);
  vtkSMPropertyHelper(scalarBarProxy,"LookupTable").Set(
    this->EntityLUT.GetPointer());
  scalarBarProxy->UpdateVTKObjects();
  QString actual_regname = "my_entity_color_legend";
  actual_regname.append(scalarBarProxy->GetXMLName());

  proxyManager->RegisterProxy("scalar_bars",
    actual_regname.toLatin1().data(), scalarBarProxy);

*/
}

void pqSMTKModelInfo::updateBlockInfo(smtk::model::ManagerPtr mgr)
{
  this->ModelSource->getProxy()->GatherInformation(this->Info);

  std::vector<int> invis_ids;

  std::map<smtk::common::UUID, vtkIdType>::const_iterator it =
    this->Info->GetUUID2BlockIdMap().begin();

  for (; it != this->Info->GetUUID2BlockIdMap().end(); ++it)
  {
    smtk::model::EntityRef ent(mgr, it->first);
    int visible = 1;
    if (ent.hasIntegerProperties())
    {
      const smtk::model::IntegerData& iprops(ent.integerProperties());
      smtk::model::IntegerData::const_iterator pit;
      if ((pit = iprops.find("visible")) != iprops.end() && pit->second.size() > 0 &&
        pit->second[0] == 0)
        visible = 0;
    }
    invis_ids.push_back(it->second); // block id
    invis_ids.push_back(visible);    // visibility

    mgr->setIntegerProperty(it->first, "block_index", it->second);
  }

  if (invis_ids.size() > 1)
  {
    // update vtk property
    vtkSMProxy* proxy = this->Representation->getProxy();
    vtkSMPropertyHelper prop(proxy, "BlockVisibility");
    prop.SetNumberOfElements(0);
    proxy->UpdateVTKObjects();
    prop.Set(&invis_ids[0], static_cast<unsigned int>(invis_ids.size()));
    proxy->UpdateVTKObjects();
  }
}

std::size_t pqSMTKModelInfo::numberOfTessellatedEntities() const
{
  return this->Info ? this->Info->GetUUID2BlockIdMap().size() : 0;
}

/// Copy constructor.
pqSMTKModelInfo::pqSMTKModelInfo(const pqSMTKModelInfo& other)
{
  this->ModelSource = other.ModelSource;
  this->RepSource = other.RepSource;
  this->FileName = other.FileName;
  this->Representation = other.Representation;
  this->Info = other.Info;
  this->SessionId = other.SessionId;
  this->BlockSelectionSource = other.BlockSelectionSource;
  this->CompositeDataIdSelectionSource = other.CompositeDataIdSelectionSource;
  this->ShowMesh = other.ShowMesh;
  this->MeshInfos.clear();
  this->MeshInfos.insert(other.MeshInfos.begin(), other.MeshInfos.end());
}
