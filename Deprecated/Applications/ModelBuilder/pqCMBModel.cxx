//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBModel.h"

// cmb headers
#include "pqCMBModelFace.h"
#include "pqCMBFloatingEdge.h"
#include "pqCMBModelEdge.h"
#include "pqCMBModelVertex.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelMaterial.h"
#include "vtkCMBModelReadOperatorClient.h"
#include "vtkCMBModelBuilder2DClient.h"
#include "vtkCMBModelBuilderClient.h"
#include "vtkCMBMapToCMBModelClient.h"
#include "vtkCMBModelOmicronMeshInputWriterClient.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkCMBParserBase.h"
#include "vtkModelUserName.h"
#include "vtkMaterialOperatorClient.h"
#include "vtkModelEntityGroupOperatorClient.h"
#include "vtkPVModelGeometryInformation.h"
#include "vtkSelectionSplitOperatorClient.h"
#include "vtkGeoTransformOperatorClient.h"

#include "vtkDiscreteLookupTable.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkModelEntityOperatorClient.h"
#include "vtkModelItemIterator.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMSession.h"
#include "vtkMergeOperatorClient.h"
#include "vtkSplitOperatorClient.h"
#include "vtkCreateModelEdgesOperatorClient.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkCMBModelStateOperatorClient.h"
#include "vtkCMBModelWriterClient.h"
#include "vtkClientServerStream.h"
#include "vtkPVDataInformation.h"
#include "vtkUnsignedIntArray.h"
#include "vtkSelection.h"
#include "vtkStringArray.h"
#include "vtkAlgorithm.h"
#include "vtkImageData.h"

// Paraview headers
#include "pqApplicationCore.h"
#include "pqFileDialog.h"
#include "pqFileDialogModel.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqSMAdaptor.h"
#include "pqWaitCursor.h"
#include "pqActiveObjects.h"
#include "pqServerManagerModel.h"

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

#include "vtkCamera.h"
#include "vtkIdTypeArray.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMInputProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkProcessModule.h"
#include "vtkPVSelectionInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkBoundingBox.h"
#include "vtkTransform.h"

#include "vtkCMBMeshClient.h"
#include "vtkCMBModelEntityMesh.h"
#include "vtkCollection.h"
#include "vtkNew.h"
#include "vtkModelItemListIterator.h"
// Qt
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include "qtCMBApplicationOptions.h"
#include "pqCMBImportShapefile.h"
// smtk
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/model/Item.h"
#include "smtk/model/Model.h"

//-----------------------------------------------------------------------------
pqCMBModel::pqCMBModel(pqRenderView* renderView, pqServer* activeServer)
{
  this->RenderView = renderView;
  this->ActiveServer = activeServer;
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBModel::init()
{
  this->Model = NULL;
  this->ModelWrapper = NULL;
  this->CurrentGrowSource = NULL;
  this->MasterPolyProvider = NULL;
  this->GrowModelFaces = NULL;
  this->CurrentGrowSelectionSource = NULL;
  this->CurrentGrowRep = NULL;
  this->BathyMetryOperatorProxy = NULL;

  this->ModelSource = NULL;
  this->ModelRepresentation = NULL;
  this->ModelSelectionSource = NULL;

  this->CurrentModelFileName.clear();
  this->createLookupTable();

  this->VTKConnect =
    vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->VTKModelConnect =
    vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->StateOperator =
    vtkSmartPointer<vtkCMBModelStateOperatorClient>::New();
  this->LatLongTransformOperator =
    vtkSmartPointer<vtkGeoTransformOperatorClient>::New();
  this->FaceColorMode = this->EdgeColorMode = this->EdgeDomainColorMode = 0;
  this->MeshClient = NULL;
  this->ShowEdgePoints = 0;

  this->CurrentFaceAttDefType = "";
  this->CurrentEdgeAttDefType = "";
  this->CurrentDomainAttDefType = "";
  this->AttManager = smtk::attribute::ManagerPtr();

  this->TextureOperatorProxy = NULL;
}

//-----------------------------------------------------------------------------
pqCMBModel::~pqCMBModel()
{
  this->clearClientModel(false);

  if(this->ModelSource)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(
      this->ModelSource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->RemoveAllProxies();
    pqApplicationCore::instance()->getObjectBuilder()->destroy(this->ModelSource);
    this->ModelSource = NULL;
    }

  if(this->ModelWrapper)
    {
    //pqApplicationCore* core = pqApplicationCore::instance();
    //pqServerManagerModel* sm = core->getServerManagerModel();
    //if(!sm->findServer(this->ModelWrapper->GetConnectionID()))
    //  {
    //  vtkSMProxyManager::GetProxyManager()->UnRegisterProxy(this->ModelWrapper);
    //  }
    this->ModelWrapper->Delete();
    this->ModelWrapper = 0;
    }

  if(this->Model)
    {
    this->Model->Delete();
    this->Model = NULL;
    }
}
//-----------------------------------------------------------------------------
vtkDiscreteModel* pqCMBModel::getModel()
{
  if(!this->Model)
    {
    this->Model = vtkDiscreteModel::New();
    //this->VTKModelConnect->Connect(this->Model,
    //  vtkCommand::ModifiedEvent, this, SLOT(onModelModified()));
    }
  return this->Model;
}
//-----------------------------------------------------------------------------
void pqCMBModel::getSelectedModelEntities(QList<vtkIdType>& selEntities)
{
// ParaView is now NOT using representation::ConvertSelection() framework
// to output selection of individual reps. For details, look at the
// vtkGeometryRepresentation's selection painter set up!!!
/*
  vtkSMIdTypeVectorProperty* selModelEntities =
  vtkSMIdTypeVectorProperty::SafeDownCast(
    this->ModelRepresentation->getProxy()->GetProperty("LastSelectedEntityIds"));
  this->ModelRepresentation->getProxy()->UpdateVTKObjects();
  this->ModelRepresentation->getProxy()->UpdatePropertyInformation();
*/
  if(!this->ModelRepresentation)
    {
    return;
    }
  pqOutputPort* modelPort = this->ModelRepresentation->getOutputPortFromInput();
  pqPipelineSource *source = modelPort? modelPort->getSource() : NULL;
  if(!source)
    {
    return;
    }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  if(!smSource || !smSource->GetSelectionInput(0))
    {
    return;
    }

  selEntities.clear();

  vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
  selSource->UpdatePipeline();
  vtkNew<vtkPVSelectionInformation> selInfo;
  selSource->GatherInformation(selInfo.GetPointer());

  if(selInfo->GetSelection() &&
    selInfo->GetSelection()->GetNumberOfNodes())
    {
    vtkIdType entId;
    unsigned int flat_idx;
    vtkUnsignedIntArray* blockIds = vtkUnsignedIntArray::SafeDownCast(
      selInfo->GetSelection()->GetNode(0)->GetSelectionList());
    if(blockIds)
      {
      for(vtkIdType ui=0;ui<blockIds->GetNumberOfTuples();ui++)
        {
        flat_idx = blockIds->GetValue(ui);
        entId = this->ModelDataInfo->GetModelEntityId(flat_idx);
        if(entId >= 0 && !selEntities.contains(entId))
          {
          selEntities.append(entId);
          }
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
        flat_idx = selIDs.GetAsInt(3*cc);
        entId = this->ModelDataInfo->GetModelEntityId(flat_idx);
        if(entId >= 0 && !selEntities.contains(entId))
          {
          selEntities.append(entId);
          }
        }
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBModel::updateModelSource()
{
  if(this->ModelSource)
    {
    vtkSMSourceProxy::SafeDownCast(
      this->ModelSource->getProxy() )->MarkModified(NULL);
    vtkSMSourceProxy::SafeDownCast(
      this->ModelSource->getProxy() )->UpdatePipeline();
    }
}
//-----------------------------------------------------------------------------
vtkSMProxy* pqCMBModel::getServerMeshProxy()
{
  if(this->MeshClient)
    {
    return this->MeshClient->GetServerMeshProxy();
    }
  return NULL;
}
//-----------------------------------------------------------------------------
vtkSMProxy* pqCMBModel::getModelWrapper()
{
  if(!this->ModelWrapper)
    {
    this->initModel();
    }
  return this->ModelWrapper;
}

//-----------------------------------------------------------------------------
int pqCMBModel::getModelDimension()
{
  return this->getModel()->GetModelDimension();
}

//-----------------------------------------------------------------------------
bool pqCMBModel::has2DEdges()
{
  return this->EdgeIDToEdgeMap.count()>0;
}

//-----------------------------------------------------------------------------
bool pqCMBModel::getModelBounds(double bounds[6])
{
  if(this->getModel())
    {
    this->getModel()->GetBounds( bounds );
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBModel::hasGeometryEntity()
{
  return ((this->getModelDimension() == 3 && this->FaceIDToFaceMap.count()>0) ||
      (this->EdgeIDToEdgeMap.count()>0)) ? true : false;
}

//-----------------------------------------------------------------------------
void pqCMBModel::clearClientModel(bool emitSignal)
{
  if(this->VTKConnect)
    {
    this->VTKConnect->Disconnect();
    }

  pqActiveObjects::instance().setActiveSource(NULL);
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->clearEntityMap(this->FaceIDToFaceMap);
  this->clearEntityMap(this->NGIDToNGRepMap);
  this->clearEntityMap(this->EdgeID2RepMap);
  this->clearEntityMap(this->EdgeIDToEdgeMap);
  this->clearEntityMap(this->VertexIDToVertexMap);

  if(this->CurrentGrowSource)
    {
    builder->destroy(this->CurrentGrowSource);
    this->CurrentGrowSource = 0;
    }
  if(this->CurrentGrowSelectionSource)
    {
    builder->destroy(this->CurrentGrowSelectionSource);
    this->CurrentGrowSelectionSource = 0;
    }

  if(this->GrowModelFaces)
    {
    this->GrowModelFaces->Delete();
    this->GrowModelFaces = 0;
    }
  if(this->BathyMetryOperatorProxy)
    {
    this->BathyMetryOperatorProxy->Delete();
    this->BathyMetryOperatorProxy = 0;
    }
  if (this->ModelBathymetryFilter)
    {
    builder->destroy(this->ModelBathymetryFilter);
    this->ModelBathymetryFilter = NULL;
    }

  if(this->TextureOperatorProxy)
    {
    this->TextureOperatorProxy->Delete();
    this->TextureOperatorProxy = 0;
    }
  if (this->TextureImageSource)
    {
    builder->destroy( this->TextureImageSource );
    this->TextureImageSource = 0;
    }
  if (this->RegisterTextureFilter)
    {
    builder->destroy(this->RegisterTextureFilter);
    this->RegisterTextureFilter = NULL;
    }

  this->BathymetrySource = 0;

  if(this->MasterPolyProvider)
    {
    builder->destroy(this->MasterPolyProvider);
    this->MasterPolyProvider = 0;
    }

  if(this->VTKModelConnect)
    {
    this->VTKModelConnect->Disconnect();
    }
  if(this->Model)
    {
    this->Model->Reset();
    //this->Model->Delete();
    //this->Model = NULL;
    }
  if(this->ModelSelectionSource)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(
      this->ModelSelectionSource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->RemoveAllProxies();
    builder->destroy(this->ModelSelectionSource);
    this->ModelSelectionSource = NULL;
    }

  if(this->ModelRepresentation)
    {
    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(
      this->ModelSource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->RemoveAllProxies();
    builder->destroy(this->ModelRepresentation);
    this->ModelRepresentation = NULL;
    }

  this->CurrentModelFileName.clear();

  if(this->RenderView)
    {
    this->RenderView->render();
    }
  this->CurrentFaceAttDefType = "";
  this->CurrentEdgeAttDefType = "";
  this->CurrentDomainAttDefType = "";
  this->AttManager = smtk::attribute::ManagerPtr();
  if(emitSignal)
    {
    emit this->currentModelCleared();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::clearEntityMap(ModelEntityMap& entitymap)
{
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = entitymap.begin();
    mapIter != entitymap.end(); ++mapIter)
    {
    entity = mapIter.value();
    if(entity)
      {
      delete entity;
      }
    }
  entitymap.clear();
}

//-----------------------------------------------------------------------------
int pqCMBModel::showModel()
{
  vtkDiscreteModel* cmbModel = this->getModel();
  if(!cmbModel)
    {
    return 0;
    }

  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();

  // If there is no Region association, that means we have pure 2D model
  // Other wise we have a pure 3D model or mixed 2d and 3d model.
  // In either case, since we now also support edges in 3D model, we will go
  // through model faces and edges. At the same time, we also need
  // model vertex in 3D model if there are edges (except floating edges)

  // Model faces
  vtkNew<vtkModelItemListIterator> fiter;
  fiter->SetRoot(cmbModel);
  fiter->SetItemType(vtkModelFaceType);
  for(fiter->Begin();!fiter->IsAtEnd();fiter->Next())
    {
    vtkDiscreteModelFace* fEntity =
      vtkDiscreteModelFace::SafeDownCast(fiter->GetCurrentItem());
    if(fEntity)
      {
      pqCMBModelFace* eFace = this->createNewFace(fEntity->GetUniquePersistentId());
      }
    }

  // Model Edges (including floating edges in 3D)
  vtkNew<vtkModelItemListIterator> eiter;
  eiter->SetRoot(cmbModel);
  eiter->SetItemType(vtkModelEdgeType);
  for(eiter->Begin();!eiter->IsAtEnd();eiter->Next())
    {
    vtkDiscreteModelEdge* feEntity =
      vtkDiscreteModelEdge::SafeDownCast(eiter->GetCurrentItem());
    if(feEntity)
      {
      if(feEntity->GetNumberOfAssociations(vtkModelRegionType))
        {
        if(!this->EdgeID2RepMap.contains(feEntity->GetUniquePersistentId()))
          {
          pqCMBFloatingEdge* cmbFE = pqCMBFloatingEdge::createObject(
            this->ModelWrapper, feEntity, this->ActiveServer, this->RenderView, false);
          if(cmbFE)
            {
            this->EdgeID2RepMap.insert(feEntity->GetUniquePersistentId(), cmbFE);
            }
          }
        }
      else
        {
        pqCMBModelEdge* eEdge = this->createNewEdge(feEntity->GetUniquePersistentId());
        }
      }
    }

  // Model vertexes
  vtkNew<vtkModelItemListIterator> viter;
  viter->SetRoot(cmbModel);
  viter->SetItemType(vtkModelVertexType);
  for(viter->Begin();!viter->IsAtEnd();viter->Next())
    {
    vtkDiscreteModelVertex* vtxEntity =
      vtkDiscreteModelVertex::SafeDownCast(viter->GetCurrentItem());
    if(vtxEntity)
      {
      this->createNewVertex(vtxEntity->GetUniquePersistentId());
      }
    }

  this->ModelRepresentation =
          builder->createDataRepresentation(this->ModelSource->getOutputPort(0),
          this->RenderView, "PVModelRepresentation");
  if(this->ModelRepresentation)
    {
    //pqSMAdaptor::setEnumerationProperty(
    //  this->ModelRepresentation->getProxy()->GetProperty("Representation"), "Model");
    pqSMAdaptor::setElementProperty(this->ModelRepresentation->getProxy()->
      GetProperty("SuppressLOD"),  0);
    vtkSMProxyProperty* proxyModel =
      vtkSMProxyProperty::SafeDownCast(this->ModelRepresentation->getProxy()
      ->GetProperty("ModelWrapper"));
    proxyModel->RemoveAllProxies();
    proxyModel->AddProxy(this->ModelWrapper);

    pqSMAdaptor::setEnumerationProperty(
      qobject_cast<pqPipelineRepresentation*>(this->ModelRepresentation)->
      getRepresentationProxy()->GetProperty("Representation"),
      qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

    pqSMAdaptor::setElementProperty(this->ModelRepresentation->getProxy()->
      GetProperty("SelectionOpacity"), 0.99);

    this->ModelRepresentation->getProxy()->UpdateVTKObjects();
    this->setupRepresentationVTKConnection(this->ModelRepresentation);
    }

  this->ModelSelectionSource = builder->createSource("CMBModelGroup",
    "CmbModelSelectionSource", this->ActiveServer);
  vtkSMDataSourceProxy* newSelSource = vtkSMDataSourceProxy::SafeDownCast(
    this->ModelSelectionSource->getProxy());
  vtkSMProxyProperty* proxyproperty =
    vtkSMProxyProperty::SafeDownCast(newSelSource->GetProperty("ModelWrapper"));
  proxyproperty->RemoveAllProxies();
  proxyproperty->AddProxy(this->ModelWrapper);
  newSelSource->UpdateVTKObjects();
//  this->onLookupTableModified();

  this->RenderView->resetCamera();
  this->RenderView->render();

  return 1;
}

//----------------------------------------------------------------------------
int pqCMBModel::canLoadFile(const QString& filename)
{
  QFileInfo finfo(filename);
  pqFileDialogModel fModel(this->ActiveServer);
  QString fullpath;
  if(!fModel.fileExists(filename, fullpath))
    {
    return 0;
    }

  if(!(this->CurrentModelFileName.isNull()) &&
      filename.compare(this->CurrentModelFileName, Qt::CaseInsensitive) == 0)
    {
    return 0;
    }
  else
    {
    this->clearClientModel();
    }

  return 1;
}

//----------------------------------------------------------------------------
int pqCMBModel::initModel()
{
  if(!this->ModelWrapper)
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    this->ModelWrapper = pxm->NewProxy("CMBModelGroup", "CMBModelWrapper");
    if(!this->ModelWrapper)
      {
      return 0;
      }

    if(!this->ModelSource)
      {
      this->ModelSource = pqApplicationCore::instance()->getObjectBuilder()->
        createSource("sources", "CMBModelSource", this->ActiveServer);
      }

    vtkSMProxyProperty* proxyproperty =
      vtkSMProxyProperty::SafeDownCast(
      this->ModelSource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->RemoveAllProxies();
    proxyproperty->AddProxy(this->ModelWrapper);
    this->ModelSource->getProxy()->UpdateVTKObjects();
    }

  if(!this->Model)
    {
    this->getModel();
    }
  this->CurrentFaceAttDefType = "";
  this->CurrentEdgeAttDefType = "";
  this->CurrentDomainAttDefType = "";
  this->AttManager = smtk::attribute::ManagerPtr();

  return 1;
}

//----------------------------------------------------------------------------
int pqCMBModel::loadModelFile(const QString& filename)
{
  QFileInfo fInfo(filename);
  pqFileDialogModel fModel(this->ActiveServer);
  QString fullpath;
  if(!fModel.fileExists(filename, fullpath))
    {
    //    qCritical() << "File does not exist: " << filename;
    return 0;
    }

  if(!this->initModel())
    {
    return 0;
    }
  pqWaitCursor cursor;


  bool readSuccess = false;
  if (fInfo.suffix().toLower() == "cmb")
    {
    vtkSmartPointer<vtkCMBModelReadOperatorClient> ReaderOperator =
      vtkSmartPointer<vtkCMBModelReadOperatorClient>::New();
    ReaderOperator->SetFileName(filename.toStdString().c_str());
    readSuccess = ReaderOperator->Operate(this->Model, this->ModelWrapper);
    }
  //else if(fInfo.suffix().toLower() == "3dm")
  //  {
  //  vtkSmartPointer<vtkCMB3dmReadOperatorClient> ReaderOperator =
  //    vtkSmartPointer<vtkCMB3dmReadOperatorClient>::New();
  //  ReaderOperator->SetFileName(filename.toStdString().c_str());
  //  readSuccess = ReaderOperator->Operate(this->Model, this->ModelWrapper);
  //  }

  if(readSuccess)
    {
    if(this->showModel())
      {
      this->CurrentModelFileName = filename;
      this->setupClientModel();
      this->saveModelState();

      return 1;
      }
    else
      {
      this->clearClientModel();
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void pqCMBModel::setupClientModel()
{
  pqWaitCursor cursor;

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if(this->hasGeometryEntity())
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    if(this->getMasterPolyProvider())
      {
      vtkSMProxyProperty* proxyproperty =
        vtkSMProxyProperty::SafeDownCast(
            this->MasterPolyProvider->getProxy()->GetProperty("ModelWrapper"));
      proxyproperty->AddProxy(this->ModelWrapper);
      pqSMAdaptor::setElementProperty(this->MasterPolyProvider->getProxy()->
          GetProperty("ItemType"),  vtkModelType);
      this->MasterPolyProvider->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast(
          this->MasterPolyProvider->getProxy() )->UpdatePipeline();

      // set up Grow Source, Filters, and Rep
      if (this->getModelDimension()==3)
        {
        this->CurrentGrowSource = builder->createSource("sources",
            "HydroModelPolySource", this->ActiveServer);
        vtkSMDataSourceProxy::SafeDownCast(
            this->CurrentGrowSource->getProxy())->CopyData(
            vtkSMSourceProxy::SafeDownCast(this->MasterPolyProvider->getProxy()) );
        vtkSMSourceProxy::SafeDownCast(
            this->CurrentGrowSource->getProxy() )->MarkModified(NULL);
        vtkSMSourceProxy::SafeDownCast(
            this->CurrentGrowSource->getProxy() )->UpdatePipeline();

        this->GrowModelFaces = vtkSMSourceProxy::SafeDownCast(
            pxm->NewProxy("CMBModelGroup", "SeedGrow"));

        this->CurrentGrowSelectionSource = builder->createSource("sources",
            "HydroModelSelectionSource", this->ActiveServer);
        this->CurrentGrowRep =
            builder->createDataRepresentation(
              this->CurrentGrowSource->getOutputPort(0),
              this->RenderView, "GeometryRepresentation");
        pqSMAdaptor::setElementProperty(
            this->CurrentGrowRep->getProxy()->
            GetProperty("Opacity"), 0.0);
        pqSMAdaptor::setElementProperty(
            this->CurrentGrowRep->getProxy()->
            GetProperty("Representation"), "Outline");// Outline
        pqSMAdaptor::setElementProperty(
            this->CurrentGrowRep->getProxy()->
            GetProperty("SelectionRepresentation"), 1);// Wireframes
        pqSMAdaptor::setElementProperty(
            this->CurrentGrowRep->getProxy()->
            GetProperty("SelectionOpacity"), 1.0);

        this->clearGrowResult();
        }
      this->updateModelDataInfo();
      }
    }

  emit this->currentModelLoaded();
}

//----------------------------------------------------------------------------
void pqCMBModel::saveModelState()
{
  this->StateOperator->SaveModelState(this->Model, this->ModelWrapper);
}

//----------------------------------------------------------------------------
void pqCMBModel::reloadSavedModelState()
{
  // clear model?
  this->clearClientModel();

  if(this->StateOperator->LoadModelState(this->Model, this->ModelWrapper))
    {
    if(this->showModel())
      {
      this->setupClientModel();
      }
    }
}

//----------------------------------------------------------------------------
void pqCMBModel::updateModelEntityRepresentation(vtkIdType inFaceId)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();

  if(entityMap.contains(inFaceId))
    {
    entityMap[inFaceId]->updateRepresentation(this->ModelWrapper);
    }
  else if(this->getModelDimension() == 3 && this->has2DEdges() &&
    this->EdgeIDToEdgeMap.contains(inFaceId))
    {
    this->EdgeIDToEdgeMap[inFaceId]->updateRepresentation(this->ModelWrapper);
    }
}

//----------------------------------------------------------------------------
void pqCMBModel::getVisibleModelEntityIDs(QList<vtkIdType> &visEntities)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();

  vtkIdType faceId;
  pqCMBModelEntity* entity;

  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    faceId = mapIter.key();
    entity = mapIter.value();
    if(entity->getVisibility()&& !visEntities.contains(faceId))
      {
      visEntities.append(faceId);
      }
    }

  if(this->getModelDimension() == 3 && this->has2DEdges())
    {
    for (EntityMapIterator mapIter = this->EdgeIDToEdgeMap.begin();
      mapIter != this->EdgeIDToEdgeMap.end(); ++mapIter)
      {
      faceId = mapIter.key();
      entity = mapIter.value();
      if(entity->getVisibility()&& !visEntities.contains(faceId))
        {
        visEntities.append(faceId);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::splitModelFaces(QList<vtkIdType>& selFaceIds, double angle)
{
  QMap< vtkIdType, QList<vtkIdType> > SplitFaceToFacesMap;
  QMap< vtkIdType, QList<vtkIdType> > SplitEdgeToEdgesMap;
  vtkSmartPointer<vtkSplitOperatorClient> SplitOperator =
    vtkSmartPointer<vtkSplitOperatorClient>::New();
  SplitOperator->SetFeatureAngle(angle);
  for(int i=0; i<selFaceIds.count(); i++)
    {
    vtkIdType selFaceId = selFaceIds.value(i);
    SplitOperator->SetId(selFaceId);
    if(SplitOperator->Operate(this->getModel(), this->ModelWrapper))
      {
      vtkIdTypeArray* newFaceIds = SplitOperator->GetCreatedModelFaceIDs();
      QList<vtkIdType> newFaces;
      vtkIdType faceId;
      for(int j=0;j<newFaceIds->GetNumberOfTuples();j++)
        {
        faceId = newFaceIds->GetValue(j);
        if(this->createNewFace(faceId))
          {
          newFaces.append(faceId);
          }
        }
      if(newFaces.count()>0)
        {
        SplitFaceToFacesMap.insert(selFaceId, newFaces);
        if(this->getModelDimension() == 3 && this->has2DEdges())
          {
          this->updateHybridModelEdges(
            SplitOperator->SplitEdgeMap,
            SplitOperator->SplitVertMap,
            SplitOperator->NewEdges,
            SplitOperator->NewVerts,
            SplitEdgeToEdgesMap);
          }
        }
      this->updateModelEntityRepresentation(selFaceId);
      }
    }

  if(SplitFaceToFacesMap.count()>0)
    {
    this->updateModelRepresentation();
    this->updateModelDataInfo();
    emit this->entitiesSplit(SplitFaceToFacesMap);
    if(this->getModelDimension() == 3 && this->has2DEdges() && SplitEdgeToEdgesMap.count())
      {
      emit this->entitiesSplit(SplitEdgeToEdgesMap, true);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateHybridModelEdges(
  std::map<vtkIdType, std::vector<vtkIdType> >& SplitEdgeMap,
  std::map<vtkIdType, std::vector<vtkIdType> >& SplitVertMap,
  std::vector<vtkIdType>& NewEdges, std::vector<vtkIdType>& NewVerts,
  QMap< vtkIdType, QList<vtkIdType> > &SplitEdgeToEdgesMap)
{
  // new edgs
  QList<vtkIdType> newVTKArcs;
  for(std::map<vtkIdType, std::vector<vtkIdType> >::iterator it=
    SplitEdgeMap.begin(); it !=SplitEdgeMap.end(); ++it)
    {
    QList<vtkIdType> edges = QList<vtkIdType>::fromVector(
      QVector<vtkIdType>::fromStdVector(it->second));
    newVTKArcs.append(edges);
    SplitEdgeToEdgesMap[it->first].append(edges);
    }

  QList<vtkIdType> nEdges = QList<vtkIdType>::fromVector(
    QVector<vtkIdType>::fromStdVector(NewEdges));
  newVTKArcs.append(nEdges);

  // new verts
  QList<vtkIdType> newVTKVTXs;
  for(std::map<vtkIdType, std::vector<vtkIdType> >::iterator it=
    SplitVertMap.begin(); it !=SplitVertMap.end(); ++it)
    {
    QList<vtkIdType> verts = QList<vtkIdType>::fromVector(
      QVector<vtkIdType>::fromStdVector(it->second));
    newVTKVTXs.append(verts);
    }

  QList<vtkIdType> nVerts = QList<vtkIdType>::fromVector(
    QVector<vtkIdType>::fromStdVector(NewVerts));
  newVTKVTXs.append(nVerts);

  // create new client/cmb edges and verts
  QList<vtkIdType> newCMBArcs;
  this->createCMBArcs(newVTKArcs, newVTKVTXs, newCMBArcs);
}

//-----------------------------------------------------------------------------
void pqCMBModel::createModelEdges()
{
  // Once we create new edges on the model, currently we have to re-serialize
  // the whole model to bring the edges to client model, therefore we
  // have to clear the client model
  this->clearClientModel();
  vtkStringArray* serialModel = this->StateOperator->GetSerializedModelString();
  vtkNew<vtkCreateModelEdgesOperatorClient> edgeOperator;
  if(edgeOperator->Operate(this->getModel(), this->ModelWrapper,
                           serialModel->GetValue(0).c_str()))
    {
    if(this->showModel())
      {
      this->setupClientModel();
      }
    }
}

//----------------------------------------------------------------------------
vtkModelEntity* pqCMBModel::acceptGrowResult(QList<vtkIdType> &outBCSFaces)
{
  outBCSFaces.clear();

  vtkPVSelectionInformation* selInfo = vtkPVSelectionInformation::New();
  vtkIdTypeArray* masterSelIDs = this->getGrowSelectionIds(selInfo);
  if(masterSelIDs && masterSelIDs->GetNumberOfTuples()>0)
    {
    vtkSmartPointer<vtkSelectionSplitOperatorClient> SelectionSplitOperator =
      vtkSmartPointer<vtkSelectionSplitOperatorClient>::New();

    pqWaitCursor cursor;

    bool Success = SelectionSplitOperator->Operate(
        this->Model, this->ModelWrapper,
        this->CurrentGrowSelectionSource->getProxy());
    if(Success)
      {
      QMap< vtkIdType, QList<vtkIdType> > SplitFaceToFacesMap;
      QMap< vtkIdType, QList<vtkIdType> > SplitEdgeToEdgesMap;

      vtkIdType id, srcId, tgtId, numTuples;
      vtkIdTypeArray* modifiedPairs = SelectionSplitOperator->GetModifiedPairIDs();
      if(modifiedPairs)
        {
        numTuples = modifiedPairs->GetNumberOfTuples();
        // each tuple has two values (srcid, tgtid)
        for(id=0;id<numTuples;id++)
          {
          QList<vtkIdType> faces;
          SelectionSplitOperator->GetModifiedPair(id, srcId, tgtId);
          //outBCSFaces.append(srcId);
          outBCSFaces.append(tgtId);
          faces.append(tgtId);
          this->createNewFace(tgtId);
          SplitFaceToFacesMap.insert(srcId, faces);
          if(this->getModelDimension() == 3 && this->has2DEdges())
            {
            this->updateHybridModelEdges(
              SelectionSplitOperator->SplitEdgeMap,
              SelectionSplitOperator->SplitVertMap,
              SelectionSplitOperator->NewEdges,
              SelectionSplitOperator->NewVerts,
              SplitEdgeToEdgesMap);
            }
          this->updateModelEntityRepresentation(srcId);
          }
        }
      vtkIdTypeArray* unChangedFaces = SelectionSplitOperator->GetCompletelySelectedIDs();
      if(unChangedFaces)
        {
        for(id=0;id<unChangedFaces->GetNumberOfTuples();id++)
          {
          outBCSFaces.append( unChangedFaces->GetValue(id));
          }
        }
      if(SplitFaceToFacesMap.count())
        {
        this->updateModelDataInfo();
        emit this->entitiesSplit(SplitFaceToFacesMap);
        if(this->getModelDimension() == 3 && this->has2DEdges() && SplitEdgeToEdgesMap.count())
          {
          emit this->entitiesSplit(SplitEdgeToEdgesMap, true);
          }
        }
      this->updateModelRepresentation();
      }
    }

  selInfo->Delete();
  //this->saveModelState();
  if(outBCSFaces.count()==0)
    {
    return NULL;
    }

  vtkModelEntity* bcEnt = NULL;
  bcEnt = this->createBCS(vtkModelFaceType);
  if(bcEnt)
    {
    this->addEntitiesToBCGroups(bcEnt->GetUniquePersistentId(), outBCSFaces);
    }

  return bcEnt;
}

//----------------------------------------------------------------------------
void pqCMBModel::clearGrowResult()
{
  if(!this->CurrentGrowSelectionSource)
    {
    return;
    }
  vtkSMDataSourceProxy* masterSelSource = vtkSMDataSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy());

  // Empty selection
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
      pxm->NewProxy("sources", "cmbIDSelectionSource"));
  selSource->UpdateProperty("RemoveAllIDs", 1);
  selSource->UpdateVTKObjects();
  selSource->UpdatePipeline();

  //vtkSMPropertyHelper newSelIDs(selSource, "IDs");
  //newSelIDs.RemoveAllValues();

  masterSelSource->CopyData(selSource);
  masterSelSource->UpdatePipeline();
  selSource->Delete();

  vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSource->getProxy())->SetSelectionInput(
      0, NULL, 0);

  pqCMBModelFace::setRepresentationVisibility(this->CurrentGrowRep, 0);
  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::updateModel()
{
  this->updateModelInternal();
}

//----------------------------------------------------------------------------
void pqCMBModel::updateModelInternal()
{
  this->updateEntityRepresentations(this->FaceIDToFaceMap);
  this->updateEntityRepresentations(this->NGIDToNGRepMap);
  this->updateEntityRepresentations(this->EdgeID2RepMap);
  this->updateEntityRepresentations(this->EdgeIDToEdgeMap);
  this->updateEntityRepresentations(this->VertexIDToVertexMap);
  this->updateModelRepresentation();

  this->RenderView->resetCamera();
  this->RenderView->render();

}
//----------------------------------------------------------------------------
void pqCMBModel:: removeModelEntities(QList<vtkIdType>& entityIds,
  QMap<vtkIdType, pqCMBModelEntity*> &entityMap)
{
  vtkIdType entityId;
  for(int id=0; id<entityIds.count();id++)
    {
    entityId = entityIds.value(id);
    if(entityMap.keys().contains(entityId))
      {
      if(entityMap[entityId])
        {
        delete entityMap[entityId];
        }
      entityMap.remove(entityId);
      }
    }
}
//----------------------------------------------------------------------------
void pqCMBModel::removeModelEntities(QList<vtkIdType>& faces)
{
  this->removeModelEntities(faces, this->FaceIDToFaceMap);
  this->removeModelEntities(faces, this->EdgeIDToEdgeMap);
}

//-----------------------------------------------------------------------------
void pqCMBModel::mergeModelFaces(QList<vtkIdType>& selFaceIds)
{
  if(selFaceIds.count() <= 1)
    {
    return;
    }
  vtkIdType toFaceId = selFaceIds.value(0);
  for(int id=1; id<selFaceIds.count();id++)
    {
    vtkIdType faceId = selFaceIds.value(id);
    if(faceId < toFaceId)
      {
      toFaceId = faceId;
      }
    }
  int findex = selFaceIds.indexOf(toFaceId);
  if(findex >=0)
    {
    selFaceIds.removeAt(findex);
    }

  QList<vtkIdType> mergedFaceIds;
  for(int i=0;i<selFaceIds.count();i++)
    {
    vtkIdType sourceId = selFaceIds.value(i);
    vtkSmartPointer<vtkMergeOperatorClient> MergeOperator =
      vtkSmartPointer<vtkMergeOperatorClient>::New();
    MergeOperator->SetTargetId(toFaceId);
    MergeOperator->SetSourceId(sourceId);
    if(MergeOperator->AbleToOperate(this->getModel()))
      {
      if(MergeOperator->Operate(this->getModel(), this->ModelWrapper))
        {
        mergedFaceIds.append(sourceId);
        }
      }
    }
  this->updateModelSource();
  this->updateModelDataInfo();
  emit this->entitiesMerged(toFaceId, mergedFaceIds);
  this->updateModelEntityRepresentation(toFaceId);
}
//----------------------------------------------------------------------------
void pqCMBModel::updateEntityRepresentations(
  QMap<vtkIdType, pqCMBModelEntity*> &entityMap)
{
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    mapIter.value()->updateRepresentation(
        this->ModelWrapper);
    }

  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::updateModelRepresentation()
{
  if(this->ModelRepresentation)
    {
    vtkSMSourceProxy* modelSource = vtkSMSourceProxy::SafeDownCast(
      this->modelSource()->getProxy());
    modelSource->MarkModified(NULL);
    modelSource->UpdateVTKObjects();
    this->modelSource()->updatePipeline();
    this->ModelRepresentation->getProxy()->UpdateVTKObjects();
    vtkSMRepresentationProxy::SafeDownCast(
      this->ModelRepresentation->getProxy())->UpdatePipeline();
    }
}

//----------------------------------------------------------------------------
void pqCMBModel::setAllRepresentationsVisibility(int visible)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();

  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    this->changeModelEntityVisibility(mapIter.value(), visible, false);
    }

  if(this->getModelDimension() == 3 && this->has2DEdges())
    {
    for (EntityMapIterator mapIter = this->EdgeIDToEdgeMap.begin();
      mapIter != this->EdgeIDToEdgeMap.end(); ++mapIter)
      {
      this->changeModelEntityVisibility(mapIter.value(), visible, false);
      }
    }
  this->RenderView->render();
}
//----------------------------------------------------------------------------
void pqCMBModel::changeModelEntityVisibility(
  vtkIdType entityid, int visible, bool render)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();
  pqCMBModelEntity* entity = NULL;
  if(entityMap.keys().contains(entityid))
    {
    entity = entityMap[entityid];
    }
  else if(this->getModelDimension() == 3 && this->has2DEdges()
    && this->EdgeIDToEdgeMap.contains(entityid))
    {
    entity = this->EdgeIDToEdgeMap[entityid];
    }
  if(entity)
    {
    this->changeModelEntityVisibility(entity, visible, render);
    }
}

//----------------------------------------------------------------------------
void pqCMBModel::changeModelEntityVisibility(
    pqCMBModelEntity* entity, int visible, bool render)
{
  if(!visible)
    {
    entity->setHighlight(0);
    }

  if(this->ModelWrapper)
    {
    /*

    === NEW SMTK MODEL OPERATOR EXAMPLE ===

    smtkOperator op = this->ModelWrapper->createOperator("modify model entity");
    op->setParameter(Parameter("visibility", visible));
    op->setParameter(Parameter("entity", entity));

    if (op->ableToOperate()) {
      OperatorResult result = op->Operate(this->ModelWrapper);
      // You can access result.parameter("fooOutputData") here.
    }
     */
    vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEntityOperatorClient>::New();
    ModelEntityOperator->SetVisibility(visible);
    ModelEntityOperator->SetItemType(entity->getModelEntity()->GetType());
    ModelEntityOperator->SetId(entity->getModelEntity()->GetUniquePersistentId());

    if(ModelEntityOperator->Operate(this->Model, this->ModelWrapper))
      {
      // client side should have been updated by the operator
      entity->setRepresentationVisibility(
        entity->getRepresentation(), visible);
      }
    }
  else
    {
    entity->setVisibility(visible);
    }

  // for 2D model we also need to change the visibility of
  // associated model vertex when model edge visibility is changed
  vtkDiscreteModelEdge* modelEdge = vtkDiscreteModelEdge::SafeDownCast(
    entity->getModelEntity());
  if(this->has2DEdges() && modelEdge )
    {
    if(vtkModelVertex* vtxEntity = modelEdge->GetAdjacentModelVertex(0))
      {
      int iVis;
      vtkIdType vtxId = vtxEntity->GetUniquePersistentId();
      if(this->VertexIDToVertexMap.contains(vtxId))
        {
        iVis = visible ? visible : this->shouldVertexBeVisible(
            vtkModelVertex::SafeDownCast(this->VertexIDToVertexMap[vtxId]->getModelEntity()));
        this->changeModelEntityVisibility(this->VertexIDToVertexMap[vtxId], iVis, false);
        //this->VertexIDToVertexMap[vtxId]->setVisibility(iVis);
        }
      vtxEntity = modelEdge->GetAdjacentModelVertex(1);
      if(vtxEntity)
        {
        vtxId = vtxEntity->GetUniquePersistentId();
        if(this->VertexIDToVertexMap.contains(vtxId))
          {
          iVis = visible ? visible : this->shouldVertexBeVisible(
              vtkModelVertex::SafeDownCast(this->VertexIDToVertexMap[vtxId]->getModelEntity()));
          this->changeModelEntityVisibility(this->VertexIDToVertexMap[vtxId], iVis, false);
          //this->VertexIDToVertexMap[vtxId]->setVisibility(iVis);
          }
        }
      }
    }

  // update the selection
  QList<vtkIdType> selEntities;
  this->getSelectedModelEntities(selEntities);
  if(selEntities.contains(entity->getUniqueEntityId()) && !visible)
    {
    selEntities.removeAt(selEntities.indexOf(entity->getUniqueEntityId()));
    this->highlightModelEntities(selEntities);
    }
  if(this->ModelWrapper)
    {
    vtkSMSourceProxy* modelSource = vtkSMSourceProxy::SafeDownCast(
      this->modelSource()->getProxy());
    modelSource->MarkModified(NULL);
    this->modelSource()->updatePipeline();
    }

  if(render)
    {
    if(visible)
      {
      this->onLookupTableModified();
      }
    else
      {
      this->RenderView->render();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::resetRepresentationColors()
{
  this->onLookupTableModified();
}

//-----------------------------------------------------------------------------
void pqCMBModel::onLookupTableModified()
{
  if(!this->hasGeometryEntity())
    {
    return;
    }

  //color attributes first
  QList<vtkIdType> assignedIds;
  if(this->getFaceColorMode()==pqCMBModelEntity::ColorByAttribute)
    {
    this->colorByAttributes(assignedIds, this->CurrentFaceAttDefType);
    }
  if(this->getEdgeColorMode()==pqCMBModelEntity::ColorByAttribute)
    {
    this->colorByAttributes(assignedIds, this->CurrentEdgeAttDefType);
    }

  if(this->getModelDimension() == 3)
    {
    // The colorMode is always decided from the first rep
    vtkIdType faceId = this->FaceIDToFaceMap.keys().value(0);
    pqCMBModelFace* cmbFace = qobject_cast<pqCMBModelFace*>(
        this->FaceIDToFaceMap[faceId]);
    for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
      mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
      {
      cmbFace = qobject_cast<pqCMBModelFace*>(
          mapIter.value());
      if(!assignedIds.contains(cmbFace->getUniqueEntityId()))
        {
        cmbFace->colorByColorMode(
          this->ColorLookupTable, this->getFaceColorMode());
        }
      }
    }

  if(this->has2DEdges())
    {
    for (EntityMapIterator mapIter = this->EdgeIDToEdgeMap.begin();
      mapIter != this->EdgeIDToEdgeMap.end(); ++mapIter)
      {
      pqCMBModelEdge* cmbEdge = qobject_cast<pqCMBModelEdge*>(
          mapIter.value());
      if(!assignedIds.contains(cmbEdge->getUniqueEntityId()))
        {
        cmbEdge->colorByColorMode(
            this->ColorLookupTable, this->getEdgeColorMode());
        }
      }
    }

  if(this->getModelDimension() == 2)
    {
    assignedIds.clear();
    if(this->getColorEdgeDomainMode()==
       pqCMBModelFace::PolygonFaceColorByAttribute)
      {
      this->colorByAttributes(
        assignedIds, this->CurrentDomainAttDefType, true);
      }

    pqCMBModelFace* polygonFace;
    for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
      mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
      {
      polygonFace = qobject_cast<pqCMBModelFace*>(
        mapIter.value());
      if(!assignedIds.contains(polygonFace->getUniqueEntityId()))
        {
        polygonFace->colorByEdgeDomainMode(
          this->ColorLookupTable, this->getColorEdgeDomainMode());
        }
      }
    }

  // we need to pass the representation and rep-color to model mapper on the server
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::onModelModified()
{
  emit this->currentModelModified();
}

//-----------------------------------------------------------------------------
void pqCMBModel::onBCGroupNumberChanged()
{
  //emit this->bcGroupNumberChanged();
}

//----------------------------------------------------------------------------
void pqCMBModel::createLookupTable()
{
  this->ColorLookupTable =
    vtkSmartPointer<vtkDiscreteLookupTable>::New();

  this->ColorLookupTable->Build();

  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  vtkSMProxy* lut = NULL;
  lut = builder->createProxy("lookup_tables", "PVLookupTable",
      this->ActiveServer, "lookup_tables");

  if(lut)
    {
    QList<QVariant> values;
    values << 0.0 << 0.0 << 0.0 << 1.0
      << 1.0 << 1.0 << 0.0 << 0.0;
    pqSMAdaptor::setMultipleElementProperty(
        lut->GetProperty("RGBPoints"), values);

    pqSMAdaptor::setEnumerationProperty(
        lut->GetProperty("ColorSpace"), "HSV");
    pqSMAdaptor::setEnumerationProperty(
        lut->GetProperty("VectorMode"), "Component");
    lut->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::highlightModelEntities(QList<vtkIdType> selVisibleModelEntities)
{
  if(selVisibleModelEntities.count()==0)
    {
    this->clearAllEntityHighlights(false);
    return;
    }
  std::vector<vtkIdType> ids;
  for(int i=0; i<selVisibleModelEntities.count(); i++)
    {
    ids.push_back(selVisibleModelEntities.value(i));
    }
  vtkSMSourceProxy* modelSource = vtkSMSourceProxy::SafeDownCast(
    this->modelSource()->getProxy());

  vtkSMDataSourceProxy* newSelSource = vtkSMDataSourceProxy::SafeDownCast(
    this->ModelSelectionSource->getProxy());
  vtkSMPropertyHelper newSelIDs(newSelSource, "SelectedEntityIds");
  newSelIDs.Set(&ids[0], static_cast<unsigned int>(ids.size()));
  newSelSource->UpdateVTKObjects();
  newSelSource->UpdatePipeline();
  modelSource->SetSelectionInput(0, newSelSource, 0);

  vtkSMProxy* modelRepProxy = this->ModelRepresentation->getProxy();
  vtkSMPropertyHelper repSelIDs(modelRepProxy, "SelectedEntityIds");
  repSelIDs.Set(&ids[0], static_cast<unsigned int>(ids.size()));
  modelRepProxy->UpdateVTKObjects();

  this->ModelWrapper->MarkModified(NULL);
  modelSource->MarkModified(NULL);
  this->modelSource()->updatePipeline();

  this->CurrentSelectedEntities.clear();
  this->CurrentSelectedEntities.append(selVisibleModelEntities);

  this->RenderView->render();
}

//-----------------------------------------------------------------------------
pqCMBModelEntity* pqCMBModel::getFirstSelectedModelEntity()
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();

  if(entityMap.count()<=0)
    {
    return NULL;
    }


  QList<vtkIdType> selEntities;
  this->getSelectedModelEntities(selEntities);
  // find selected face representations, then copy the
  // changed property to them
  if (selEntities.count()>0)
    {
    vtkIdType selId = selEntities[0];
    if(this->FaceIDToFaceMap.contains(selId))
      {
      return this->FaceIDToFaceMap[selId];
      }
    else if(this->EdgeIDToEdgeMap.contains(selId))
      {
      return this->EdgeIDToEdgeMap[selId];
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBModel::setupRepresentationVTKConnection(
    pqDataRepresentation* connRep, bool /*updateRep*/)
{
  this->VTKConnect->Disconnect();
  this->connectTransformProperties(connRep);

  emit this->currentVTKConnectionChanged(connRep);
}

//-----------------------------------------------------------------------------
void pqCMBModel::connectTransformProperties(
    pqDataRepresentation* connRep)
{
  this->VTKConnect->Connect(connRep->getProxy()
      ->GetProperty("Orientation"),
      vtkCommand::ModifiedEvent, this,
      SLOT(onTransformProperyModified(vtkObject*, unsigned long, void*)),
      const_cast<char*>("Orientation"));
  this->VTKConnect->Connect(connRep->getProxy()
      ->GetProperty("Origin"),
      vtkCommand::ModifiedEvent, this,
      SLOT(onTransformProperyModified(vtkObject*, unsigned long, void*)),
      const_cast<char*>("Origin"));
  this->VTKConnect->Connect(connRep->getProxy()
      ->GetProperty("Position"),
      vtkCommand::ModifiedEvent, this,
      SLOT(onTransformProperyModified(vtkObject*, unsigned long, void*)),
      const_cast<char*>("Position"));
  this->VTKConnect->Connect(connRep->getProxy()
      ->GetProperty("Scale"),
      vtkCommand::ModifiedEvent, this,
      SLOT(onTransformProperyModified(vtkObject*, unsigned long, void*)),
      const_cast<char*>("Scale"));
}

//-----------------------------------------------------------------------------
int pqCMBModel::modify2DModelTransformProperties(
  vtkObject* senderObj, unsigned long, void* evtPropertyName)
{
  if(!senderObj || !evtPropertyName)
    {
    return 0;
    }
  vtkSMProperty *changedProp = vtkSMProperty::SafeDownCast(senderObj);
  if(!changedProp)
    {
    return 0;
    }
  const char* propertyName = reinterpret_cast<const char*>(evtPropertyName);
  QMap<vtkIdType, pqCMBModelEntity*> entityMap;
  entityMap.unite(this->EdgeIDToEdgeMap);
  entityMap.unite(this->FaceIDToFaceMap);

  if( !propertyName || !(*propertyName) ||
     entityMap.count()<=1)
    {
    return 0;
    }
  entityMap.unite(this->VertexIDToVertexMap);

  // find selected face representations, then copy the
  // changed property to them
  vtkIdType faceId;
  vtkSMSourceProxy* source = NULL;
  vtkSMProxy* rep = NULL;
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = entityMap.begin();
       mapIter != entityMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    faceId = mapIter.key();
    source = vtkSMSourceProxy::SafeDownCast(
      entity->getSource()->getProxy());
    if(entity->getRepresentation())
      {
      rep = entity->getRepresentation()->getProxy();
      vtkSMProperty *toProp = rep->GetProperty(propertyName);
      if(toProp && toProp != changedProp)
        {
        toProp->Copy(changedProp);
        rep->UpdateVTKObjects();
        }
      }
    entity->synchronizeMeshRepresentation(changedProp, propertyName);
    }

  this->RenderView->render();
  return 1;

}

//-----------------------------------------------------------------------------
void pqCMBModel::onTransformProperyModified(
    vtkObject* senderObj, unsigned long ulEvent, void* evtPropertyName)
{
  // if it is a 2D model, we need to include the model face representations
  if(this->getModelDimension() == 2)
    {
    this->modify2DModelTransformProperties(
      senderObj, ulEvent, evtPropertyName);
    }
  else if(this->CurrentGrowRep != NULL)
    {
    vtkSMProperty *changedProp = vtkSMProperty::SafeDownCast(senderObj);
    if(!changedProp)
      {
      return;
      }
    vtkSMProxy* rep = this->CurrentGrowRep->getProxy();
    const char* propertyName = reinterpret_cast<const char*>(evtPropertyName);
    vtkSMProperty *toProp = rep->GetProperty(propertyName);
    if(toProp && toProp != changedProp)
      {
      toProp->Copy(changedProp);
      rep->UpdateVTKObjects();
      int visible = pqSMAdaptor::getElementProperty(
          rep->GetProperty("Visibility")).toInt();
      if(visible)
        {
        this->RenderView->render();
        }
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::clearAllEntityHighlights(bool rerender)
{
  this->CurrentSelectedEntities.clear();

  if(this->ModelSelectionSource)
    {
    vtkSMDataSourceProxy* newSelSource = vtkSMDataSourceProxy::SafeDownCast(
      this->ModelSelectionSource->getProxy());
    vtkSMPropertyHelper newSelIDs(newSelSource, "SelectedEntityIds");
    newSelIDs.SetNumberOfElements(0);
    newSelSource->UpdateVTKObjects();
    newSelSource->UpdatePipeline();
    }

  vtkSMSourceProxy* modelSource = vtkSMSourceProxy::SafeDownCast(
    this->modelSource()->getProxy());
  modelSource->SetSelectionInput(0, NULL, 0);

  if(rerender)
    {
    this->RenderView->render();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::setFacesColor(const QColor& fcolor)
{
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
    mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    this->modifyUserSpecifiedColor(vtkModelFaceType,
      entity->getUniqueEntityId(), fcolor);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::setEdgesColor(const QColor& ecolor)
{
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = this->EdgeIDToEdgeMap.begin();
    mapIter != this->EdgeIDToEdgeMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    this->modifyUserSpecifiedColor(vtkModelEdgeType,
      entity->getUniqueEntityId(), ecolor);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::modifyUserSpecifiedColor(
    int entityType, vtkIdType entityId, const QColor& newColor, bool rerender)
{
  double RGBA[4];
  newColor.getRgbF(&RGBA[0], &RGBA[1], &RGBA[2], &RGBA[3]);
  bool updateClient = true;
  if(this->ModelWrapper)
    {
    vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEntityOperatorClient>::New();
    ModelEntityOperator->SetItemType(entityType);
    ModelEntityOperator->SetId(entityId);
    ModelEntityOperator->SetRGBA(RGBA);

    updateClient = ModelEntityOperator->Operate(this->Model, this->ModelWrapper);
    }

  if(updateClient)
    {
    if(entityType == vtkModelFaceType)
      {
      this->FaceIDToFaceMap[entityId]->setColor(RGBA[0], RGBA[1], RGBA[2], RGBA[3]);
      }
    else if(entityType == vtkModelEdgeType)
      {
      this->EdgeIDToEdgeMap[entityId]->setColor(RGBA[0], RGBA[1], RGBA[2], RGBA[3]);
      }
    if(rerender)
      {
      this->RenderView->render();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::modifyUserSpecifiedName(
    int entityType, vtkIdType entityId, const char* newName)
{
  if(this->ModelWrapper)
    {
    vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
      vtkSmartPointer<vtkModelEntityOperatorClient>::New();
    ModelEntityOperator->SetUserName(newName);
    ModelEntityOperator->SetItemType(entityType);
    ModelEntityOperator->SetId(entityId);

    if(ModelEntityOperator->Operate(this->Model, this->ModelWrapper))
      {
      emit this->modelEntityNameChanged(
        this->Model->GetModelEntity(entityType, entityId));
      }
    }
  else
    {
    vtkModelEntity* entity = this->Model->GetModelEntity(entityType, entityId);
    if(entity)
      {
      vtkModelUserName::SetUserName(entity, newName);
      emit this->modelEntityNameChanged(
        this->Model->GetModelEntity(entityType, entityId));
      }
    }
}

//-----------------------------------------------------------------------------
vtkModelMaterial* pqCMBModel::createMaterial()
{
  if(this->ModelWrapper && this->hasGeometryEntity())
    {
    vtkSmartPointer<vtkMaterialOperatorClient> MaterialOperator =
      vtkSmartPointer<vtkMaterialOperatorClient>::New();
    vtkIdType newId = MaterialOperator->Build(this->Model, this->ModelWrapper);
    if(newId>=0)
      {
      return vtkModelMaterial::SafeDownCast(
        this->Model->GetModelEntity(vtkModelMaterialType, newId));
      }
    }
  else // we only have a client model
    {
    vtkModelMaterial* Material = this->Model->BuildMaterial();
    return Material;
    }

  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBModel::removeMaterial(vtkModelMaterial* materialEntity)
{
  if(!materialEntity)
    {
    return;
    }
  if(this->ModelWrapper)
    {
    vtkSmartPointer<vtkMaterialOperatorClient> MaterialOperator =
      vtkSmartPointer<vtkMaterialOperatorClient>::New();
    MaterialOperator->SetId(materialEntity->GetUniquePersistentId());
    if(!MaterialOperator->Destroy(this->Model, this->ModelWrapper))
      {
      //cout << "Problem destroying a material.\n";
      }
    }
  else
    {
    this->Model->DestroyMaterial(materialEntity);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::changeShellMaterials(
    vtkModelMaterial* materialEntity, QList<vtkIdType> shellIds)
{
  if(!materialEntity || shellIds.count()==0)
    {
    return;
    }
  vtkSmartPointer<vtkMaterialOperatorClient> MaterialOperator =
    vtkSmartPointer<vtkMaterialOperatorClient>::New();
  MaterialOperator->SetId(materialEntity->GetUniquePersistentId());

  for(int i=0; i<shellIds.count();i++)
    {
    MaterialOperator->AddModelGeometricEntity(shellIds.value(i));
    }
  MaterialOperator->Operate(this->Model, this->ModelWrapper);
}

//-----------------------------------------------------------------------------
vtkDiscreteModelEntityGroup* pqCMBModel::createBCS(int enEntityType)
{
  bool isClientOnly = !(this->ModelWrapper && this->hasGeometryEntity());
  if(enEntityType != vtkModelFaceType && enEntityType != vtkModelEdgeType)
    {
    enEntityType = vtkModelFaceType;
    if(this->getModelDimension() == 3)
      {
      if(this->has2DEdges())
        {
        if(QMessageBox::question(NULL,
          "Choose Model Entity Group Type",
          "Create model face group (Yes) or model edge group (No)?",
          QMessageBox::Yes|QMessageBox::No, QMessageBox::No) ==
          QMessageBox::No)
          {
          enEntityType = vtkModelEdgeType;
          }
        }
      }
    else
      {
      enEntityType = vtkModelEdgeType;
      }
    }

  if(!isClientOnly)
    {
    vtkSmartPointer<vtkModelEntityGroupOperatorClient> GroupOperator =
      vtkSmartPointer<vtkModelEntityGroupOperatorClient>::New();
    GroupOperator->SetBuildEnityType(enEntityType);
    vtkIdType newId = GroupOperator->Build(this->Model, this->ModelWrapper);
    if(newId>=0)
      {
      this->onBCGroupNumberChanged();
      return vtkDiscreteModelEntityGroup::SafeDownCast(
        this->Model->GetModelEntity(vtkDiscreteModelEntityGroupType, newId));
      }
    }
  else // we only have a client model
    {
    vtkDiscreteModelEntityGroup* EntityGroup = this->Model->BuildModelEntityGroup(
      enEntityType, 0, 0);
    return EntityGroup;
    }

  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBModel::removeBCS(vtkIdType bcId)
{
  if(this->ModelWrapper)
    {
    vtkSmartPointer<vtkModelEntityGroupOperatorClient> GroupOperator =
      vtkSmartPointer<vtkModelEntityGroupOperatorClient>::New();
    GroupOperator->SetId(bcId);
    GroupOperator->Destroy(this->Model, this->ModelWrapper);
    }
  else
    {
    vtkDiscreteModelEntityGroup* EntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
      this->Model->GetModelEntity(vtkDiscreteModelEntityGroupType, bcId));
    this->Model->DestroyModelEntityGroup(EntityGroup);
    }
  this->onBCGroupNumberChanged();
}

//-----------------------------------------------------------------------------
void pqCMBModel::addEntitiesToBCGroups(
    vtkIdType bcId, QList<vtkIdType>& faceIds)
{
  vtkSmartPointer<vtkModelEntityGroupOperatorClient> GroupOperator =
    vtkSmartPointer<vtkModelEntityGroupOperatorClient>::New();
  GroupOperator->SetId(bcId);
  for(int i=0; i<faceIds.count(); i++)
    {
    GroupOperator->AddModelEntity(faceIds.value(i));
    }
  GroupOperator->Operate(this->Model, this->ModelWrapper);
}

//-----------------------------------------------------------------------------
void pqCMBModel::removeModelFacesFromBCS(
    vtkIdType bcId, QList<vtkIdType>& faceIds)
{
  vtkSmartPointer<vtkModelEntityGroupOperatorClient> GroupOperator =
    vtkSmartPointer<vtkModelEntityGroupOperatorClient>::New();
  GroupOperator->SetId(bcId);
  for(int i=0; i<faceIds.count(); i++)
    {
    GroupOperator->RemoveModelEntity(faceIds.value(i));
    }
  GroupOperator->Operate(this->Model, this->ModelWrapper);
}

//-----------------------------------------------------------------------------
void pqCMBModel::growModelFacesWithCellId(
    vtkIdType compositeIndex, vtkIdType cellId,
    double angle, int growMode)
{
  QList<vtkIdType> visibleFaces;
  this->getVisibleModelEntityIDs(visibleFaces);
  if (visibleFaces.count() == 0)
    {
    return;
    }

  if(!this->GrowModelFaces)
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    this->GrowModelFaces = vtkSMSourceProxy::SafeDownCast(
        pxm->NewProxy("CMBModelGroup", "SeedGrow"));
    }

  if(!this->GrowModelFaces)
    {
    return;
    }

  pqWaitCursor cursor;
  QList<QVariant> facecellId;
  facecellId << compositeIndex << cellId;

  pqSMAdaptor::setMultipleElementProperty(
      this->GrowModelFaces->GetProperty("FaceCellId"), facecellId);

  pqSMAdaptor::setElementProperty(
      this->GrowModelFaces->GetProperty("FeatureAngle"), angle);

  QList<QVariant> growFaces;
  for(int i=0; i<visibleFaces.count(); i++)
    {
    growFaces << visibleFaces[i];
    }
  growFaces << -1;

  pqSMAdaptor::setMultipleElementProperty(
      this->GrowModelFaces->GetProperty("GrowFaceIds"), growFaces);

  if(growMode > 0)
    {
    vtkSMSourceProxy* masterSelSource = vtkSMSourceProxy::SafeDownCast(
        this->CurrentGrowSelectionSource->getProxy());

    vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
        this->GrowModelFaces->GetProperty("Selection"));
    pp->RemoveAllProxies();
    pp->AddProxy(masterSelSource);
    }

  pqSMAdaptor::setElementProperty(
      this->GrowModelFaces->GetProperty("GrowMode"), growMode);

  vtkSMProxyProperty* Wrapper =
    vtkSMProxyProperty::SafeDownCast(
        this->GrowModelFaces->GetProperty("ModelWrapper"));
  Wrapper->AddProxy(this->ModelWrapper);

  this->GrowModelFaces->UpdateVTKObjects();
  this->GrowModelFaces->UpdatePipeline();

  vtkSMDataSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy())->CopyData(
      this->GrowModelFaces);
  vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy() )->MarkModified(NULL);
  vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy() )->UpdatePipeline();

  vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSource->getProxy())->SetSelectionInput(0,
      vtkSMSourceProxy::SafeDownCast(
        this->CurrentGrowSelectionSource->getProxy() ), 0);
  vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSource->getProxy())->UpdatePipeline();

  //this->highlightFaceRepresentation(data->GrowModelFaces,
  //  this->Internal->CurrentGrowRep);

  pqCMBModelFace::setRepresentationVisibility(this->CurrentGrowRep, 1);
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::growModelFacesWithAngle(
    pqOutputPort* selPort, double angle, int growMode)
{
  pqPipelineSource *source = selPort? selPort->getSource() : NULL;
  if(!source)
    {
    return;
    }

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  if(!smSource || !smSource->GetSelectionInput(0))
    {
    return;
    }

  vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
  vtkSMVectorProperty* vp = vtkSMVectorProperty::SafeDownCast(
      selSource->GetProperty("IDs"));
  QList<QVariant> ids = pqSMAdaptor::getMultipleElementProperty(vp);
  int numElemsPerCommand = vp->GetNumberOfElementsPerCommand();

  // [composite index, process Id, cellIndex]
  vtkIdType cellId = ids[numElemsPerCommand-1].value<vtkIdType>();
  vtkIdType flatIndex = ids[numElemsPerCommand-3].value<vtkIdType>();
  //clear selected cell on the model face
  smSource->SetSelectionInput(0, NULL, 0);

  if(ids.size()>=numElemsPerCommand)
    {
    // The flatIndex starts from root (the whole composite data) as 0
    // so the actual compositeIndex = flatIndex-1;
    this->growModelFacesWithCellId(
        flatIndex-1, cellId, angle, growMode);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::modifyCellGrowSelections(QList<pqOutputPort*>& selPorts,
    int growMode)
{
  QList<vtkIdType> ids;

  vtkNew<vtkPVModelGeometryInformation> pdSourceInfo;
  //QList<vtkIdType> faces = this->FaceIDToFaceMap.keys();
  for(int p=0; p<selPorts.count(); p++)
    {
    pqOutputPort* opPort = selPorts.value(p);
    pqPipelineSource *source = opPort? opPort->getSource() : NULL;
    if(!source )
      {
      continue;
      }
    vtkSMSourceProxy* selSource = opPort->getSelectionInput();
    if(selSource && selSource->GetProperty("IDs"))
      {
      //vtkSMSourceProxy::SafeDownCast(source->getProxy() )->UpdatePipeline();
      source->getProxy()->GatherInformation(pdSourceInfo.GetPointer());

      vtkSMPropertyHelper selIDs(selSource, "IDs");
      unsigned int cc, flatIdx;
      vtkIdType masterId;
      unsigned int count = selIDs.GetNumberOfElements();
      // [composite_index, process_id, index]
      for (cc=0; cc < (count/3); cc++)
        {
        flatIdx = selIDs.GetAsInt(3*cc);
        masterId = pdSourceInfo->GetMasterCellId(
          flatIdx, selIDs.GetAsIdType(3*cc+2));

        if(masterId >= 0)
          {
          ids.push_back(masterId);
          }
        }
      }
    vtkSMSourceProxy::SafeDownCast(source->getProxy())->SetSelectionInput(0, NULL, 0);
    }

  this->modifyCellGrowSelection(ids, growMode);
  pqCMBModelFace::setRepresentationVisibility(this->CurrentGrowRep, 1);
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::modifyCellGrowSelection(QList<vtkIdType>& selCellIds,
    int growMode)
{
  if(selCellIds.count() == 0)
    {
    return;
    }

  vtkSMSourceProxy* masterSelSource = vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy());

  std::vector<vtkIdType> ids;
  vtkPVSelectionInformation* selInfo = vtkPVSelectionInformation::New();
  vtkIdTypeArray* masterSelIDs = this->getGrowSelectionIds(selInfo);
  if(masterSelIDs && masterSelIDs->GetNumberOfTuples()>0)
    {
    vtkIdType cellId;
    for (int cc=0; cc < masterSelIDs->GetNumberOfTuples(); cc++)
      {
      cellId = masterSelIDs->GetValue(cc);
      // if not merge-grow mode
      if(growMode != 1 && selCellIds.contains(cellId))
        {
        selCellIds.removeAt(selCellIds.indexOf(cellId));
        }
      else
        {
        ids.push_back(0);
        ids.push_back(cellId);
        }
      }
    }

  // if not merge-remove mode
  if(growMode != 2)
    {
    for(int i=0; i<selCellIds.count(); i++)
      {
      ids.push_back(0);
      ids.push_back(selCellIds.value(i));
      }
    }

  //This "IDs" only have two components [processId, Index]
  // because the grow_source is not a Composite Dataset
  vtkSMSourceProxy* growSource = vtkSMSourceProxy::SafeDownCast(
    this->CurrentGrowSource->getProxy());
  if(ids.size() != 0)
    {
    vtkSMPropertyHelper newSelIDs(masterSelSource, "IDs");
    newSelIDs.Set(&ids[0], static_cast<unsigned int>(ids.size()));
    masterSelSource->UpdateVTKObjects();
    masterSelSource->UpdatePipeline();

    if(!growSource->GetSelectionInput(0))
      {
      growSource->SetSelectionInput(0, masterSelSource, 0);
      }
    }
  else
    {
    growSource->SetSelectionInput(0, NULL, 0);
    }
  growSource->UpdatePipeline();
  selInfo->Delete();

}

//-----------------------------------------------------------------------------
void pqCMBModel::zoomOnSelection()
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();
  if(entityMap.keys().count()==0)
    {
    return;
    }
  pqCMBModelEntity* modelEntity =
    this->getFirstSelectedModelEntity();
  if(modelEntity)
    {
    this->zoomOnObject(modelEntity);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::zoomOnObject(pqCMBModelEntity* modelEntity)
{
  double bounds[6], transformedBounds[6];
  if(this->ModelRepresentation && modelEntity->getBounds(transformedBounds))
    {
    vtkNew<vtkTransform> modelTransform;
    this->getModelTransform(modelTransform.GetPointer());

    vtkBoundingBox bb;
    //take the transform matrix
    double p[3], tp[3];
    for (int i=0; i < 2; ++i)
      {
      p[0] = transformedBounds[i];
      for (int j=0; j < 2; ++j)
        {
        p[1] = transformedBounds[2 + j];
        for (int k=0; k < 2; ++k)
          {
          p[2] = transformedBounds[4 + k];
          modelTransform->TransformPoint(p,tp);
          bb.AddPoint(tp);
          }
        }
      }
    bb.GetBounds(bounds);
    vtkSMRenderViewProxy* rm = this->RenderView->getRenderViewProxy();
    rm->ResetCamera(bounds);
    this->RenderView->render();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::getModelTransform(vtkTransform* modelTransform)
{
  double position[3], scale[3], orientation[3], origin[3];
  vtkSMProxy* modelRepProxy = this->ModelRepresentation->getProxy();
  vtkSMPropertyHelper(modelRepProxy, "Position").Get(position, 3);
  vtkSMPropertyHelper(modelRepProxy, "Orientation").Get(orientation, 3);
  vtkSMPropertyHelper(modelRepProxy, "Scale").Get(scale, 3);
  vtkSMPropertyHelper(modelRepProxy, "Origin").Get(origin, 3);

  // build the transformation
  modelTransform->Identity();
  modelTransform->PreMultiply();
  modelTransform->Translate( position[0] + origin[0],
                       position[1] + origin[1],
                       position[2] + origin[2] );
  modelTransform->RotateZ( orientation[2] );
  modelTransform->RotateX( orientation[0] );
  modelTransform->RotateY( orientation[1] );
  modelTransform->Scale( scale );
  modelTransform->Translate( -origin[0], -origin[1], -origin[2] );
}

//-----------------------------------------------------------------------------
void pqCMBModel::onSaveData(QWidget* parent)
{
  if (this->GetCurrentModelEntityMap().count() == 0)
    {
    this->OutputFileName.clear();
    return;
    }

  QString filters = "CMB Files(*.cmb)";
  pqFileDialog file_dialog(
      this->ActiveServer,
      parent, tr("Save Model:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->OutputFileName = files[0];
      this->saveData(files[0]);
      return;
      }
    }
  this->OutputFileName.clear();
}

//-----------------------------------------------------------------------------
void pqCMBModel::saveData(const QString& filename)
{
  if (this->GetCurrentModelEntityMap().count() == 0)
    {
    return;
    }

  if(this->ModelWrapper)
    {
    pqWaitCursor cursor;
    vtkSmartPointer<vtkCMBModelWriterClient> WriterOperator =
      vtkSmartPointer<vtkCMBModelWriterClient>::New();
    WriterOperator->SetFileName(filename.toStdString().c_str());
    if(WriterOperator->Operate(this->Model, this->ModelWrapper))
      {
      //return;
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::onCloseData()
{
  this->closeData();
}

//-----------------------------------------------------------------------------
void pqCMBModel::closeData()
{
  this->clearClientModel();

  if(this->ModelSource)
    {
    vtkSMProxyProperty* proxyproperty =
    vtkSMProxyProperty::SafeDownCast(
    this->ModelSource->getProxy()->GetProperty("ModelWrapper"));
    proxyproperty->RemoveAllProxies();
    pqApplicationCore::instance()->getObjectBuilder()->destroy(this->ModelSource);
    this->ModelSource = NULL;
    }

  if(this->ModelWrapper)
    {
    vtkClientServerStream stream;
    stream  << vtkClientServerStream::Invoke
      << VTKOBJECT(this->ModelWrapper)
      << "ResetModel"
      << vtkClientServerStream::End;
    this->ModelWrapper->GetSession()->ExecuteStream(this->ModelWrapper->GetLocation(), stream);
    this->ModelWrapper->Delete();
    this->ModelWrapper = 0;
    }

  this->StateOperator =
    vtkSmartPointer<vtkCMBModelStateOperatorClient>::New();
  this->LatLongTransformOperator =
    vtkSmartPointer<vtkGeoTransformOperatorClient>::New();

}

//-----------------------------------------------------------------------------
void pqCMBModel::onSaveBCSs(QWidget* parent)
{
  QString filters = "CMB BCS Files(*.bcs)";
  pqFileDialog file_dialog(
      this->ActiveServer,
      parent, tr("Save Boundary Conditions:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->saveBCSs(files[0]);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::saveBCSs(const QString& filename)
{
  if (this->GetCurrentModelEntityMap().count() == 0)
    {
    return;
    }

  pqWaitCursor cursor;


  QFileInfo fileInfo(filename);
  if (fileInfo.suffix().toLower() == "bcs")
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    vtkSMSourceProxy* writer = vtkSMSourceProxy::SafeDownCast(
        pxm->NewProxy("CMBModelGroup", "XMLBCSWriter"));
    if(writer)
      {
      vtkSMProxyProperty* proxyproperty =
        vtkSMProxyProperty::SafeDownCast(
            writer->GetProperty("ModelWrapper"));
      proxyproperty->AddProxy(this->ModelWrapper);

      pqSMAdaptor::setElementProperty(
          writer->GetProperty("FileName"), filename.toAscii().data());

      writer->UpdateVTKObjects();

      vtkClientServerStream stream;
      stream  << vtkClientServerStream::Invoke
        << VTKOBJECT(writer)
        << "Update"
        << vtkClientServerStream::End;

      writer->GetSession()->ExecuteStream(writer->GetLocation(), stream);

      writer->Delete();
      }
    }
}

//-----------------------------------------------------------------------------
bool pqCMBModel::writeOmicronMeshInput(const QString& filename,
    const QString& bcsFilename,
    const QString &tetgenCmds)
{
  if(this->ModelWrapper)
    {
    vtkSmartPointer<vtkCMBModelOmicronMeshInputWriterClient> meshInputWriter =
      vtkSmartPointer<vtkCMBModelOmicronMeshInputWriterClient>::New();
    meshInputWriter->SetFileName( filename.toAscii().constData() );
    meshInputWriter->SetGeometryFileName( bcsFilename.toAscii().constData() );
    meshInputWriter->SetTetGenOptions( tetgenCmds.toAscii().constData() );
    if (meshInputWriter->Operate(this->Model, this->ModelWrapper))
      {
      return true;
      }
    }
  return false;
}


//----------------------------------------------------------------------------
bool pqCMBModel::isShellTranslationPointsLoaded()
{
  vtkDiscreteModel *cmbModel = this->Model;
  vtkModelItemIterator* iter=cmbModel->NewIterator(vtkModelRegionType);
  bool isLoaded = false;
  vtkDiscreteModelRegion* regionEntity = NULL;
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    regionEntity = vtkDiscreteModelRegion::SafeDownCast(iter->GetCurrentItem());
    if(regionEntity->GetPointInside())
      {
      isLoaded = true;
      break;
      }
    }
  iter->Delete();
  return isLoaded;
}

//----------------------------------------------------------------------------
int pqCMBModel::getNumberOfModelEntitiesWithType(int itemType)
{
  vtkDiscreteModel *cmbModel = this->getModel();
  vtkModelItemIterator* iter=cmbModel->NewIterator(itemType);
  vtkModelEntity* modelEntity = NULL;
  int numEntities = 0;
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    modelEntity = vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    if(modelEntity)
      {
      numEntities++;
      }
    }
  iter->Delete();
  return numEntities;
}

//----------------------------------------------------------------------------
int pqCMBModel::loadReaderSource(
    const QString& filename, pqPipelineSource* inputsource)
{
  QFileInfo fileInfo(filename);
  if (fileInfo.suffix().toLower() == "cmb")// || fileInfo.suffix().toLower() == "3dm")
    {
    return this->loadModelFile(filename);
    }

  if(!this->initModel())
    {
    return 0;
    }

  pqWaitCursor cursor;
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  QPointer<pqPipelineSource> source = inputsource;
  QString readerType(source->getProxy()->GetXMLName() );


  vtkSmartPointer<vtkCMBModelBuilderClient> BuilderOperator =
    vtkSmartPointer<vtkCMBModelBuilderClient>::New();

  /*
   *Note: Unless otherwise stated all these properties are required,
   * and missing one will cause segfaults or other forms of unexpected termination
   *
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Requirements pqCMBModel makes about input data if Not a Map,Moab or Geom.
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *  - Cell based int array called Region
   *
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * GeomReader 3D Files:
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *   - Cell based int array called Material
   *   - Cell based int array called Region,
   *      which should have same values as material array.
   *      This dependency is from vtkMasterPolyDataNormals, and
   *      MergeDuplicateCells
   *
   *
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * 2D NON MAP models when passed to BuilderOperator->Operate (MAP files below)
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * - FieldData vtkIdTypeArray called Vertices required to process as a 2d mesh.
   *    This field data must be the length of the number of model verts
   *
   * - FieldData vtkStringArray called "Edge Names"
   *    This field data must be the length of edges in the 2d mesh, and stores
   *    the name of each each. This is required.
   *
   * - FieldData vtkStringArray called "Loop Names"
   *    This field data must be the length of loop in the 2d mesh, and stores
   *    the name of each each. This is required.
   *
   *    The names in the Loop Names directly determine the outer and inner
   *    loops of a model. The name at index 0 is the first outer loop, all
   *    following names are inner loops, intill the Region value at the start
   *    of that lookud up loop array is different, which makes that the next
   *    regions outer loop. Iterate like this over all loop names
   *
   *  For each name in Loop Names we need a FieldData vtkIdTypeArray,
   *    which has the same name as the value in the Loop Names array.
   *    Each of these arrays is packed as:
   *      First IdType is Region of the loop
   *      Second IdType is Unused/Unkown
   *      Edge 1 Point Id 1
   *      Edge 1 Point Id 2
   *      Edge 1 Direction
   *      Edge 2 Point Id 1
   *      Edge 2 Point Id 2
   *      Edge 2 Direction
   *      ... and so on for the rest of the edges
   *
   *
   * - Cell Data int array called Region which marks the polys/regions i think.
   *
   * - Cell based int array called "Original Region", I have no clue
   *    what this represents compared to the Region array
   *
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * 3D models when passed to BuilderOperator->Operate,
   *  over and above GeomReader 3D Files requirements ( see above )
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *
   * - Field Data array called ModelFaceRegionsMap required to process as 3d Mesh
   *    Currently, the MergeDuplicateCells and CompleteShells filters add this field tag
   *
   * - Field Data array called FileName which has to store the volume file,
   *    so we can use that as the analysis grid
   *
   * - Cell Data called modelfaceids which stores the model faces, and is used
   *    to generate the region to model face mapping
   *
   * - Cells must have normals generated by the reader, or by using MasterPolyDataNormals
   *
   * - Cell based IdTypeArray called OrigCellIds, which refers to the original
   *    volume element that the shell triangle/quad came from
   *
   * - Cell based charArray called CellFaceIds which stores the face of the
   *    original cell that this shell cell comes from
   *
   * - Optional Point based vtkIdTypeArray called vtkOriginalPointIds
   *    which labels the each point in the shell with the id of the
   *    point from the original volume input data
   *
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Presumptions about 2d MAP models when passed to BuilderOperator->Operate,
   * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   *
   *
   *  Many, haven't had time to list all the propteries required to be on
   *  the data set
   *
   *Other Notes:
   *
   * vtkDataSetRegionSurfaceFilter will remove the vtkOriginalPointIds
   * array from its output if it doesn't produce any geometry, causing
   * a bad error message, rather than having a zero length array
   */


  bool hasBoundaryEdges = false, regionIdentifiersModified = false;
  const bool isCMBGeom2DReader = readerType == "CMBGeometry2DReader";
  const bool isCMBGeomReader = readerType == "CMBGeometryReader";
  const bool isMapReader = readerType == "CmbMapReader";
  const bool isMoabReader = readerType == "CmbMoabSolidReader";

  if (!isCMBGeom2DReader && !isCMBGeomReader && !isMapReader && !isMoabReader)
    {
    QPointer<pqPipelineSource> dsSurface = builder->createFilter("filters",
        "DataSetRegionSurfaceFilter", source);
    pqSMAdaptor::setElementProperty(
        dsSurface->getProxy()->GetProperty("RegionArrayName"),
        vtkCMBParserBase::GetShellTagName());
    //pqSMAdaptor::setElementProperty(
    //  dsSurface->getProxy()->GetProperty("PassThroughPointIds"), 1);
    dsSurface->getProxy()->UpdateVTKObjects();
    source = dsSurface;
    }
  else if ( isCMBGeomReader )
    {
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      source->getProxy() );
    vtkSMPropertyHelper(smSource, "PrepNonClosedSurfaceForModelCreation").Set(true);
    smSource->UpdateVTKObjects();
    smSource->UpdatePipeline();
    smSource->UpdatePropertyInformation();
    hasBoundaryEdges = pqSMAdaptor::getElementProperty(
      smSource->GetProperty("HasBoundaryEdges")).toBool();
    regionIdentifiersModified = pqSMAdaptor::getElementProperty(
      smSource->GetProperty("RegionIdentifiersModified")).toBool();
    }

  bool buildResult;
  vtkSMSourceProxy *masterPolyDataNormals = NULL, *CmbMeshMap = NULL, *mergeDuplicateCells = NULL;
  vtkSMSourceProxy* geom2D = NULL;
  if (fileInfo.suffix().toLower() == "poly" || fileInfo.suffix().toLower() == "smesh")
    {
    // for now skip master normals and merge duplicate
    buildResult = BuilderOperator->Operate(this->Model, this->ModelWrapper,
        source->getProxy());
    }
  else if(isMoabReader)
    {
    buildResult = BuilderOperator->Operate(this->Model, this->ModelWrapper,
        source->getProxy());
    }
  // For now shapefiles have their own model builder. This should become
  // a generic model builder for 2D models.
  else if (isCMBGeom2DReader)
    {
    vtkNew<vtkCMBModelBuilder2DClient> Builder2DOperator;
    buildResult = Builder2DOperator->Operate(
      this->Model, this->ModelWrapper, source->getProxy(),
      /*cleanVerts:*/ 0);
    }
  //For now map files have their own model builder FIXME later
  else if(fileInfo.suffix().toLower() == "map")
    {
    vtkSmartPointer<vtkCMBMapToCMBModelClient> BuilderOperatorMap =
      vtkSmartPointer<vtkCMBMapToCMBModelClient>::New();

    CmbMeshMap = vtkSMSourceProxy::SafeDownCast(
        vtkSMProxyManager::GetProxyManager()->NewProxy("filters", "CmbTriangleMesher"));
    pqSMAdaptor::setInputProperty(CmbMeshMap->GetProperty("Input"), source->getProxy(), 0);
    pqSMAdaptor::setElementProperty(CmbMeshMap->GetProperty("PreserveEdgesAndNodes"),true);

    CmbMeshMap->UpdateVTKObjects();
    buildResult = BuilderOperatorMap->Operate(this->Model, this->ModelWrapper,
        CmbMeshMap);
    }
  else
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();

    if (hasBoundaryEdges)
      {
      buildResult = BuilderOperator->Operate(this->Model, this->ModelWrapper,
        source->getProxy());
      }
    else
      {
      // master polydata normals (make sure each region has normals pointing out)
      masterPolyDataNormals = vtkSMSourceProxy::SafeDownCast(
          pxm->NewProxy("filters", "MasterPolyDataNormals"));
      pqSMAdaptor::setInputProperty(masterPolyDataNormals->GetProperty("Input"),
          source->getProxy(), 0);
      if(!masterPolyDataNormals)
        {
        return 0;
        }
      masterPolyDataNormals->UpdateVTKObjects();

      //Merge duplicate cells filter
      mergeDuplicateCells = vtkSMSourceProxy::SafeDownCast(
          pxm->NewProxy("filters", "MergeDuplicateCells"));
      if(!mergeDuplicateCells)
        {
        return 0;
        }

      pqSMAdaptor::setInputProperty(mergeDuplicateCells->GetProperty("Input"),
          masterPolyDataNormals, 0);
      pqSMAdaptor::setElementProperty(
          mergeDuplicateCells->GetProperty("RegionArrayName"),
          vtkCMBParserBase::GetShellTagName());
      pqSMAdaptor::setElementProperty(
          mergeDuplicateCells->GetProperty("ModelFaceArrayName"),
          vtkCMBParserBase::GetModelFaceTagName());

      mergeDuplicateCells->UpdateVTKObjects();
      buildResult = BuilderOperator->Operate(this->Model, this->ModelWrapper,
        mergeDuplicateCells);
      }
    }

  if(buildResult)
    {
    if(this->showModel())
      {
      this->CurrentModelFileName = filename;
      this->setupClientModel();
      this->saveModelState();
      pqServer* server = source->getServer();

      // Add this to the list of recent server resources ...
      pqServerResource resource = server->getResource();
      resource.setPath(filename);
      resource.addData("readergroup", source->getProxy()->GetXMLGroup());
      resource.addData("reader", source->getProxy()->GetXMLName());
      core->recentlyUsedResources().add(resource);
      core->recentlyUsedResources().save(*core->settings());

      if (masterPolyDataNormals)
        {
        masterPolyDataNormals->Delete();
        mergeDuplicateCells->Delete();
        }
      if(CmbMeshMap)
        {
        CmbMeshMap->Delete();
        }
      if (geom2D)
        {
        geom2D->Delete();
        }
/*
      // If original region array exist, we should use those region Ids to
      // create materials (See vtkCMBModelBuilder::ProcessAs2DMesh),
      // Therefore, we don't really need this step anymore.
      if (regionIdentifiersModified)
        {
        QString filters = "GMS 2D Mesh (*.2dm *.3dm)";
        pqFileDialog file_dialog(
          this->ActiveServer,
          0, tr("Save Modified 2dm/3dm Mesh for Analysis"), QString(), filters);
        file_dialog.setObjectName("FileSaveDialog");
        file_dialog.setFileMode(pqFileDialog::AnyFile);
        QApplication::restoreOverrideCursor();
        if (file_dialog.exec() == QDialog::Accepted)
          {
          QStringList files = file_dialog.getSelectedFiles();
          if (files.size() > 0)
            {
            pqWaitCursor newcursor;
            pqPipelineSource *meshWriter = builder->createFilter(
              "writers", "GMSMesh2DWriter", source);
            vtkSMPropertyHelper(meshWriter->getProxy(), "FileName").Set(
              files[0].toAscii().constData() );
            meshWriter->getProxy()->UpdateVTKObjects();
            vtkSMSourceProxy::SafeDownCast(meshWriter->getProxy())->UpdatePipeline();
            builder->destroy(meshWriter);
            }
          }
        }
*/
      return 1;
      }
    else
      {
      this->clearClientModel();
      }
    }

  if (masterPolyDataNormals)
    {
    masterPolyDataNormals->Delete();
    mergeDuplicateCells->Delete();
    }
  if( CmbMeshMap )
    {
    CmbMeshMap->Delete();
    }
  if (geom2D)
    {
    geom2D->Delete();
    }
  return 0;
}

//----------------------------------------------------------------------------
bool pqCMBModel::createRectangleModel(
  double* boundingBox, int xResolution, int yResolution)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

  // first we create a set a source and set of filters to create our rectangle
  vtkSmartPointer<vtkSMProxy> planeSource(manager->NewProxy("sources", "PlaneSource"));
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Origin"), 0, boundingBox[0]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Origin"), 1, boundingBox[1]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Origin"), 2, 0);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point1"), 0, boundingBox[2]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point1"), 1, boundingBox[1]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point1"), 2, 0);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point2"), 0, boundingBox[0]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point2"), 1, boundingBox[3]);
  pqSMAdaptor::setMultipleElementProperty(
    planeSource->GetProperty("Point2"), 2, 0);

  pqSMAdaptor::setElementProperty(planeSource->GetProperty("XResolution"),xResolution);
  pqSMAdaptor::setElementProperty(planeSource->GetProperty("YResolution"),yResolution);
  planeSource->UpdateVTKObjects();

  return this->createSimpleModel(planeSource);
}

//----------------------------------------------------------------------------
bool pqCMBModel::createEllipseModel(double* values, int resolution)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

  // first we create a set a source and set of filters to create our
  // ellipse shape
  // create a "circle" -- the source outputs degenerate quads
  vtkSmartPointer<vtkSMProxy> diskSource(manager->NewProxy("sources", "DiskSource"));
  pqSMAdaptor::setElementProperty(diskSource->GetProperty("InnerRadius"),0);
  pqSMAdaptor::setElementProperty(diskSource->GetProperty("OuterRadius"),1);
  pqSMAdaptor::setElementProperty(diskSource->GetProperty("RadialResolution"),1);
  pqSMAdaptor::setElementProperty(diskSource->GetProperty("CircumferentialResolution"),resolution);
  diskSource->UpdateVTKObjects();
  // we don't need to update the pipeline yet -- we'll let the operator do that

  // transform the circle to an ellipse
  vtkSmartPointer<vtkSMProxy> transformation(manager->NewProxy("extended_sources", "Transform3"));
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Translate"), 0, values[0]);
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Translate"), 1, values[1]);
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Translate"), 2, 0);
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Scale"), 0, values[2]);
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Scale"), 1, values[3]);
  pqSMAdaptor::setMultipleElementProperty(transformation->GetProperty("Scale"), 2, 0);
  transformation->UpdateVTKObjects();
  vtkSmartPointer<vtkSMProxy> transformFilter(manager->NewProxy("filters", "TransformFilter"));
  pqSMAdaptor::setProxyProperty(transformFilter->GetProperty("Transform"),transformation);
  pqSMAdaptor::setInputProperty(transformFilter->GetProperty("Input"), diskSource, 0);
  transformFilter->UpdateVTKObjects();

  return this->createSimpleModel(transformFilter);
}

//----------------------------------------------------------------------------
bool pqCMBModel::createSimpleModel(vtkSMProxy* inputFilter)
{
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

  vtkSmartPointer<vtkSMOperatorProxy> operatorProxy(
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "GenerateSimpleModelOperator")));
  if(!operatorProxy)
    {
    QMessageBox::warning(NULL, "Failure to create operator",
      "Unable to create GenerateSimpleModelOperator proxy.");
    return false;
    }

  operatorProxy->UpdateVTKObjects();
  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(operatorProxy) << "Operate"
          << VTKOBJECT(this->getModelWrapper())
          << VTKOBJECT(inputFilter)
          << 1
          << vtkClientServerStream::End;

  this->ModelWrapper->GetSession()->ExecuteStream(this->ModelWrapper->GetLocation(), stream);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));
  operatorProxy->UpdatePropertyInformation();

  if(operateSucceeded->GetElement(0))
    {
    operatorProxy->UpdatePropertyInformation();
    }
  else
    {
    QMessageBox::warning(NULL, "Failure of server operator",
      "Server side create rectangle model failed.");
    }
  bool success = vtkCMBModelBuilderClient::UpdateClientModel(
    this->getModel(), this->getModelWrapper());

  if(success && this->showModel())
    {
    this->CurrentModelFileName.clear();
    this->setupClientModel();
    this->saveModelState();
    }

  operatorProxy->Delete();
  operatorProxy = 0;
  return true;
}

//----------------------------------------------------------------------------
vtkIdTypeArray* pqCMBModel::getGrowSelectionIds(
    vtkPVSelectionInformation* selInfo)
{
  vtkSMSourceProxy* masterSelSource = vtkSMSourceProxy::SafeDownCast(
      this->CurrentGrowSelectionSource->getProxy());

  masterSelSource->GatherInformation(selInfo);

  vtkIdTypeArray* masterSelIDs = NULL;
  if(selInfo && selInfo->GetSelection() &&
      selInfo->GetSelection()->GetNumberOfNodes())
    {
    masterSelIDs = vtkIdTypeArray::SafeDownCast(
        selInfo->GetSelection()->GetNode(0)->GetSelectionList());
    }

  return masterSelIDs;
}

//----------------------------------------------------------------------------
pqCMBModelFace* pqCMBModel::createNewFace(vtkIdType faceId)
{
  pqCMBModelFace* cmbFace = NULL;
  vtkDiscreteModelFace* faceEntity = vtkDiscreteModelFace::SafeDownCast(
      this->Model->GetModelEntity(vtkModelFaceType, faceId));
  if(faceEntity && !this->FaceIDToFaceMap.contains(faceId))
    {
    cmbFace = pqCMBModelFace::createObject(
        this->ModelWrapper, faceEntity,
        this->ActiveServer, this->RenderView, false);
    if(cmbFace)
      {
      this->FaceIDToFaceMap.insert(faceId, cmbFace);
      }
    }
  return cmbFace;
}

//----------------------------------------------------------------------------
pqCMBModelEdge* pqCMBModel::createNewEdge(vtkIdType edgeId)
{
  pqCMBModelEdge* cmbEdge = NULL;
  vtkDiscreteModelEdge* edgeEntity = vtkDiscreteModelEdge::SafeDownCast(
      this->Model->GetModelEntity(vtkModelEdgeType, edgeId));
  if(edgeEntity && !this->EdgeIDToEdgeMap.contains(edgeId))
    {
    cmbEdge = pqCMBModelEdge::createObject(
        this->ModelWrapper, edgeEntity,
        this->ActiveServer, this->RenderView, false);
    if(cmbEdge)
      {
//      this->synchronizeModelRepTransformation(cmbEdge->getRepresentation());
      this->EdgeIDToEdgeMap.insert(edgeId, cmbEdge);
      }
    }
  return cmbEdge;
}

//----------------------------------------------------------------------------
pqCMBModelVertex* pqCMBModel::createNewVertex(vtkIdType VertexId)
{
  pqCMBModelVertex* cmbVertex = NULL;
  vtkDiscreteModelVertex* VertexEntity = vtkDiscreteModelVertex::SafeDownCast(
      this->Model->GetModelEntity(vtkModelVertexType, VertexId));
  if(VertexEntity && !this->VertexIDToVertexMap.contains(VertexId))
    {
    cmbVertex = pqCMBModelVertex::createObject(
        this->ModelWrapper, VertexEntity,
        this->ActiveServer, this->RenderView, false);
    if(cmbVertex)
      {
//      this->synchronizeModelRepTransformation(cmbVertex->getRepresentation());
      this->VertexIDToVertexMap.insert(VertexId, cmbVertex);
      }
    }
  return cmbVertex;
}

//-----------------------------------------------------------------------------
void pqCMBModel::setModelEdgesResolution(QList<vtkIdType> edges, int res)
{
  for(int i=0; i<edges.count(); i++)
    {
    this->setModelEdgeResolution(edges.value(i), res, false);
    }
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::setModelEdgeResolution(vtkIdType id, int res, bool rerender)
{
  if(this->EdgeID2RepMap.keys().contains(id))
    {
    pqCMBFloatingEdge* entity = qobject_cast<pqCMBFloatingEdge*>(
      this->EdgeID2RepMap[id]);
    if(entity)
      {
      entity->setLineResolution(res, this->Model, this->ModelWrapper);
      }

    if(rerender)
      {
      this->RenderView->render();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::changeModelEdgeVisibility(vtkIdType id, int visible)
{
  if(this->EdgeID2RepMap.keys().contains(id))
    {
    if(!visible)
      {
      this->EdgeID2RepMap[id]->setHighlight(0);
      }
    this->EdgeID2RepMap[id]->setVisibility(visible);
    this->RenderView->render();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::highlightModelEdges(QList<vtkIdType> sels)
{
  vtkIdType id;
  for(int i=0; i<sels.count(); i++)
    {
    id = sels.value(i);
    if(this->EdgeID2RepMap.keys().contains(id))
      {
      this->EdgeID2RepMap[id]->setHighlight(1);
      }
    }

  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::clearAllModelEdgesHighlights(bool rerender)
{
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = this->EdgeID2RepMap.begin();
    mapIter != this->EdgeID2RepMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entity->setHighlight(0);
    }

  if(rerender)
    {
    this->RenderView->render();
    }
}

//-----------------------------------------------------------------------------
QList<vtkIdType> pqCMBModel::getModelEntitiesFromBCSs(
    QList<vtkIdType> bcNodes)
{
  int enumEnType =
    this->getModelDimension() == 2 ? vtkModelEdgeType : vtkModelFaceType;

  vtkDiscreteModel *cmbModel = this->Model;
  QList<vtkIdType> faceIds;
  vtkIdType faceId;
  vtkDiscreteModelEntityGroup* bcsEntity = NULL;
  for(int i=0; i< bcNodes.count(); i++)
    {
    bcsEntity = vtkDiscreteModelEntityGroup::SafeDownCast(
        cmbModel->GetModelEntity(vtkDiscreteModelEntityGroupType, bcNodes.value(i)));
    if(bcsEntity)
      {
      vtkModelItemIterator* iterFace=bcsEntity->NewIterator(enumEnType);
      for(iterFace->Begin();!iterFace->IsAtEnd();iterFace->Next())
        {
        vtkModelEntity* entity =
          vtkModelEntity::SafeDownCast(iterFace->GetCurrentItem());
        if(entity)
          {
          faceId = entity->GetUniquePersistentId();
          if(!faceIds.contains(faceId))
            {
            faceIds.append(faceId);
            }
          }
        }
      iterFace->Delete();
      }
    }
  return faceIds;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBModel::getMasterPolyProvider()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  if(!this->MasterPolyProvider)
    {
    this->MasterPolyProvider = builder->createSource(
        "CMBModelGroup", "PolyDataProvider", this->ActiveServer);
    if(this->MasterPolyProvider)
      {
      vtkSMProxyProperty* proxyproperty =
        vtkSMProxyProperty::SafeDownCast(
            this->MasterPolyProvider->getProxy()->GetProperty("ModelWrapper"));
      proxyproperty->AddProxy(this->ModelWrapper);
      pqSMAdaptor::setElementProperty(this->MasterPolyProvider->getProxy()->
          GetProperty("ItemType"),  vtkModelType);
      this->MasterPolyProvider->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast(
          this->MasterPolyProvider->getProxy() )->UpdatePipeline();
      }
    }
  return this->MasterPolyProvider;
}

//-----------------------------------------------------------------------------
const QMap<vtkIdType, pqCMBModelEntity*>& pqCMBModel::GetCurrentModelEntityMap()
{
  return ((this->getModelDimension()==2) ? (this->EdgeIDToEdgeMap) : (this->FaceIDToFaceMap));
}

//-----------------------------------------------------------------------------
bool pqCMBModel::shouldVertexBeVisible(vtkModelVertex* vtx)
{
  if(!vtx)
    {
    return false;
    }

  vtkModelItemIterator* iterEdge=
    vtx->NewAdjacentModelEdgeIterator();

  for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
    {
    vtkDiscreteModelEdge* edgeEntity =
      vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
    if(edgeEntity && this->EdgeIDToEdgeMap.contains(edgeEntity->GetUniquePersistentId()))
      {
      if(this->EdgeIDToEdgeMap[edgeEntity->GetUniquePersistentId()]->getVisibility())
        {
        iterEdge->Delete();
        return true;
        }
      }
    }
  iterEdge->Delete();
  return false;
}

//----------------------------------------------------------------------------
void pqCMBModel::convertSelectedNodes()
{
  QMap<vtkIdType, pqCMBModelEntity*> vtxMap =
    this->Get2DVertexID2VertexMap();
  QMap<vtkIdType, pqCMBModelEntity*> arcMap =
    this->Get2DEdgeID2EdgeMap();
  if (!(vtxMap.count()+arcMap.count()))
    {
    return;
    }
  vtkSMSourceProxy* modelSource = vtkSMSourceProxy::SafeDownCast(
    this->modelSource()->getProxy());
  vtkSMSourceProxy* selSource = modelSource->GetSelectionInput(0);
  if(!selSource)
    {
    return;
    }

  vtkSMPropertyHelper selIDs(selSource, "IDs");
  unsigned int cc, flatIdx;
  vtkIdType masterPId, entId;
  unsigned int count = selIDs.GetNumberOfElements();
  vtkNew<vtkPVModelGeometryInformation> pdSourceInfo;
  modelSource->GatherInformation(pdSourceInfo.GetPointer());
  QList<vtkIdType> selVTXs;
  QList<vtkIdType> selPointIds;
  // [composite_index, process_id, pointid]
  // go through the vertex first
  for (cc=0; cc < (count/3); cc++)
    {
    flatIdx = selIDs.GetAsInt(3*cc);
    entId = pdSourceInfo->GetModelEntityId(flatIdx);
    masterPId = selIDs.GetAsIdType(3*cc+2);
    if(entId>=0 && masterPId >= 0 &&
     !selPointIds.contains(masterPId) && vtxMap.contains(entId))
      {
      selVTXs.push_back(entId);
      selPointIds.push_back(masterPId);
      }
    }

  // now go through the edges
  QMap< vtkIdType, QList<vtkIdType> > selArcs;
  // [composite_index, process_id, pointid]
  for (cc=0; cc < (count/3); cc++)
    {
    flatIdx = selIDs.GetAsInt(3*cc);
    entId = pdSourceInfo->GetModelEntityId(flatIdx);
    masterPId = selIDs.GetAsIdType(3*cc+2);

    if(entId>=0 && masterPId >= 0 &&
      !selPointIds.contains(masterPId) && arcMap.contains(entId))
      {
      selArcs[entId].push_back(masterPId);
      selPointIds.push_back(masterPId);
      }
    }

  pqWaitCursor cursor;

  // Promotion is always being processed before Demotion
  if (selVTXs.count()>0)
    {
    this->convertSelectedEndNodes(selVTXs);
    }
  else if (selArcs.count()>0)
    {
    this->splitSelectedEdgeNodes(selArcs);
    }
  modelSource->SetSelectionInput(0, NULL, 0);
  this->updateModelRepresentation();
}

//-----------------------------------------------------------------------------
bool pqCMBModel::splitSelectedEdgeNodes(
 const QMap< vtkIdType, QList<vtkIdType> >& selArcs)
{
  QMap< vtkIdType, QList<vtkIdType> > SplitEdgeToEdgesMap;
  QList <vtkModelEntity*> AdjacentFaces;
  vtkIdType selEdgeId;
  bool res = true;
  QMap< vtkIdType, QList<vtkIdType> >::const_iterator arcIt;
  for(arcIt=selArcs.begin(); arcIt!=selArcs.end(); ++arcIt)
    {
    selEdgeId = arcIt.key();
    if(this->EdgeIDToEdgeMap.contains(selEdgeId))
      {
      if(pqCMBModelEdge* cmbEdge = qobject_cast<pqCMBModelEdge*>(
            this->EdgeIDToEdgeMap[selEdgeId]))
        {
        QList<vtkIdType> newVTKArcs;
        QList<vtkIdType> newVTKVTXs;
        res = cmbEdge->splitSelectedNodes(arcIt.value(), this,
            this->ModelWrapper,newVTKArcs, newVTKVTXs);
        if(!res)
          {
          continue;
          }

        // create new edges
        QList<vtkIdType> newCMBArcs;
        this->createCMBArcs(newVTKArcs, newVTKVTXs, newCMBArcs);

        this->EdgeIDToEdgeMap[selEdgeId]->updateMeshEntity(this);
        if(newCMBArcs.count()>0)
          {
          SplitEdgeToEdgesMap.insert(selEdgeId, newCMBArcs);
          }
        // Get the adjacent faces for updating their mesh
        cmbEdge->GetAdjacentModelFaces(AdjacentFaces);
        }
      }
    }

  // Update the adjacent face mesh
  this->updateAdjacentFacesMesh(AdjacentFaces);

  if(res && SplitEdgeToEdgesMap.count()>0)
    {
    this->updateModelSource();
    this->updateModelDataInfo();
    emit this->entitiesSplit(SplitEdgeToEdgesMap);
    }

  return res;
}

//-----------------------------------------------------------------------------
void pqCMBModel::createCMBArcs(QList<vtkIdType>& newVTKArcs,
  QList<vtkIdType>& newVTKVTXs, QList<vtkIdType>& newCMBArcs)
{
  vtkIdType edgeId;
  for(int j=0;j<newVTKArcs.count();j++)
    {
    edgeId = newVTKArcs.value(j);
    if(this->EdgeIDToEdgeMap.contains(edgeId))
      {
      continue;
      }
    if(pqCMBModelEdge* NewEdge = this->createNewEdge(edgeId))
      {
      if(this->ShowEdgePoints)
        {
        NewEdge->setEdgePointsVisibility(this->ShowEdgePoints);
        }

      newCMBArcs.append(edgeId);
      if(this->MeshClient)
        {
        vtkCMBModelEntityMesh*  entMesh = this->MeshClient->GetModelEntityMesh(
            vtkModelGeometricEntity::SafeDownCast(NewEdge->getModelEntity()));
        NewEdge->setMeshEntity(entMesh);
        NewEdge->updateMeshEntity(this);
        }
      }
    }
  // create new vertex
  for(int v=0;v<newVTKVTXs.count();v++)
    {
    vtkIdType newVTXId = newVTKVTXs.value(v);
    if(!this->VertexIDToVertexMap.contains(newVTXId))
      {
      this->createNewVertex(newVTXId);
      }
    }
}

//-----------------------------------------------------------------------------
bool pqCMBModel::convertSelectedEndNodes(
    const QList<vtkIdType>& selVTXs)
{
  if(!selVTXs.count() || !this->VertexIDToVertexMap.count())
    {
    return false;
    }

  QList <vtkModelEntity*> AdjacentFaces;
  for(int id=0;id<selVTXs.count();id++)
    {
    int vtxId = selVTXs.value(id);
    vtkDiscreteModelVertex* vtx = vtkDiscreteModelVertex::SafeDownCast(
        this->VertexIDToVertexMap[vtxId]->getModelEntity());
    if(!vtx)
      {
      continue;
      }

    QList<vtkIdType> selEdgeIds;
    vtkModelItemIterator* iterEdge = vtx->NewAdjacentModelEdgeIterator();
    for(iterEdge->Begin();!iterEdge->IsAtEnd();iterEdge->Next())
      {
      vtkDiscreteModelEdge* edgeEntity =
        vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if(edgeEntity && this->EdgeIDToEdgeMap.contains(edgeEntity->GetUniquePersistentId()))
        {
        selEdgeIds.append(edgeEntity->GetUniquePersistentId());
        }
      }
    iterEdge->Delete();
    if(selEdgeIds.count() != 2)
      {
      continue;
      }

    vtkIdType toEdgeId = selEdgeIds.value(0);
    vtkIdType fromEdgeId = selEdgeIds.value(1);
    if(fromEdgeId < toEdgeId)
      {
      toEdgeId = fromEdgeId;
      fromEdgeId = selEdgeIds.value(0);
      }

    /*

    === NEW SMTK MODEL OPERATOR EXAMPLE ===

     smtkOperatorPtr op = this->ModelWrapper->createOperator("merge");
     op->setParameter(Parameter("target", toEdgeId));
     op->setParameter(Parameter("source", fromEdgeId));
     op->setParameter(Parameter("lower-dimensional entities", vtxId)); // vtxId could also be UUIDArray

     if (!op->AbleToOperate()) continue

     pqActiveObjects::instance().setActiveSource(NULL);
     if (op->Operate())
       {
       // ....
       }
     */
    vtkSmartPointer<vtkMergeOperatorClient> MergeOperator =
      vtkSmartPointer<vtkMergeOperatorClient>::New();
    MergeOperator->SetTargetId(toEdgeId);
    MergeOperator->SetSourceId(fromEdgeId);
    MergeOperator->AddLowerDimensionalId(vtxId);
    if(MergeOperator->AbleToOperate(this->getModel()) == 0)
      {
      continue;
      }
    pqActiveObjects::instance().setActiveSource(NULL);

    if(MergeOperator->Operate(this->getModel(), this->ModelWrapper))
      {
      // update vertex the mapping
      delete this->VertexIDToVertexMap[vtxId];
      this->VertexIDToVertexMap.remove(vtxId);

      QList<vtkIdType> fromEdgeIds;
      fromEdgeIds.append(fromEdgeId);
      this->updateModelDataInfo();
      emit this->entitiesMerged(toEdgeId, fromEdgeIds);
      }

    this->updateModelEntityRepresentation(toEdgeId);
    this->EdgeIDToEdgeMap[toEdgeId]->updateMeshEntity(
      this);

    // Get the adjacent faces for updating their mesh
    pqCMBModelEdge* cmbEdge = qobject_cast<pqCMBModelEdge*>(
      this->EdgeIDToEdgeMap[toEdgeId]);
    cmbEdge->GetAdjacentModelFaces(AdjacentFaces);
    }

  // Update the adjacent face mesh
  this->updateAdjacentFacesMesh(AdjacentFaces);

  // HACK: trigger the vtkSMHardwareSelector to clear buffer. Otherwise,
  // the SelectSurface Points sometime will not work.
  this->RenderView->getRenderViewProxy()->GetActiveCamera()->Modified();
  this->RenderView->forceRender();

  return true;
}

//-----------------------------------------------------------------------------
int pqCMBModel::getFaceColorMode()
{
  return this->FaceColorMode;
}

//-----------------------------------------------------------------------------
void pqCMBModel::setFaceColorMode(int mode)
{
  if(this->FaceColorMode != mode)
  {
    this->FaceColorMode = mode;
    this->onLookupTableModified();
  }
}

//-----------------------------------------------------------------------------
int pqCMBModel::getEdgeColorMode()
{
  return this->EdgeColorMode;
}

//-----------------------------------------------------------------------------
void pqCMBModel::setEdgeColorMode(int mode)
{
  if(this->EdgeColorMode != mode)
    {
    this->EdgeColorMode = mode;
    this->onLookupTableModified();
    }
}


//-----------------------------------------------------------------------------
void pqCMBModel::colorByAttributes(
  QList<vtkIdType>& coloredEntIds, const QString& strAttDef, bool byDomain)
{
  if(!this->AttManager || strAttDef.isEmpty())
    {
    return;
    }
  std::vector<smtk::attribute::AttributePtr> result;
  this->AttManager->findDefinitionAttributes(
    strAttDef.toStdString(), result);
  smtk::model::ModelPtr refModel = this->AttManager->refModel();
//  QMap<smtk::attribute::AttributePtr, QColor> attColorMap;
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    const double* rgba = (*it)->color();
    QColor attColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
//    attColorMap.insert((*it), attColor);

    // figure out model entities associated with this attribute
    QList<vtkIdType> assignedIds;
    int numAstItems = static_cast<int>((*it)->numberOfAssociatedEntities());
    std::set<smtk::model::ItemPtr>::const_iterator ait = (*it)->associatedEntities();
    for(int ai=0; ai<numAstItems; ++ait, ++ai)
      {
      if(!assignedIds.contains((*ait)->id()))
        {
        assignedIds.append((*ait)->id());
        }
      }
    // color the model
    this->colorAttributeEntities(assignedIds, attColor, coloredEntIds, byDomain);
    }
}
//-----------------------------------------------------------------------------
int pqCMBModel::getColorEdgeDomainMode()
{
  return this->EdgeDomainColorMode;
}

//-----------------------------------------------------------------------------
void pqCMBModel::setColorEdgeDomainMode(int mode)
{
  if(this->EdgeDomainColorMode != mode)
  {
    this->EdgeDomainColorMode = mode;
    this->onLookupTableModified();
  }
}
//-----------------------------------------------------------------------------
void pqCMBModel::convertLatLong(bool convert)
{
  if(this->GetCurrentModelEntityMap().count()==0 )
    {
    return;
    }

  this->LatLongTransformOperator->SetConvertFromLatLongToXYZ(convert);
  if(this->LatLongTransformOperator->Operate(
    this->Model, this->ModelWrapper))
    {
    this->updateModel();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::setupMesh(vtkCMBMeshClient* meshClient)
{
  if(  this->MeshClient == meshClient)
    {
    return;
    }
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetCurrentModelEntityMap();
  if(!meshClient || meshClient->GetModel() != this->getModel()
    || entityMap.count() ==0)
    {
    return;
    }
  this->MeshClient = meshClient;
  pqCMBModelEntity* entity;
  vtkCMBModelEntityMesh* entMesh;
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entMesh = meshClient->GetModelEntityMesh(
      vtkModelGeometricEntity::SafeDownCast(entity->getModelEntity()));
    entity->setMeshEntity(entMesh);
    }
  // polygon meshes
  for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
    mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entMesh = meshClient->GetModelEntityMesh(
      vtkModelGeometricEntity::SafeDownCast(entity->getModelEntity()));
    entity->setMeshEntity(entMesh);
    }
}
//-----------------------------------------------------------------------------
void pqCMBModel::updateEdgeMesh()
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->Get2DEdgeID2EdgeMap();
  if(entityMap.count() ==0)
    {
    return;
    }
  if(!this->MeshClient)
    {
    return;
    }
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entity->updateMeshEntity(this);
    this->synchronizeModelRepTransformation(entity->getMeshRepresentation());
   }
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateFaceMesh()
{
  if(!this->MeshClient)
    {
    return;
    }
  pqCMBModelEntity* entity;
  // polygon meshes
  for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
    mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entity->updateMeshEntity(this);
    this->synchronizeModelRepTransformation(entity->getMeshRepresentation());
    }
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateSelectedFaceMesh(vtkCollection* selFaces)
{
  if(!selFaces || selFaces->GetNumberOfItems()==0)
    {
    return;
    }

  vtkIdType faceId;
  for(int i=0; i<selFaces->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCMBModelEntityMesh> faceMesh =
      vtkCMBModelEntityMesh::SafeDownCast(
      selFaces->GetItemAsObject(i));
    if(faceMesh)
      {
      faceId = faceMesh->GetModelGeometricEntity()->GetUniquePersistentId();
      this->FaceIDToFaceMap[faceId]->updateMeshEntity(this);
      }
    }
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateSelectedEdgeMesh(vtkCollection* selEdges)
{
  if(!selEdges || selEdges->GetNumberOfItems()==0)
    {
    return;
    }
  vtkIdType edgeId;
  for(int i=0; i<selEdges->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCMBModelEntityMesh> edgeMesh =
      vtkCMBModelEntityMesh::SafeDownCast(
      selEdges->GetItemAsObject(i));
    if(edgeMesh)
      {
      edgeId = edgeMesh->GetModelGeometricEntity()->GetUniquePersistentId();
      this->EdgeIDToEdgeMap[edgeId]->updateMeshEntity(this);
      }
    }
  this->RenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateAdjacentFacesMesh(QList<vtkModelEntity*> AdjacentFaces)
{
  if(!this->MeshClient)
    {
    return;
    }
  for(int i=0; i<AdjacentFaces.count(); i++)
    {
    vtkModelEntity* faceEntity = AdjacentFaces.value(i);
    vtkIdType faceId = faceEntity->GetUniquePersistentId();
    if(this->FaceIDToFaceMap.contains(faceId))
      {
      this->FaceIDToFaceMap[faceId]->updateMeshEntity(this);
      }
    }
  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::setFaceMeshVisibility(int visible)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->GetFaceIDToFaceMap();
  if(entityMap.count() ==0)
    {
    return;
    }

  // polygon meshes
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
    mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entity->setEntityMeshVisibility(visible);
    }

  this->RenderView->render();
}
//----------------------------------------------------------------------------
void pqCMBModel::setEdgeMeshVisibility(int visible)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->Get2DEdgeID2EdgeMap();
  if(entityMap.count() ==0)
    {
    return;
    }

  // edge meshes
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    entity->setEntityMeshVisibility(visible);
    }

  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::setEdgePointsVisibility(int visible)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->EdgeIDToEdgeMap;
  if(entityMap.count() ==0 || !this->ModelRepresentation)
    {
    return;
    }

  pqCMBModelEdge* entEdge;
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    entEdge = qobject_cast<pqCMBModelEdge*>(mapIter.value());
    entEdge->setEdgePointsVisibility(visible);
    }
  vtkSMProxy *repProxy = this->ModelRepresentation->getProxy();
  vtkSMPropertyHelper(repProxy, "ShowEdgePoints").Set(visible);
  this->updateModelRepresentation();
  this->ShowEdgePoints = visible;
  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::setEdgeMeshPointsVisibility(int visible)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
    this->EdgeIDToEdgeMap;
  if(entityMap.count() ==0)
    {
    return;
    }

  pqCMBModelEdge* entEdge;
  for (EntityMapIterator mapIter = entityMap.begin();
    mapIter != entityMap.end(); ++mapIter)
    {
    entEdge = qobject_cast<pqCMBModelEdge*>(mapIter.value());
    entEdge->setEdgeMeshPointsVisibility(visible);
    }

  this->RenderView->render();
}

//----------------------------------------------------------------------------
void pqCMBModel::setModelFacesPickable(bool pickable)
{
  for (EntityMapIterator mapIter = this->FaceIDToFaceMap.begin();
       mapIter != this->FaceIDToFaceMap.end(); ++mapIter)
    {
    pqCMBModelFace* cmbFace = qobject_cast<pqCMBModelFace*>(
      mapIter.value());
    /*
    if(cmbFace->getRepresentation())
      {
      pqSMAdaptor::setElementProperty(
        cmbFace->getRepresentation()->getProxy()->GetProperty("Pickable"), pickable);
      cmbFace->getRepresentation()->getProxy()->UpdateVTKObjects();
      }
      */
    if(cmbFace)
      {
      cmbFace->setPickable(pickable ? 1 : 0);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::applyBathymetry(pqPipelineSource* bSource, double eleRadius,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow,
  bool applyOnlyToVisibleMesh)
{
  if (this->BathymetrySource && this->BathymetrySource != bSource)
    {
    this->removeBathymetry();
    }

  if ((!this->BathymetrySource && bSource) || this->BathymetrySource)
      /*(this->BathymetrySource && this->BathElevationRadious != eleRadius))*/
    {
    pqWaitCursor cursor;
    this->BathymetrySource = bSource;
    vtkSMSourceProxy::SafeDownCast(this->BathymetrySource
      ->getProxy())->UpdatePipeline();

    this->BathHighestZValue=eleHigh;
    this->BathUseHighestZValue=useHighLimit;
    this->BathLowestZValue=eleLow;
    this->BathUseLowestZValue=useLowLimit;
    this->BathElevationRadious = eleRadius;
    this->BathOnlyOnVisibleMesh = applyOnlyToVisibleMesh;

    if(!applyOnlyToVisibleMesh)
      {
      // Add in the ApplyBathymetry Filter, by default it is just
      // a ShallowCopy (NoOP)
      pqPipelineSource* bathymetryFilter = this->getModelBathymetryFilter();
      vtkSMSourceProxy* smFilter = vtkSMSourceProxy::SafeDownCast(
        bathymetryFilter->getProxy());
      vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
        smFilter->GetProperty("Source"));
      pSource->RemoveAllProxies();
      pSource->AddProxy(this->BathymetrySource->getProxy());
      vtkSMPropertyHelper(smFilter, "ElevationRadius").Set(eleRadius);
      vtkSMPropertyHelper(smFilter, "HighestZValue").Set(eleHigh);
      vtkSMPropertyHelper(smFilter, "UseHighestZValue").Set(useHighLimit);
      vtkSMPropertyHelper(smFilter, "LowestZValue").Set(eleLow);
      vtkSMPropertyHelper(smFilter, "UseLowestZValue").Set(useLowLimit);

      vtkSMPropertyHelper(smFilter, "NoOP").Set(0);
      smFilter->UpdateVTKObjects();
      smFilter->UpdatePipeline();
      if(!this->BathyMetryOperatorProxy)
        {
        vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
        this->BathyMetryOperatorProxy = vtkSMOperatorProxy::SafeDownCast(
          manager->NewProxy("CMBModelGroup", "CmbModelPointsOperator"));
        if(!this->BathyMetryOperatorProxy)
          {
          QMessageBox::critical(NULL,
            tr("Create BathyMetryOperator Error"),
            tr("The BathyMetryOperator proxy can not be created."),
            QMessageBox::Ok);
          return;
          }
        this->BathyMetryOperatorProxy->SetLocation(smFilter->GetLocation());
        }

      vtkSMProxyProperty* ptsSource = vtkSMProxyProperty::SafeDownCast(
        this->BathyMetryOperatorProxy->GetProperty("ModelPointsInput"));
      ptsSource->RemoveAllProxies();
      ptsSource->AddProxy(smFilter);
      this->BathyMetryOperatorProxy->Operate(
        this->getModel(), this->ModelWrapper);
      // check to see if the operation succeeded on the server
      vtkSMIntVectorProperty* OperateSucceeded =
        vtkSMIntVectorProperty::SafeDownCast(
        this->BathyMetryOperatorProxy->GetProperty("OperateSucceeded"));
      this->BathyMetryOperatorProxy->UpdatePropertyInformation();
      int Succeeded = OperateSucceeded->GetElement(0);
      if(!Succeeded)
        {
        QMessageBox::critical(NULL,
          tr("Apply Bathymetry Error"),
          tr("The BathyMetryOperator failed on the server."),
          QMessageBox::Ok);
        return;
        }
      }

    this->applyMeshBathymetry(this->BathymetrySource->getProxy(), eleRadius,
      useHighLimit, eleHigh, useLowLimit, eleLow, applyOnlyToVisibleMesh);

    this->updateModel();
    this->updateEdgeMesh();
    this->updateFaceMesh();
    }
}
//-----------------------------------------------------------------------------
void pqCMBModel::removeBathymetry(bool update)
{
  if (this->BathymetrySource)
    {
    this->BathymetrySource = 0;
    }
  if(this->ModelBathymetryFilter && this->BathyMetryOperatorProxy)
    {
    vtkSMSourceProxy* smFilter = vtkSMSourceProxy::SafeDownCast(
      this->ModelBathymetryFilter->getProxy());
    vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
      smFilter->GetProperty("Source"));
    pSource->RemoveAllProxies();
    vtkSMPropertyHelper(smFilter, "NoOP").Set(1);
    vtkSMPropertyHelper(smFilter, "ElevationRadius").Set(1.0);
    this->BathElevationRadious = 0.0;
    this->BathHighestZValue=0.0;
    this->BathUseHighestZValue=false;
    this->BathLowestZValue=0.0;
    this->BathUseLowestZValue=false;

    smFilter->UpdateVTKObjects();
    smFilter->UpdatePipeline();
    vtkSMProxyProperty* ptsSource = vtkSMProxyProperty::SafeDownCast(
      this->BathyMetryOperatorProxy->GetProperty("ModelPointsInput"));
    ptsSource->RemoveAllProxies();
    ptsSource->AddProxy(smFilter);
    this->BathyMetryOperatorProxy->Operate(
      this->getModel(), this->ModelWrapper);
    // check to see if the operation succeeded on the server
    vtkSMIntVectorProperty* OperateSucceeded =
      vtkSMIntVectorProperty::SafeDownCast(
      this->BathyMetryOperatorProxy->GetProperty("OperateSucceeded"));
    this->BathyMetryOperatorProxy->UpdatePropertyInformation();
    int Succeeded = OperateSucceeded->GetElement(0);
    if(!Succeeded)
      {
      QMessageBox::critical(NULL,
        tr("Remove Bathymetry Error"),
        tr("The BathyMetryOperator failed on the server."),
        QMessageBox::Ok);
      return;
      }
    this->applyMeshBathymetry(NULL, 0, false, 0, false, 0);
    if(update)
      {
      smFilter->UpdateVTKObjects();
      smFilter->UpdatePipeline();
      this->updateModel();
      this->updateEdgeMesh();
      this->updateFaceMesh();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateMeshBathymetry()
{
  if(this->BathymetrySource)
    {
    this->applyMeshBathymetry(this->BathymetrySource->getProxy(),
      this->BathElevationRadious, this->BathUseHighestZValue,
      this->BathHighestZValue, this->BathUseLowestZValue,
      this->BathLowestZValue, this->BathOnlyOnVisibleMesh);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModel::applyMeshBathymetry(
  vtkSMProxy* bathymetrySource, double eleRaius,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow,
  bool applyOnlyToVisibleMesh)
{
  QMap<vtkIdType, pqCMBModelEntity*> entityMap =
  this->GetCurrentModelEntityMap();
  if(entityMap.count() ==0)
    {
    return;
    }
  if(!this->MeshClient)
    {
    return;
    }
  this->applyBathymetryToMeshEntities(
    bathymetrySource, eleRaius, this->EdgeIDToEdgeMap,
    useHighLimit, eleHigh, useLowLimit, eleLow, applyOnlyToVisibleMesh);
  this->applyBathymetryToMeshEntities(
    bathymetrySource, eleRaius, this->FaceIDToFaceMap,
    useHighLimit, eleHigh, useLowLimit, eleLow, applyOnlyToVisibleMesh);
}
//-----------------------------------------------------------------------------
void pqCMBModel::applyBathymetryToMeshEntities(
  vtkSMProxy* bathymetrySource, double eleRaius,
  QMap<vtkIdType, pqCMBModelEntity*> & entityMap,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow,
  bool applyOnlyToVisibleMesh)
{
  pqCMBModelEntity* entity;
  for (EntityMapIterator mapIter = entityMap.begin();
       mapIter != entityMap.end(); ++mapIter)
    {
    entity = mapIter.value();
    if(!entity->getMeshEntity() ||
       !entity->getMeshEntity()->IsModelEntityMeshed() ||
       (applyOnlyToVisibleMesh && !entity->meshRepresentationVisible()))
      {
      continue;
      }
    if(bathymetrySource)
      {
      entity->applyMeshBathymetry(bathymetrySource, eleRaius,
        useHighLimit, eleHigh, useLowLimit, eleLow);
      }
    else
      {
      entity->unapplyMeshBathymetry();
      }
    }
}
//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBModel::getModelBathymetryFilter()
{
  if(!this->ModelBathymetryFilter)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->ModelBathymetryFilter =
      builder->createFilter("filters",
      "CmbApplyBathymetry",  this->MasterPolyProvider);
    vtkSMPropertyHelper(this->ModelBathymetryFilter->getProxy(),
      "NoOP").Set(1);
    this->BathElevationRadious = 0.0;
    this->BathHighestZValue=0.0;
    this->BathUseHighestZValue=false;
    this->BathLowestZValue=0.0;
    this->BathUseLowestZValue=false;
    }
  return this->ModelBathymetryFilter;
}

//-----------------------------------------------------------------------------
void pqCMBModel::synchronizeModelRepTransformation(
  pqDataRepresentation* targetRep)
{
  if(!this->ModelRepresentation || !targetRep)
    {
    return;
    }
  vtkSMProxy *srcProxy = this->ModelRepresentation->getProxy();
  srcProxy->UpdateVTKObjects();
  vtkSMRepresentationProxy *repProxy =
    vtkSMRepresentationProxy::SafeDownCast(targetRep->getProxy());
  repProxy->GetProperty("Origin")->Copy(srcProxy->GetProperty("Origin"));
  repProxy->GetProperty("Position")->Copy(srcProxy->GetProperty("Position"));
  repProxy->GetProperty("Orientation")->Copy(srcProxy->GetProperty("Orientation"));
  repProxy->GetProperty("Scale")->Copy(srcProxy->GetProperty("Scale"));

  repProxy->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBModel::updateModelDataInfo()
{
  if(this->ModelWrapper)
    {
    this->ModelWrapper->GatherInformation(
      this->ModelDataInfo.GetPointer());
    }
}
//-----------------------------------------------------------------------------
void pqCMBModel::setCurrentFaceAttributeColorInfo(
  smtk::attribute::ManagerPtr attManager, const QString& type)
{
  this->AttManager = attManager;
  this->CurrentFaceAttDefType = type;
}
//-----------------------------------------------------------------------------
void pqCMBModel::setCurrentEdgeAttributeColorInfo(
  smtk::attribute::ManagerPtr attManager, const QString& type)
{
  this->AttManager = attManager;
  this->CurrentEdgeAttDefType = type;
}
//-----------------------------------------------------------------------------
void pqCMBModel::setCurrentDomainAttributeColorInfo(
  smtk::attribute::ManagerPtr attManager, const QString& type)
{
  this->AttManager = attManager;
  this->CurrentDomainAttDefType = type;
}

//-----------------------------------------------------------------------------
void pqCMBModel::colorAttributeEntities (QList<vtkIdType>& entIds,
  QColor color, QList<vtkIdType>& coloredEntIds, bool byDomain)
{
  if(!this->ModelWrapper)
    {
    return;
    }
  vtkIdType entId;
  // Only Material/Domain or BC-group type entities can have attributes.
  // This search may slow if there are many entities in the model.
  foreach(vtkIdType entityId, entIds)
    {
    QList<vtkIdType> geoEntList;
    vtkModelEntity* entityGroup = this->Model->GetModelEntity(entityId);
    if(entityGroup->GetType() == vtkDiscreteModelEntityGroupType)
      {
      vtkDiscreteModelEntityGroup* entGroup =
        vtkDiscreteModelEntityGroup::SafeDownCast(entityGroup);
      vtkModelItemIterator* GroupEntIt = entGroup->NewModelEntityIterator();
      for(GroupEntIt->Begin();!GroupEntIt->IsAtEnd();GroupEntIt->Next())
        {
        vtkModelEntity* entity = vtkModelEntity::SafeDownCast(
          GroupEntIt->GetCurrentItem());
        entId = entity->GetUniquePersistentId();
        if(!coloredEntIds.contains(entId) && this->setEntityColor(entId, color))
          {
          coloredEntIds.append(entId);
          }
        }
      GroupEntIt->Delete();
      }
    else if(entityGroup->GetType() == vtkModelMaterialType)
      {
      vtkModelMaterial* matGroup =
        vtkModelMaterial::SafeDownCast(entityGroup);
      ModelEntityTypes entityType = vtkModelType;
      if(matGroup->GetNumberOfAssociations(vtkModelRegionType) > 0) // 3D model
        {
        entityType = vtkModelRegionType;
        }
      else if(matGroup->GetNumberOfAssociations(vtkModelFaceType) > 0) // 2D model
        {
        entityType = vtkModelFaceType;
        }
      if(entityType != vtkModelType)
        {
        vtkModelItemIterator* entities = matGroup->NewIterator(entityType);
        for(entities->Begin();!entities->IsAtEnd();entities->Next())
          {
          if(entityType == vtkModelRegionType)
            {
            vtkDiscreteModelRegion* region = vtkDiscreteModelRegion::SafeDownCast(
              entities->GetCurrentItem());
            vtkModelItemIterator* faceIt = region->NewAdjacentModelFaceIterator();;
            for(faceIt->Begin();!faceIt->IsAtEnd();faceIt->Next())
              {
              vtkModelEntity* entity = vtkModelEntity::SafeDownCast(
                faceIt->GetCurrentItem());
              entId = entity->GetUniquePersistentId();
              if(!coloredEntIds.contains(entId) && this->setEntityColor(entId, color))
                {
                coloredEntIds.append(entId);
                }
              }
            faceIt->Delete();
            }
          else // 2d edges
            {
            vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
              entities->GetCurrentItem());
            if(byDomain)
              {
              entId = face->GetUniquePersistentId();
              if(!coloredEntIds.contains(entId) && this->setEntityColor(entId, color))
                {
                coloredEntIds.append(entId);
                }
              }
            else
              {
              vtkModelItemIterator* edgeIt = face->NewAdjacentModelEdgeIterator();;
              for(edgeIt->Begin();!edgeIt->IsAtEnd();edgeIt->Next())
                {
                vtkModelEntity* entity = vtkModelEntity::SafeDownCast(
                  edgeIt->GetCurrentItem());
                entId = entity->GetUniquePersistentId();
                if(!coloredEntIds.contains(entId) && this->setEntityColor(entId, color))
                  {
                  coloredEntIds.append(entId);
                  }
                }
              edgeIt->Delete();
              }
            }
          }
        entities->Delete();
        }
      }
    }
}

//-----------------------------------------------------------------------------
bool pqCMBModel::setEntityColor (vtkIdType entId, QColor color)
{
  vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
    vtkSmartPointer<vtkModelEntityOperatorClient>::New();
  ModelEntityOperator->SetId(entId);
  ModelEntityOperator->SetRepresentationRGBA(
    color.redF(), color.greenF(), color.blueF(), color.alphaF());
  return ModelEntityOperator->Operate(
    this->Model, this->ModelWrapper);
}

//-----------------------------------------------------------------------------
void pqCMBModel::prepTexturedObject(
  pqServer * /*server*/, pqRenderView * /*view*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource* modSource = this->getMasterPolyProvider();
  if(!modSource)
    {
    QMessageBox::critical(NULL,
      tr("Apply Texture Error"),
      tr("No Model Source. Cannot apply texture."),
      QMessageBox::Ok);
    return;
    }

  this->RegisterTextureFilter =
    builder->createFilter("filters",
                          "RegisterPlanarTextureMapFilter",
                          modSource);
  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(0);
  this->RegisterTextureFilter->getProxy()->UpdateVTKObjects();

  if(!this->TextureOperatorProxy)
    {
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
    this->TextureOperatorProxy = vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "CmbModelPointsOperator"));
    if(!this->TextureOperatorProxy)
      {
      QMessageBox::critical(NULL,
        tr("Create TextureOperatorProxy Error"),
        tr("The TextureOperator proxy can not be created."),
        QMessageBox::Ok);
      return;
      }
    this->TextureOperatorProxy->SetLocation(
      this->RegisterTextureFilter->getProxy()->GetLocation());
    }

  if(this->modelRepresentation())
    {
    // regardless of object type (although primarily for LIDAR),
    // set initial point size to be 2
    vtkSMPropertyHelper(this->modelRepresentation()->getProxy(), "PointSize").Set(2);
    vtkSMPropertyHelper(this->modelRepresentation()->getProxy(), "MapScalars").Set(0);
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
      this->modelRepresentation()->getProxy(), NULL, vtkDataObject::POINT);

//    vtkSMPropertyHelper(this->modelRepresentation()->getProxy(), "ColorArrayName").Set("");
    }
}
//-----------------------------------------------------------------------------
void pqCMBModel::getRegistrationPointPair(int i,
                                                  double xy[2],
                                                  double st[2]) const
{
  int j = 4*i;
  xy[0] = this->RegistrationPoints[j++];
  xy[1] = this->RegistrationPoints[j++];
  st[0] = this->RegistrationPoints[j++];
  st[1] = this->RegistrationPoints[j++];
}
//-----------------------------------------------------------------------------
void pqCMBModel::unsetTextureMap()
{
  this->NumberOfRegistrationPoints = 0;
  this->TextureFileName = "";
  if(!this->RegisterTextureFilter)
    {
    return;
    }
  if (this->TextureImageSource)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    vtkSMPropertyHelper(this->modelRepresentation()->getProxy(),
                        "LargeTextureInput").Set(static_cast<vtkSMProxy*>(0));
    this->modelRepresentation()->getProxy()->UpdateProperty("LargeTextureInput");

    builder->destroy( this->TextureImageSource );
    this->TextureImageSource = 0;
    }

  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(0);

  this->updateModelTexture();
}
//-----------------------------------------------------------------------------
void pqCMBModel::setTextureMap(const char *filename, int numberOfRegistrationPoints,
                                       double *points)
{
  if(!this->RegisterTextureFilter || !this->TextureOperatorProxy)
    {
    this->prepTexturedObject(this->ActiveServer, this->RenderView);
    }

  if (this->hasTexture() && this->TextureImageSource &&
    this->TextureFileName.compare( filename ))
    {
    this->unsetTextureMap();
    }

  this->NumberOfRegistrationPoints = numberOfRegistrationPoints;
  int i, n = 4*numberOfRegistrationPoints;
  for (i = 0; i < n; i++)
    {
    this->RegistrationPoints[i] = points[i];
    }

  if (!this->TextureImageSource)
    {
    this->TextureImageSource = ReadTextureImage(
      pqApplicationCore::instance()->getObjectBuilder(),
      this->ActiveServer, filename);
    this->TextureFileName = filename;
    vtkSMPropertyHelper(this->modelRepresentation()->getProxy(), "LargeTextureInput").Set(
      this->TextureImageSource->getProxy() );
    this->modelRepresentation()->getProxy()->UpdateProperty("LargeTextureInput");

    vtkAlgorithm* source = vtkAlgorithm::SafeDownCast(
      this->TextureImageSource->getProxy()->GetClientSideObject());
    vtkImageData* image = vtkImageData::SafeDownCast(source->GetOutputDataObject(0));

    int extents[6];
    double ev[2];
    image->GetExtent(extents);
    ev[0] = extents[0];
    ev[1] = extents[1];
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
      "SRange").Set(ev, 2);
    ev[0] = extents[2];
    ev[1] = extents[3];
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
      "TRange").Set(ev, 2);
    }

  if (numberOfRegistrationPoints == 2)
    {
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                        "TwoPointRegistration").Set(this->RegistrationPoints, 8);
    }
  else
    {
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                        "ThreePointRegistration").Set(this->RegistrationPoints, 12);
    }

  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(1);
  this->updateModelTexture();
}

//-----------------------------------------------------------------------------
void pqCMBModel::updateModelTexture()
{
  if(!this->TextureOperatorProxy || !this->RegisterTextureFilter)
    {
    return;
    }
  this->RegisterTextureFilter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(
    this->RegisterTextureFilter->getProxy())->UpdatePipeline();

  vtkSMProxyProperty* ptsSource = vtkSMProxyProperty::SafeDownCast(
    this->TextureOperatorProxy->GetProperty("ModelPointDataInput"));
  ptsSource->RemoveAllProxies();
  ptsSource->AddProxy(this->RegisterTextureFilter->getProxy());
  this->TextureOperatorProxy->Operate(
    this->getModel(), this->ModelWrapper);
  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
    this->TextureOperatorProxy->GetProperty("OperateSucceeded"));
  this->TextureOperatorProxy->UpdatePropertyInformation();
  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    QMessageBox::critical(NULL,
      tr("Apply Texture Error"),
      tr("The TextureOperator failed on the server."),
      QMessageBox::Ok);
    return;
    }

  this->updateModelRepresentation();
}

//-----------------------------------------------------------------------------
bool pqCMBModel::setEntityShowTexture(vtkIdType faceId, int show)
{
  vtkSmartPointer<vtkModelEntityOperatorClient> ModelEntityOperator =
    vtkSmartPointer<vtkModelEntityOperatorClient>::New();
  ModelEntityOperator->SetShowTexture(show);
  ModelEntityOperator->SetItemType(vtkModelFaceType);
  ModelEntityOperator->SetId(faceId);

  bool res = ModelEntityOperator->Operate(this->Model, this->ModelWrapper);
  if(res)
    {
    this->RenderView->forceRender();
    }
  return res;
}
