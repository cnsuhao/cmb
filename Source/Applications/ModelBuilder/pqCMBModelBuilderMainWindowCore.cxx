/*=========================================================================

Program:   CMB
Module:    pqCMBModelBuilderMainWindowCore.cxx

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
#include "pqRecentlyUsedResourcesList.h"
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
#include "SimBuilder/pqSMTKUIManager.h"

#include "pqCMBProcessWidget.h"
#include "pqCMBPreviewDialog.h"
#include "pqCMBEnumPropertyWidget.h"
#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBRubberBandHelper.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneReader.h"
#include "pqCMBModelBuilderOptions.h"
#include "pqCMBLoadDataReaction.h"
#include "pqCMBFileExtensions.h"
#include "pqSMTKMeshPanel.h"
#include "pqSMTKModelPanel.h"
#include "pqActiveObjects.h"
#include "pqCMBModelManager.h"
#include "pqModelBuilderViewContextMenuBehavior.h"
#include "pqMultiBlockInspectorPanel.h"

#include "qtCMBProgressWidget.h"
#include "qtCMBSceneObjectFilterDialog.h"
#include "qtCMBApplicationOptionsDialog.h"
#include "vtkPVSceneGenFileInformation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Operator.h"
#include "smtk/extension/qt/qtModelView.h"

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
#include <algorithm>
#include "assert.h"

using namespace smtk::attribute;
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
      this->RubberSelectionMode = 0;
      this->AppOptions = new pqCMBModelBuilderOptions();
      }

    ~vtkInternal()
      {
      if(this->SimBuilder)
        {
        delete this->SimBuilder;
        }
      }

    bool InCreateSource;

    QPointer<pqCMBEnumPropertyWidget> RepresentationWidget;
    QPointer<pqCMBEnumPropertyWidget> SelectionRepresentationWidget;
    QPointer<QComboBox> SelectionModeBox;
    QPointer<QAction> ColorFaceWidget;
    QPointer<QComboBox> ColorFaceCombo;

    int RubberSelectionMode;

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

}

//-----------------------------------------------------------------------------
pqCMBModelBuilderMainWindowCore::~pqCMBModelBuilderMainWindowCore()
{
  //pqActiveView::instance().setCurrent(0);
  delete Internal;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setupSelectionRepresentationToolbar(QToolBar* toolbar)
{
/*
  this->Internal->SelectionModeBox = new QComboBox(toolbar);
  this->Internal->SelectionModeBox->setObjectName("selectByModelEntityTypeBox");
  //toolbar->addWidget(SelectionLabel);
  toolbar->addWidget(this->Internal->SelectionModeBox);
  QStringList list;
  list << "Model Faces" << "Regions" << "Domain Sets";
  this->Internal->SelectionModeBox->addItems(list);
  this->Internal->SelectionModeBox->setToolTip("Selection Mode");
  this->Internal->SelectionModeBox->setCurrentIndex(0);
  QObject::connect(
      this->Internal->SelectionModeBox, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setRubberSelectionMode(int)));
*/
  this->Internal->SelectionRepresentationWidget = new pqCMBEnumPropertyWidget(
      toolbar)
    << pqSetName("selectionRepresentation");
  this->Internal->SelectionRepresentationWidget->setPropertyName("SelectionRepresentation");
  this->Internal->SelectionRepresentationWidget->setLabelText("Selection representation style");
  toolbar->addWidget(this->Internal->SelectionRepresentationWidget);

  //QObject::connect(this->getObjectInspectorDriver(),
  //                 SIGNAL(representationChanged(pqDataRepresentation*, pqView*)),
  //                 this->Internal->SelectionRepresentationWidget,
  //                 SLOT(setRepresentation(pqDataRepresentation*)));

  QObject::connect(this, SIGNAL(postAccept()),
      this->Internal->SelectionRepresentationWidget, SLOT(reloadGUI()));
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setRubberSelectionMode(int mode)
{
  if (mode == this->Internal->RubberSelectionMode)
    {
    return;
    }
  this->Internal->RubberSelectionMode = mode;
  emit this->rubberSelectionModeChanged();
}

//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::getRubberSelectionMode()
{
  return this->Internal->RubberSelectionMode;
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
    pqApplicationCore* core = pqApplicationCore::instance();
    // Add this to the list of recent server resources ...
    pqServerResource resource = this->getActiveServer()->getResource();
    resource.setPath(files[0]);
    resource.addData("modelmanager", "pqCMBModelManager");
    resource.addData("readoperator", "read");
    core->recentlyUsedResources().add(resource);
    core->recentlyUsedResources().save(*core->settings());
    //resource.setScheme(QString("cmbmodel"));
    }
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
  this->Internal->SelectionRepresentationWidget->setRepresentation(connRep);
  this->Internal->SelectionRepresentationWidget->reloadGUI();
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
//    this->Internal->SelectionModeBox->addItems(list);
//    this->Internal->SelectionModeBox->setCurrentIndex(0);


    if(this->getSimBuilder()->isSimModelLoaded() &&
      !this->getSimBuilder()->isLoadingScenario())
      {
      this->getSimBuilder()->clearCMBModel();
      }
    // Make sure the mesh is cleared first
    //this->getSimBuilder()->getMeshManager()->clearMesh();

//    this->Internal->SelectionModeBox->blockSignals(false);

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
  this->getSimBuilder()->attributeUIManager()->setModelManager(
      this->modelManager()->managerProxy()->modelManager());
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
    this->Internal->SimBuilder->Initialize();
    }
  if(this->Internal->SimBuilder)
    {
//    this->Internal->SimBuilder->getMeshManager()->clearMesh();
//    this->Internal->SimBuilder->getMeshManager()->setCMBModel(0);
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
    this->getSimBuilder()->clearCMBModel();
//    this->getSimBuilder()->setCMBModel(0);
    this->getSimBuilder()->clearSimulationModel();
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
    this->modelPanel()->resetUI();

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
      pqSMTKUIManager* simUIManager = this->getSimBuilder()->getUIManager();
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
/*
  if (lastExt == "cmb")
    {
    this->loadModelFile(filename);
    return;
    }
*/
  if(lastExt == "crf")
    {
    this->getSimBuilder()->attributeUIManager()->setModelManager(
        this->modelManager()->managerProxy()->modelManager());
    this->getSimBuilder()->LoadResources(reader, this->Internal->SceneGeoTree);
    return;
    }
  if(fInfo.completeSuffix().toLower() == "simb.xml"||
    lastExt== "sbt" || lastExt== "sbs" || lastExt== "sbi")
    {
/*
    if(this->getSimBuilder()->isSimModelLoaded() &&
      QMessageBox::question(this->parentWidget(),
      "Load Scenario File?",
      "A SimBuilder file is already loaded. Do you want to close current SimBuilder file?",
      QMessageBox::Yes|QMessageBox::No, QMessageBox::No) ==
      QMessageBox::No)
      {
      return;
      }
    this->clearSimBuilder();
    this->getSimBuilder()->Initialize();
*/
    this->getSimBuilder()->attributeUIManager()->setModelManager(
        this->modelManager()->managerProxy()->modelManager());
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
    this->processModelInfo(filename, reader);
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
    this->Internal->SimBuilder->clearCMBModel();
    this->Internal->SimBuilder->clearSimulationModel();
    this->Internal->SimBuilder->Initialize();
    this->Internal->SimBuilder->setServer(this->getActiveServer());
    this->Internal->SimBuilder->setRenderView(this->activeRenderView());
    }

  if(this->Internal->smtkModelManager)
    {
    delete this->Internal->smtkModelManager;
    }
  this->Internal->smtkModelManager = new pqCMBModelManager(this->getActiveServer());
  QObject::connect(this->Internal->smtkModelManager,
    SIGNAL(operationFinished(const smtk::model::OperatorResult&,
    const smtk::model::SessionRef&, bool)),
    this, SLOT(processModelInfo( const smtk::model::OperatorResult&,
    const smtk::model::SessionRef&, bool)));

  QObject::connect(this->Internal->ViewContextBehavior,
    SIGNAL(representationBlockPicked(pqDataRepresentation*, unsigned int)),
    this, SLOT(selectRepresentationBlock( pqDataRepresentation*, unsigned int )));

  // We need to block this so that the display and info panel only
  // works on the model geometry, not scene, or anyting else
//  pqActiveObjects::instance().disconnect(this->activeRenderView());
//  pqActiveObjects::instance().blockSignals(true);
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
      pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
      smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
      std::vector<smtk::attribute::AttributePtr> result;
      attSystem->findDefinitionAttributes(
        strDefType.toStdString(), result);

      QList<QVariant> annotationList;
      QList<QColor> indexColors;
      std::vector<smtk::attribute::AttributePtr>::iterator it;
      for (it=result.begin(); it!=result.end(); ++it)
        {
        const double* rgba = (*it)->color();
        QColor attColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
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

//----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::processModelInfo(
  const smtk::model::OperatorResult& result,
  const smtk::model::SessionRef& sref, bool hasNewModels)
{
  if (result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    return false;
    }

  // we may need to update model representation for display properties
  // of the list of entities that were potentially modified.
  // FIXME, we need more info regarding what changed in the result entities,
  // for example, is this a color change, visibility change, etc
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("modified");

  // The result could be from multiple models.
  // For example, if Session(s) is clicked to change visibility or color,
  // all models under it should change
  QMap<cmbSMTKModelInfo*, QList<unsigned int> >visBlocks;
  QMap<cmbSMTKModelInfo*, QMap<smtk::model::EntityRef, QColor> >colorEntities;
  QColor color;
  bool visible = true;

  if(resultEntities && resultEntities->numberOfValues() > 0)
    {
    std::cout << " client associated entities " << resultEntities->numberOfValues() << std::endl;

    smtk::model::EntityRefArray::const_iterator it;
    for(it = resultEntities->begin(); it != resultEntities->end(); ++it)
      {
      cmbSMTKModelInfo* minfo = NULL;
      // take care of potential coloring related changes
      if(it->hasColor() && (it->isVolume() || it->isGroup() ||
         it->hasIntegerProperty("block_index")) // a geometric entity
         && (minfo = this->Internal->smtkModelManager->modelInfo(*it)))
        {
        // this could also be removing colors already being set
        smtk::model::FloatList rgba = (*it).color();
        if ((rgba.size() == 3 || rgba.size() ==4) &&
           !(rgba[0]+rgba[1]+rgba[2] == 0))
          color.setRgbF(rgba[0], rgba[1], rgba[2]);

        colorEntities[minfo].insert(*it, color);
        }

      // For potential visibility changes
      unsigned int flatIndex;
      if(it->hasIntegerProperty("block_index") &&
         (minfo = this->Internal->smtkModelManager->modelInfo(*it)))
        {
        const smtk::model::IntegerList& prop((*it).integerProperty("block_index"));
        if(!prop.empty())
          {
          flatIndex = prop[0];
          if(it->hasVisibility())
            {
            visBlocks[minfo] << flatIndex+1;
            visible = (*it).visible();
            }
          }
        }
      }
    }

  // update visibility
  foreach(cmbSMTKModelInfo* minfo, visBlocks.keys())
    {
    if(minfo->Representation && visBlocks[minfo].count())
      this->Internal->ViewContextBehavior->syncBlockVisibility(
        minfo->Representation, visBlocks[minfo], visible,
        minfo->Info->GetUUID2BlockIdMap().size());
    }
  // update color
  foreach(cmbSMTKModelInfo* minfo, colorEntities.keys())
    {
    if(minfo->Representation && colorEntities[minfo].count())
      this->Internal->ViewContextBehavior->updateColorForEntities(
        minfo->Representation, colorEntities[minfo]);
    }

  this->modelPanel()->modelView()->updateWithOperatorResult(sref, result);
  if(hasNewModels)
    {
    this->activeRenderView()->resetCamera();
    emit this->newModelCreated();
    }
  this->activeRenderView()->render();
  return true;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorByModeChanged(
  const QString & colorMode)
{
  this->Internal->ViewContextBehavior->onColorByModeChanged(colorMode);
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
      this->parentWidget());
    this->Internal->ViewContextBehavior->setModelPanel(
      this->Internal->ModelDock);
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
  pqDataRepresentation* repr, unsigned int blockIndex)
{
  if(!repr)
    return;
  cmbSMTKModelInfo* minfo = this->Internal->smtkModelManager->modelInfo(repr);
  if(!minfo)
    return;

  this->Internal->smtkModelManager->clearModelSelections();

  vtkSMProxy* selectionSource = minfo->BlockSelectionSource;
  vtkSMPropertyHelper prop(selectionSource, "Blocks");
  std::vector<vtkIdType> selIds;
  selIds.push_back(static_cast<vtkIdType>(blockIndex));
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
    outport->setSelectionInput(selectionSourceProxy, 0);
//    this->requestRender();
    this->updateSMTKSelection();
    selectionManager->blockSignals(true);
    pqPVApplicationCore::instance()->selectionManager()->select(outport);
    selectionManager->blockSignals(false);
//    pqActiveObjects::instance().setActivePort(outport);
    }
}
