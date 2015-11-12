//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqSMTKMeshInfo.h"

#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqServer.h"

#include "vtkPVSMTKMeshInformation.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"

#include "smtk/common/UUID.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/IntegerData.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

//----------------------------------------------------------------------------
void pqSMTKMeshInfo::init(
  pqPipelineSource* meshsource, pqPipelineSource* repsource, pqDataRepresentation* rep,
  const std::string& filename, smtk::model::ManagerPtr mgr)
{
  this->MeshSource = meshsource;
  this->RepSource = repsource;
  this->FileName = filename;
  this->Representation = rep;
  this->Info = vtkSmartPointer<vtkPVSMTKMeshInformation>::New();

  // create block selection source proxy
  vtkSMSessionProxyManager *proxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  this->BlockSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "BlockSelectionSource"));
  // [composite_index, process_id, cell_id]
  this->CompositeDataIdSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "CompositeDataIDSelectionSource"));

  this->updateBlockInfo(mgr);
}

void pqSMTKMeshInfo::updateBlockInfo(smtk::model::ManagerPtr mgr)
{
  this->MeshSource->getProxy()->GatherInformation(this->Info);
  smtk::mesh::CollectionPtr c = mgr->meshes()->collection(
    this->Info->GetMeshCollectionID());

  std::vector<int> invis_ids;

  std::map<smtk::mesh::MeshSet, unsigned int>::const_iterator it =
    this->Info->GetMesh2BlockIdMap().begin();

  for(; it != this->Info->GetMesh2BlockIdMap().end(); ++it)
    {
    int visible = 1;
    if (c->hasIntegerProperty(it->first, "visible"))
      {
      const smtk::model::IntegerList& prop(c->integerProperty(it->first, "visible"));
      if(!prop.empty())
        visible = (prop[0] != 0);
      }
    invis_ids.push_back(it->second + 1); // block id
    invis_ids.push_back(visible); // visibility
    // do we really need this? It should have been set by serialization.
    c->setIntegerProperty(it->first, "block_index", it->second);
    }

  if(invis_ids.size() > 1)
    {
    // update vtk property
    vtkSMProxy *proxy = this->Representation->getProxy();
    vtkSMPropertyHelper prop(proxy, "BlockVisibility");
    prop.SetNumberOfElements(0);
    proxy->UpdateVTKObjects();
    prop.Set(&invis_ids[0], static_cast<unsigned int>(invis_ids.size()));
    proxy->UpdateVTKObjects();
    }
}

/// Copy constructor.
pqSMTKMeshInfo::pqSMTKMeshInfo(const pqSMTKMeshInfo& other)
{
  this->MeshSource = other.MeshSource;
  this->RepSource = other.RepSource;
  this->FileName = other.FileName;
  this->Info = other.Info;
  this->Representation = other.Representation;
  this->BlockSelectionSource = other.BlockSelectionSource;
  this->CompositeDataIdSelectionSource = other.CompositeDataIdSelectionSource;
}