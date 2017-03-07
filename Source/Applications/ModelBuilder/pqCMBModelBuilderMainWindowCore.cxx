//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBModelBuilderMainWindowCore.h"
#include "pqCMBAppCommonConfig.h" // for CMB_TEST_DATA_ROOT

#include "pqPVApplicationCore.h"
#include "pqFileDialogModel.h"
#include "pqLinksManager.h"
#include "pqObjectBuilder.h"
#include "pqOutputWindow.h"
#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqDataRepresentation.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqScalarBarWidget.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqSettings.h"
#include "pqSMAdaptor.h"
#include "pqFileDialog.h"
#include "pqSetName.h"
#include "pqWaitCursor.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProxyProperty.h"

#include "vtkToolkits.h"
#include "vtksys/Process.h"
#include "vtkCollection.h"
#include "vtkSMProxyLink.h"

///////////////////////////////////////////////////////////////////////////
#include "SimBuilder/SimBuilderCore.h"
#include "SimBuilder/pqSimBuilderUIManager.h"
#include "SimBuilder/pqSMTKUIHelper.h"

#include "pqActiveObjects.h"
#include "pqCMBContextMenuHelper.h"
#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBEnumPropertyWidget.h"
#include "pqCMBFileExtensions.h"
#include "pqCMBLoadDataReaction.h"
#include "pqCMBModelBuilderOptions.h"
#include "pqCMBModelManager.h"
#include "pqCMBPreviewDialog.h"
#include "pqCMBProcessWidget.h"
#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"
#include "pqCMBRubberBandHelper.h"
#include "pqCMBSceneReader.h"
#include "pqCMBSceneTree.h"
#include "pqModelBuilderViewContextMenuBehavior.h"
#include "pqMultiBlockInspectorPanel.h"
#include "pqSMTKMeshInfo.h"
#include "pqSMTKMeshPanel.h"
#include "pqSMTKModelInfo.h"
#include "pqSMTKModelPanel.h"
#include "qtCMBApplicationOptionsDialog.h"
#include "qtCMBProgressWidget.h"
#include "qtCMBSceneObjectFilterDialog.h"
#include "vtkPVSceneGenFileInformation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Operator.h"
#include "smtk/extension/qt/qtAttributeDisplay.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtSelectionManager.h"
#include "smtk/extension/paraview/appcomponents/pqPluginSMTKViewBehavior.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QTreeWidget>
#include <QMainWindow>

#include "vtkSMModelManagerProxy.h"
#include "vtkPVSMTKModelInformation.h"
#include "vtkPVSMTKMeshInformation.h"
#include <algorithm>
#include "assert.h"

using namespace smtk::attribute;
using namespace smtk::extension;
using namespace smtk;

///////////////////////////////////////////////////////////////////////////
#include "vtkPVPlugin.h"
PV_PLUGIN_IMPORT_INIT(ModelBridge_Plugin)
// PV_PLUGIN_IMPORT_INIT(SimBuilderMesh_Plugin)
// PV_PLUGIN_IMPORT_INIT(SMTKModel_Plugin)

///////////////////////////////////////////////////////////////////////////
// pqCMBModelBuilderMainWindowCore::vtkInternal

/// Private implementation details for pqCMBModelBuilderMainWindowCore
class pqCMBModelBuilderMainWindowCore::vtkInternal
{
  public:
    vtkInternal(QWidget* /*parent*/) :
      InCreateSource(false),
      SimBuilder(0),
      SceneGeoTree(0)
      {
      this->SelectByMode = 0;
      this->AppOptions = new pqCMBModelBuilderOptions();
      }

    ~vtkInternal()
      {
      if(this->smtkModelManager)
          this->smtkModelManager->clear();
      delete this->smtkModelManager;

      if(this->SimBuilder)
        {
        delete this->SimBuilder;
        }
      }

    bool InCreateSource;

    QPointer<pqCMBEnumPropertyWidget> RepresentationWidget;
    QPointer<QComboBox> SelectByBox;
    QPointer<QToolBar> ColorByAttToolbar;

    int SelectByMode;

    ///////////////////////////////////////////////////////////////////////////
    // The Model related variables
    pqCMBRubberBandHelper cmbRenderViewSelectionHelper;

    ///////////////////////////////////////////////////////////////////////////
    // The SimBuilder variables

    SimBuilderCore* SimBuilder;
    QPointer<pqCMBSceneTree> SceneGeoTree;
    QString CurrentSceneFileName;

    QPointer<pqCMBModelBuilderOptions> AppOptions;

    QPointer<pqSMTKModelPanel> ModelDock;
    QPointer<pqSMTKMeshPanel> MeshDock;
    QPointer<pqCMBModelManager> smtkModelManager;
    QPointer<pqModelBuilderViewContextMenuBehavior> ViewContextBehavior;
    QPointer<smtk::extension::qtAttributeDisplay> AttributeVisWidget;
    QPointer<QAction> AttVisAction;
    QPointer<pqPluginSMTKViewBehavior> smtkViewBehavior;
};

///////////////////////////////////////////////////////////////////////////
// pqCMBModelBuilderMainWindowCore

pqCMBModelBuilderMainWindowCore::pqCMBModelBuilderMainWindowCore(QWidget* parent_widget) :
  pqCMBCommonMainWindowCore(parent_widget), Internal(new vtkInternal(parent_widget))
{
  this->buildRenderWindowContextMenuBehavior(parent_widget);
  this->ProgramKey = qtCMBProjectServerManager::ModelBuilder;
  this->setObjectName("CMBMainWindowCore");

  // Set up connection with selection helpers for all views.
  this->cmbRenderViewSelectionHelper()->setView(this->activeRenderView());

  pqApplicationCore* core = pqApplicationCore::instance();
  core->setUndoStack(NULL);

  this->Internal->smtkViewBehavior = new pqPluginSMTKViewBehavior(this);
}

//-----------------------------------------------------------------------------
pqCMBModelBuilderMainWindowCore::~pqCMBModelBuilderMainWindowCore()
{
  //pqActiveView::instance().setCurrent(0);
  delete Internal;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setupColorByAttributeToolbar(QToolBar* toolbar)
{
  this->Internal->ColorByAttToolbar = toolbar;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setRubberSelectionMode(int mode)
{
  if (mode == this->Internal->SelectByMode)
    {
    return;
    }
  this->Internal->SelectByMode = mode;
  emit this->rubberSelectionModeChanged();
}

//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::getRubberSelectionMode()
{
  return this->Internal->SelectByMode;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::clearpqCMBSceneTree()
{
  if(this->Internal->SceneGeoTree)
    {
    this->Internal->SceneGeoTree->empty();
    this->Internal->CurrentSceneFileName.clear();
    if(this->Internal->SimBuilder)
      {
      this->Internal->SimBuilder->updateSimBuilder(this->Internal->SceneGeoTree);
      }
    }
}
//-----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::isSceneEmpty()
{
  return (this->Internal->SceneGeoTree &&
          !(this->Internal->SceneGeoTree->isEmpty()));
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setpqCMBSceneTree(pqCMBSceneTree * tree)
{
  if(this->Internal->SceneGeoTree == tree)
    {
    return;
    }
  this->Internal->SceneGeoTree = tree;
  this->Internal->SceneGeoTree->setCurrentView(this->activeRenderView());
  this->Internal->SceneGeoTree->setCurrentServer(this->getActiveServer());
  QObject::connect(this->Internal->SceneGeoTree,
      SIGNAL(requestSceneUpdate()),
      this,
      SLOT(requestRender()));
  if(this->Internal->SimBuilder)
    {
    this->Internal->SimBuilder->updateSimBuilder(this->Internal->SceneGeoTree);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onLoadScene()
{
  QString filters = "SceneGen files(*.sg)";

  bool cancelled;
  QStringList files;
  pqCMBLoadDataReaction::loadData(cancelled, files, filters);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onFileOpen(const QStringList& files)
{
  // This handles the special extensions of "cmb" and "bc", which do not
  // have readers. These extension are handled by model operators.
  if(files.size()>0 && this->loadModelFile(files[0]))
    {
    // Add this to the list of recent server resources ...
    pqCMBRecentlyUsedResourceLoaderImplementatation::
        addModelFileToRecentResources(this->getActiveServer(), files[0]);
    }
}

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::startNewSession(
  const std::string& sessionName)
{
  return this->startNewSession(sessionName,
    this->Internal->AppOptions->createDefaultSessionModel(),
    false);
}

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::startNewSession(
  const std::string& sessionName,
  bool createDefaultModel,
  bool useExistingSession)
{
  return this->modelManager()->startNewSession(
    sessionName, createDefaultModel, useExistingSession);
}

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::zoomToSelection()
{
  typedef std::vector<double> boundingBox;// follow vtk's bbox rule

  boundingBox finalBBox;
  for (int i = 0; i < 6;  i++)
  {
    double tmp = (i%2 == 1) ? -DBL_MAX : DBL_MAX;
    finalBBox.push_back(tmp);
  }

  smtk::extension::qtSelectionManager* selManager = this->modelPanel()->selectionManager();
  smtk::model::ManagerPtr modelMgr = this->modelManager()->managerProxy()->modelManager();
  smtk::common::UUIDs selEntities;
  smtk::mesh::MeshSets selMeshSets;
  selManager->getSelectedEntities(selEntities);
  selManager->getSelectedMeshes(selMeshSets);


  // loop through seleced entities get bounding Box
  for (smtk::common::UUIDs::iterator selEnt = selEntities.begin(); selEnt != selEntities.end();++selEnt)
  {
    smtk::model::EntityRef entRef(modelMgr,*selEnt);
    finalBBox = entRef.unionBoundingBox(entRef.boundingBox(), finalBBox);
  }

  // loop through seleced meshes get bounding Box
  for (smtk::mesh::MeshSets::iterator selMesh = selMeshSets.begin();
       selMesh !=  selMeshSets.end(); selMesh++)
  {
    smtk::common::UUIDArray UUIDArray = selMesh->modelEntityIds();
    for (auto uuid = UUIDArray.begin(); uuid != UUIDArray.end(); ++uuid)
    {
      smtk::model::EntityRef entRef(modelMgr,*uuid);
      finalBBox = entRef.unionBoundingBox(entRef.boundingBox(), finalBBox);
    }
  }

  // check whether we have invalid bbox
  for  (int i = 0; i < 6; i++)
  {
    if (finalBBox[i] == -DBL_MAX || finalBBox[i] == DBL_MAX)
      {return false;}
  }

  vtkSMRenderViewProxy* rm = this->activeRenderView()->getRenderViewProxy();
  rm->ResetCamera(finalBBox[0],finalBBox[1],finalBBox[2],finalBBox[3],
      finalBBox[4],finalBBox[5]);
  this->activeRenderView()->render();
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::processSceneInfo(const QString& filename,
    pqPipelineSource* source)
{
  //if(!this->Internal->SceneGeoTree->isEmpty())
  //  {
  //  if(filename.compare(this->Internal->CurrentSceneFileName,
  //    Qt::CaseInsensitive) == 0)
  //    {
  //    return;
  //    }
  //  else
  //    {
  //    this->clearpqCMBSceneTree();
  //    }
  //  }

  // force read
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();
  vtkNew<vtkPVSceneGenFileInformation> info;
  source->getProxy()->GatherInformation(info.GetPointer());

  QStringList objTypes;
  pqCMBSceneReader typeReader;
  typeReader.getUserDefinedObjectTypes(
      info->GetFileContents(), objTypes);

  if (objTypes.count())
    {
    qtCMBSceneObjectFilterDialog FilterDialog(this->parentWidget());
    double modBounds[6] = {0,-1,0,-1,0,-1};
//    if(this->Internal->CMBModel->hasGeometryEntity())
//      {
//      this->Internal->CMBModel->getModelBounds(modBounds);
//      }
    FilterDialog.setSceneFile(filename.toAscii().constData());
    FilterDialog.setObjectTypes(objTypes);
    FilterDialog.setBounds(modBounds);
    if(FilterDialog.exec())
      {
      FilterDialog.getSelectedObjectTypes(objTypes);
      FilterDialog.getBounds(modBounds);
      if(!this->Internal->SceneGeoTree->isEmpty())
        {
        this->clearpqCMBSceneTree();
        }

      pqCMBSceneReader reader;
      reader.setTree(this->Internal->SceneGeoTree);
      reader.setFileName(filename.toStdString().c_str());
      if(FilterDialog.getUseBoundsConstraint())
        {
        reader.setUseBoundsConstraint(1);
        reader.setBoundsConstraint(modBounds);
        }

      reader.setFilterObjectByType(1);
      reader.setFilterObjectTypes(objTypes);
      if (reader.process(info->GetFileContents()))
        {
        qCritical() << "Problems loading File, " << filename << ". \n" << reader.getStatusMessage().c_str();
        return;
        }

      this->Internal->CurrentSceneFileName = filename;
      this->activeRenderView()->resetCamera();
      this->activeRenderView()->render();
      this->Internal->SimBuilder->updateSimBuilder(this->Internal->SceneGeoTree);
      this->Internal->SceneGeoTree->getWidget()->expandAll();
      emit this->newSceneLoaded();
      }
    }
  else
    {
    QString message = "There are no UserDefinedObjectTypes in the file.\n";
    message.append("Note: UserDefinedObjectTypes is only available in Version 2.0 or later Scene files.");
    QMessageBox::warning(this->parentWidget(), "Unable to load scene file!",
       message);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::processMapInfo(const QString& vtkNotUsed(filename),
    pqPipelineSource* source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  // force read
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();

  source->getProxy()->UpdateVTKObjects();
  source->getProxy()->UpdatePropertyInformation();

  QList<QVariant> arcIdArray = pqSMAdaptor::getMultipleElementProperty(
      source->getProxy()->GetProperty("ArcIds"));
  int numArcs = pqSMAdaptor::getElementProperty(
      source->getProxy()->GetProperty("NumArcs")).toInt();

  for(int i = 0; i < numArcs; i++)
    {
    pqPipelineSource* extract = builder->createFilter("filters",
        "CmbExtractMapContour", source);

    pqSMAdaptor::setElementProperty(
        extract->getProxy()->GetProperty("ExtractSingleContour"), arcIdArray[i].toInt());
    extract->getProxy()->InvokeCommand("ExtractSingleContour");
    extract->getProxy()->UpdatePropertyInformation();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();
    }

  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();

  emit this->newSceneLoaded();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onVTKConnectionChanged(
    pqDataRepresentation* connRep)
{
  this->setDisplayRepresentation(connRep);

  // this->Internal->ColorFaceWidget->setRepresentation(connRep);
}

//-----------------------------------------------------------------------------

/**\brief Updated UI when the model manager changes.
  *
  * When notified by this->Internal->smtkModelManager that the model manager
  * has changed, tell the mesher panel to use the new one.
  *
  * Any other panels that hold onto an smtk::model::ManagerPtr instead
  * of the pqCMBModelManager (which owns the instance for the application)
  * should be updated here.
  */
void pqCMBModelBuilderMainWindowCore::modelManagerChanged(vtkSMModelManagerProxy* proxy)
{
  (void)proxy;
  this->meshPanel()->updateModel(
    this->Internal->smtkModelManager,
    this->meshServiceMonitor());
}

//-----------------------------------------------------------------------------
SimBuilderCore* pqCMBModelBuilderMainWindowCore::getSimBuilder()
{
  if(!this->Internal->SimBuilder)
    {
    //this->Internal->SimBuilder = new SimBuilderCore(
    //    this->getActiveServer(), this->activeRenderView());
    this->Internal->SimBuilder = new SimBuilderCore(
        this->getActiveServer(), this->activeRenderView());
    if(this->Internal->ModelDock)
      this->Internal->SimBuilder->uiManager()->
        setModelPanel(this->Internal->ModelDock);
    }

  return this->Internal->SimBuilder;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onCMBModelCleared()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onModelLoaded()
{
//    this->Internal->SelectByBox->addItems(list);
//    this->Internal->SelectByBox->setCurrentIndex(0);


    if(this->getSimBuilder()->isSimModelLoaded() &&
      !this->getSimBuilder()->isLoadingScenario())
      {
      this->getSimBuilder()->clearCMBModel();
      }
    // Make sure the mesh is cleared first
    //this->getSimBuilder()->getMeshManager()->clearMesh();

//    this->Internal->SelectByBox->blockSignals(false);

}

//-----------------------------------------------------------------------------
pqCMBRubberBandHelper* pqCMBModelBuilderMainWindowCore::cmbRenderViewSelectionHelper() const
{
  return &this->Internal->cmbRenderViewSelectionHelper;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onRubberBandSelect(bool checked)
{
  if (checked)
    {
    // reset all mesh selection operations we are doing
    if(this->Internal->ModelDock)
      this->Internal->ModelDock->resetMeshSelectionItems();

    // always use Frustum selection since the surface selection does not
    // work if opacity < 1.

    // Frustum selection is not working well for this use case because if you do that
    // on a mine top, it will select top, bottom and likely side, so switch back to surface selection.
    // So if user wants to do selection, the opacity has to be one, for now.
    //this->cmbRenderViewSelectionHelper().beginFrustumSelection();

    this->Internal->cmbRenderViewSelectionHelper.beginBlockSelection();
    }
  else
    {
    this->Internal->cmbRenderViewSelectionHelper.endSelection();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onRubberBandSelectCells(bool checked)
{
  if (checked)
    {
    this->Internal->cmbRenderViewSelectionHelper.beginSurfaceSelection();
    }
  else
    {
    this->Internal->cmbRenderViewSelectionHelper.endSelection();
    }
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onRubberBandSelectPoints(bool checked)
{
  if (checked)
    {
    this->Internal->cmbRenderViewSelectionHelper.beginSurfacePointsSelection();
    }
  else
    {
    this->Internal->cmbRenderViewSelectionHelper.endSelection();
    }
}

//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::onLoadSimulation(bool templateOnly, bool isScenario)
{
  this->setSimBuilderModelManager();
  int res = this->getSimBuilder()->LoadSimulation(templateOnly, isScenario);
  return res;
}
//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::onLoadScenario()
{
  return this->onLoadSimulation(false, true);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSaveScenario()
{
  this->getSimBuilder()->SaveSimulation(true);
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSaveSimulation()
{
  this->getSimBuilder()->SaveSimulation(false);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onExportSimFile()
{
  this->getSimBuilder()->ExportSimFile(
    this->modelManager()->managerProxy());
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::zoomOnSelection()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onCloseData(bool modelOnly)
{
  if(!modelOnly)
    {
    this->clearSimBuilder();
    }

  // destroy the new smtk model
  if(this->Internal->ModelDock)
    {
    this->Internal->ModelDock->clearUI();
    }

  if(this->Internal->smtkModelManager)
    {
    this->Internal->smtkModelManager->clear();
    }
//  this->getCMBModel()->onCloseData();

  // Some scene object source may be used by model bathymetry filter,
  // so it should be cleaned after model.
  if(!modelOnly)
    {
    this->clearpqCMBSceneTree();
    }

}

void pqCMBModelBuilderMainWindowCore::clearSimBuilder()
{
  if(this->Internal->SimBuilder)
    {
    this->Internal->SimBuilder->clearCMBModel();
    this->resetSimulationModel();
    }
}

void pqCMBModelBuilderMainWindowCore::resetSimulationModel()
{
  this->getSimBuilder()->clearSimulationModel();
  this->getSimBuilder()->Initialize();
  if(this->Internal->AttVisAction &&
     this->Internal->AttVisAction->isVisible())
  {
    this->onColorByModeChanged("None");
    if(this->Internal->AttributeVisWidget)
    {
      delete this->Internal->AttributeVisWidget;
      this->Internal->AttributeVisWidget = NULL;
    }
  }
}

//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::previewWindow(QString path)
{

  QFileInfo info(path);
  int result = 0;
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (fModel.fileExists(path, fullpath))
    {
    pqApplicationCore* const core = pqApplicationCore::instance();
    pqObjectBuilder* const builder = core->getObjectBuilder();
    pqRenderView *tmpView = qobject_cast<pqRenderView*>(
        builder->createView(pqRenderView::renderViewType(),
          this->getActiveServer() ));

    pqPipelineSource* reader = 0;
    QStringList fileList;
    fileList << path;

    builder->blockSignals(true);

    // really expect vtk, but...
    QFileInfo finfo(fileList[0]);
    if (finfo.suffix().toLower() == "2dm" ||
        finfo.suffix().toLower() == "3dm" || finfo.suffix().toLower() == "sol" ||
        finfo.suffix().toLower() == "vtk" || finfo.suffix().toLower() == "vtp" ||
        finfo.suffix().toLower() == "poly" || finfo.suffix().toLower() == "obj" ||
        finfo.suffix().toLower() == "tin" || finfo.suffix().toLower() == "stl")
      {
      reader = builder->createReader("sources", "CMBGeometryReader",
          fileList, this->getActiveServer());
      }
    builder->blockSignals(false);
    if (!reader)
      {
      qCritical() << "Unknown file extension " << info.suffix();
      return 0;
      }
    pqDataRepresentation* repr =
        builder->createDataRepresentation(reader->getOutputPort(0), tmpView, "GeometryRepresentation");

    this->previewDialog()->setRepresentationAndView(repr, tmpView);

    this->previewDialog()->show();
    }
  else // no file to show, so just reenable external process
    {
    emit enableExternalProcesses(true);
    }

  return result;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::resetCenterOfRotationToCenterOfCurrentData()
{
  this->Superclass::resetCenterOfRotationToCenterOfCurrentData();
}

//----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::loadModelFile(const QString& filename)
{
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (!fModel.fileExists(filename, fullpath))
    {
    qCritical() << "File does not exist: " << filename;
    return 0;
    }

  smtk::model::OperatorPtr fileOp = this->Internal->smtkModelManager->
    createFileOperator(filename.toStdString());
  if (!fileOp)
    {
    qCritical()
      << "Could not create file (read or import) operator for file: "
      << filename << "\n";
    return 0;
    }

  // if ableToOperate, no UI is need for this op
  bool succeeded = false;
  if(!this->modelPanel()->modelView())
    {
    this->modelPanel()->resetUI();

    //telll the mesh panel what is the new model manager
    //it should track
    this->meshPanel()->updateModel(
          this->Internal->smtkModelManager,
          this->meshServiceMonitor());
    }

  if(this->modelPanel()->modelView())
    succeeded = this->modelPanel()->modelView()->requestOperation(
      fileOp, !fileOp->ableToOperate());
  else if(fileOp->ableToOperate())
    succeeded = this->Internal->smtkModelManager->startOperation(fileOp);
  else
    qCritical() << "No proper file operator found for: " << filename;
  return succeeded ? 1 : 0;
/*
  if(this->getCMBModel() &&
      this->getCMBModel()->canLoadFile(filename))
    {
    this->onCloseData(true);
    if(int retVal = this->getCMBModel()->loadModelFile(filename))
      {
      // try updating the smtk entities after loading in a vtkModel.
      pqSimBuilderUIManager* simUIManager = this->getSimBuilder()->getUIManager();
      simUIManager->attModel()->setDiscreteModel(
        this->Internal->CMBModel->getModel());
      simUIManager->updateModelItems();
      //smtkModel* attModel = simUIManager->attModel();
      return retVal;
      }
    }

  return 0;
*/
}

//-----------------------------------------------------------------------------
/// Called when a new reader is created by the GUI.
void pqCMBModelBuilderMainWindowCore::onReaderCreated(
    pqPipelineSource* reader, const QString& filename)
{
  if (!reader)
    {
    return;
    }
  QFileInfo fInfo(filename);
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (!fModel.fileExists(filename, fullpath))
    {
    qCritical() << "File does not exist: " << filename;
    return;
    }

  QString lastExt = fInfo.suffix().toLower();
  // If this is simulation file, and there is already a simulation file loaded
  if((lastExt == "crf" || lastExt== "sbt" || lastExt== "sbs" || lastExt== "sbi"
     || fInfo.completeSuffix().toLower() == "simb.xml" ) &&
    (this->getSimBuilder()->currentTemplate() == filename.toStdString()
     || this->getSimBuilder()->isSimModelLoaded()))
    {
    if(QMessageBox::question(this->parentWidget(),
    "Load Simulation?",
    tr("A SimBuilder file is already loaded. Do you want to close current SimBuilder file?\n") +
    tr("NOTE: all unsaved changes to simulation database will be lost if answering Yes."),
    QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
      {
      return;
      }
    else
      {
      QApplication::processEvents();
      this->resetSimulationModel();
      }
    }

  if(lastExt == "crf")
    {
    this->setSimBuilderModelManager();
    this->getSimBuilder()->LoadResources(reader, this->Internal->SceneGeoTree);
    return;
    }
  if(fInfo.completeSuffix().toLower() == "simb.xml" ||
    lastExt== "sbt" || lastExt== "sbs" || lastExt== "sbi")
    {
    this->setSimBuilderModelManager();
    this->getSimBuilder()->LoadSimulation(reader, this->Internal->SceneGeoTree);
    return;
    }

  if(fInfo.suffix().toLower() == "sg")
    {
    this->processSceneInfo(filename, reader);
    return;
    }
/*
  if(fInfo.suffix().toLower() == "json")
    {
    this->processOperatorResult(filename, reader);
    return;
    }

  if(this->getCMBModel() &&
      this->getCMBModel()->canLoadFile(filename))
    {
    this->onCloseData(true);
    this->getCMBModel()->loadReaderSource(filename, reader);
    }
*/
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onServerCreationFinished(pqServer *server)
{
  //this->Internal->SceneGeoTree->blockSignals(true);
  this->Superclass::onServerCreationFinished(server);

  //import in the ModelBridge plugin after the common plugin(s) has been loaded
  //in case it depends on any symbols of the common plugins(s)
  PV_PLUGIN_IMPORT(ModelBridge_Plugin)

  emit this->newModelCreated();

  this->Internal->cmbRenderViewSelectionHelper.setView(this->activeRenderView());
  // Set up connection with selection helpers for all views.
  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
    &this->Internal->cmbRenderViewSelectionHelper, SLOT(setView(pqView*)));

  QObject::connect(
    this->renderViewSelectionHelper(), SIGNAL(startSelection()),
    &this->Internal->cmbRenderViewSelectionHelper, SLOT(DisabledPush()));
  QObject::connect(
    this->renderViewSelectionHelper(), SIGNAL(stopSelection()),
    &this->Internal->cmbRenderViewSelectionHelper, SLOT(DisabledPop()));

//  pqSMAdaptor::setElementProperty(this->activeRenderView()->
//      getProxy()->GetProperty("LODThreshold"),  102400);

 //this->Internal->SceneGeoTree->blockSignals(false);
  if(this->Internal->SceneGeoTree)
    {
    this->Internal->SceneGeoTree->setCurrentView(this->activeRenderView());
    this->Internal->SceneGeoTree->setCurrentServer(this->getActiveServer());
    }
  if(this->Internal->SimBuilder)
    {
    this->clearSimBuilder();
    this->Internal->SimBuilder->setServer(this->getActiveServer());
    this->Internal->SimBuilder->setRenderView(this->activeRenderView());
    }

  delete this->Internal->smtkModelManager;
  this->Internal->smtkModelManager = new pqCMBModelManager(this->getActiveServer());
  QObject::connect(this->Internal->smtkModelManager,
    SIGNAL(operationFinished(const smtk::model::OperatorResult&,
    const smtk::model::SessionRef&, bool, bool, bool)),
    this, SLOT(processOperatorResult( const smtk::model::OperatorResult&,
    const smtk::model::SessionRef&, bool, bool, bool)));

  QObject::connect(
    this->Internal->smtkModelManager, SIGNAL(newModelManagerProxy(vtkSMModelManagerProxy*)),
    this, SLOT(modelManagerChanged(vtkSMModelManagerProxy*)));

  QObject::connect(this->Internal->ViewContextBehavior,
    SIGNAL(representationBlockPicked(pqDataRepresentation*, unsigned int, bool)),
    this, SLOT(selectRepresentationBlock( pqDataRepresentation*, unsigned int, bool)));

  // We need to block this so that the display and info panel only
  // works on the model geometry, not scene, or anyting else
//  pqActiveObjects::instance().disconnect(this->activeRenderView());
//  pqActiveObjects::instance().blockSignals(true);
  this->activeRenderView()->setOrientationAxesVisibility(false);
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onRemovingServer(pqServer *server)
{
  this->Superclass::onRemovingServer(server);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onEditSettings()
{
  if(!this->appSettingsDialog())
    {
    return;
    }

  this->appSettingsDialog()->addOptions("App Specific",
    this->Internal->AppOptions);
  QStringList pages = this->Internal->AppOptions->getPageList();
  if(pages.size())
    {
    this->appSettingsDialog()->setCurrentPage(pages[0]);
    }
  this->Superclass::onEditSettings();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::applyAppSettings()
{
  this->Superclass::applyAppSettings();
 /*
  int index = this->Internal->ColorFaceCombo->findText(
    this->Internal->AppOptions->default3DModelFaceColorMode().c_str());
  index = (index==-1) ? 0 : index;
  this->Internal->ColorFaceCombo->setCurrentIndex(index);
  index = this->Internal->ColorEdgeCombo->findText(
    this->Internal->AppOptions->default2DModelEdgeColorMode().c_str());
  index = (index==-1) ? 0 : index;
  this->Internal->ColorEdgeCombo->setCurrentIndex(index);
  index = this->Internal->ColorEdgeDomainCombo->findText(
    this->Internal->AppOptions->default2DModelFaceColorMode().c_str());
  index = (index==-1) ? 0 : index;
  this->Internal->ColorEdgeDomainCombo->setCurrentIndex(index);
  this->applyColorSettings();
*/
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::updateScalarBarWidget(
    pqScalarBarWidget* scalarBar, const QString& strDefType, bool show)
{
  if(scalarBar)
    {
    scalarBar->setVisible(show);
    if(show)
      {
      pqSimBuilderUIManager* simUIManager = this->getSimBuilder()->uiManager();
      smtk::attribute::SystemPtr attSystem = simUIManager->attributeSystem();
      std::vector<smtk::attribute::AttributePtr> result;
      attSystem->findDefinitionAttributes(
        strDefType.toStdString(), result);

      QList<QVariant> annotationList;
      QList<QColor> indexColors;
      std::vector<smtk::attribute::AttributePtr>::iterator it;
      for (it=result.begin(); it!=result.end(); ++it)
        {
        const double* rgba = (*it)->color();
        double alpha = std::max(0., std::min(rgba[3], 1.0));
        QColor attColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2], alpha);
        indexColors.append(attColor);
        const char* attname = (*it)->name().c_str();
        annotationList.append(attname);
        annotationList.append(attname);
        }
      scalarBar->setTitle(strDefType);
      scalarBar->setIndexedColors(indexColors);
      scalarBar->setAnnotations(annotationList);
      }
    this->activeRenderView()->render();
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::loadJSONFile(const QString& filename)
{
  // Now, all the extensions (except .cmb)
  // checking is moved into vtkCMBGeometryReader.

  QFileInfo finfo(filename);
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (!fModel.fileExists(filename, fullpath))
    {
    qCritical() << "File does not exist: " << filename;
    return;
    }

  // Load the file and set up the pipeline to display the dataset.
  QStringList files;
  files << filename;

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource* reader = 0;

  if (finfo.suffix().toLower() == "json")
    {
    reader = builder->createReader("CMBModelGroup", "smtkJsonModelReader",
        files, this->getActiveServer());
    }
}

//----------------------------------------------------------------------------
pqCMBModelManager* pqCMBModelBuilderMainWindowCore::modelManager()
{
  return this->Internal->smtkModelManager;
}

// we may need to update model representation for display properties
// of the list of entities that were potentially modified.
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::processModifiedEntities(
  const smtk::attribute::ModelEntityItemPtr& resultEntities)
{
  smtk::model::ManagerPtr mgr = this->modelManager()->managerProxy()->modelManager();

  // The result could be from multiple models.
  // For example, if Session(s) is clicked to change visibility or color,
  // all models under it should change
  QMap<pqSMTKModelInfo*, QMap<bool, QList<unsigned int> > >visBlocks;
  QMap<pqSMTKModelInfo*, QMap<smtk::model::EntityRef, QColor> >colorEntities;
  // Map for aux-geo-url visibility
  QMap<std::string, bool> auxGeoVisibles;
  QMap<std::string, QColor> auxGeoColors;
  smtk::model::EntityRefArray::const_iterator it;
  for(it = resultEntities->begin(); it != resultEntities->end(); ++it)
    {
    pqSMTKModelInfo* minfo = NULL;
    smtk::model::EntityRef curRef(mgr, it->entity());
    // take care of potential coloring related changes
    QColor color;
    if(curRef.hasColor() && (curRef.isVolume() || curRef.isGroup() ||
       curRef.hasIntegerProperty("block_index")) // a geometric entity
       && (minfo = this->Internal->smtkModelManager->modelInfo(curRef)))
      {
      pqCMBContextMenuHelper::getValidEntityColor(color, curRef);
      // this could also be removing colors already being set,
      // so if even the color is invalid, we still record it
      colorEntities[minfo].insert(curRef, color);        
      }
    else if(pqSMTKUIHelper::isAuxiliaryShownSeparate(curRef)
            && curRef.hasColor())
      {
      pqCMBContextMenuHelper::getValidEntityColor(color, curRef);
      smtk::model::AuxiliaryGeometry aux(curRef);
      auxGeoColors[aux.url()] = color;
      }

    // For potential visibility changes
    bool visible = true;
    unsigned int flatIndex;
    if(curRef.hasIntegerProperty("block_index") &&
       (minfo = this->Internal->smtkModelManager->modelInfo(curRef)))
      {
      const smtk::model::IntegerList& prop(curRef.integerProperty("block_index"));
      if(!prop.empty())
        {
        flatIndex = prop[0];
        if(curRef.hasVisibility())
          {
          const smtk::model::IntegerList& vprop(curRef.integerProperty("visible"));
          if(!vprop.empty())
            visible = (vprop[0] != 0);
          visBlocks[minfo][visible] << flatIndex;
          }
        }
      }
    else if(pqSMTKUIHelper::isAuxiliaryShownSeparate(curRef)
            && curRef.hasVisibility())
      {
      smtk::model::AuxiliaryGeometry aux(curRef);
      const smtk::model::IntegerList& vprop(curRef.integerProperty("visible"));
      if(!vprop.empty())
        visible = (vprop[0] != 0);
      auxGeoVisibles[aux.url()] = visible;
      }
    }

  // update visibility
  foreach(pqSMTKModelInfo* minfo, visBlocks.keys())
    {
    if(minfo->Representation && visBlocks[minfo].count())
      foreach(bool isVisible, visBlocks[minfo].keys())
        this->Internal->ViewContextBehavior->syncBlockVisibility(
          minfo->Representation, visBlocks[minfo][isVisible], isVisible,
          minfo->Info->GetUUID2BlockIdMap().size());
    }

  // update entities color
  foreach(pqSMTKModelInfo* minfo, colorEntities.keys())
    {
    if(minfo->Representation && colorEntities[minfo].count())
      {
      this->Internal->ViewContextBehavior->updateColorForEntities(
        minfo->Representation, minfo->ColorMode, colorEntities[minfo]);
      this->Internal->smtkModelManager->updateEntityColorTable(
        minfo->Representation, colorEntities[minfo], minfo->ColorMode);
      minfo->Representation->renderViewEventually();
      }
    }

  // update auxiliary visibility
  foreach(std::string aux_url, auxGeoVisibles.keys())
    {
    smtkAuxGeoInfo* auxInfo = this->Internal->smtkModelManager->auxGeoInfo(aux_url);
    if(auxInfo)
      {
      pqCMBContextMenuHelper::updateVisibilityForAuxiliary(
        auxInfo->Representation, auxGeoVisibles[aux_url]);
      }
    }

  // update auxiliary colors
  foreach(std::string aux_url, auxGeoColors.keys())
    {
    smtkAuxGeoInfo* auxInfo = this->Internal->smtkModelManager->auxGeoInfo(aux_url);
    if(auxInfo)
      {
      pqCMBContextMenuHelper::updateColorForAuxiliary(
        auxInfo->Representation, auxGeoColors[aux_url]);
      }
    }

}

// we may need to update mesh representation for display properties
// of the list of mehse that were potentially modified.
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::processModifiedMeshes(
  const smtk::attribute::MeshItemPtr& modifiedMeshes)
{
  smtk::model::ManagerPtr mgr = this->modelManager()->managerProxy()->modelManager();

  // The result could be from multiple collections.
  // For example, if Session(s) is clicked to change visibility or color,
  // all meshes under it should change
  QMap<pqSMTKMeshInfo*, QMap<bool, QList<unsigned int> > >visBlocks;
  QMap<pqSMTKMeshInfo*, QMap<smtk::mesh::MeshSet, QColor> >colorEntities;
  QString lastColorMode;

  smtk::attribute::MeshItem::const_mesh_it it;
  for(it = modifiedMeshes->begin(); it != modifiedMeshes->end(); ++it)
    {
    smtk::mesh::CollectionPtr c = it->collection();
    if(!c->hasIntegerProperty(*it, "block_index"))
      continue;

    smtk::model::EntityRef curRef(mgr, c->associatedModel());
    pqSMTKModelInfo* minfo = this->Internal->smtkModelManager->modelInfo(curRef);
    if(!minfo)
      continue;
    if(minfo->MeshInfos.find(c->entity()) == minfo->MeshInfos.end())
      return;
    lastColorMode = minfo->ColorMode;
    pqSMTKMeshInfo* meshInfo = &(minfo->MeshInfos[c->entity()]);

    // take care of potential coloring related changes
    if(c->hasFloatProperty(*it, "color"))
      {
      QColor color;
      pqCMBContextMenuHelper::getValidMeshColor(color, *it);
      // this could also be removing colors already being set,
      // so if even the color is invalid, we still record it
      colorEntities[ meshInfo ].insert(*it, color);        
      }

    // For potential visibility changes
    bool visible = true;
    unsigned int flatIndex;
    const smtk::model::IntegerList& prop(c->integerProperty(*it, "block_index"));
    if(!prop.empty())
      {
      flatIndex = prop[0];
      if(c->hasIntegerProperty(*it, "visible"))
        {
        const smtk::model::IntegerList& vprop(c->integerProperty(*it, "visible"));
        if(!vprop.empty())
          visible = (vprop[0] != 0);
        visBlocks[meshInfo][visible] << flatIndex+1;
        }
      }
    }

  // update mesh representation visibility
  foreach(pqSMTKMeshInfo* meshinfo, visBlocks.keys())
    {
    if(meshinfo->Representation && visBlocks[meshinfo].count())
      foreach(bool isVisible, visBlocks[meshinfo].keys())
        this->Internal->ViewContextBehavior->syncBlockVisibility(
          meshinfo->Representation, visBlocks[meshinfo][isVisible], isVisible,
          meshinfo->Info->GetMesh2BlockIdMap().size());
    }

  // update mesh representation color
  foreach(pqSMTKMeshInfo* meshinfo, colorEntities.keys())
    {
    if(meshinfo->Representation && colorEntities[meshinfo].count())
      {
      this->Internal->ViewContextBehavior->updateColorForMeshes(
        meshinfo->Representation, lastColorMode, colorEntities[meshinfo]);
//      this->Internal->smtkModelManager->updateEntityColorTable(
//        meshinfo->Representation, colorEntities[meshinfo], lastColorMode);
      meshinfo->Representation->renderViewEventually();
      }
    }

}

void internal_collectAllMeshes(smtk::mesh::MeshSets& meshes,
                      smtk::mesh::CollectionPtr c,
                      smtk::mesh::DimensionType dim)
{
  smtk::mesh::MeshSet dimMesh = c->meshes(dim);
  meshes.insert(dimMesh);
  for(std::size_t i=0; i<dimMesh.size(); ++i)
    meshes.insert(dimMesh.subset(i));
}

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::processOperatorResult(
  const smtk::model::OperatorResult& result,
  const smtk::model::SessionRef& sref,
  bool hasNewModels, bool bModelGeometryChanged, bool hasNewMeshes)
{
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    return false;
    }

  smtk::extension::qtModelView* modelTree = this->modelPanel()->modelView();
  if (modelTree)
    {
    // update the icons and color swatches on model tree view.
    modelTree->updateWithOperatorResult(sref, result);
    }
  this->modelPanel()->update();

  // FIXME, we need more info regarding what changed in the result entities,
  // for example, is this a color change, visibility change, etc
  // modified model entities
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("modified");
  if(resultEntities && resultEntities->numberOfValues() > 0 &&
    !bModelGeometryChanged && !hasNewMeshes)
    {
    this->processModifiedEntities(resultEntities);
    }

  // modified meshes
  smtk::attribute::MeshItem::Ptr modifiedMeshes =
    result->findMesh("mesh_modified");
  if(modifiedMeshes && modifiedMeshes->numberOfValues() > 0 &&
    !hasNewMeshes)
    {
    this->processModifiedMeshes(modifiedMeshes);
    }

  if(hasNewModels)
    {
    this->activeRenderView()->resetCamera();
    emit this->newModelCreated();
    }
  this->activeRenderView()->render();

  if(hasNewMeshes)
    {
    emit this->newMeshCreated();
    }
  // if both models and meshes are created, we hide all meshes, assuming the users really
  // want to see the models without coincident meshes
 if(hasNewMeshes && hasNewModels)
   {
   smtk::attribute::ModelEntityItem::Ptr modelWithMeshes =
     result->findModelEntity("mesh_created");
  if(modelWithMeshes)
    {
    smtk::mesh::ManagerPtr meshMgr = this->modelManager()->managerProxy()->modelManager()->meshes();
    smtk::mesh::MeshSets meshes;
    smtk::model::EntityRefArray::const_iterator it;
    for(it = modelWithMeshes->begin(); it != modelWithMeshes->end(); ++it)
      {
      std::vector<smtk::mesh::CollectionPtr> colls = meshMgr->associatedCollections(*it);
      std::vector<smtk::mesh::CollectionPtr>::const_iterator cit;
      for(cit = colls.begin(); cit != colls.end(); ++cit)
        {
        meshes.insert((*cit)->meshes());
        internal_collectAllMeshes(meshes, *cit, smtk::mesh::Dims3);
        internal_collectAllMeshes(meshes, *cit, smtk::mesh::Dims2);
        internal_collectAllMeshes(meshes, *cit, smtk::mesh::Dims1);
        internal_collectAllMeshes(meshes, *cit, smtk::mesh::Dims0);
        }
      }
    if(meshes.size() > 0)
      {
      this->modelPanel()->modelView()->syncEntityVisibility(
        sref.session(), smtk::common::UUIDs(), meshes, 0);
      this->modelPanel()->update();
      }
    }

   }
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorByModeChanged(
  const QString & colorMode)
{
  bool byAttribute = colorMode ==
    vtkModelMultiBlockSource::GetAttributeTagName();
  if(byAttribute && !this->Internal->AttributeVisWidget)
  {
    QToolBar* toolbar = this->Internal->ColorByAttToolbar;
    SimBuilderCore* sbCore = this->getSimBuilder();
    this->Internal->AttributeVisWidget =
      new smtk::extension::qtAttributeDisplay(toolbar,
        sbCore->uiManager()->attributeUIManager())
      << pqSetName("colorByAttributeWidget");;
    this->Internal->AttVisAction = toolbar->addWidget(
        this->Internal->AttributeVisWidget);

    QObject::connect(
        this->Internal->AttributeVisWidget,
        SIGNAL(attributeFieldSelected(const QString&, const QString&)),
        this, SLOT(colorByAttributeFieldSelected(const QString&, const QString&)));
  }

  if(this->Internal->AttVisAction)
  {
    this->Internal->AttVisAction->setVisible(byAttribute);
  }

  if(byAttribute)
  {
    this->onColorByAttribute();
  }
  else
  {
    this->Internal->ViewContextBehavior->colorByEntity(colorMode);
  }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorByAttribute()
{
  if(this->Internal->AttributeVisWidget &&
     this->Internal->AttVisAction->isVisible())
  {
    this->Internal->AttributeVisWidget->onShowCategory();
  }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::colorByAttributeFieldSelected(
  const QString& attdeftype, const QString& itemname)
{
  this->Internal->ViewContextBehavior->colorByAttribute(
    this->getSimBuilder()->uiManager()->attributeSystem(),
    attdeftype, itemname);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::updateSMTKSelection()
{
  this->modelPanel()->updateTreeSelection();
}

//----------------------------------------------------------------------------
pqSMTKModelPanel* pqCMBModelBuilderMainWindowCore::modelPanel()
{
  if(!this->Internal->ModelDock)
  {
    this->Internal->ModelDock = new pqSMTKModelPanel(
      this->Internal->smtkModelManager,
      this->parentWidget(), this->smtkSelectionManager());
    this->Internal->ViewContextBehavior->setModelPanel(
      this->Internal->ModelDock);
    if(this->Internal->SimBuilder)
    {
      this->Internal->SimBuilder->uiManager()->
        setModelPanel(this->Internal->ModelDock);
    }
    // store a model manager ptr in smtkSelectionManager
    if (this->modelManager()->managerProxy()->modelManager())
    {
      this->smtkSelectionManager()->setModelManager(this->
       modelManager()->managerProxy()->modelManager());
    }
  }
  return this->Internal->ModelDock;
}

//----------------------------------------------------------------------------
pqSMTKMeshPanel* pqCMBModelBuilderMainWindowCore::meshPanel()
{
  if(!this->Internal->MeshDock)
    {
    this->Internal->MeshDock = new pqSMTKMeshPanel(
      this->Internal->smtkModelManager,
      this->meshServiceMonitor(),
      this->parentWidget());
    // connect the entity selection from mesh panel, so that the model
    // tree view and render window will show entites being selected
    QObject::connect(this->Internal->MeshDock,
      SIGNAL(entitiesSelected(const smtk::common::UUIDs&)),
      this->modelPanel(),
      SLOT(requestEntitySelection(const smtk::common::UUIDs&)));
    }
  return this->Internal->MeshDock;
}

//-----------------------------------------------------------------------------
pqCMBDisplayProxyEditor* pqCMBModelBuilderMainWindowCore::getAppearanceEditor()
{
  if(!this->Superclass::getAppearanceEditor())
    {
    pqCMBDisplayProxyEditor* displayEditor = new pqCMBDisplayProxyEditor(
        NULL,
        this->parentWidget());
    this->setAppearanceEditor(displayEditor);
    displayEditor->setObjectName("DisplayPropertyEditor");
    }

  return this->Superclass::getAppearanceEditor();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::buildRenderWindowContextMenuBehavior(
  QObject *parent_widget)
{
  this->Internal->ViewContextBehavior =
    new pqModelBuilderViewContextMenuBehavior(parent_widget);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::selectRepresentationBlock(
  pqDataRepresentation* repr, unsigned int blockIndex, bool ctrlKey)
{
  if(!repr)
    return;
  pqSMTKModelInfo* modinfo = this->Internal->smtkModelManager->modelInfo(repr);
  pqSMTKMeshInfo* meshinfo = this->Internal->smtkModelManager->meshInfo(repr);
  if(!modinfo && !meshinfo)
    return;

  vtkSMProxy* selectionSource = modinfo ?
    modinfo->BlockSelectionSource : meshinfo->BlockSelectionSource;
  vtkSMPropertyHelper prop(selectionSource, "Blocks");
  std::vector<vtkIdType> selIds;
  // if ctrlKey, we will flip selection of the block being picked
  if(ctrlKey)
    {
    selIds = prop.GetIdTypeArray();
    std::vector<vtkIdType>::iterator it = std::find(
      selIds.begin(), selIds.end(), blockIndex);
    if(it != selIds.end())
      selIds.erase(it);
    else
      selIds.push_back(blockIndex);
    }
  else
    {
    this->Internal->smtkModelManager->clearModelSelections();
    selIds.push_back(static_cast<vtkIdType>(blockIndex));
    }

  // set selected blocks
  prop.Set(&selIds[0], static_cast<unsigned int>(
    selIds.size()));
  selectionSource->UpdateVTKObjects();

  vtkSMSourceProxy *selectionSourceProxy =
    vtkSMSourceProxy::SafeDownCast(selectionSource);
  pqPipelineSource* source = repr->getInput();
  pqOutputPort* outport = source->getOutputPort(0);
  pqSelectionManager *selectionManager =
    qobject_cast<pqSelectionManager*>(
      pqApplicationCore::instance()->manager("SelectionManager"));

  if(outport && selectionManager)
    {
    if(selIds.size() > 0)
      outport->setSelectionInput(selectionSourceProxy, 0);
    else
      outport->setSelectionInput(0, 0);
//    this->requestRender();
    this->updateSMTKSelection();
    selectionManager->blockSignals(true);
    pqPVApplicationCore::instance()->selectionManager()->select(outport);
    selectionManager->blockSignals(false);
//    pqActiveObjects::instance().setActivePort(outport);
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setSimBuilderModelManager()
{
  this->getSimBuilder()->uiManager()->setModelManager(
      this->modelManager()->managerProxy()->modelManager());
  if(this->Internal->ModelDock)
    {
    this->getSimBuilder()->uiManager()->setModelPanel(this->Internal->ModelDock);
    }

  QObject::connect(this->getSimBuilder()->uiManager(),
    SIGNAL(attColorChanged()), this,
    SLOT(onColorByAttribute()));
  QObject::connect(this->getSimBuilder()->uiManager(),
    SIGNAL(attAssociationChanged()), this,
    SLOT(onColorByAttribute()));
}
