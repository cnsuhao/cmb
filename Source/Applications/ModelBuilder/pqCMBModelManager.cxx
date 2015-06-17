//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/StringItem.h"
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
  pqPipelineSource* modelsource, pqPipelineSource* repsource, pqDataRepresentation* rep,
  const std::string& filename, smtk::model::ManagerPtr mgr)
{
  this->ModelSource = modelsource;
  this->RepSource = repsource;
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
  this->AttributeLUT.TakeReference(
    proxyManager->NewProxy("lookup_tables", "PVLookupTable"));
  vtkSMPropertyHelper(this->AttributeLUT, "IndexedLookup", true).Set(1);

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
  this->ModelSource->getProxy()->GatherInformation(this->Info);

  std::vector<int> invis_ids;

  std::map<smtk::common::UUID, unsigned int>::const_iterator it =
    this->Info->GetUUID2BlockIdMap().begin();

  for(; it != this->Info->GetUUID2BlockIdMap().end(); ++it)
    {
    smtk::model::EntityRef ent(mgr, it->first);
    int visible = 1;
    if (ent.hasIntegerProperties())
      {
      const smtk::model::IntegerData& iprops(ent.integerProperties());
      smtk::model::IntegerData::const_iterator pit;
      if (
        (pit = iprops.find("visible")) != iprops.end() &&
        pit->second.size() > 0 &&
        pit->second[0] == 0)
        visible = 0;
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
  this->ModelSource = other.ModelSource;
  this->RepSource = other.RepSource;
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

  void updateEntityGroupFieldArrayAndAnnotations(const smtk::model::EntityRef& model)
  {
    cmbSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    modelInfo->grp_annotations.clear();
    smtk::model::Groups modGroups =
      model.as<smtk::model::Model>().groups();
    // if there are no groups at all in the model, just return
    if(modGroups.size() == 0)
      return;
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      modelInfo->RepSource->getProxy());
    vtkSMPropertyHelper(smSource,"AddGroupArray").Set(0);
    smSource->UpdateVTKObjects();
    vtkSMPropertyHelper(smSource,"AddGroupArray").Set(1);
    smSource->UpdateVTKObjects();
    modelInfo->RepSource->updatePipeline();

    for(smtk::model::Groups::iterator it = modGroups.begin();
       it != modGroups.end(); ++it)
      {
      modelInfo->grp_annotations.push_back( it->entity().toString());
      modelInfo->grp_annotations.push_back( it->name());
      }
    modelInfo->grp_annotations.push_back( "no group");
    modelInfo->grp_annotations.push_back( "no group");
  }

  void updateGeometryEntityAnnotations(const smtk::model::EntityRef& model)
  {
    smtk::model::ManagerPtr mgr = this->ManagerProxy->modelManager();

    cmbSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    vtkPVSMTKModelInformation* pvinfo = modelInfo->Info;
    modelInfo->ent_annotations.clear();
    modelInfo->vol_annotations.clear();
    std::map<smtk::common::UUID, unsigned int>::const_iterator it =
      pvinfo->GetUUID2BlockIdMap().begin();
    for(; it != pvinfo->GetUUID2BlockIdMap().end(); ++it)
      {
      this->Entity2Models[it->first] = model.entity();
      modelInfo->ent_annotations.push_back( it->first.toString());
      modelInfo->ent_annotations.push_back( mgr->name(it->first));
      }

    smtk::model::CellEntities modVols =
      model.as<smtk::model::Model>().cells();
    for(smtk::model::CellEntities::iterator it = modVols.begin();
       it != modVols.end(); ++it)
      {
      if((*it).isVolume())
        {
        modelInfo->vol_annotations.push_back( it->entity().toString());
        modelInfo->vol_annotations.push_back( it->name());
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

        pqPipelineSource* repSrc = builder->createFilter(
          "ModelBridge", "SMTKModelFieldArrayFilter", modelSrc);
        smwrapper =
          vtkSMProxyProperty::SafeDownCast(
          repSrc->getProxy()->GetProperty("ModelManagerWrapper"));
        smwrapper->RemoveAllProxies();
        smwrapper->AddProxy(smProxy);
        repSrc->getProxy()->UpdateVTKObjects();
        repSrc->updatePipeline();

        pqDataRepresentation* rep = builder->createDataRepresentation(
          repSrc->getOutputPort(0), view);
        if(rep)
          {
          std::cout << "Add model: " << model.entity().toString().c_str() << "\n";

          smtk::model::ManagerPtr mgr = smProxy->modelManager();
          this->ModelInfos[model.entity()].init(modelSrc, repSrc, rep, filename, mgr);
  
          vtkSMPropertyHelper(rep->getProxy(), "PointSize").Set(8.0);
          this->updateGeometryEntityAnnotations(model);
          this->updateEntityGroupFieldArrayAndAnnotations(model);
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

      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        this->ModelInfos[*mit].RepSource);
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        this->ModelInfos[*mit].ModelSource);
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

    pqPipelineSource* modelSrc = this->ModelInfos[model.entity()].ModelSource;
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
    this->updateGeometryEntityAnnotations(model);
    this->updateEntityGroupFieldArrayAndAnnotations(model);
    this->resetColorTable(model);

    view->render();
  }

  void clear()
  {
    for(itModelInfo mit = this->ModelInfos.begin(); mit != this->ModelInfos.end(); ++mit)
      {
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        mit->second.RepSource);
      pqApplicationCore::instance()->getObjectBuilder()->destroy(
        mit->second.ModelSource);
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
    cmbSMTKModelInfo* modelInfo = &this->ModelInfos[model.entity()];
    this->resetColorTable(modelInfo, modelInfo->EntityLUT, modelInfo->ent_annotations);
    this->resetColorTable(modelInfo, modelInfo->VolumeLUT, modelInfo->vol_annotations);
    this->resetColorTable(modelInfo, modelInfo->GroupLUT, modelInfo->grp_annotations);
   }

  void resetColorTable(cmbSMTKModelInfo* modelInfo,
                       vtkSMProxy* lutProxy,
                       const std::vector<std::string> & in_annotations)
  {
    if(!modelInfo)
      return;

    pqDataRepresentation* rep = modelInfo->Representation;
    this->rebuildColorTable(in_annotations.size()/2);
    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), lutProxy,
      this->LUTColors, in_annotations);
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
    if(!mit->second.RepSource)
      {
      continue;
      }

    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      mit->second.RepSource->getProxy());
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
    if(!mit->second.RepSource)
      {
      continue;
      }
    pqPipelineSource* source = mit->second.RepSource;
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
        << vtkModelMultiBlockSource::GetVolumeTagName()
        << vtkModelMultiBlockSource::GetAttributeTagName();
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateEntityColorTable(
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
  std::vector<std::string>* annList;
  if(colorByMode == vtkModelMultiBlockSource::GetVolumeTagName())
    {
    lutProxy = modInfo->VolumeLUT;
    annList = &modInfo->vol_annotations;
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetGroupTagName())
    {
    lutProxy = modInfo->GroupLUT;
//    annList = modInfo->grp_annotations.GetPointer();
    annList = &modInfo->grp_annotations;
    }
  else if(colorByMode == vtkModelMultiBlockSource::GetEntityTagName())
    {
    lutProxy = modInfo->EntityLUT;
//    annList = modInfo->ent_annotations.GetPointer();
    annList = &modInfo->ent_annotations;
    }
  if(!lutProxy || annList->size() == 0)
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
  std::vector<std::string>::const_iterator cit;
  foreach(smtk::model::EntityRef entref, colorEntities.keys())
    {
    cit = std::find(annList->begin(), annList->end(),
                    entref.entity().toString());
    if(cit == annList->end())
      continue;
//    idx = annList->GetIndex(entref.entity().toString().c_str());
    idx = cit - annList->begin();
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
void pqCMBModelManager::colorRepresentationByEntity(
  pqDataRepresentation* rep, const QString& entityMode)
{
 // set rep colorByArray("...")
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo || modInfo->ColorMode == entityMode)
    return;

  vtkSMProxy* lutProxy = NULL;
  if(entityMode == vtkModelMultiBlockSource::GetVolumeTagName())
    lutProxy = modInfo->VolumeLUT;
  else if(entityMode == vtkModelMultiBlockSource::GetGroupTagName())
    lutProxy = modInfo->GroupLUT;
  else if(entityMode == vtkModelMultiBlockSource::GetEntityTagName())
    lutProxy = modInfo->EntityLUT;

  RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
    rep->getProxy(), entityMode == "None" ?
     NULL : entityMode.toStdString().c_str(), lutProxy,
    vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);
  modInfo->ColorMode = entityMode;
//  if(colorByMode != "None")
//    RepresentationHelperFunctions::MODELBUILDER_SYNCUP_DISPLAY_LUT(
//      colorByMode.toStdString().c_str(), lutProxy);

  rep->renderViewEventually();
}

//----------------------------------------------------------------------------
void pqCMBModelManager::updateAttributeColorTable(
  pqDataRepresentation* rep,
  vtkSMProxy* lutProxy,
  const QMap<std::string, QColor >& colorAtts,
  const std::vector<std::string>& annList)
{
  if(colorAtts.count() == 0)
    return;
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;

  if(!lutProxy || annList.size() == 0)
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
  std::vector<std::string>::const_iterator cit;
  foreach(std::string strkey, colorAtts.keys())
    {
    cit = std::find(annList.begin(), annList.end(),
                    strkey);
    if(cit == annList.end())
      continue;
    idx = cit - annList.begin();
    if(idx >= 0 && (idx%2) == 0 && (idx/2) < numColors)
      {
      idx = 3*idx/2;

      rgbColors[idx]   = colorAtts[strkey].redF();
      rgbColors[idx+1] = colorAtts[strkey].greenF();
      rgbColors[idx+2] = colorAtts[strkey].blueF();
      changed = true;
      }
    }

  if (changed )
    {
    indexedColors.Set(&rgbColors[0],
      static_cast<unsigned int>(rgbColors.size()));
    lutProxy->UpdateVTKObjects();
    }

}

//----------------------------------------------------------------------------
void pqCMBModelManager::colorRepresentationByAttribute(
    pqDataRepresentation* rep, smtk::attribute::SystemPtr attsys,
    const QString& attDefType, const QString& attItemName)
{
 // set rep colorByArray("...")
  cmbSMTKModelInfo* modInfo = this->modelInfo(rep);
  if(!modInfo)
    return;

  modInfo->ColorMode = vtkModelMultiBlockSource::GetAttributeTagName();
  vtkSMProxy* lutProxy = modInfo->AttributeLUT;

  // rebuild the lookup table with selected attribute and its item if it exists
  std::vector<smtk::attribute::AttributePtr> result;
  attsys->findAttributes(attDefType.toStdString(), result);
  if(result.size() == 0)
    {
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
      rep->getProxy(),
      NULL,
      lutProxy, vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);
    return;
    }

  std::string simContents;
  smtk::io::AttributeWriter xmlw;
  xmlw.includeDefinitions(true);
  xmlw.includeInstances(true);
  xmlw.includeModelInformation(true);
  xmlw.includeViews(false);

  smtk::io::Logger logger;
  bool errStatus = xmlw.writeContents( *attsys, simContents, logger);
  if(errStatus)
    {
    std::cerr << logger.convertToString() << std::endl;
    return;
    }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    modInfo->RepSource->getProxy());
  vtkSMPropertyHelper(smSource,"AttributeDefinitionType").Set("");
  vtkSMPropertyHelper(smSource,"AttributeItemName").Set("");
  vtkSMPropertyHelper(smSource,"AttributeSystemContents").Set("");
  vtkSMPropertyHelper(smSource,"AddGroupArray").Set(0);
  smSource->UpdateVTKObjects();

  vtkSMPropertyHelper(smSource,"AttributeDefinitionType").Set(
    attDefType.toStdString().c_str());
  if(!attItemName.isEmpty())
    vtkSMPropertyHelper(smSource,"AttributeItemName").Set(
      attItemName.toStdString().c_str());
  vtkSMPropertyHelper(smSource,"AttributeSystemContents").Set(
    simContents.c_str());

//  smSource->InvokeCommand("MarkDirty");
  smSource->UpdateVTKObjects();
  modInfo->RepSource->updatePipeline();

  QMap<std::string, QColor> attWithColors;
  smtk::common::UUIDs assignedAtts;
  std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
  std::vector<std::string> att_annotations;
  for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
    {
    smtk::common::UUIDs associatedEntities = (*itAtt)->associatedModelEntityIds();
    if(associatedEntities.size() > 0 &&
       assignedAtts.find((*itAtt)->id()) == assignedAtts.end())
      {
      assignedAtts.insert((*itAtt)->id());
      smtk::attribute::ItemPtr attitem;
      std::string keystring = (*itAtt)->name();
      std::string valstring = (*itAtt)->name();
      std::string stritemval;
      // Figure out which variant of the item to use, if it exists
      if(!attItemName.isEmpty() && (attitem = (*itAtt)->find(attItemName.toStdString())))
        {
        if(attitem->type() == smtk::attribute::Item::DOUBLE)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(attitem)->valueAsString();
          }
        else if (attitem->type() == smtk::attribute::Item::INT)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(attitem)->valueAsString();
          }    
        else if (attitem->type() == smtk::attribute::Item::STRING)
          {
          stritemval =
            smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(attitem)->value();
          }
        if(!stritemval.empty())
          {
          valstring = stritemval;
          keystring = stritemval;
          }
        }
      if(std::find(att_annotations.begin(), att_annotations.end(), keystring) ==
         att_annotations.end())
        {
        att_annotations.push_back(keystring);
        att_annotations.push_back(valstring);
        if((*itAtt)->isColorSet())
          {
          const double* attcolor = (*itAtt)->color();
          attWithColors[keystring] =
            QColor::fromRgbF(attcolor[0], attcolor[1], attcolor[2]);
          }
        }
      }
    }

  if(assignedAtts.size() > 0)
    {
    att_annotations.push_back("no attribute");
    att_annotations.push_back("no attribute");
    this->Internal->rebuildColorTable(assignedAtts.size() + 1);

    RepresentationHelperFunctions::MODELBUILDER_SETUP_CATEGORICAL_CTF(
      rep->getProxy(), lutProxy,
      this->Internal->LUTColors, att_annotations);

    this->updateAttributeColorTable(rep,
      lutProxy, attWithColors, att_annotations);
    }

  // reference vtkSMTKModelFieldArrayFilter for array name
  std::string arrayname = attDefType.toStdString();
  if(!attItemName.isEmpty())
    arrayname.append(" (").append(attItemName.toStdString()).append(")");
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_INDEXED_LUT(
    rep->getProxy(),
    (assignedAtts.size() > 0) ? arrayname.c_str() : NULL,
    lutProxy, vtkDataObject::FIELD /*, pqActiveObjects::instance().activeView()->getProxy()*/);

  vtkSMProxyProperty* barProp = vtkSMProxyProperty::SafeDownCast(
                      rep->getProxy()->GetProperty("ScalarBarActor"));
  if (barProp && barProp->GetNumberOfProxies())
    {
    vtkSMProxy* scalarBarProxy = barProp->GetProxy(0);
    if(scalarBarProxy && !arrayname.empty())
      vtkSMPropertyHelper(scalarBarProxy,"ComponentTitle").Set(
        arrayname.c_str());
    }

/*
    }
  else
    {
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
      rep->getProxy(), attDefType.toStdString().c_str(), vtkDataObject::FIELD);
    }
*/
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
  smtk::common::UUIDs groupChangedModels;
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
      }
    else if(it->isGroup())
      {
      cmbSMTKModelInfo* minfo = this->modelInfo(*it);
      if(minfo)
       groupChangedModels.insert(minfo->Info->GetModelUUID());
      }
    }

  smtk::attribute::ModelEntityItem::Ptr tessChangedEntities =
    result->findModelEntity("tess_changed");
  if(tessChangedEntities )
    {
    for(it = tessChangedEntities->begin(); it != tessChangedEntities->end(); ++it)
      {
      if(it->isModel())
        geometryChangedModels.insert(it->entity());
      else if (it->isCellEntity() && !it->isVolume() )
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        }
      }
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
        }
      else if(it->isGroup())
        {
        cmbSMTKModelInfo* minfo = this->modelInfo(*it);
        if(minfo)
         groupChangedModels.insert(minfo->Info->GetModelUUID());
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
        }
      else if (it->isCellEntity() && !it->isVolume() &&
        !it->hasIntegerProperty("block_index")) // a new entity?
        {
        geometryChangedModels.insert(
          it->as<smtk::model::CellEntity>().model().entity());
        }
      else if(it->isGroup())
        {
        cmbSMTKModelInfo* minfo = this->modelInfo(*it);
        if(minfo)
         groupChangedModels.insert(minfo->Info->GetModelUUID());
        }
      }

  bGeometryChanged = geometryChangedModels.size() > 0;

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
      else if(geometryChangedModels.find(it->entity()) != geometryChangedModels.end())
        // update representation
        {
        this->clearModelSelections();
        this->Internal->updateModelRepresentation(*it, view);
        }
      else if(groupChangedModels.find(it->entity()) != groupChangedModels.end())
        // update group LUT
        {
        this->Internal->updateEntityGroupFieldArrayAndAnnotations(*it);
        cmbSMTKModelInfo* modelInfo = this->modelInfo(*it);
        this->Internal->resetColorTable(modelInfo, modelInfo->GroupLUT, modelInfo->grp_annotations);
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
