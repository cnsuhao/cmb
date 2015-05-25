//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "SimBuilderMeshManager.h"

#include "vtkDataObject.h"
#include "vtkCMBMeshClient.h"
#include "vtkCMBModelEdgeMeshClient.h"
#include "vtkCMBModelFaceMeshClient.h"
#include "vtkSMProxy.h"
#include "vtkDiscreteModel.h"
#include "vtkModelEdge.h"
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkCollection.h"
#include "vtkCMBParserBase.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMStringVectorProperty.h"
#include "pqWaitCursor.h"
#include "pqColorChooserButton.h"
#include "pqDataRepresentation.h"
#include "pqSMAdaptor.h"
#include "pqPipelineSource.h"
#include "pqOutputPort.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqApplicationCore.h"

#include "ui_qtCMBPanel.h"
#include "../qtCMBPanelWidget.h"
#include "../pqCMBModel.h"
#include "../pqCMBModelEntity.h"
#include "../qtCMBModelTree.h"
#include "../pqCMBTreeItem.h"

#include <QDoubleValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QSet>
#include <QList>

//----------------------------------------------------------------------------
SimBuilderMeshManager::SimBuilderMeshManager():
  AnalysisMeshIsCurrent(false),
  CMBModel(NULL),
  UIPanel(NULL),
  MeshClient(NULL)
{
}

//----------------------------------------------------------------------------
SimBuilderMeshManager::~SimBuilderMeshManager()
{
  this->clearMesh();
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::Initialize()
{
  if(this->CMBModel)
    {
    if(this->MeshClient)
      {
      this->MeshClient->Delete();
      }
    this->MeshClient = vtkCMBMeshClient::New();
    this->MeshClient->Initialize(this->CMBModel->getModel(),
      this->CMBModel->getModelWrapper());
    this->CMBModel->setupMesh(this->MeshClient);
    this->setupUIConnection();
    }
  else
    {
    this->clearMesh();
    }
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::clearMesh()
{
  if(this->MeshClient)
    {
    this->MeshClient->Reset();
    this->MeshClient->Delete();
    this->MeshClient = NULL;
    }
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::setUIPanel(qtCMBPanelWidget* uiPanelIn)
{
  this->UIPanel = uiPanelIn;
  if(this->UIPanel)
    {
    Ui_qtCMBPanel* uiPanel = this->UIPanel->getGUIPanel();
    //if(!uiPanel->frameMeshTree->layout())
    //  {
    //  QVBoxLayout* layout = new QVBoxLayout(uiPanel->frameMeshTree);
    //  }
    uiPanel->groupBox_MeshLocal->setEnabled(0);
    QDoubleValidator *validator = new QDoubleValidator(this->UIPanel);
    uiPanel->lineEdit_MeshGlobalLen->setValidator(validator);
    uiPanel->lineEdit_MeshLocalLen->setValidator(validator);
    uiPanel->lineEdit_MeshMinAngle->setValidator(validator);
    }
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::setCMBModel(pqCMBModel* newModel)
{
  if(this->CMBModel == newModel)
    {
    return;
    }
  if(this->CMBModel && this->CMBModel != newModel)
    {
    this->clearMesh();
    }
  this->CMBModel = newModel;
  this->Initialize();
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::setModelTree(qtCMBModelTree* modelTree)
{
  this->ModelTree = modelTree;
  QObject::connect(this->ModelTree,
    SIGNAL(selectionChanged(qtCMBTree*)),
    this, SLOT(onModelSelectionChanged(qtCMBTree*)));
  QObject::connect(this->ModelTree,
    SIGNAL(meshItemChanged(QTreeWidgetItem* , int)),
    this, SLOT(onMeshItemChanged(QTreeWidgetItem* , int)));
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::clearUIConnection()
{
  Ui_qtCMBPanel* uiPanel = this->UIPanel->getGUIPanel();
  uiPanel->pushButton_MeshApply->disconnect(this);
  uiPanel->checkBoxShowFaceMesh->disconnect(this->CMBModel);
  uiPanel->checkBoxShowEdgeMesh->disconnect(this->CMBModel);
  uiPanel->checkBoxShowEdgePoints->disconnect(this->CMBModel);
  uiPanel->checkBoxShowEdgeMeshPoints->disconnect(this->CMBModel);
  uiPanel->pushButtonSaveAnalysisMesh->disconnect(this);
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::setupUIConnection()
{
  if(!this->UIPanel || !this->CMBModel || !this->MeshClient)
    {
    return;
    }
  this->clearUIConnection();
  Ui_qtCMBPanel* uiPanel = this->UIPanel->getGUIPanel();
  QObject::connect(uiPanel->pushButton_MeshApply, SIGNAL(clicked()),
    this, SLOT(onStartMeshing()));
  QObject::connect(uiPanel->checkBoxShowFaceMesh, SIGNAL(stateChanged(int)),
    this->CMBModel, SLOT(setFaceMeshVisibility(int)));
  QObject::connect(uiPanel->checkBoxShowEdgeMesh, SIGNAL(stateChanged(int)),
    this->CMBModel, SLOT(setEdgeMeshVisibility(int)));
  QObject::connect(uiPanel->checkBoxShowEdgePoints, SIGNAL(stateChanged(int)),
    this->CMBModel, SLOT(setEdgePointsVisibility(int)));
  QObject::connect(uiPanel->checkBoxShowEdgeMeshPoints, SIGNAL(stateChanged(int)),
    this->CMBModel, SLOT(setEdgeMeshPointsVisibility(int)));
  QObject::connect(uiPanel->pushButtonSaveAnalysisMesh, SIGNAL(clicked()),
    this, SLOT(exportAnalysisMesh()));
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::onStartMeshing()
{
  if(!this->UIPanel || !this->CMBModel || !this->MeshClient)
    {
    return;
    }
  pqWaitCursor cursor;
  bool updateGlobalEdge, updateLocalEdge;
  bool updateGlobalAngle, updateLocalAngle;
  bool updatedFaceLength;

  Ui_qtCMBPanel* uiPanel = this->UIPanel->getGUIPanel();

  // The updating order here is important for efficiency.
  // local edge->global edge->local angle->global angle

  // First Local Edge for selected entities
  vtkSmartPointer<vtkCollection> selEdges = vtkSmartPointer<vtkCollection>::New();
  this->ModelTree->getSelectedMeshEdges(selEdges);
  updateLocalEdge = this->MeshClient->SetLocalMeshLength(selEdges,
    uiPanel->lineEdit_MeshLocalLen->text().toDouble());

  // Then Global Edge
  updateGlobalEdge = this->MeshClient->SetGlobalLength(
    uiPanel->lineEdit_MeshGlobalLen->text().toDouble());

  // Updating Faces - Local Parameters
  vtkSmartPointer<vtkCollection> selFaces = vtkSmartPointer<vtkCollection>::New();
  this->ModelTree->getSelectedMeshFaces(selFaces);
  updatedFaceLength = this->MeshClient->SetLocalMeshLength(selFaces,
    uiPanel->lineEdit_MeshLocalLen->text().toDouble());
  updateLocalAngle = this->MeshClient->SetLocalMeshMinimumAngle(selFaces,
    uiPanel->lineEdit_MeshLocalMinAngle->text().toDouble());

  // Then Global angle
  updateGlobalAngle = this->MeshClient->SetGlobalMinimumAngle(
    uiPanel->lineEdit_MeshMinAngle->text().toDouble());

  bool meshUpdated = this->MeshClient->BuildModelEntityMeshes();
  this->AnalysisMeshIsCurrent = !meshUpdated;

  // Now update the representations
  this->MeshClient->GetServerMeshProxy()->MarkModified(NULL);
  this->MeshClient->GetServerMeshProxy()->UpdateVTKObjects();

  // global
  if(updateGlobalEdge)
    {
    this->CMBModel->updateEdgeMesh();
    this->CMBModel->updateFaceMesh();
    }
  else if(updateGlobalAngle)
    {
    this->CMBModel->updateFaceMesh();
    if(updateLocalEdge)
      {
      this->CMBModel->updateSelectedEdgeMesh(selEdges);
      }
    }
  else if(updateLocalEdge || updatedFaceLength)
    {
    this->updateLocalEdgeMesh(selEdges, selFaces);
    }
  else if(updateLocalAngle)
    {
    this->CMBModel->updateSelectedFaceMesh(selFaces);
    }

  if(updateLocalEdge)
    {
    this->ModelTree->updateEdgeMeshInfo(selEdges);
    }
  if(updatedFaceLength || updateLocalAngle)
    {
    this->ModelTree->updateFaceMeshInfo(selFaces);
    }
  this->CMBModel->updateMeshBathymetry();
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::onModelSelectionChanged(qtCMBTree* tree)
{
  Ui_qtCMBPanel* uiPanel = this->UIPanel->getGUIPanel();
  bool hasSelection = tree->getSelectedItems().count()>0;
  uiPanel->groupBox_MeshLocal->setEnabled(hasSelection);
  if(!hasSelection)
    {
    return;
    }
  // Pick the minimum length/angle to set the local mesh size GUI
  vtkSmartPointer<vtkCollection> selEdges = vtkSmartPointer<vtkCollection>::New();
  this->ModelTree->getSelectedMeshEdges(selEdges);
  vtkSmartPointer<vtkCollection> selFaces = vtkSmartPointer<vtkCollection>::New();
  this->ModelTree->getSelectedMeshFaces(selFaces);
  bool hasArea=false, hasLength=false;
  double minLength = VTK_DOUBLE_MAX;
  double minArea = VTK_DOUBLE_MAX;
  vtkIdType firstEntityId = -1;
  for(int i=0; i<selEdges->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCMBModelEdgeMesh> meshEntity =
      vtkCMBModelEdgeMesh::SafeDownCast(
      selEdges->GetItemAsObject(i));
    if(meshEntity)
      {
      double dLength = meshEntity->GetLength();
      minLength = dLength<minLength ? dLength : minLength;
      firstEntityId = meshEntity->GetModelEdge()->GetUniquePersistentId();
      hasLength = true;
      }
    }
  for(int i=0; i<selFaces->GetNumberOfItems(); i++)
    {
    vtkSmartPointer<vtkCMBModelFaceMesh> meshEntity =
      vtkCMBModelFaceMesh::SafeDownCast(
      selFaces->GetItemAsObject(i));
    if(meshEntity)
      {
      double dLength = meshEntity->GetLength();
      double dArea = meshEntity->GetMinimumAngle();
      minLength = dLength<minLength ? dLength : minLength;
      minArea = dArea<minArea ? dArea : minArea;
      hasArea = true;
      hasLength = true;
      firstEntityId = meshEntity->GetModelFace()->GetUniquePersistentId();
      }
    }

  // Updating GUI Local Mesh Parameters
  uiPanel->lineEdit_MeshLocalLen->setText(
    hasLength ? QString::number(minLength) : "");
  uiPanel->lineEdit_MeshLocalMinAngle->setText(
    hasArea ? QString::number(minArea) : "");

  pqCMBModelEntity* modEnt = NULL;
  if(hasArea && firstEntityId>=0)
    {
    modEnt = this->CMBModel->GetFaceIDToFaceMap()[firstEntityId];
    }
  else if(hasLength && firstEntityId >=0)
    {
    modEnt = this->CMBModel->Get2DEdgeID2EdgeMap()[firstEntityId];
    }
  if(modEnt && modEnt->getMeshRepresentation())
    {
    double color[4];
    modEnt->getMeshRepresentationColor(color);
    QColor qcolor;
    qcolor.setRgbF(color[0], color[1], color[2], color[3]);
    QVariant curRType = pqSMAdaptor::getEnumerationProperty(
      modEnt->getMeshRepresentation()->getProxy()->GetProperty("Representation"));
    int idx = uiPanel->comboBoxFaceMeshRep->findText(curRType.toString());
    uiPanel->comboBoxFaceMeshRep->setCurrentIndex(idx);
    uiPanel->groupBox_MeshLocal->findChild<pqColorChooserButton*>("meshEntityColorButton")
      ->setChosenColor(qcolor);
    }
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::onMeshItemChanged(
  QTreeWidgetItem* item, int col)
{
  pqCMBTreeItem* cmbItem = static_cast<pqCMBTreeItem*>(item);
  if(!cmbItem)
    {
    return;
    }
  bool ok = false;
  double val = item->text(col).toDouble(&ok);
  if(!ok)
    {
    return;
    }

  pqWaitCursor wait;

  vtkSmartPointer<vtkCollection> selEntities = vtkSmartPointer<vtkCollection>::New();
  pqCMBModelEntity* modelEntity = cmbItem->getModelObject();
  selEntities->AddItem(modelEntity->getMeshEntity());
  int nodeType = modelEntity->getModelEntity()->GetType();
  if(nodeType == vtkModelFaceType && col == MTree_MESH_MIN_ANGLE_COL)
    {
    if(this->MeshClient->SetLocalMeshMinimumAngle(selEntities,val))
      {
      this->CMBModel->updateSelectedFaceMesh(selEntities);
      this->ModelTree->updateFaceMeshInfo(selEntities);
      }
    }
  else if(nodeType == vtkModelFaceType && col == MTree_MESH_LENGTH_COL)
    {
    if(this->MeshClient->SetLocalMeshLength(selEntities,val))
      {
      this->CMBModel->updateSelectedFaceMesh(selEntities);
      this->ModelTree->updateFaceMeshInfo(selEntities);
      }
    }
  else if(nodeType == vtkModelEdgeType && col == MTree_MESH_LENGTH_COL)
    {
    if(this->MeshClient->SetLocalMeshLength(selEntities, val))
      {
      this->updateLocalEdgeMesh(selEntities);
      this->ModelTree->updateEdgeMeshInfo(selEntities);
      }
    }
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::updateLocalEdgeMesh(
  vtkCollection* selEdges, vtkCollection* selFaces)
{
  QSet<vtkCMBModelEdgeMeshClient*>edgesToBeUpdated;
  QSet<vtkCMBModelFaceMeshClient*>facesToBeUpdated;

  // Traverse all the faces that have been changed explicitly and record
  // their edges
  vtkModelFace *modelFace;
  vtkCMBModelFaceMeshClient *meshFace;
  vtkModelEdge *modelEdge;
  vtkCMBModelEdgeMeshClient *meshEdge;
  if(selFaces)
    {
    selFaces->InitTraversal();
    while((meshFace = vtkCMBModelFaceMeshClient::SafeDownCast(selFaces->GetNextItemAsObject())))
      {
      facesToBeUpdated.insert(meshFace);
      modelFace = vtkModelFace::SafeDownCast(meshFace->GetModelGeometricEntity());
      vtkModelItemIterator *faceEdges = modelFace->NewAdjacentModelEdgeIterator();
      for(faceEdges->Begin(); !faceEdges->IsAtEnd(); faceEdges->Next())
        {
        modelEdge = vtkModelEdge::SafeDownCast(faceEdges->GetCurrentItem());
        meshEdge =
          vtkCMBModelEdgeMeshClient::SafeDownCast(this->MeshClient->GetModelEntityMesh(modelEdge));
        edgesToBeUpdated.insert(meshEdge);
        }
      faceEdges->Delete();
      }
    }

  // Add the selected edges
  for(int i=0; i<selEdges->GetNumberOfItems(); i++)
    {
    meshEdge =
      vtkCMBModelEdgeMeshClient::SafeDownCast(selEdges->GetItemAsObject(i));
    edgesToBeUpdated.insert(meshEdge);
    }

  // Now traverse the set of edges and add all their faces to face set
  foreach (meshEdge, edgesToBeUpdated)
    {
    modelEdge = vtkModelEdge::SafeDownCast(meshEdge->GetModelGeometricEntity());
    vtkModelItemIterator* faces = modelEdge->NewAdjacentModelFaceIterator();
    for(faces->Begin();!faces->IsAtEnd();faces->Next())
      {
      modelFace = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
      meshFace =
        vtkCMBModelFaceMeshClient::SafeDownCast(this->MeshClient->GetModelEntityMesh(modelFace));
      facesToBeUpdated.insert(meshFace);
      }
    faces->Delete();
    }

  // OK Now we need to convert these sets back to collections
  vtkCollection* updateFaces = vtkCollection::New();
  vtkCollection* updateEdges = vtkCollection::New();

  foreach (meshEdge, edgesToBeUpdated)
    {
    updateEdges->AddItem(meshEdge);
    }

  foreach(meshFace, facesToBeUpdated)
    {
    updateFaces->AddItem(meshFace);
    }

  this->CMBModel->updateSelectedEdgeMesh(updateEdges);
  this->CMBModel->updateSelectedFaceMesh(updateFaces);
  updateFaces->Delete();
  updateEdges->Delete();
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::getModelFaceMeshes(QList<pqOutputPort*>& meshes)
{
  QMap< vtkIdType, pqCMBModelEntity* > entityMap =
    this->CMBModel->GetFaceIDToFaceMap();
  QMap< vtkIdType, pqCMBModelEntity* >::iterator mapIter=entityMap.begin();
  vtkCMBModelEntityMesh* meshEntity;
  pqCMBModelEntity* modelEntity;
  for (; mapIter != entityMap.end(); ++mapIter)
    {
    modelEntity = mapIter.value();
    meshEntity = modelEntity->getMeshEntity();
    if(meshEntity && meshEntity->IsModelEntityMeshed() &&
      modelEntity->getMeshSource())
      {
      meshes.push_back(modelEntity->getMeshSource()->getOutputPort(0));
      }
    }
}

//----------------------------------------------------------------------------
bool SimBuilderMeshManager::hasMesh()
{
  QMap< vtkIdType, pqCMBModelEntity* > entityMap =
    this->CMBModel->GetFaceIDToFaceMap();
  QMap< vtkIdType, pqCMBModelEntity* >::iterator mapIter=entityMap.begin();
  vtkCMBModelEntityMesh* meshEntity;
  pqCMBModelEntity* modelEntity;
  for (; mapIter != entityMap.end(); ++mapIter)
    {
    modelEntity = mapIter.value();
    meshEntity = modelEntity->getMeshEntity();
    if(meshEntity && meshEntity->IsModelEntityMeshed() &&
      modelEntity->getMeshSource())
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool SimBuilderMeshManager::analysisMeshIsCurrent()
{
  return AnalysisMeshIsCurrent;
}

//----------------------------------------------------------------------------
bool SimBuilderMeshManager::useAsAnalysisMesh()
{
  //first method
  //now prompt the user if they want to use this saved mesh as the analysis mesh
  QMessageBox msgUseAsAnalysisMesh;
  msgUseAsAnalysisMesh.setText("Would you like to use the generated mesh as your analysis mesh?");
  msgUseAsAnalysisMesh.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgUseAsAnalysisMesh.exec();
  return ( msgUseAsAnalysisMesh.result() == QMessageBox::Yes);
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::saveAsAnalysisMesh()
{
  QList<pqOutputPort*> inputs;
  this->getModelFaceMeshes(inputs);

  if (inputs.size() == 0)
    {
    QMessageBox::warning(NULL, "No Face Meshes",
      "There are no model face meshes to export!");
    return;
    }
  this->saveAnalysisMesh(inputs, true);
}
//----------------------------------------------------------------------------
void SimBuilderMeshManager::saveAnalysisMesh(
  QList<pqOutputPort*>& inputs, bool /*isAnalysisMesh*/)
{
  QMap<QString, QList<pqOutputPort*> > namedInputs;
  namedInputs["Input"] = inputs;

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource *appendSolids = builder->createFilter( "filters",
    "AppendSolids", namedInputs, this->CMBModel->getMasterPolyProvider()->getServer());
  pqSMAdaptor::setElementProperty(appendSolids->getProxy()->GetProperty("RegionArrayName"),
    vtkCMBParserBase::GetShellTagName());
  appendSolids->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast( appendSolids->getProxy() )->UpdatePipeline();

  //second method
  // get output filename
  QString filters = "GMS 2D Mesh (*.2dm *.3dm)";
  pqFileDialog file_dialog(
    this->CMBModel->getMasterPolyProvider()->getServer(),
    0, tr("Save Analysis Mesh"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  QApplication::restoreOverrideCursor();
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    this->AnalysisMeshIsCurrent =
      this->MeshClient->BuildModelMeshRepresentation(
      files[0].toStdString().c_str(),true,appendSolids->getProxy());
    }
  builder->destroy(appendSolids);
}

//----------------------------------------------------------------------------
void SimBuilderMeshManager::exportAnalysisMesh()
{
  QList<pqOutputPort*> inputs;
  this->getModelFaceMeshes(inputs);

  if (inputs.size() == 0)
    {
    QMessageBox::warning(NULL, "No Face Meshes",
      "There are no model face meshes to export!");
    return;
    }
  bool isAnalysisMesh = this->useAsAnalysisMesh();
  this->saveAnalysisMesh(inputs, isAnalysisMesh);
}
