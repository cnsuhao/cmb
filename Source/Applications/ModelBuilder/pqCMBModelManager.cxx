/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "pqCMBModelManager.h"

#include "vtkDataObject.h"
#include "vtkDiscreteLookupTable.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMModelManagerProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSmartPointer.h"
#include "vtkSMSourceProxy.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqRenderView.h"
#include "pqRepresentationHelperFunctions.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "vtksys/SystemTools.hxx"

#include "smtk/common/UUID.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/extension/vtk/vtkModelMultiBlockSource.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"

#include "cJSON.h"

#include <map>
#include <set>
#include <QDebug>

namespace
{

/// Fetch children for volum and group entities.
bool _internal_ContainsGroup(
  const smtk::common::UUID& grpId, const smtk::model::EntityRef& toplevel)
  {
  if(toplevel.isGroup())
    {
    if(grpId == toplevel.entity())
      return true;
    smtk::model::EntityRefs members =
      toplevel.as<smtk::model::Group>().members<smtk::model::EntityRefs>();
    for (smtk::model::EntityRefs::const_iterator it = members.begin();
       it != members.end(); ++it)
      {
      if(it->entity() == grpId)
        return true;
      // Do this recursively since a group may contain other groups
      if(_internal_ContainsGroup(grpId, *it))
        return true;
      }
    }
  return false;
  }
}

//----------------------------------------------------------------------------
void cmbSMTKModelInfo::init(
  pqPipelineSource* source, pqDataRepresentation* rep,
  const std::string& filename, smtk::model::ManagerPtr mgr)
{
  this->Source = source;
  this->FileName = filename;
  this->Representation = rep;
  this->Info = vtkSmartPointer<vtkPVSMTKModelInformation>::New();

  // create block selection source proxy
  vtkSMSessionProxyManager *proxyManager =
    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

  this->BlockSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "BlockSelectionSource"));
  // [composite_index, process_id, cell_id]
  this->CompositeDataIdSelectionSource.TakeReference(
    proxyManager->NewProxy("sources", "CompositeDataIDSelectionSource"));


  this->EntityLUT.TakeReference(
    proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->EntityLUT, "IndexedLookup", true).Set(1);
  this->GroupLUT.TakeReference(
    proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->GroupLUT, "IndexedLookup", true).Set(1);
  this->VolumeLUT.TakeReference(
    proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->VolumeLUT, "IndexedLookup", true).Set(1);
  this->ColorMode = "None";
  this->updateBlockInfo(mgr);
  smtk::model::Model modelEntity(mgr,this->Info->GetModelUUID());
  if (modelEntity.isValid())
    this->Session = modelEntity.session().session();
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
    actual_regname.toAscii().data(), scalarBarProxy);

*/
}

void cmbSMTKModelInfo::updateBlockInfo(smtk::model::ManagerPtr mgr)
{
  this->Source->getProxy()->GatherInformation(this->Info);

  std::vector<int> invis_ids;

  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    this->Info->GetUUID2BlockIdMap().begin();

  for(; it != this->Info->GetUUID2BlockIdMap().end(); ++it)
    {
    int visible = 1;

    smtk::model::UUIDWithIntegerProperties pit = mgr->integerPropertiesForEntity(it->first);
    smtk::model::PropertyNameWithIntegers pnit;
    for (pnit = pit->second.begin(); pnit != pit->second.end(); ++pnit)
      {
      if (pnit->first == "visible" && pnit->second.size() == 1 && pnit->second[0] == 0)
        {
        visible = 0;
        }
      }
    invis_ids.push_back(it->second + 1); // block id
    invis_ids.push_back(visible); // visibility

    mgr->setIntegerProperty(it->first, "block_index", it->second);
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
cmbSMTKModelInfo::cmbSMTKModelInfo(const cmbSMTKModelInfo& other)
{
  this->Source = other.Source;
  this->FileName = other.FileName;
  this->Representation = other.Representation;
  this->Info = other.Info;
  this->BlockSelectionSource = other.BlockSelectionSource;
  this->CompositeDataIdSelectionSource = other.CompositeDataIdSelectionSource;
}

//-----------------------------------------------------------------------------
class pqCMBModelManager::qInternal
{
public:

  // <ModelEnity, modelInfo>
  std::map<smtk::common::UUID, cmbSMTKModelInfo> ModelInfos;
  typedef std::map<smtk::common::UUID, cmbSMTKModelInfo >::iterator itModelInfo;
  std::map<smtk::common::UUID, smtk::common::UUID> Entity2Models;
  typedef std::map<smtk::common::UUID, cmbSMTKModelInfo >::iterator itModelEnt;

  pqServer* Server;
  vtkSmartPointer<vtkSMModelManagerProxy> ManagerProxy;
  vtkNew<vtkDiscreteLookupTable> ModelColorLUT;
  std::vector<vtkTuple<double, 3> > LUTColors;

  void updateModelAnnotations(const smtk::model::EntityRef& model)
  {
    pqDataRepresentation* rep = this->ModelInfos[model.entity()].Representation;
    smtk::model::ManagerPtr mgr = this->ManagerProxy->modelManager();

    cmbSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    vtkPVSMTKModelInformation* pvinfo = modelInfo->Info;
    modelInfo->ent_annotations->RemoveAllItems();
    modelInfo->vol_annotations->RemoveAllItems();
    modelInfo->grp_annotations->RemoveAllItems();

    std::map<smtk::common::UUID, unsigned int>::const_iterator it =
      pvinfo->GetUUID2BlockIdMap().begin();
    for(; it != pvinfo->GetUUID2BlockIdMap().end(); ++it)
      {
      this->Entity2Models[it->first] = model.entity();
      modelInfo->ent_annotations->AddString( it->first.toString().c_str() );
      modelInfo->ent_annotations->AddString( mgr->name(it->first).c_str() );
      }

    smtk::model::Groups modGroups =
      model.as<smtk::model::Model>().groups();
    for(smtk::model::Groups::iterator it = modGroups.begin();
       it != modGroups.end(); ++it)
      {
      modelInfo->grp_annotations->AddString( it->entity().toString().c_str() );
      modelInfo->grp_annotations->AddString( it->name().c_str() );
      }

    smtk::model::CellEntities modVols =
      model.as<smtk::model::Model>().cells();
    for(smtk::model::CellEntities::iterator it = modVols.begin();
       it != modVols.end(); ++it)
      {
      if((*it).isVolume())
        {
        modelInfo->vol_annotations->AddString( it->entity().toString().c_str() );
        modelInfo->vol_annotations->AddString( it->name().c_str() );
        }
      }
  }

  bool addModelRepresentation(const smtk::model::EntityRef& model,
    pqRenderView* view, vtkSMModelManagerProxy* smProxy,
    const std::string& filename)
  {
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      {
      pqApplicationCore* core = pqApplicationCore::instance();
      pqObjectBuilder* builder = core->getObjectBuilder();
      pqPipelineSource* modelSrc = builder->createSource(
          "ModelBridge", "SMTKModelSource", this->Server);
      if(modelSrc)
        {
        // ModelManagerWrapper Proxy
        vtkSMProxyProperty* smwrapper =
          vtkSMProxyProperty::SafeDownCast(
          modelSrc->getProxy()->GetProperty("ModelManagerWrapper"));
        smwrapper->RemoveAllProxies();
        smwrapper->AddProxy(smProxy);
        vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set(
          model.entity().toString().c_str());

        modelSrc->getProxy()->UpdateVTKObjects();
        modelSrc->updatePipeline();

        pqDataRepresentation* rep = builder->createDataRepresentation(
          modelSrc->getOutputPort(0), view);
        if(rep)
          {
          std::cout << "Add model: " << model.entity().toString().c_str() << "\n";

          smtk::model::ManagerPtr mgr = smProxy->modelManager();
          this->ModelInfos[model.entity()].init(modelSrc, rep, filename, mgr);
  
          vtkSMPropertyHelper(rep->getProxy(), "PointSize").Set(8.0);
          this->updateModelAnnotations(model);
          this->resetColorTable(model);
          RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
            rep->getProxy(), NULL, vtkDataObject::FIELD);

          return true;         
          }
        }
      // Should not get here.
      qCritical() << "Failed to create a pqPipelineSource or pqRepresentation for the model: "
        << model.entity().toString().c_str();
      return false;
      }
    else
      {
      qCritical() << "There is already a pqPipelineSource for the model: "
        << model.entity().toString().c_str();
      return false;
      }
  }

  void removeModelRepresentations(const smtk::common::UUIDs& modeluids,
                                 pqRenderView* view)
  {
    bool modelRemoved = false;
    for (smtk::common::UUIDs::const_iterator mit = modeluids.begin();
        mit != modeluids.end(); ++mit)
      {
      if(this->ModelInfos.find(*mit) == this->ModelInfos.end())
        {
        continue;
        }

      pqPipelineSource* modelSrc = this->ModelInfos[*mit].Source;
      pqApplicationCore::instance()->getObjectBuilder()->destroy(modelSrc);
      std::map<smtk::common::UUID, unsigned int>::const_iterator it;

      for(it = this->ModelInfos[*mit].Info->GetUUID2BlockIdMap().begin();
        it != this->ModelInfos[*mit].Info->GetUUID2BlockIdMap().end(); ++it)
        {
        this->Entity2Models.erase(it->first);
        }

      this->ModelInfos.erase(*mit);
      modelRemoved = true;
      }
  if(modelRemoved)
    view->render();
  }

  void updateModelRepresentation(const smtk::model::EntityRef& model,
    pqRenderView* view)
  {
    if(this->ModelInfos.find(model.entity()) == this->ModelInfos.end())
      {
      return;
      }

    pqPipelineSource* modelSrc = this->ModelInfos[model.entity()].Source;
    vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set("");
    modelSrc->getProxy()->UpdateVTKObjects();

    vtkSMPropertyHelper(modelSrc->getProxy(), "ModelEntityID").Set(
      model.entity().toString().c_str());

    modelSrc->getProxy()->InvokeCommand("MarkDirty");
    modelSrc->getProxy()->UpdateVTKObjects();

    modelSrc->updatePipeline();
    modelSrc->getProxy()->UpdatePropertyInformation();
    vtkSMRepresentationProxy::SafeDownCast(
        this->ModelInfos[model.entity()].Representation->getProxy())->UpdatePipeline();

//    view->forceRender();

    this->ModelInfos[model.entity()].updateBlockInfo(
      this->ManagerProxy->modelManager());
    this->updateModelAnnotations(model);
    this->resetColorTable(model);

    view->render();
  }

  void clear()
  {
    for(itModelInfo mit = this->ModelInfos.begin(); mit != this->ModelInfos.end(); ++mit)
      {
      pqApplicationCore::instance()->getObjectBuilder()->destroy(mit->second.Source);
      }
    this->Entity2Models.clear();
    this->ModelInfos.clear();
  }

  qInternal(pqServer* server): Server(server)
  {
    // this->rebuildColorTable(256);
  }

  void rebuildColorTable(vtkIdType numOfColors)
  {
    if(numOfColors < 1)
      return;

//    this->ModelColorLUT->Modified();
    this->ModelColorLUT->SetNumberOfValues(numOfColors);
    this->ModelColorLUT->Build();
    // raw interleaved array with the layout [X1, R1, G1, B1, X2, R2, G2,
    // B2, ..., Xn, Rn, Gn, Bn]
    this->LUTColors.resize(this->ModelColorLUT->GetNumberOfAvailableColors());
 //   double* rawData = this->ModelColorLUT->GetDataPointer();
    double rgba[4];
    for (vtkIdType i = 0; i < this->ModelColorLUT->GetNumberOfAvailableColors(); ++i)
      {
      this->ModelColorLUT->GetIndexedColor(i, rgba);
      this->LUTColors[i].GetData()[0] = rgba[0];
      this->LUTColors[i].GetData()[1] = rgba[1];
      this->LUTColors[i].GetData()[2] = rgba[2];
      }
  }

  void resetColorTable(const smtk::model::EntityRef& model)
  {
    pqDataRepresentation* rep = this->ModelInfos[model.entity()].Representation;
    smtk::model::ManagerPtr mgr = this->ManagerProxy->modelManager();

    cmbSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    this->resetColorTable(modelInfo);
  }

  void resetColorTable(cmbSMTKModelInfo* modelInfo)
  {
    if(!modelInfo)
      return;

    pqDataRepresentation* rep = modelInfo->Representation;

    this->rebuildColorTable(modelInfo->ent_annotations->GetLength()/2);
    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), modelInfo->EntityLUT,
      this->LUTColors, modelInfo->ent_annotations.GetPointer());
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      vtkModelMultiBlockSource::GetEntityTagName(), modelInfo->EntityLUT);

    this->rebuildColorTable(modelInfo->grp_annotations->GetLength()/2);
    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), modelInfo->GroupLUT,
      this->LUTColors, modelInfo->grp_annotations.GetPointer());
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      vtkModelMultiBlockSource::GetGroupTagName(), modelInfo->GroupLUT);

    this->rebuildColorTable(modelInfo->vol_annotations->GetLength()/2);
    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), modelInfo->VolumeLUT,
      this->LUTColors, modelInfo->vol_annotations.GetPointer());
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      vtkModelMultiBlockSource::GetVolumeTagName(), modelInfo->VolumeLUT);
  }

};

//----------------------------------------------------------------------------
pqCMBModelManager::pqCMBModelManager(pqServer* server)
{
  this->Internal = new qInternal(server);
  this->initialize();
}

//----------------------------------------------------------------------------
pqCMBModelManager::~pqCMBModelManager()
{
  this->clear();
}

//----------------------------------------------------------------------------
pqServer* pqCMBModelManager::server()
{
  return this->Internal->Server;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::initialize()
{
  if(!this->Internal->ManagerProxy)
    {
    // create block selection source proxy
    vtkSMSessionProxyManager *proxyManager =
      vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();
    vtkSMProxy* proxy = proxyManager->NewProxy("ModelBridge", "ModelManager");
    if(proxy)
      {
      this->Internal->ManagerProxy.TakeReference(
        vtkSMModelManagerProxy::SafeDownCast(proxy));
      }
    if(!this->Internal->ManagerProxy)
      {
      qCritical() << "Failed to create an Model Manager Proxy!";
      }
    else
      {
      QObject::connect(pqApplicationCore::instance()->getPluginManager(),
        SIGNAL(pluginsUpdated()),
        this, SLOT(onPluginLoaded()));
      }
    }
}

//----------------------------------------------------------------------------
vtkSMModelManagerProxy* pqCMBModelManager::managerProxy()
{
  this->initialize();
  return this->Internal->ManagerProxy;
}

//----------------------------------------------------------------------------
cmbSMTKModelInfo* pqCMBModelManager::modelInfo(const smtk::model::EntityRef& selentity)
{
  smtk::common::UUID modelId;
  if(selentity.isModel())
    {
    modelId = selentity.entity();
    }
  else if(selentity.isVolume())
    {
    modelId = selentity.as<smtk::model::Volume>().model().entity();
    }
  else if(selentity.isGroup()) // group is tricky
    {
    smtk::model::Models modelEnts = this->Internal->ManagerProxy->
      modelManager()->entitiesMatchingFlagsAs<smtk::model::Models>(
      smtk::model::MODEL_ENTITY);
    for (smtk::model::Models::iterator it = modelEnts.begin();
        it != modelEnts.end(); ++it)
      {
      if(it->isValid())
        {
        smtk::model::Groups modGroups = it->groups();
        for(smtk::model::Groups::iterator grit = modGroups.begin();
           grit != modGroups.end(); ++grit)
          {
          if(_internal_ContainsGroup(selentity.entity(), *grit))
            {
            modelId = it->entity();
            break;
            }
          }
        }
      // if found, break;
      if(modelId.isNull())
        break;
      }
    }
  else if(this->Internal->Entity2Models.find(selentity.entity())
    != this->Internal->Entity2Models.end())
    {
    modelId = this->Internal->Entity2Models[selentity.entity()];
    }

  if(!modelId.isNull() &&
    this->Internal->ModelInfos.find(modelId) !=
     this->Internal->ModelInfos.end())
    return &this->Internal->ModelInfos[modelId];

  return NULL;
}

//----------------------------------------------------------------------------
cmbSMTKModelInfo* pqCMBModelManager::modelInfo(pqDataRepresentation* rep)
{
  if(!rep)
    return NULL;

  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(mit->second.Representation == rep)
      {
      return &mit->second;
      }
    }

  return NULL;
}

//----------------------------------------------------------------------------
QList<cmbSMTKModelInfo*>  pqCMBModelManager::selectedModels()
{
  QList<cmbSMTKModelInfo*> selModels;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Source)
      {
      continue;
      }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      mit->second.Source->getProxy());
    if(smSource && smSource->GetSelectionInput(0))
      {
      selModels.append(&mit->second);
      }
    }
  return selModels;
}

//----------------------------------------------------------------------------
QList<cmbSMTKModelInfo*>  pqCMBModelManager::allModels()
{
  QList<cmbSMTKModelInfo*> selModels;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
      mit != this->Internal->ModelInfos.end(); ++mit)
    {
    selModels.append(&mit->second);
    }
  return selModels;
}


//----------------------------------------------------------------------------
void pqCMBModelManager::clearModelSelections()
{
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Source)
      {
      continue;
      }
    pqPipelineSource* source = mit->second.Source;
    pqOutputPort* outport = source->getOutputPort(0);
    if(outport)
      {
      outport->setSelectionInput(0, 0);
      }
    }
}

//----------------------------------------------------------------------------
int pqCMBModelManager::numberOfModels()
{
  return (int)this->Internal->ModelInfos.size();
}

//----------------------------------------------------------------------------
pqDataRepresentation* pqCMBModelManager::activeModelRepresentation()
{
  return pqActiveObjects::instance().activeRepresentation();
}

//----------------------------------------------------------------------------
QList<pqDataRepresentation*> pqCMBModelManager::modelRepresentations()
{
  QList<pqDataRepresentation*> result;
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(!mit->second.Representation)
      {
      continue;
      }
    result.append(mit->second.Representation);
    }
  return result;
}

//----------------------------------------------------------------------------
std::set<std::string> pqCMBModelManager::supportedFileTypes(
  const std::string& sessionName)
{
  std::set<std::string> resultSet;
  if(this->Internal->ManagerProxy)
    {
    smtk::model::StringData bftypes = this->Internal->ManagerProxy->supportedFileTypes(sessionName);
    smtk::model::PropertyNameWithStrings typeIt;
    QString filetype, descr, ext;
    for(typeIt = bftypes.begin(); typeIt != bftypes.end(); ++typeIt)
      {
      for (smtk::model::StringList::iterator tpit = typeIt->second.begin();
        tpit != typeIt->second.end(); ++tpit)
        {
        // ".cmb (Conceptual Model Builder)"
        // we want to convert the format into Paraview format for its fileOpen dialog
        // "Conceptual Model Builder (*.cmb)"
        filetype = (*tpit).c_str();
        int idx = filetype.indexOf('(');
        descr = filetype.mid(idx + 1, filetype.indexOf(')') - idx -1);
        ext = filetype.left(filetype.indexOf('('));
        resultSet.insert(
                descr.append(" (*").append(ext.simplified()).append(")").toStdString());
        }
      }
    }
  return resultSet;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::supportedColorByModes(QStringList& types)
{
  types.clear();
  types << "None"
        << vtkModelMultiBlockSource::GetEntityTagName()
        << vtkModelMultiBlockSource::GetGroupTagName()
        << vtkModelMultiBlockSource::GetVolumeTagName();
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateColorTable(
  pqDataRepresentation* rep,
  const QMap<smtk::model::EntityRef, QColor >& colorEntities,
  const QString& colorByMode)
{
  if(colorEntities.count() == 0)
    return;
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;
  vtkSMProxy* lutProxy = NULL;
  vtkStringList* annList = NULL;
  if(colorByMode == vtkModelMultiBlockSource::GetVolumeTagName())
    {
    lutProxy = modInfo->VolumeLUT;
    annList = modInfo->vol_annotations.GetPointer();
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetGroupTagName())
    {
    lutProxy = modInfo->GroupLUT;
    annList = modInfo->grp_annotations.GetPointer();
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetEntityTagName())
    {
    lutProxy = modInfo->EntityLUT;
    annList = modInfo->ent_annotations.GetPointer();
    }
  if(!lutProxy || !annList)
    return;
  // Assuming numbers in the LUT color is equal or greater than the
  // number of annotations, and the order of mapping from annotation
  // to the LUT is the same while switching colorby arrays.
  vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
  std::vector<double> rgbColors = indexedColors.GetDoubleArray();
  int numColors = (int)rgbColors.size()/3;
  if(numColors < 1)
    return;
  int idx;
  bool changed = false;
  foreach(smtk::model::EntityRef entref, colorEntities.keys())
    {
    idx = annList->GetIndex(entref.entity().toString().c_str());
    if(idx >= 0 && (idx%2) == 0 && (idx/2) < numColors)
      {
      idx = 3*idx/2;

      rgbColors[idx]   = colorEntities[entref].redF();
      rgbColors[idx+1] = colorEntities[entref].greenF();
      rgbColors[idx+2] = colorEntities[entref].blueF();
      changed = true;
      }
    }

  if (changed )
    {
    indexedColors.Set(&rgbColors[0],
      static_cast<unsigned int>(rgbColors.size()));
    lutProxy->UpdateVTKObjects();
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      colorByMode.toStdString().c_str(), lutProxy);
    }

}

//----------------------------------------------------------------------------
void pqCMBModelManager::syncDisplayColorTable(
  pqDataRepresentation* rep)
{
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;
/*
  RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
    vtkModelMultiBlockSource::GetEntityTagName(), modInfo->EntityLUT);
  RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
    vtkModelMultiBlockSource::GetGroupTagName(), modInfo->GroupLUT);
  RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
    vtkModelMultiBlockSource::GetVolumeTagName(), modInfo->VolumeLUT);
*/
}

//----------------------------------------------------------------------------
void pqCMBModelManager::colorRepresentationBy(
  pqDataRepresentation* rep, const QString& colorByMode)
{
 // set rep colorByArray("...")
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo || modInfo->ColorMode == colorByMode)
    return;

  vtkSMProxy* lutProxy = NULL;
  if(colorByMode == vtkModelMultiBlockSource::GetVolumeTagName())
    lutProxy = modInfo->VolumeLUT;
  else if(colorByMode == vtkModelMultiBlockSource::GetGroupTagName())
    lutProxy = modInfo->GroupLUT;
  else if(colorByMode == vtkModelMultiBlockSource::GetEntityTagName())
    lutProxy = modInfo->EntityLUT;

  RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
    rep->getProxy(), colorByMode == "None" ?
     NULL : colorByMode.toStdString().c_str(), lutProxy,
    vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);
  modInfo->ColorMode = colorByMode;
//  if(colorByMode != "None")
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      colorByMode.toStdString().c_str(), lutProxy);

  rep->renderViewEventually();
}

//----------------------------------------------------------------------------
smtk::model::StringData pqCMBModelManager::fileModelSessions(const std::string& filename)
{
  smtk::model::StringData retBrEns;
  if(!this->Internal->ManagerProxy)
    {
    return retBrEns;
    }

  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(filename);
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::model::StringList bnames = pxy->sessionNames();
  for (smtk::model::StringList::iterator it = bnames.begin(); it != bnames.end(); ++it)
    {
    smtk::model::StringData bftypes = pxy->supportedFileTypes(*it);
    smtk::model::PropertyNameWithStrings typeIt;
    for(typeIt = bftypes.begin(); typeIt != bftypes.end(); ++typeIt)
      {
      for (smtk::model::StringList::iterator tpit = typeIt->second.begin();
        tpit != typeIt->second.end(); ++tpit)
        {
        if (tpit->find(lastExt) == 0)
          {
          retBrEns[*it].push_back(typeIt->first);
          }
        }
      }
    }

  return retBrEns;
}

//----------------------------------------------------------------------------
smtk::model::OperatorPtr pqCMBModelManager::createFileOperator(
  const std::string& filename)
{
  this->initialize();
  if(!this->Internal->ManagerProxy)
    {
    return smtk::model::OperatorPtr();
    }

  smtk::model::StringData sessionTypes = this->fileModelSessions(filename);
  if (sessionTypes.size() == 0)
    {
    std::cerr << "Could not identify a modeling kernel to use.\n";
    return smtk::model::OperatorPtr();
    }

  smtk::model::PropertyNameWithStrings typeIt = sessionTypes.begin();
  std::string sessionType, engineType;
  sessionType = typeIt->first.c_str();
  if(typeIt->second.size() > 0)
    engineType = (*typeIt->second.begin()).c_str();

  // If there are more than one session or more than one engine on a session
  // can handle this file, we need to pick the session and/or engine.
  if((sessionTypes.size() > 1 || typeIt->second.size() > 1) &&
     !this->DetermineFileReader(filename, sessionType, engineType, sessionTypes))
    {
    return smtk::model::OperatorPtr();
    }

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  std::cout << "Should start session \"" << sessionType << "\"\n";
  smtk::common::UUID sessId = pxy->beginSession(sessionType);
  std::cout << "Started " << sessionType << " session: " << sessId << "\n";

  smtk::model::OperatorPtr fileOp = this->managerProxy()->newFileOperator(
    filename, sessionType, engineType);

  return fileOp;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startOperation(const smtk::model::OperatorPtr& brOp)
{
  this->initialize();
  if(!this->Internal->ManagerProxy || !brOp || !brOp->ableToOperate())
    {
    return false;
    }
  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::common::UUID sessId = brOp->session()->sessionId();
  std::cout << "Found session: " << sessId << "\n";

 // sessId = pxy->beginSession("cgm");

  if(!pxy->validSession(sessId))
    {
    return false;
    }

  smtk::model::OperatorResult result = brOp->operate();
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed: " << brOp->name() << "\n";
//    pxy->endSession(sessId);
    return false;
    }

  bool hasNewModels = false;
  bool bGeometryChanged = false;
  bool sucess = this->handleOperationResult(result, sessId, hasNewModels, bGeometryChanged);
  if(sucess)
    {
    smtk::model::SessionRef sref(pxy->modelManager(), sessId);
    emit this->operationFinished(result, sref, hasNewModels, bGeometryChanged);
    }

  return sucess;
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::handleOperationResult(
  const smtk::model::OperatorResult& result,
  const smtk::common::UUID& sessionId,
  bool &hasNewModels, bool& bGeometryChanged)
{
/*
  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::forOperatorResult(result, json);
  std::cout << "Result " << cJSON_Print(json) << "\n";
*/
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "operator failed\n";
    return false;
    }

  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());

  smtk::common::UUIDs geometryChangedModels;
  smtk::common::UUIDs generalModifiedModels;
  smtk::attribute::ModelEntityItem::Ptr remEntities =
    result->findModelEntity("expunged");
  smtk::model::EntityRefArray::const_iterator it;
  for(it = remEntities->begin(); it != remEntities->end(); ++it)
    {
    // if this is a block index, its pv representation needs to be updated
    if(it->hasIntegerProperty("block_index"))
      {
      geometryChangedModels.insert(this->Internal->Entity2Models[it->entity()]);
      bGeometryChanged = true;
      }
    }

  smtk::attribute::ModelEntityItem::Ptr tessChangedEntities =
    result->findModelEntity("tess_changed");
  if(tessChangedEntities )
    {
    bGeometryChanged = true;
    //in future we should only update the models that have changed, not
    //all models
    }

  // process "modified" in result to figure out if models are changed
  // or there are new cell entities
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("modified");
  for(it = resultEntities->begin(); it != resultEntities->end(); ++it)
      {
      if(it->isModel())
        generalModifiedModels.insert(it->entity()); // TODO: check what kind of operations on the model
      else if (it->isCellEntity() && !it->isVolume() &&
        !it->hasIntegerProperty("block_index")) // a new entity?
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        bGeometryChanged = true;
        }
      }

  // process "created" in result to figure out if there are new cell entities
  smtk::attribute::ModelEntityItem::Ptr newEntities =
    result->findModelEntity("created");
  if(newEntities)
    for(it = newEntities->begin(); it != newEntities->end(); ++it)
      {
      if(it->isModel())
        {
        geometryChangedModels.insert(it->entity());
//        bGeometryChanged = true;
        }
      else if (it->isCellEntity() && !it->isVolume() &&
        !it->hasIntegerProperty("block_index")) // a new entity?
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        bGeometryChanged = true;
        }
      }

  // check if there is "selection", such as from "grow" operator.
  //
  smtk::attribute::MeshSelectionItem::Ptr meshSelections =
    result->findMeshSelection("selection");

  vtkSMModelManagerProxy* pxy = this->Internal->ManagerProxy;
  smtk::common::UUIDs modelids =
    pxy->modelManager()->entitiesMatchingFlags(
    smtk::model::MODEL_ENTITY);
  smtk::common::UUIDs remmodels;
  // Clean out models that are not in the manager after operation
  for(qInternal::itModelInfo mit = this->Internal->ModelInfos.begin();
    mit != this->Internal->ModelInfos.end(); ++mit)
    {
    if(modelids.find(mit->first) == modelids.end())
      remmodels.insert(mit->first);
    }
  this->Internal->removeModelRepresentations(remmodels, view);

  smtk::model::Models modelEnts =
    pxy->modelManager()->entitiesMatchingFlagsAs<smtk::model::Models>(
    smtk::model::MODEL_ENTITY);
  bool success = true;
  hasNewModels = false;
  smtk::model::SessionRef sref(pxy->modelManager(), sessionId);
  for (smtk::model::Models::iterator it = modelEnts.begin();
      it != modelEnts.end(); ++it)
    {
    if((*it).isValid())
      {
      if(this->Internal->ModelInfos.find(it->entity()) ==
        this->Internal->ModelInfos.end())
        {
        hasNewModels = true;
        it->setSession(sref);
        success = this->Internal->addModelRepresentation(
          *it, view, this->Internal->ManagerProxy, "");
        }
      else if(bGeometryChanged ||
        geometryChangedModels.find(it->entity()) != geometryChangedModels.end())
        // update representation
        {
        this->clearModelSelections();
        this->Internal->updateModelRepresentation(*it, view);
        }
      // for grow "selection" operations, the model is writen to "modified" result
      else if(meshSelections/* && meshSelections->numberOfValues() > 0 */&&
        generalModifiedModels.find(it->entity()) != generalModifiedModels.end())
        {
        // this->Internal->updateModelSelection(*it, meshSelections, view);
        cmbSMTKModelInfo* minfo = &this->Internal->ModelInfos[it->entity()];
        if(minfo)
          emit this->requestMeshSelectionUpdate(meshSelections, minfo);
        }
      }
    }

  return success;
}

//-----------------------------------------------------------------------------
bool pqCMBModelManager::DetermineFileReader(
  const std::string& filename,
  std::string& sessionType,
  std::string& engineType,
  const smtk::model::StringData& sessionTypes)
{
  QString readerType,readerGroup;
  vtkNew<vtkStringList> list;
  smtk::model::StringData::const_iterator typeIt;
  std::string desc;
  for(typeIt = sessionTypes.begin(); typeIt != sessionTypes.end(); ++typeIt)
    {
    for (smtk::model::StringList::const_iterator tpit = typeIt->second.begin();
      tpit != typeIt->second.end(); ++tpit)
      {
      desc = typeIt->first;
      list->AddString(desc.c_str()); // session
      list->AddString((*tpit).c_str()); // engine
      engineType = (*tpit).c_str();
      sessionType = desc.c_str();
      desc += "::";
      desc += *tpit;
      list->AddString(desc.c_str()); // session::engine
      }
    }

  if(list->GetLength() > 3)
    {
    // If more than one readers found.
    pqSelectReaderDialog prompt(filename.c_str(), NULL,
      list.GetPointer(), pqCoreUtilities::mainWidget());
    if (prompt.exec() == QDialog::Accepted)
      {
      engineType = prompt.getReader().toStdString();
      sessionType = prompt.getGroup().toStdString();
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::clear()
{
  this->Internal->clear();
  if(this->Internal->ManagerProxy)
    this->Internal->ManagerProxy->endSessions();
  this->Internal->ManagerProxy = NULL;
  emit currentModelCleared();
}

//----------------------------------------------------------------------------
bool pqCMBModelManager::startNewSession(const std::string& sessionName)
{
  smtk::common::UUID sessionId =
    this->Internal->ManagerProxy->beginSession(sessionName, true);
  smtk::model::SessionRef sref(
    this->managerProxy()->modelManager(), sessionId);

  if (!sref.session())
    {
    std::cerr << "Could not start new session of type \"" << sessionName << "\"\n";
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelManager::onPluginLoaded()
{
  // force remote server to refetch sessions incase a new session is loaded
  if(this->Internal->ManagerProxy)
    {
    QStringList newFileTypes;
    QStringList newSessionNames;
    smtk::model::StringList oldBnames = this->Internal->ManagerProxy->sessionNames();
    smtk::model::StringList newBnames = this->Internal->ManagerProxy->sessionNames(true);

    for (smtk::model::StringList::iterator it = newBnames.begin(); it != newBnames.end(); ++it)
      {
      // if there is the new session
      if(std::find(oldBnames.begin(), oldBnames.end(), *it) == oldBnames.end())
        {
        newSessionNames << (*it).c_str();
        std::set<std::string> bftypes = this->supportedFileTypes(*it);
        for (std::set<std::string>::iterator tpit = bftypes.begin(); tpit != bftypes.end(); ++tpit)
          {
          newFileTypes << (*tpit).c_str();
          }
        }
      }
    if(newSessionNames.count() > 0)
      {
      emit newSessionLoaded(newSessionNames);
      }
    if(newFileTypes.count() > 0)
      {
      emit newFileTypesAdded(newFileTypes);
      }
    }
}
