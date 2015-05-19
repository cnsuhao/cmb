//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBSceneBuilderMainWindowCore.h"
#include "pqCMBAppCommonConfig.h" // for CMB_TEST_DATA_ROOT

#include "qtCMBSceneMesherDialog.h"
#include "qtCMBMeshingMonitor.h"
#include "qtCMBSceneSurfaceMesherDialog.h"
#include "qtCMBTINStitcherDialog.h"
#include "pqCMBDisplayProxyEditor.h"
#include "qtCMBArcWidgetManager.h"
#include "qtCMBArcModifierInputDialog.h"
#include "pqCMBModifierArcManager.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <QTextEdit>
#include <QDir>
#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QProgressDialog>

#include "pq3DWidget.h"
#include "pqActionGroupInterface.h"
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCameraDialog.h"
#include "qtCMBArcWidget.h"
#include "qtCMBArcEditWidget.h"
#include "pqCustomFilterDefinitionModel.h"
#include "pqCustomFilterDefinitionWizard.h"
#include "pqCustomFilterManager.h"
#include "pqCustomFilterManagerModel.h"
#include "pqDataInformationWidget.h"
#include "pqCMBLineWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqDockWindowInterface.h"
#include "pqFileDialogModel.h"
#include "pqLinksManager.h"
#include "pqObjectBuilder.h"
#include "pqObjectPanel.h"
#include "pqOutputWindow.h"

#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqPipelineModel.h"
#include "pqDataRepresentation.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqCMBRubberBandHelper.h"

#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqServerManagerObserver.h"
#include "pqSettings.h"
#include "pqSMAdaptor.h"
#include "pqSpreadSheetView.h"
#include "pqTimerLogDisplay.h"
#include "pqToolTipTrapper.h"
#include "pqUndoStackBuilder.h"
#include "pqViewContextMenuManager.h"
#include "pqView.h"
#include "pqSaveSnapshotDialog.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "pq3DWidgetFactory.h"
#include <pqFileDialog.h>
#include "pqCMBProcessWidget.h"
#include <qtCMBProgressWidget.h>
#include <pqSetData.h>
#include <pqSetName.h>
#include <pqUndoStack.h>
#include <pqWriterDialog.h>
#include <pqWaitCursor.h>

#include <QVTKWidget.h>

#include <vtkAddCellDataFilter.h>
#include "vtkBoxWidget2.h"
#include <vtkDataObject.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDoubleArray.h>
#include "vtkEvent.h"
#include <vtkHydroModelCreator.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkImageData.h>
#include <vtkMultiBlockWrapper.h>
#include <vtkPolyDataReader.h>
#include <vtkProcessModule.h>
#include "vtkPVDataInformation.h"
#include "vtkPVSceneGenFileInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include <vtkPVDisplayInformation.h>
#include <vtkPVOptions.h>
#include <vtkPVXMLElement.h>
#include <vtkPVXMLParser.h>
#include "vtkSelectionNode.h"
#include <vtkSmartPointer.h>
#include <vtkSMDoubleRangeDomain.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMIdTypeVectorProperty.h>
#include "vtkSMPropertyHelper.h"
#include <vtkSMProxyIterator.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMOutputPort.h>
#include <vtkSMStringVectorProperty.h>
#include "vtkSMRepresentationProxy.h"
#include <vtkToolkits.h>
#include <vtkUnstructuredGridReader.h>
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "pqOmicronModelWriter.h"

#include <smtk/bridge/discrete/kernel/vtkDiscreteModel.h>
#include <smtk/bridge/discrete/kernel/vtkDiscreteModelEdge.h>
#include <smtk/bridge/discrete/kernel/vtkDiscreteModelRegion.h>
#include <smtk/bridge/discrete/kernel/vtkDiscreteModelWrapper.h>
#include <smtk/bridge/discrete/kernel/Model/vtkModelItemIterator.h>
#include <smtk/bridge/discrete/kernel/vtkModelUserName.h>
#include <smtk/bridge/discrete/operation/vtkCMBIncorporateMeshOperator.h>
#include <smtk/bridge/discrete/operation/vtkCMBModelBuilder.h>
#include <smtk/bridge/discrete/operation/vtkCMBModelReadOperator.h>
#include <smtk/bridge/discrete/operation/vtkCMBModelWriterBase.h>
#include <smtk/bridge/discrete/operation/vtkCMBParserBase.h>
#include <smtk/bridge/discrete/operation/vtkCompleteShells.h>
#include <smtk/bridge/discrete/operation/vtkEnclosingModelEntityOperator.h>
#include <smtk/bridge/discrete/operation/vtkMasterPolyDataNormals.h>

#include "vtkSGXMLBCSWriter.h"
#include "vtkPVArcInfo.h"
#include "vtkSMCMBGlyphPointSourceProxy.h"

#include <algorithm>
#include <vector>

#include "assert.h"
#include <vtksys/Process.h>
#include "vtkHydroModelPolySource.h"
#include "vtkOmicronModelInputReader.h"
#include "vtkOmicronMeshInputFilter.h"
#include "vtkOmicronMeshInputWriter.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMPropertyLink.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkGenericDataObjectWriter.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include "pqCMBSceneTree.h"
#include "qtCMBSceneBuilderContextMenuBehavior.h"
#include "pqCMBSceneNode.h"
#include "cmbSceneNodeReplaceEvent.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBArc.h"
#include "pqCMBPoints.h"
#include "pqCMBLine.h"
#include "pqCMBVOI.h"
#include "pqCMBPolygon.h"
#include "vtkEventQtSlotConnect.h"
#include "pqCMBSceneV2Writer.h"
#include "pqCMBSceneReader.h"
#include "pqCMBSceneNodeIterator.h"
#include "qtCMBNewSceneUnitsDialog.h"
#include <vtksys/SystemTools.hxx>

#include "vtkTimerLog.h"
#include "vtkNew.h"
#include "pqCMBSolidMesh.h"
#include "vtkTransformFilter.h"

#include <remus/client/Client.h>

#include "pqCMBPreviewDialog.h"
#include "qtSceneBuilderOptions.h"
#include "qtCMBApplicationOptionsDialog.h"

///////////////////////////////////////////////////////////////////////////
#include "vtkPVPlugin.h"
PV_PLUGIN_IMPORT_INIT(CMBModel_Plugin)

///////////////////////////////////////////////////////////////////////////
// pqCMBSceneBuilderMainWindowCore::vtkInternal

/// Private implementation details for pqCMBSceneBuilderMainWindowCore
class pqCMBSceneBuilderMainWindowCore::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/) :
    PositionLink(0),
    OrientationLink(0),
    ScaleLink(0)
  {
  this->VTKColorConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->VTKOpacityConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->VTKBoxConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->RenderWindowMenuBehavior = NULL;
  this->BoxWidget = NULL;
  this->AppOptions = new qtSceneBuilderOptions();
  this->ArcModManager = NULL;
  this->ArcDepress = NULL;
  }

  ~vtkInternal()
  {
    if (this->MesherOptionDlg)
      {
      delete this->MesherOptionDlg;
      }
    if (this->SurfaceMesherOptionDlg)
      {
      delete this->SurfaceMesherOptionDlg;
      }
    if (this->TINStitcherDlg)
      {
      delete this->TINStitcherDlg;
      }
    if (this->ArcModOptionDlg)
      {
      delete this->ArcModOptionDlg;
      }
  }

  QString CurrentSceneFileName;
  QString OutputFileName;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKColorConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKOpacityConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKBoxConnect;
  vtkSMNewWidgetRepresentationProxy* BoxWidget;

  QPointer<qtCMBSceneMesherDialog> MesherOptionDlg;
  QPointer<qtCMBSceneSurfaceMesherDialog> SurfaceMesherOptionDlg;
  QPointer<qtCMBArcModifierInputDialog> ArcModOptionDlg;
  QPointer<qtCMBTINStitcherDialog> TINStitcherDlg;
  QPointer<qtCMBSceneBuilderContextMenuBehavior> RenderWindowMenuBehavior;
  int LastMesherPathIndex;
  pqCMBSceneNode *VOINodeForMesher;
  QString DefaultSurfaceMesher;
  QString MesherOutputPrefix;
  QString OmicronInputFileName;
  vtkSMPropertyLink *PositionLink;
  vtkSMPropertyLink *OrientationLink;
  vtkSMPropertyLink *ScaleLink;

  pqCMBModifierArcManager *ArcModManager;
  pqPipelineSource *ArcDepress;

  struct NodeInfoForBuilder
    {
    double Color[4];
    std::string UserName;
    pqCMBSceneObjectBase* Object;
    NodeInfoForBuilder() : Object(0) {}
    };

  std::vector< NodeInfoForBuilder > NodeInfo;
  std::map<std::string, QStringList> TemporaryPointsFileMap;
  QMap<pqCMBGlyphObject*, QList<vtkIdType> > selGlyphs;

  QPointer<qtSceneBuilderOptions> AppOptions;
};


///////////////////////////////////////////////////////////////////////////
// pqCMBSceneBuilderMainWindowCore

pqCMBSceneBuilderMainWindowCore::pqCMBSceneBuilderMainWindowCore(QWidget* parent_widget) :
  pqCMBCommonMainWindowCore(parent_widget),
  Internal(new vtkInternal(parent_widget))
{
  this->buildRenderWindowContextMenuBehavior(parent_widget);
  this->ProgramKey = qtCMBProjectServerManager::SceneBuilder;
  this->setObjectName("SceneGenMainWindowCore");

    // Set up connection with selection helpers for all views.
  //QObject::connect(
  //  &pqActiveView::instance(), SIGNAL(changed(pqView*)),
  //  this->renderViewSelectionHelper(), SLOT(setView(pqView*)));

  this->SelectionMode = 0;
  this->Tree = NULL;
  this->ColorNode = NULL;
  this->BoxTranslation[0] = this->BoxTranslation[1] =
    this->BoxTranslation[2] = 0.0;
  this->BoxOrientation[0] = this->BoxOrientation[1] =
    this->BoxOrientation[2] = 0.0;
  this->BoxScale[0] = this->BoxScale[1] =
    this->BoxScale[2] = 1.0;

  // default surface mesher
  QDir dir(QApplication::applicationDirPath());
  dir.makeAbsolute();
  // unix?
  this->Internal->DefaultSurfaceMesher = dir.path() + "/model";
  QFileInfo finfo(this->Internal->DefaultSurfaceMesher);
  if (!finfo.exists())
    {
    // Windows
    this->Internal->DefaultSurfaceMesher += ".exe";
    finfo.setFile(this->Internal->DefaultSurfaceMesher);
    if (!finfo.exists())
      {
      // Mac install
      this->Internal->DefaultSurfaceMesher = dir.path() + "/../bin/model";
      QFileInfo finfoMac(this->Internal->DefaultSurfaceMesher);
      if (!finfoMac.exists())
        {
        // Mac build
        this->Internal->DefaultSurfaceMesher = dir.path() + "/../../../model";
        QFileInfo finfoMacBuild(this->Internal->DefaultSurfaceMesher);
        if (!finfoMacBuild.exists())
          {
          this->Internal->DefaultSurfaceMesher.clear();
          }
        }
      }
    }

  this->SelectedTransform = vtkTransform::New();

  this->Internal->VOINodeForMesher = 0;

  this->Internal->TINStitcherDlg =
    new qtCMBTINStitcherDialog(NULL);
  this->Internal->TINStitcherDlg->setMinimumAngle(25);
  this->Internal->TINStitcherDlg->setUseQuads(true);
  this->Internal->TINStitcherDlg->setAllowInteriorPointInsertion(false);
  this->Internal->TINStitcherDlg->setTolerance(1e-6);
  this->Internal->TINStitcherDlg->setUserSpecifiedTINType(0);
  this->TINStitcher = 0;
}

//-----------------------------------------------------------------------------
pqCMBSceneBuilderMainWindowCore::~pqCMBSceneBuilderMainWindowCore()
{
//  pqActiveView::instance().setCurrent(0);
  delete Internal;
  this->SelectedTransform->Delete();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::buildRenderWindowContextMenuBehavior(QObject * /*parent_widget*/)
{
  if ( this->Internal->RenderWindowMenuBehavior)
    {
    delete this->Internal->RenderWindowMenuBehavior;
    }
  this->Internal->RenderWindowMenuBehavior =
    new qtCMBSceneBuilderContextMenuBehavior(this->Tree, this->parent());
}


//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::setupBoxWidget()
{
  this->Internal->BoxWidget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("BoxWidgetRepresentation", this->getActiveServer());
  pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->GetProperty("Visibility"), false);
  vtkSMPropertyHelper(this->Internal->BoxWidget,
    "PlaceFactor").Set(1.05);

  this->Internal->BoxWidget->UpdateVTKObjects();

  pqSMAdaptor::addProxyProperty(this->activeRenderView()->getProxy()->GetProperty("Representations"),
    this->Internal->BoxWidget);

  this->activeRenderView()->getProxy()->UpdateVTKObjects();
  this->Internal->VTKBoxConnect->Disconnect();
  this->Internal->VTKBoxConnect->Connect(this->Internal->BoxWidget,
    vtkCommand::InteractionEvent,
    this, SLOT(updateBoxInteraction()));

  // Lets change the event mapping of the box widget
  vtkBoxWidget2 *aw = vtkBoxWidget2::SafeDownCast(this->Internal->BoxWidget->GetWidget());
  aw->GetEventTranslator()->RemoveTranslation(vtkCommand::LeftButtonPressEvent);
  aw->GetEventTranslator()->RemoveTranslation(vtkCommand::LeftButtonReleaseEvent);
  aw->GetEventTranslator()->RemoveTranslation(vtkCommand::MiddleButtonPressEvent);
  aw->GetEventTranslator()->RemoveTranslation(vtkCommand::MiddleButtonReleaseEvent);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::MiddleButtonPressEvent,
    vtkWidgetEvent::Select);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::MiddleButtonReleaseEvent,
    vtkWidgetEvent::EndSelect);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonPressEvent,
    vtkEvent::NoModifier,
    0, 0, NULL,
    vtkWidgetEvent::Translate);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::NoModifier,
    0, 0, NULL,
    vtkWidgetEvent::EndTranslate);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier,
    0, 0, NULL,
    vtkWidgetEvent::Scale);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier,
    0, 0, NULL,
    vtkWidgetEvent::EndScale);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonPressEvent,
    vtkEvent::ControlModifier,
    0, 0, NULL,
    vtkWidgetEvent::Select);
  aw->GetEventTranslator()->SetTranslation(vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ControlModifier,
    0, 0, NULL,
    vtkWidgetEvent::EndSelect);

}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::processScene(const QString& filename,
                                             pqPipelineSource* source)
{
  if(!this->Tree->isEmpty())
    {
    if(filename.compare(this->Internal->CurrentSceneFileName,
                        Qt::CaseInsensitive) == 0)
      {
      return;
      }
    else
      {
      this->closeData();
      this->Internal->CurrentSceneFileName.clear();
      }
    }

  // Do not create events for nodes being read from file
  // If the user wnats to undo this they would just empty the tree
  bool recordingEventsState = this->Tree->recordingEvents();
  this->Tree->turnOffEventRecording();

  pqWaitCursor cursor;
  this->Internal->CurrentSceneFileName = filename;
  QFileInfo finfo(filename);

  if (finfo.completeSuffix().toLower() == "osd.txt")
    {
    this->Internal->OutputFileName.clear();
    this->processOSDLInfo(filename, source);
    }
  if (finfo.completeSuffix().toLower() == "map")
  {
    this->Internal->OutputFileName.clear();
    this->processMapInfo(filename,source);
  }
  else if  (finfo.completeSuffix().toLower() == "sg")
    {
    this->Internal->OutputFileName = filename;
    this->processSceneInfo(filename, source);
    }

  if (recordingEventsState)
    {
    this->Tree->turnOnEventRecording();
    }
}
//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::processSceneInfo(const QString& filename,
                                            pqPipelineSource* source)
{
  // force read
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();
  vtkNew<vtkPVSceneGenFileInformation> info;
  source->getProxy()->GatherInformation(info.GetPointer());
  pqCMBSceneReader reader;
  reader.setTree(this->Tree);
  reader.setFileName(filename.toStdString().c_str());
  if (reader.process(info->GetFileContents()))
    {
    QTextEdit *qte = new QTextEdit();
    QString t("Problems loading File \"");
    qte->setWindowTitle(t + filename + "\"");
    qte->setText(reader.getStatusMessage().c_str());
    //qte->setText(info->GetFileContents());
    qte->setReadOnly(true);
    qte->show();
    }

  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();
  emit this->newSceneLoaded();
}
//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::processMapInfo(const QString& /*filename*/,
                                            pqPipelineSource* source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  /*vtkProcessModule* pm = */vtkProcessModule::GetProcessModule();

  // force read
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();

  source->getProxy()->UpdateVTKObjects();
  source->getProxy()->UpdatePropertyInformation();

  QList<QVariant> arcIdArray = pqSMAdaptor::getMultipleElementProperty(
    source->getProxy()->GetProperty("ArcIds"));
  int numArcs = pqSMAdaptor::getElementProperty(
    source->getProxy()->GetProperty("NumArcs")).toInt();

  //create the filter that extracts each contour
  pqPipelineSource* extract = builder->createFilter("filters",
        "CmbExtractMapContour", source);

  //if the root node hasn't been create, create it
  if ( this->Tree->getRoot() == NULL )
    {
    this->Tree->createRoot("Scene");
    }

  QProgressDialog progress("", "Abort Load", 0, numArcs, this->Tree->getWidget());
  progress.setWindowTitle("Loading Map File");
  progress.setWindowModality(Qt::WindowModal);

  cmbSceneNodeReplaceEvent *event = NULL;
  // Are we recording events?
  if (this->Tree->recordingEvents())
    {
    event = new cmbSceneNodeReplaceEvent(numArcs, 0);
    this->Tree->insertEvent(event);
    }

  for(int i = 0; i < numArcs; i++)
    {
      progress.setValue(i);
      if ( progress.wasCanceled() )
        {
        break;
        }

      pqSMAdaptor::setElementProperty(
        extract->getProxy()->GetProperty("ExtractSingleContour"), arcIdArray[i].toInt());
      extract->getProxy()->UpdateProperty("ExtractSingleContour");

      vtkSMSourceProxy *sourceProxy =
        vtkSMSourceProxy::SafeDownCast( extract->getProxy() );
      sourceProxy->UpdatePipeline();

      pqCMBArc* obj = new pqCMBArc(sourceProxy);
      this->Tree->createNode(arcIdArray[i].toString().toStdString().c_str(),
                             this->Tree->getArcTypeNode(), obj, event);
    }

  //destroy the filters and readers
  builder->destroy( extract );


  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();

  emit this->newSceneLoaded();
}
//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::processOSDLInfo(const QString& /*filename*/,
                                                 pqPipelineSource* source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  /*vtkProcessModule* pm =*/ vtkProcessModule::GetProcessModule();

  // force read so we can get the number of blocks
  vtkSMSourceProxy::SafeDownCast( source->getProxy() )->UpdatePipeline();
  vtkSMOutputPort *outputPort = vtkSMSourceProxy::SafeDownCast( source->getProxy() )->
    GetOutputPort(static_cast<unsigned int>(0));
  vtkPVCompositeDataInformation* compositeInformation =
    outputPort->GetDataInformation()->GetCompositeDataInformation();
  int numBlocks = compositeInformation->GetNumberOfChildren();
  double color[4];

  pqCMBSceneNode *scene = this->Tree->createRoot("Scene");
  pqCMBSceneNode *surfaces = this->Tree->createNode("Surfaces", scene, NULL, NULL);
  color[0] = 0.3; color[1] = 0.2; color[2] = 0.01; color[3] = 1.0;
  surfaces->setExplicitColor(color);
  pqCMBSceneNode *solids = this->Tree->createNode("Solids", scene, NULL, NULL);
  color[0] = 0.3; color[1] = 0.4; color[2] = 0.3; color[3] = 1.0;
  solids->setExplicitColor(color);
  pqCMBSceneNode *occluders = this->Tree->createNode("Occluders", scene, NULL, NULL);
  color[0] = 0.2; color[1] = 0.4; color[2] = 0.2; color[3] = 1.0;
  occluders->setExplicitColor(color);
  pqCMBSceneNode *vois = this->Tree->createNode("VOIs", scene, NULL, NULL);
  pqCMBSceneNode *parent;
  QString name;
  // create an actor for each block
  for(int i = 0; i < numBlocks; i++)
    {
    pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

    pqSMAdaptor::setElementProperty(
          extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

    pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", this->getActiveServer());
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
    pdSource->updatePipeline();

    vtkNew<vtkPVSceneGenObjectInformation> info;
    pdSource->getProxy()->GatherInformation(info.GetPointer());

    if (info->GetNumberOfPoints() <= 0)
      {
      continue;
      }

    pqCMBSceneObjectBase *obj ;
    pqDataRepresentation* repr ;

     // if not ROI, then get the transformation and color info
    if (strcmp(info->GetObjectType(),"RegionOfInterest"))
      {
      // See if the file is a points file
      if (pqCMBPoints::isPointsFile(info->GetObjectFileName()))
        {
        obj = new pqCMBPoints(pdSource, this->activeRenderView(),
                                 this->getActiveServer(),
                                 info->GetObjectFileName());
        }
      else
        {
        obj = new pqCMBFacetedObject(pdSource, this->activeRenderView(),
                                 this->getActiveServer(),
                                 info->GetObjectFileName());
        }
      repr = obj->getRepresentation();
      if(repr)
        {
        vtkTransform *transform = info->GetTransform();
        double orientation[3], translation[3], scale[3];
        transform->GetOrientation(orientation);
        transform->GetPosition(translation);
        transform->GetScale(scale);

        QList<QVariant> values;
        vtkSMPropertyHelper(repr->getProxy(), "MapScalars").Set(0);

        values << orientation[0] << orientation[1] << orientation[2];
        pqSMAdaptor::setMultipleElementProperty(
          repr->getProxy()->GetProperty("Orientation"), values);
        values.clear();
        values << translation[0] << translation[1] << translation[2];
        pqSMAdaptor::setMultipleElementProperty(
          repr->getProxy()->GetProperty("Position"), values);
        values.clear();
        values << scale[0] << scale[1] << scale[2];
        pqSMAdaptor::setMultipleElementProperty(
          repr->getProxy()->GetProperty("Scale"), values);

        }
      }
    else
      {
      double orientation[3], translation[3], scale[3];
      info->GetOrientation(orientation);
      info->GetTranslation(translation);
      info->GetScale(scale);
      double bounds[6] = {-1,1,-1,1,-1,1};
      obj = new pqCMBVOI(translation,bounds,
                            this->getActiveServer(),this->activeRenderView());
      repr = obj->getRepresentation();

      QList<QVariant> values;
      values << orientation[0] << orientation[1] << orientation[2];
      pqSMAdaptor::setMultipleElementProperty(
        repr->getProxy()->GetProperty("Orientation"), values);
      values.clear();
      values << scale[0] << scale[1] << scale[2];
      pqSMAdaptor::setMultipleElementProperty(
        repr->getProxy()->GetProperty("Scale"), values);
      }

    if (dynamic_cast<pqCMBVOI*>(obj))
      {
      name = "VOI ";
      parent = vois;
      obj->setUserDefinedType("-VOI");
      }
    else
      {
      pqCMBFacetedObject *fobj =
        dynamic_cast<pqCMBFacetedObject*>(obj);
      if (fobj)
        {
        if (!strcmp(info->GetObjectType(),"Solid"))
          {
          fobj->setSurfaceType(pqCMBSceneObjectBase::Solid);
          fobj->setUserDefinedType("-Solid");
          name = "Solid ";
          parent = solids;
          }
        else
          {
          fobj->setSurfaceType(pqCMBSceneObjectBase::Other);
          fobj->setUserDefinedType("-Other");
          name = "Occluder ";
          parent = occluders;
          }
        }
      else
        {
        pqCMBPoints *pobj = dynamic_cast<pqCMBPoints*>(obj);
        if (pobj)
          {
          pobj->setUserDefinedType("-LIDAR");
          // needed by OmicronWriter
          pobj->setInitialSurfaceTranslation( info->GetTranslation() );
          name = "Surface ";
          parent = surfaces;
          }
        }
      }
    QString n;
    n.setNum(parent->getChildren().size());
    name += n;
    this->Tree->createNode(name.toStdString().c_str(), parent, obj, NULL);
    // Since adding this representation will cause the color to be set
    if(repr)
      {
      repr->getProxy()->UpdateVTKObjects();
      repr->getProxy()->UpdatePropertyInformation();
      vtkSMRepresentationProxy::SafeDownCast(repr->getProxy())->UpdatePipeline();
      }
    }

  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();

  emit this->newSceneLoaded();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::
updateSelection(const QList<pqCMBSceneNode*> *unselected,
                const QList<pqCMBSceneNode*> *newlySelected)
{
  int i, n;

  // First deal with the unselected nodes
  n = unselected->count();
  for (i = 0; i < n; i++)
    {
    this->unselectNode(unselected->at(i), false);
    }

  // then deal with the newly selected nodes
  n = newlySelected->count();
  for (i = 0; i < n; i++)
    {
    this->selectNode(newlySelected->at(i), false);
    }

  // Nothing selected yet we still have SelectedLeaves... didn't think this
  // would happen, but can if we just deleted nodes
  if (!this->Tree->getSelected().size() && this->SelectedLeaves.size())
    {
    this->SelectedLeaves.clear();
    this->clearColorNode();
    }

  if(this->SelectedLeaves.size()==1 &&
    (this->SelectedLeaves[0]->getDataObject()->getType() == pqCMBSceneObjectBase::Line ||
      this->SelectedLeaves[0]->getDataObject()->getType() == pqCMBSceneObjectBase::Arc))
    {
    this->updateWidgetPanel(this->SelectedLeaves[0]->getDataObject());
    }
  else if(this->getAppearanceEditor())
    {
    this->getAppearanceEditor()->setDisabled(
      (this->SelectedLeaves.size()!=1) ? true : false);
    }
  this->clearSelectedGlyphList();
  this->updateBoxWidget();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateWidgetPanel(
  pqCMBSceneObjectBase* widgetObject)
{
  if(!widgetObject)
    {
    return;
    }

  pq3DWidget* sel3dWidget = NULL;
  QWidget* selUiWidget = NULL;
  QString widgetName("");
  pqCMBSceneObjectBase::enumObjectType type = widgetObject->getType();
  if(type == pqCMBSceneObjectBase::Line)
    {
    pqCMBLine* lineObj = static_cast<pqCMBLine*>(widgetObject);
    sel3dWidget = lineObj ? lineObj->getLineWidget() : NULL;
    selUiWidget = sel3dWidget;
    }
  else if (type == pqCMBSceneObjectBase::Arc)
    {
    qtCMBArcWidgetManager* manager = this->Tree->getArcWidgetManager();
    selUiWidget = manager->getActiveWidget();
    sel3dWidget = qobject_cast<pq3DWidget*>(selUiWidget);
    }
  if(!selUiWidget)
    {
    return;
    }
  widgetName = selUiWidget->objectName();
  this->setAppearanceEditor(NULL);
  QWidget* pWidget = this->getAppearanceEditorContainer()->layout()->parentWidget();

  // we need to make invisible all 3d widget UI panels
  QList< pq3DWidget* > user3dWidgets = pWidget->findChildren<pq3DWidget*>();
  QList< QWidget* > userUiWidgets;
  // if this is not a 3d widget
  if(!sel3dWidget)
    {
    userUiWidgets = pWidget->findChildren<QWidget*>(widgetName);
    }
  userUiWidgets.append(reinterpret_cast< QList<QWidget*>& >(user3dWidgets));
  bool found = false;
  for(int i=0; i<userUiWidgets.count(); i++)
    {
    if(!found && userUiWidgets.value(i) == selUiWidget)
      {
      found = true;
      }
    else
      {
      userUiWidgets.value(i)->setVisible(0);
      }
    }
  if(!found)
    {
    selUiWidget->setParent(pWidget);
    this->getAppearanceEditorContainer()->layout()->
      addWidget(selUiWidget);
    }
  if(sel3dWidget && sel3dWidget->widgetVisible())
    {
    sel3dWidget->select();
    sel3dWidget->setVisible(true);
    sel3dWidget->setEnabled(true);
    }
  else if(!sel3dWidget)
    {
    selUiWidget->setVisible(true);
    selUiWidget->setEnabled(true);
    selUiWidget->show();
    }

  if(this->getAppearanceEditor())
    {
    this->getAppearanceEditor()->setDisabled(false);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::unselectNode(pqCMBSceneNode *node,
                                              bool updateBox)
{
  pqCMBSceneObjectBase *obj;
  if (node->isTypeNode())
    {
    pqCMBSceneNode *objNode;
    SceneObjectNodeIterator iter(node);
    while((objNode = iter.next()))
      {
      this->SelectedLeaves.removeAt(this->SelectedLeaves.indexOf(objNode));
      obj= objNode->getDataObject();

      // Is this the node being used for color?
      if (this->ColorNode == objNode)
        {
        this->clearColorNode();
        }
//      if(obj->getType() != pqCMBSceneObjectBase::Line)
//        {
        obj->setSelectionInput(NULL);
//        }
      }
    }
  else
    {
    obj= node->getDataObject();
    this->SelectedLeaves.removeAt(this->SelectedLeaves.indexOf(node));
    if (node == this->ColorNode)
      {
      this->clearColorNode();
      }
//    if(obj->getType() != pqCMBSceneObjectBase::Line)
//      {
      obj->setSelectionInput(NULL);
//      }
    }

  this->RemoveBoxWidgetPropertyLinks();

  if (updateBox)
    {
    this->updateBoxWidget();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::RemoveBoxWidgetPropertyLinks()
{
  if (this->Internal->PositionLink)
    {
    this->Internal->PositionLink->RemoveAllLinks();
    this->Internal->PositionLink->Delete();
    this->Internal->PositionLink = 0;
    this->Internal->OrientationLink->RemoveAllLinks();
    this->Internal->OrientationLink->Delete();
    this->Internal->OrientationLink = 0;
    this->Internal->ScaleLink->RemoveAllLinks();
    this->Internal->ScaleLink->Delete();
    this->Internal->ScaleLink = 0;
    }

}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::prepSelectedObject(pqCMBSceneObjectBase *obj)
{
  if(!obj)
    {
    return;
    }

  if(obj->getType() == pqCMBSceneObjectBase::Line)
    {
    return;
    }

  if (obj->getRepresentation() == NULL)
    {
    return;
    }

  vtkSMProxy* selRep;
  if(1 || obj->getType() == pqCMBSceneObjectBase::UniformGrid)
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    vtkSMSourceProxy* selSource;
    selSource = vtkSMSourceProxy::SafeDownCast(
      pxm->NewProxy("sources", "cmbIDSelectionSource"));
    if(obj->getType() == pqCMBSceneObjectBase::Glyph)
      {
      pqSMAdaptor::setElementProperty(
        selSource->GetProperty("FieldType"), vtkSelectionNode::POINT);
      }
    selSource->UpdateProperty("RemoveAllIDs", 1);
    pqSMAdaptor::setElementProperty(
      selSource->GetProperty("InsideOut"), 1);
    selSource->UpdateVTKObjects();

    obj->setSelectionInput(selSource);
    selSource->Delete();
    }

  selRep = obj->getRepresentation()->getProxy();

  if ( obj->getType() == pqCMBSceneObjectBase::Arc )
    {
    //we force the type of rep for contours.
    //since we never ever ever want outline, but instead just
    //want the contour lines colored themselves
    pqSMAdaptor::setElementProperty(selRep->
                                    GetProperty("SelectionUseOutline"), 0);
    pqSMAdaptor::setElementProperty(selRep->
                                    GetProperty("SelectionRepresentation"),2);
    }
  else if (!this->SelectionMode)
    {
    pqSMAdaptor::setElementProperty(selRep->
                                    GetProperty("SelectionUseOutline"), 1);
    }
  else
    {
    pqSMAdaptor::setElementProperty(selRep->
                                    GetProperty("SelectionUseOutline"), 0);
    pqSMAdaptor::setElementProperty(selRep->
                                    GetProperty("SelectionRepresentation"),
                                    this->SelectionMode -1);
    }
  selRep->UpdateVTKObjects();

  // Is there no node being used for color?
  if (!this->ColorNode)
    {
    if(obj->getType() != pqCMBSceneObjectBase::Arc)
      {
      // to be consistent from previous version
      if(obj->getRepresentation())
        {
        obj->getRepresentation()->getProxy()->GetProperty(
         "Visibility")->SetPanelVisibility("default");
        }
      this->setDisplayRepresentation(obj->getRepresentation());
      }

    this->Internal->VTKColorConnect->
      Connect(selRep->GetProperty("DiffuseColor"),
              vtkCommand::ModifiedEvent,
              this, SLOT(updateNodeColor()));
    this->Internal->VTKOpacityConnect->
      Connect(selRep->GetProperty("Opacity"),
              vtkCommand::ModifiedEvent,
              this, SLOT(updateNodeColor()));
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::selectNode(pqCMBSceneNode *node,
                                            bool updateBox)
{
  pqCMBSceneObjectBase *obj = NULL;
  /*vtkSMProxyManager* pxm = */vtkSMProxyManager::GetProxyManager();
  //vtkSMSourceProxy* selSource;
  //vtkSMProxy* selRep;
  pqCMBSceneNode *objNode = NULL;

  if (node->isTypeNode())
    {
    SceneObjectNodeIterator iter(node);
    while((objNode = iter.next()))
      {
      this->SelectedLeaves.append(objNode);
      obj= objNode->getDataObject();
      this->prepSelectedObject(obj);

      // Is there no node being used for color?
      if (!this->ColorNode && obj->getType() != pqCMBSceneObjectBase::Line)
        {
        this->ColorNode  = objNode;
        }
      }
    }
  else
    {
    // Leaf Node was selected
    obj= node->getDataObject();
    this->SelectedLeaves.append(node);
    this->prepSelectedObject(obj);

    // Is there no node being used for color?
    if (!this->ColorNode && obj->getType() != pqCMBSceneObjectBase::Line)
      {
      this->ColorNode  = node;
      }
    }

  if (updateBox)
    {
    this->updateBoxWidget();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateBoxWidget()
{
  // Get the bounds of all selected nodes and determine if there
  // is more than one node selected
  int numMoveableObj = 0;
  pqCMBSceneObjectBase *moveable = NULL;
  pqCMBSceneObjectBase *obj;
  // Always remove existing links
  this->RemoveBoxWidgetPropertyLinks();
  bool foundGlyph = false;
  QList<pqCMBSceneNode *>::iterator iter;
  // Check to see if there are any moveable leaf nodes
  for (iter = this->SelectedLeaves.begin();
       iter != this->SelectedLeaves.end();
       ++iter)
    {
    obj = (*iter)->getDataObject();
    if (obj && obj->getSource() && (!obj->isFullyConstrained())
      && !obj->isDefaultConstrained())
      {
      ++numMoveableObj;
      moveable = obj;
      if (obj->getType() == pqCMBSceneObjectBase::Glyph)
        {
        foundGlyph = true;
        }
      }
    }
  // If there are no leaf nodes then turn off widget
  if (!numMoveableObj)
    {
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
                                    GetProperty("Visibility"), false);
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
                                    GetProperty("Enabled"), false);
    this->Internal->BoxWidget->UpdateVTKObjects();
    return;
    }

  double bounds[6];
  bool bbValid = false;
  // If there is only one object selected then set up the widget
  // using the object's transformation
  if ((!foundGlyph) && (numMoveableObj == 1))
    {
    vtkSMProxy *srcProxy = moveable->getRepresentation()->getProxy();
    this->Internal->BoxWidget->GetProperty("Position")->
      Copy(srcProxy->GetProperty("Position"));
    this->Internal->PositionLink = vtkSMPropertyLink::New();
    this->Internal->PositionLink->
      AddLinkedProperty(srcProxy, "Position",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);
    this->Internal->PositionLink->
      AddLinkedProperty(this->Internal->BoxWidget, "Position",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);

    this->Internal->BoxWidget->GetProperty("Rotation")->
      Copy(srcProxy->GetProperty("Orientation"));
    this->Internal->OrientationLink = vtkSMPropertyLink::New();
    this->Internal->OrientationLink->
      AddLinkedProperty(srcProxy, "Orientation",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);
    this->Internal->OrientationLink->
      AddLinkedProperty(this->Internal->BoxWidget, "Rotation",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);

    this->Internal->BoxWidget->GetProperty("Scale")->
      Copy(srcProxy->GetProperty("Scale"));
    this->Internal->ScaleLink = vtkSMPropertyLink::New();
    this->Internal->ScaleLink->
      AddLinkedProperty(srcProxy, "Scale",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);
    this->Internal->ScaleLink->
      AddLinkedProperty(this->Internal->BoxWidget, "Scale",
                        vtkSMLink::INPUT|vtkSMLink::OUTPUT);

    moveable->getDataBounds(bounds);
    bbValid = vtkBoundingBox::IsValid(bounds);
    if(bbValid)
      {
      QList<QVariant> values;
      values << bounds[0] << bounds[1] << bounds[2]
      << bounds[3] << bounds[4] << bounds[5];
      pqSMAdaptor::setMultipleElementProperty(this->Internal->BoxWidget->
        GetProperty("PlaceWidget"),
        values);
       // There is no translation, orientation, or scale necessary
      this->BoxTranslation[0] = this->BoxTranslation[1] =
        this->BoxTranslation[2] = 0.0;
      this->BoxOrientation[0] = this->BoxOrientation[1] =
        this->BoxOrientation[2] = 0.0;
      this->BoxScale[0] = this->BoxScale[1] =
        this->BoxScale[2] = 1.0;
      }
    }
  else // More then one object selected
    {
    // Calculate the bounds of the selected leaves
    vtkBoundingBox bb;
    for (iter = this->SelectedLeaves.begin();
         iter != this->SelectedLeaves.end();
         ++iter)
      {
      obj = (*iter)->getDataObject();
      if (obj && obj->getSource() && (!obj->isFullyConstrained()))
        {
        vtkBoundingBox tmpbb;
        if((*iter)->getBounds(&tmpbb))
          {
          bb.AddBox(tmpbb);
          }
        }
      }
    // We need to "adjust" the bounding box so that the center is at
    // the origin
    bb.GetBounds(bounds);
    bbValid = vtkBoundingBox::IsValid(bounds);
    if(bbValid)
      {
      bb.GetCenter(this->BoxTranslation);
      // The default orientation and scale :
      this->BoxOrientation[0] = this->BoxOrientation[1] =
        this->BoxOrientation[2] = 0.0;
      this->BoxScale[0] = this->BoxScale[1] =
        this->BoxScale[2] = 1.0;
      QList<QVariant> values;
      values << (bounds[0] - this->BoxTranslation[0])
             << (bounds[1] - this->BoxTranslation[0])
             << (bounds[2] - this->BoxTranslation[1])
             << (bounds[3] - this->BoxTranslation[1])
             << (bounds[4] - this->BoxTranslation[2])
             << (bounds[5] - this->BoxTranslation[2]);
      pqSMAdaptor::setMultipleElementProperty(this->Internal->BoxWidget->
                                              GetProperty("PlaceWidget"),
                                              values);
      vtkSMPropertyHelper(this->Internal->BoxWidget,
                          "Position").Set(this->BoxTranslation, 3);
      double ori[3], scale[3];
      ori[0] = ori[1] = ori[2] = 0.0;
      scale[0] = scale[1] = scale[2] = 1.0;
      vtkSMPropertyHelper(this->Internal->BoxWidget,
                          "Rotation").Set(ori, 3);
      vtkSMPropertyHelper(this->Internal->BoxWidget,
                          "Scale").Set(scale, 3);
      }
    }

  pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
    GetProperty("Visibility"), bbValid);
  pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
    GetProperty("Enabled"), bbValid);
  this->Internal->BoxWidget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::zoomOnSelection()
{
  QList<pqCMBSceneNode *>::iterator iter;
  vtkBoundingBox bb;
  for (iter = this->SelectedLeaves.begin();
       iter != this->SelectedLeaves.end();
       ++iter)
    {
    if((*iter)->getDataObject()->getRepresentation() ||
      (*iter)->getDataObject()->getType()==pqCMBSceneObjectBase::Line ||
      (*iter)->getDataObject()->getType()==pqCMBSceneObjectBase::Arc)
      {
      vtkBoundingBox tmpbb;
      if((*iter)->getBounds(&tmpbb))
        {
        bb.AddBox(tmpbb);
        }
      }
    }

  if (bb.IsValid())
    {
    double bounds[6];
    bb.GetBounds(bounds);
    pqView* view = pqActiveObjects::instance().activeView();
    pqRenderView* renderView = qobject_cast<pqRenderView*>(view);
    if (renderView)
      {
      vtkSMRenderViewProxy* rm = renderView->getRenderViewProxy();
      rm->ResetCamera(bounds);
      renderView->render();
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::setSelectionMode(int mode)
{
  if (mode == this->SelectionMode)
    {
    return;
    }
  this->SelectionMode = mode;
  // Update all currently selected nodes
  QList<pqCMBSceneNode *>::iterator iter;
  for (iter = this->SelectedLeaves.begin();
       iter != this->SelectedLeaves.end();
       ++iter)
    {
    vtkSMProxy* selRep =
      (*iter)->getDataObject()->getRepresentation()->getProxy();
    if (selRep)
      {
      if (!this->SelectionMode)
        {
        pqSMAdaptor::setElementProperty(selRep->
                                        GetProperty("SelectionUseOutline"),
                                        1);
        }
      else
        {
        pqSMAdaptor::setElementProperty(selRep->
                                        GetProperty("SelectionUseOutline"),
                                        0);
        pqSMAdaptor::setElementProperty(selRep->
                                        GetProperty("SelectionRepresentation"),
                                        this->SelectionMode -1);
        }
      selRep->UpdateVTKObjects();
      }
    }
  this->activeRenderView()->render();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onSaveAsData()
{
  if (this->Tree->isEmpty())
    {
    this->Internal->OutputFileName.clear();
    return;
    }

  QString filters = "SceneGen Files (*.sg);;";
  filters += "All files (*)";
  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Save Model:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->Internal->OutputFileName = files[0];
      this->saveData(files[0]);
      emit this->sceneSaved();
      return;
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onPackageScene()
{
  if (this->Tree->isEmpty())
    {
    this->Internal->OutputFileName.clear();
    return;
    }

  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Package Scene:"), QString());
  file_dialog.setObjectName("Package Scene Dialog");
  file_dialog.setFileMode(pqFileDialog::Directory);
  if (file_dialog.exec() != QDialog::Accepted)
    {
    return;
    }

  QStringList files = file_dialog.getSelectedFiles();

  if (files.size() == 0)
    {
    return;
    }
  QFileInfo finfo(this->Internal->OutputFileName);
  QDir fdir(files[0]);
  QString fname = fdir.absoluteFilePath(finfo.fileName());
  pqCMBSceneV2Writer writer;
  writer.setFileName(fname.toStdString().c_str());
  writer.setTree(this->Tree);
  writer.write(true);
}

//-----------------------------------------------------------------------------
QString pqCMBSceneBuilderMainWindowCore::getOutputFileName() const
{
  return this->Internal->OutputFileName;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onSaveData()
{
  if (this->Tree->isEmpty())
    {
    this->Internal->OutputFileName.clear();
    return;
    }

  if (this->Internal->OutputFileName == "")
    {
    this->onSaveAsData();
    }
  else
    {
    this->saveData(this->Internal->OutputFileName);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::saveData(const QString& filename)
{
  if (this->Tree->isEmpty())
    {
    return;
    }
  pqCMBSceneV2Writer writer;
  writer.setFileName(filename.toStdString().c_str());
  writer.setTree(this->Tree);
  writer.write();
  this->Internal->OutputFileName = filename;
  emit this->sceneSaved();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onCloseData()
{
  if (this->checkForPreviewDialog())
    {
    return;
    }

  this->closeData();
  emit this->newSceneLoaded();
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onCreateOmicronInput()
{
  // dialog to get filename and necessary parameters
  // get input filename...
  pqFileDialog file_dialog(this->getActiveServer(),
                           this->parentWidget(),
                           "Save Omicron Input As:", QString(),
                           "Omicron \"model\" Input files (*.dat)");
  file_dialog.setObjectName("SaveAsOmicronInputDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() != QDialog::Accepted)
    {
    return;
    }
  QStringList files = file_dialog.getSelectedFiles();
  if (files.size() == 0)
    {
    return;
    }

  if (!setupModelMesherDialog())
    {
    return;
    }

  if(this->Internal->MesherOptionDlg->exec() == QDialog::Accepted)
    {
    this->createOmicronInput(&files[0], this->computeVolumeContraint(0.01));
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onExport2DBCSFile()
{
  // dialog to get filename and necessary parameters
  // get input filename...
  pqFileDialog file_dialog(this->getActiveServer(),
    this->parentWidget(),
    "Save 2D BCS File As:", QString(),
    "2D BCS files (*.bcs)");
  file_dialog.setObjectName("SaveAs2DBCSDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() != QDialog::Accepted)
    {
    return;
    }
  QStringList files = file_dialog.getSelectedFiles();
  if (files.size() == 0)
    {
    return;
    }

  QString saveError;
  pqCMBSceneNode *contourTypeNode = this->Tree->getArcTypeNode();
  if (!contourTypeNode || !contourTypeNode->isTypeNode() ||
      contourTypeNode->getChildren().size()==0)
    {
    saveError = "No valid contour found !";
    }
  pqCMBArc *contourObj = static_cast<pqCMBArc*>(
    contourTypeNode->getChildren().at(0)->getDataObject());
  if(!contourObj)
    {
    saveError = "No valid contour found !";
    }
  vtkPVArcInfo* arcInfo = contourObj->getArcInfo();
  if(arcInfo && arcInfo->GetNumberOfPoints())
    {
    vtkSmartPointer<vtkSGXMLBCSWriter> bcsWriter =
      vtkSmartPointer<vtkSGXMLBCSWriter>::New();
    bcsWriter->SetFileName(files[0].toAscii().constData());
    bcsWriter->SetCoords(arcInfo->GetPointLocations());
    bcsWriter->SetModelVertexIds(arcInfo->GetEndNodeIds());
    bcsWriter->Modified();
    bcsWriter->Update();
    }
  else
    {
    saveError = "No valid contour found !";
    }
  if(!saveError.isEmpty())
    {
    QMessageBox::critical(this->parentWidget(), "Save BCS Error",saveError);
    }
}

//-----------------------------------------------------------------------------
double pqCMBSceneBuilderMainWindowCore::computeVolumeContraint(double volumeFactor)
{
  int voiNodeIndex = this->Internal->MesherOptionDlg->getCurrentVOINameIndex();

  // given voiIndex, figure out which VOI it is
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::VOI );
  nodeIter.reset();
  do
    {
    this->Internal->VOINodeForMesher = nodeIter.next();
    } while(--voiNodeIndex >= 0);

  // get DATA bounds of the VOI; expect just (-1, 1, -1, 1, -1, 1).
  // The transform really "defines" the VOI, but only need the volume, and thus
  // the scale
  double voiBounds[6];
  this->Internal->VOINodeForMesher->getDataObject()->getRepresentation()->
    getOutputPortFromInput()->getDataInformation()->GetBounds(voiBounds);

  double scale[3];
  this->Internal->VOINodeForMesher->getDataObject()->getScale(scale);

  double volume = (voiBounds[1] - voiBounds[0]) * scale[0] *
    (voiBounds[3] - voiBounds[2]) * scale[1] *
    (voiBounds[5] - voiBounds[4]) * scale[2];

  return volume * volumeFactor;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::applyArcToSurface()
{
  if(this->Internal->ArcDepress == NULL)
    return;
  //Step 1: get the set up/run the pipeline on the sever with current modifier manager
  this->Internal->ArcDepress->updatePipeline();

  //Step 3:
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource *pdSource = builder->createSource("sources",
                                                     "HydroModelPolySource", this->getActiveServer());
  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
                            vtkSMSourceProxy::SafeDownCast(this->Internal->ArcDepress->getProxy()));
  pdSource->updatePipeline();
  //destroy the filters and readers

  pqCMBSceneNode *parent = this->Tree->getRoot();
  if (this->Tree->getSelected().size() > 0)
  {
    parent = this->Tree->getSelected()[0]->getParent() != NULL ?
    this->Tree->getSelected()[0]->getParent() : this->Tree->getSelected()[0];
  }
  //Step 2: Create new surface
  pqDataRepresentation* repr = NULL;

  switch( this->Internal->ArcModManager->getSourceType() )
  {
    case pqCMBSceneObjectBase::Points:
      {
      pqCMBPoints *obj = new pqCMBPoints( pdSource,
                                                this->activeRenderView(),
                                                this->getActiveServer(), "" );
      obj->setUserDefinedType("-LIDAR");
      this->Tree->createNode("Arc Modified Surface", parent, obj,NULL);
      repr = obj->getRepresentation();
      break;
      }
    case pqCMBSceneObjectBase::Polygon:
    case pqCMBSceneObjectBase::Faceted:
    {
      pqCMBFacetedObject *obj = new pqCMBFacetedObject(pdSource,
                                                             this->activeRenderView(),
                                                             this->getActiveServer(), "" );
      obj->setSurfaceType(pqCMBSceneObjectBase::Other);
      obj->setUserDefinedType("-Other");
      this->Tree->createNode("Arc Modified Polygon", parent, obj,NULL);
      repr = obj->getRepresentation();
      break;
    }
  default: // Nothing to do
    break;
  };
  repr->getProxy()->UpdateVTKObjects();

  delete this->Internal->ArcDepress;
  this->Internal->ArcDepress = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::arcDeformData(pqCMBSceneObjectBase::enumObjectType dt)
{
  this->Internal->ArcModManager->setSourceType(dt);
  //Step 0: get the selected surface and transform them
  QStringList selectedNames;
  QList<pqPipelineSource*> transformFilters;
  QMap< QString, QList< pqOutputPort* > > namedInputs;
  this->Internal->ArcModOptionDlg->getSelectedSourceNames(selectedNames);
  //TODO Change this
  this->CreatTransPipelineMesh(selectedNames,transformFilters, dt);

  foreach (pqPipelineSource *filter, transformFilters)
  {
    namedInputs["Input"].push_back(filter->getOutputPort(0));
  }

  QObject::connect(this->Internal->ArcModManager, SIGNAL(applyFunctions()),
                   this, SLOT(applyArcToSurface()));

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  bool useNormals = this->Internal->ArcModOptionDlg->getUseNormal();
  // Send each of the nodes down as a input to the filter that will
  // mesh the contour collection with the voi, and than make the TINs
  this->Internal->ArcDepress = builder->createFilter( "filters",
                                                      "ArcDepressFilter", namedInputs,
                                                      this->getActiveServer() );
  {
    vtkSMProxy *proxy = this->Internal->ArcDepress->getProxy();
    vtkSMPropertyHelper helper(proxy, "UseNormalDirection");
    helper.Set((useNormals)?1:0);
    proxy->UpdateProperty("UseNormalDirection");
  }

  if(useNormals)
  {
    this->Internal->ArcModManager->disableAbsolute();
  }
  else
  {
    this->Internal->ArcModManager->enableAbsolute();
  }

  this->Internal->ArcModManager->clearProxies();

  this->Internal->ArcModManager->addProxy("All Source", 0, this->Internal->ArcDepress);

  this->Internal->ArcModManager->showDialog();
}


//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::createSurfaceMesh()
{
  //Step 0: Determine the Bounds of the VOI
  //Step 1: Find all the contour nodes that are visibile and inside the VOI
  //Step 1b: Find all the surface point objects that the user selected in the mesher dialog
  //         and transform all those surface objects by their represenation transformation
  //Step 2: Pass the contours and surface points to the server for extrusion
  //Step 3: Make the extruded poly data an faceted object in the scene

  //Step 0
  int voiNodeIndex = this->Internal->SurfaceMesherOptionDlg->getCurrentVOINameIndex();

  // given voiIndex, figure out which VOI it is
  pqCMBSceneNode *VOINode = NULL;
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::VOI );
  nodeIter.reset();
  do
    {
    VOINode = nodeIter.next();
    } while(--voiNodeIndex >= 0);

  vtkBoundingBox voiBB, nodeBB;
  bool validBounds = VOINode->getBounds(&voiBB);
  if (!validBounds)
    {
    QMessageBox::critical(this->parentWidget(), "Generating Surface Mesh",
     "Invalid Bounds on the VOI");
    return;
    }

  //Step 1 Get all the arc sets that are optional input
  QMap< QString, QList< pqOutputPort* > > namedInputs;
  pqCMBSceneNode *node;
  if ( this->Internal->SurfaceMesherOptionDlg->getMeshVisibleArcSets() )
    {
    nodeIter.reset();
    nodeIter.setTypeFilter(pqCMBSceneObjectBase::Arc);
    while((node = nodeIter.next()))
      {
      if (node->isVisible())
        {
        //cofirm the contour is inside the VOI
        node->getBounds(&nodeBB);
        if ( voiBB.Contains(nodeBB) )
          {
          namedInputs["Source"].push_back(
            node->getDataObject()->getSource()->getOutputPort(0));
          }
        }
      }
    }

  //Step 1b. Get all the points and transform them
  QStringList selectedNames;
  QList<pqPipelineSource*> transformFilters;
  this->Internal->SurfaceMesherOptionDlg->getSelectedSurfaceNames(selectedNames);
  this->CreatTransPipelineMesh(selectedNames,transformFilters, pqCMBSceneObjectBase::Points);

  foreach (pqPipelineSource *filter, transformFilters)
    {
    namedInputs["Input"].push_back(filter->getOutputPort(0));
    }

  //Step 2
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  // Send each of the nodes down as a input to the filter that will
  // mesh the contour collection with the voi, and than make the TINs
  pqPipelineSource *extrudedContours = builder->createFilter("filters",
    "CmbMeshTerrainWithArcs", namedInputs, this->getActiveServer() );
  vtkSMProxy *proxy = extrudedContours->getProxy();

  //send down the bounds of the VOI
  vtkSMPropertyHelper helper(proxy, "VOIBounds");
  double bounds[6];
  voiBB.GetBounds(bounds);
  helper.Set(bounds,6);
  proxy->UpdateProperty("VOIBounds");

  //send the cone radius for elevation smoothing
  vtkSMPropertyHelper rhelper(proxy, "ElevationRadius");
  rhelper.Set(this->Internal->SurfaceMesherOptionDlg->getElevationWeightRadius());
  proxy->UpdateProperty("ElevationRadius");

  extrudedContours->updatePipeline();

  //Step 3:
  pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", this->getActiveServer());
  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(extrudedContours->getProxy()));
    pdSource->updatePipeline();
  //destroy the filters and readers
  builder->destroy( extrudedContours );

  pqCMBSceneNode *parent = this->Tree->getRoot();
  if (this->Tree->getSelected().size() > 0)
    {
    parent = this->Tree->getSelected()[0]->getParent() != NULL ?
      this->Tree->getSelected()[0]->getParent() : this->Tree->getSelected()[0];
    }

  pqCMBFacetedObject *obj = new pqCMBFacetedObject(pdSource,
        this->activeRenderView(),
        this->getActiveServer(), "" );
  obj->setSurfaceType(pqCMBSceneObjectBase::Other);
  obj->setUserDefinedType("-Other");
  this->Tree->createNode("Meshed Surface", parent, obj,NULL);
  pqDataRepresentation* repr = obj->getRepresentation();
  repr->getProxy()->UpdateVTKObjects();

  foreach (pqPipelineSource *filter, transformFilters)
    {
    builder->destroy(filter);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::CreatTransPipelineMesh( const QStringList &selectedNames,
   QList<pqPipelineSource*> &transformFilters, pqCMBSceneObjectBase::enumObjectType dt)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqCMBSceneNode *node;
  pqCMBSceneObjectBase *dataObj;
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );
  nodeIter.setTypeFilter( dt );
  while((node = nodeIter.next()))
    {
    dataObj = node->getDataObject();
    if (selectedNames.contains(node->getName()))
      {
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      dataObj->getTransform( transform );
      vtkMatrix4x4 *matrix = transform->GetMatrix();
      QList<QVariant> values;
      for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
          {
          values << matrix->Element[i][j];
          }
        }

      vtkSMProxy *transformProxy = builder->createProxy("transforms", "Transform",
        this->getActiveServer(), "transforms");
      pqSMAdaptor::setMultipleElementProperty(
        transformProxy->GetProperty("Matrix"), values);
      transformProxy->UpdateVTKObjects();

      pqPipelineSource *transformFilter = builder->createFilter( "filters",
        "TransformFilter", dataObj->getSource());
      pqSMAdaptor::setProxyProperty(
        transformFilter->getProxy()->GetProperty("Transform"), transformProxy);
      transformFilter->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast( transformFilter->getProxy() )->UpdatePipeline();
      transformFilters.push_back( transformFilter );
      }
    }
}

//-----------------------------------------------------------------------------
double pqCMBSceneBuilderMainWindowCore::getMinVOIBounds()
{
  int voiNodeIndex = this->Internal->MesherOptionDlg->getCurrentVOINameIndex();

  // given voiIndex, figure out which VOI it is
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::VOI );
  nodeIter.reset();
  do
    {
    this->Internal->VOINodeForMesher = nodeIter.next();
    } while(--voiNodeIndex >= 0);

  // get bounds of the VOI;
  double voiBounds[6], l[3], minL = 0.01;
  int i;
  this->Internal->VOINodeForMesher->getDataObject()->getBounds(voiBounds);
  l[0] = voiBounds[1] - voiBounds[0];
  l[1] = voiBounds[3] - voiBounds[2];
  l[2] = voiBounds[5] - voiBounds[4];
  bool minSet = false;
  for (i = 0; i < 3; i++)
    {
    if (l[i] < 1.0e-10)
      {
      continue;
      }
    if (!minSet)
      {
      minSet = true;
      minL = l[i];
      continue;
      }
    if (minL > l[i])
      {
      minL = l[i];
      }
    }
  return minL;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::createOmicronInput(QString *filename,
                                                    double /*vConstraintNotUsed*/)
{
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );

  bool isRelative;
  double meshLength = this->Internal->MesherOptionDlg->getMeshLength(isRelative);
  double vlength = this->getMinVOIBounds();
  if (isRelative)
    {
    meshLength *= vlength;
    }
  double areaConstraint = meshLength * meshLength * 0.5;
  double volumeConstraint = areaConstraint * meshLength / 3.0;

  this->Internal->NodeInfo.clear();
  vtkInternal::NodeInfoForBuilder nodeInfo;
  double color[4];

  // color, name, etc info comes from 1st (or only) node that is found in
  // selected list
  QStringList surfaceNames;
  this->Internal->MesherOptionDlg->getSelectedSurfaceNames(surfaceNames);
  pqCMBSceneNode *lidarNode;
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::Points );
  while((lidarNode = nodeIter.next()))
    {
    if (surfaceNames.contains(lidarNode->getName()))
      {
      break;
      }
    }

  if (!lidarNode)
    {
    qDebug() << "Unable to find matching node!";
    return;
    }

  // setup the node info for the lidar node
  lidarNode->getColor(color);
  memcpy(nodeInfo.Color, color, sizeof(double) * 4);
  // if only one surface, then the join character not added
  nodeInfo.UserName = surfaceNames.join(":").toStdString();
  this->Internal->NodeInfo.push_back(nodeInfo);

  pqOmicronModelWriter *writer = new pqOmicronModelWriter;

  if (this->Tree->IsTemporaryPtsFileForMesherNeeded(surfaceNames))
    {
    if (this->Internal->MesherOptionDlg->getTemporaryPtsFileName().size() == 0)
      {
      qDebug() << "Must specify new pts filename!";
      delete writer;
      return;
      }

    std::string tmpPtsFileName =
      this->Internal->MesherOptionDlg->getTemporaryPtsFileName().toStdString();
    QFileInfo finfo(tmpPtsFileName.c_str());

    // make a quick pass to see if perhaps we already created the temporary pts file
    bool tempPtsFileNotYetWritten = true;
    std::map<std::string, QStringList>::const_iterator tmpPtsFileIter =
      this->Internal->TemporaryPointsFileMap.find(tmpPtsFileName);
    if ( tmpPtsFileIter != this->Internal->TemporaryPointsFileMap.end() &&
      finfo.exists() && tmpPtsFileIter->second.size() == surfaceNames.size() )
      {
      // make sure every entry in the previous save matches the current save
      QList<QString>::const_iterator listIter = tmpPtsFileIter->second.begin();
      for (; listIter != tmpPtsFileIter->second.end(); listIter++)
        {
        if (!surfaceNames.contains(*listIter))
          {
          break;
          }
        }
      if (listIter == tmpPtsFileIter->second.end())
        {
        // we have a match, don't rewrite the file!
        tempPtsFileNotYetWritten = false;
        }
      }

    if (tempPtsFileNotYetWritten)
      {
      this->Internal->TemporaryPointsFileMap[tmpPtsFileName] = surfaceNames;

      QStringList nodeNames;
      QList<pqOutputPort*> inputs;
      QList<pqPipelineSource*> transformedSources;
      QList<pqCMBSceneObjectBase*> tempObjects;
      pqCMBPoints *nobj, *obj;
      nodeIter.reset();
      while((lidarNode = nodeIter.next()))
        {
        if (surfaceNames.contains(lidarNode->getName()))
          {
          nodeNames.push_back( lidarNode->getName() );

          obj =
            dynamic_cast<pqCMBPoints*>(lidarNode->getDataObject());
          if (obj && obj->getReaderSource() && obj->getPieceOnRatio() != 1)
            {
            // get data object full resolution
            nobj = new pqCMBPoints(this->Tree->getCurrentServer(),
                                      this->Tree->getCurrentView(),
                                      obj->getReaderSource(),
                                      obj->getPieceId(), 1,
                                      obj->getDoubleDataPrecision());

            double origin[3], position[3], orientation[3], scale[3];
            obj->getOrigin(origin);
            obj->getPosition(position);
            obj->getOrientation(orientation);
            obj->getScale(scale);
            nobj->setOrigin(origin);
            nobj->setPosition(position);
            nobj->setOrientation(orientation);
            nobj->setScale(scale);
            // keep track of any objects we create... so we can cleanup (avoid leaking)
            tempObjects.push_back( nobj );
            }

          // even if was at full resolution, may have been transformed...
          // get transformed now (if not transformed call won't do/create anything)
          pqPipelineSource *transformedSource =
            obj->getTransformedSource(this->Tree->getCurrentServer());
          if (transformedSource != obj->getSource())
            {
            // need to keep track of any transformedSources to avoid leaking
            transformedSources.push_back( transformedSource );
            }
          inputs.push_back( transformedSource->getOutputPort(0) );
          }
        }

      QMap<QString, QList<pqOutputPort*> > namedInputs;
      namedInputs["Input"] = inputs;

      pqObjectBuilder* const builder = pqApplicationCore::instance()->getObjectBuilder();
      pqPipelineSource *ptsWriter = builder->createFilter(
        "writers", "LIDARWriter", namedInputs, this->getActiveServer() );
      vtkSMPropertyHelper(ptsWriter->getProxy(), "WriteAsSinglePiece").Set(true);
      vtkSMPropertyHelper(ptsWriter->getProxy(), "FileName").Set(
        tmpPtsFileName.c_str() );
      ptsWriter->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast(ptsWriter->getProxy())->UpdatePipeline();

      builder->destroy(ptsWriter);

      // destroy any transformed sources
      QList<pqPipelineSource*>::const_iterator srcIter = transformedSources.begin();
      for (; srcIter != transformedSources.end(); srcIter++)
        {
        builder->destroy(*srcIter);
        }

      // destroy any created data objects
      QList<pqCMBSceneObjectBase*>::const_iterator objIter = tempObjects.begin();
      for (; objIter != tempObjects.end(); objIter++)
        {
        delete *objIter;
        }
      }

    // This points file is formed using LIDAR pieces...
    std::string pieceComment;
    pieceComment = "This points file is formed using LIDAR pieces: " + surfaceNames[0].toStdString();
    for (int i = 1; i < surfaceNames.count(); i++)
      {
      pieceComment += ", " + surfaceNames[i].toStdString();
      }
    writer->SetHeaderComment( pieceComment.c_str() );

    // come up with exec directory... omicron expects to be in pt data directory
    this->setProcessExecDirectory(finfo.absolutePath());

    writer->SetSurfaceFileName( finfo.fileName().toStdString().c_str() );

    // position, orientation, scale...
    double zeros[3] = {0.0, 0.0, 0.0};
    writer->SetSurfacePosition(zeros);
    writer->SetSurfaceOrientation(zeros);
    writer->SetSurfaceInitialTranslation(zeros);
    writer->SetSurfaceScale(0.0);
    }
  else // single LIDAR Pts piece in Pts file
    {
    // come up with exec directory... omicron expects to be in pt data directory
    pqCMBPoints *obj =
      dynamic_cast<pqCMBPoints*>(lidarNode->getDataObject());
    this->setProcessExecDirectory(QString(vtksys::SystemTools::GetFilenamePath(
      obj->getFileName() ).c_str()));

    writer->SetSurfaceFileName( vtksys::SystemTools::GetFilenameName(
      obj->getFileName() ).c_str() );

    QList<QVariant> position, orientation, scale;
    position = pqSMAdaptor::getMultipleElementProperty(
      obj->getRepresentation()->getProxy()->GetProperty("Position"));
    orientation = pqSMAdaptor::getMultipleElementProperty(
      obj->getRepresentation()->getProxy()->GetProperty("Orientation"));
    scale = pqSMAdaptor::getMultipleElementProperty(
      obj->getRepresentation()->getProxy()->GetProperty("Scale"));

    double initialTranslation[3];
    obj->getInitialSurfaceTranslation(initialTranslation);

    writer->SetSurfaceInitialTranslation(initialTranslation);
    writer->SetSurfacePosition(
      position[0].toDouble(), position[1].toDouble(), position[2].toDouble() );
    writer->SetSurfaceOrientation(
      orientation[0].toDouble(), orientation[1].toDouble(), orientation[2].toDouble() );
    writer->SetSurfaceScale( scale[0].toDouble() );
    }

  if (!filename)
    {
    // write omicron input file to base directory
    this->Internal->OmicronInputFileName =
      this->getProcessExecDirectory() + "/omicronMesherInput.dat";
    writer->SetFileName(this->Internal->OmicronInputFileName.toStdString().c_str());
    }
  else
    {
    writer->SetFileName(filename->toStdString().c_str());
    }

  // MesherOutputPrefix either comes from dialog (if it does, pass on to the writer)
  // or from the surface (Pts) file name;  no extension, just the base name
  this->Internal->MesherOutputPrefix.clear();
  QString baseName = this->Internal->MesherOptionDlg->getOmicronBaseName();
  if (!baseName.isEmpty())
    {
    QFileInfo finfo(baseName);
    this->Internal->MesherOutputPrefix = finfo.baseName();
    }
  if (!this->Internal->MesherOutputPrefix.isEmpty())
    {
    writer->SetOutputBaseName( this->Internal->MesherOutputPrefix.toStdString().c_str() );
    }
  else
    {
    QFileInfo finfo( writer->GetSurfaceFileName() );
    this->Internal->MesherOutputPrefix = finfo.baseName();
    }

  this->Internal->LastMesherPathIndex =
    this->Internal->MesherOptionDlg->getCurrentMesherPathIndex();

  // VOINodeForMesher set in computeVolumeConstraint
  writer->SetVOI( this->Internal->VOINodeForMesher->getDataObject() );
  writer->SetAreaConstraint(areaConstraint);
  writer->SetVolumeConstraint(volumeConstraint);
  writer->SetDiscConstraint(this->Internal->MesherOptionDlg->getInterpolatingRadius() * meshLength);

  // loop through the solids... only include ones that are visible and in the VOI
  // Also, need to include those solid meshes.
  //nodeIter.setTypeFilter(pqCMBSceneObjectBase::Faceted,  pqCMBSceneObjectBase::Solid );
  nodeIter.addObjectTypeFilter(pqCMBSceneObjectBase::Faceted);
  nodeIter.addObjectTypeFilter(pqCMBSceneObjectBase::SolidMesh);
  nodeIter.reset();

  pqCMBSceneNode *solidNode = NULL;
  pqCMBVOI *mesherVOI = NULL;
  mesherVOI = dynamic_cast<pqCMBVOI*>(this->Internal->VOINodeForMesher->getDataObject());

  while ((solidNode = nodeIter.next()) != 0)
    {
    // 1st, are we "visible" (don't care what our opacity is)
    if (solidNode->isVisible())
      {
      bool solidInVOI = mesherVOI->contains(solidNode->getDataObject());
      if (solidInVOI)
        {
        if (solidNode->getDataObject()->getType() == pqCMBSceneObjectBase::Faceted)
          {
          if (dynamic_cast<pqCMBFacetedObject *>(solidNode->getDataObject())->
            getSurfaceType() != pqCMBSceneObjectBase::Solid)
            {
            continue;
            }
          }

        // color
        solidNode->getColor(color);
        memcpy(nodeInfo.Color, color, sizeof(double) * 4);

        // name
        nodeInfo.UserName = solidNode->getName();
        // object
        nodeInfo.Object = solidNode->getDataObject();
        this->Internal->NodeInfo.push_back(nodeInfo);

        writer->AddSolid(solidNode->getDataObject());
        }
      }
    }

  writer->Write();

  delete writer;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onSpawnSurfaceMesher()
{
  if (!this->setupSurfaceMesherDialog())
    {
    return;
    }
  if(this->Internal->SurfaceMesherOptionDlg->exec() == QDialog::Accepted)
    {
    this->createSurfaceMesh();
    }
  else
    {
    return;
    }
}

//-----------------------------------------------------------------------------

void pqCMBSceneBuilderMainWindowCore::modifierInputSelectionType()
{
  this->setupModifierDialog(this->Internal->ArcModOptionDlg->GetSourceType());
}

//-----------------------------------------------------------------------------

void pqCMBSceneBuilderMainWindowCore::onSpawnArcModifier()
{
  this->modifierInputSelectionType();

  if(this->Internal->ArcModOptionDlg->exec() == QDialog::Accepted)
  {
    this->arcDeformData(this->Internal->ArcModOptionDlg->GetSourceType());
  }
  else
  {
    return;
  }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onSpawnModelMesher()
{
  if (!setupModelMesherDialog())
    {
    return;
    }

  // if only 1, then it will be "Choose Mesher"... force user to select before
  // firing up the dialog
  if ( this->Internal->MesherOptionDlg->getNumberOfMesherPaths() == 1 &&
    !this->selectSurfaceMesher() )
    {
    qDebug() << "Must select a surface mesher to create mesh!";
    return;
    }

  if(this->Internal->MesherOptionDlg->exec() == QDialog::Accepted)
    {
    this->createOmicronInput(0, this->computeVolumeContraint(0.01));
    }
  else
    {
    return;
    }

  // the  output filename... find a unique filename
  QString mesherOutputFileName = this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + ".vtk";
  QFileInfo info(mesherOutputFileName);
  if (info.exists() == true)
    {
    if (QMessageBox::question(this->parentWidget(),
      "Remove duplicate file?",
      "Output mesh file\n  " + mesherOutputFileName +
      "\nalready exists.  Continue (the file will be deleted)?",
      QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
      {
      QFile::remove( mesherOutputFileName );
      }
    else
      {
      return;
      }
    }

  QString execString;
  if (this->Internal->MesherOptionDlg->getCurrentMesherPath() ==
    "Internal Mesher")
    {
    execString = this->Internal->DefaultSurfaceMesher;
    }
  else
    {
    execString = this->Internal->MesherOptionDlg->getCurrentMesherPath();
    }

  const QString inputString = "omicronMesherInput.dat";
  const QString commandString = execString + ";" + inputString;

  QObject::connect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)),
    this, SLOT(previewMesherOutput()));

  remus::meshtypes::SceneFile scene;
  remus::meshtypes::Mesh3DSurface surface;
  remus::common::MeshIOType mtype(scene, surface);

  //the hardcoded separator that command uses
  const QString jobData = this->getProcessExecDirectory() + ";" + commandString;
  QPointer<qtCMBMeshingMonitor> monitor = this->meshServiceMonitor();
  remus::client::Client client(monitor->connection());
  remus::proto::JobRequirementsSet canMesh = client.retrieveRequirements(mtype);
  if (canMesh.size())
    {
    remus::proto::JobSubmission submission(*canMesh.begin());
    submission["data"] = remus::proto::make_JobContent(jobData.toStdString());
    remus::proto::Job j = client.submitJob(submission);
    monitor->monitorJob(j);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::checkForChooseMesher(int mesherIndex)
{
  if (this->Internal->MesherOptionDlg->getMesherPath(mesherIndex) == "Choose Mesher")
    {
    if ( !this->selectSurfaceMesher() )
      {
      //reset to previous selected index (if user didn't select
      this->Internal->MesherOptionDlg->setCurrentMesherPathIndex(
        this->Internal->LastMesherPathIndex );
      }
    }
}

//-----------------------------------------------------------------------------
int pqCMBSceneBuilderMainWindowCore::selectSurfaceMesher()
{
  QString newMesher;
  if (this->getExistingFileName("All files (*);;Windows Executable (*.exe)",
      "Select surface mesher", newMesher) == true)
    {
    for (int i = 0; i < this->Internal->MesherOptionDlg->getNumberOfMesherPaths(); i++)
      {
      if (newMesher == this->Internal->MesherOptionDlg->getMesherPath(i))
        {
        this->Internal->MesherOptionDlg->setCurrentMesherPathIndex(i);
        return 1;
        }
      }

    this->Internal->MesherOptionDlg->blockSignals(true);
    this->Internal->MesherOptionDlg->insertMesherPath(
      this->Internal->MesherOptionDlg->getNumberOfMesherPaths() - 1,
      newMesher.toAscii().constData());
    this->Internal->MesherOptionDlg->setCurrentMesherPathIndex(
      this->Internal->MesherOptionDlg->getNumberOfMesherPaths() - 2);
    this->Internal->LastMesherPathIndex =
      this->Internal->MesherOptionDlg->getCurrentMesherPathIndex();
    this->Internal->MesherOptionDlg->blockSignals(false);
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int pqCMBSceneBuilderMainWindowCore::setupSurfaceMesherDialog()
{
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );

  // any selected nodes that are surfaces will be initially selected pieces
  QList<pqCMBSceneNode*> selectedNodes;
  if (this->Tree->getSelected().size() > 0)
    {
    std::vector<pqCMBSceneNode *>::const_iterator selectedNodesIter;
    for (selectedNodesIter = this->Tree->getSelected().begin();
      selectedNodesIter != this->Tree->getSelected().end(); selectedNodesIter++)
      {
      if ((*selectedNodesIter)->getDataObject() &&
        (*selectedNodesIter)->getDataObject()->getType() == pqCMBSceneObjectBase::Points)
        {
        selectedNodes.push_back( *selectedNodesIter );
        }
      }
    }

  // setup list for surfaces (LIDAR data)
  this->Internal->SurfaceMesherOptionDlg->removeAllSurfaceNames();
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::Points );
  nodeIter.reset();
  pqCMBSceneNode *surfaceNode;
  int surfaceIndex = 0;
  QList<int> selectedSurfaceIndices;
  while ((surfaceNode = nodeIter.next()) != 0)
    {
    if (selectedNodes.contains(surfaceNode))
      {
      selectedSurfaceIndices.push_back( surfaceIndex );
      }
    this->Internal->SurfaceMesherOptionDlg->insertSurfaceName( surfaceIndex++, surfaceNode->getName() );
    }
  // if only one surface, make sure it is selected
  if (this->Internal->SurfaceMesherOptionDlg->getNumberOfSurfaceNames() == 1 &&
    selectedSurfaceIndices.isEmpty())
    {
    selectedSurfaceIndices.push_back(0);
    }

  this->Internal->SurfaceMesherOptionDlg->setSelectedSurfaceNames( selectedSurfaceIndices );

  // if current selected node is VOI, have it be the initially selected VOI in the dialog
  pqCMBSceneNode *CurrentVOINode = NULL;
  if ((this->Tree->getSelected().size() == 1) &&
      this->Tree->getSelected()[0]->getDataObject() &&
      this->Tree->getSelected()[0]->getDataObject()->getType() ==
      pqCMBSceneObjectBase::VOI)
    {
    CurrentVOINode = this->Tree->getSelected()[0];
    }

  // setup dropdown for the VOI's... can have same name multiple times if VOI's
  // given same name (though I don't think that is possible)
  // previously selected VOI is the initial VOI (if it is still in the list)
  this->Internal->SurfaceMesherOptionDlg->removeAllVOINames();
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::VOI );
  nodeIter.reset();
  pqCMBSceneNode *voiNode;
  int voiIndex = 0;
  int initialSelectedIndex = 0;
  while ((voiNode = nodeIter.next()) != 0)
    {
    if (voiNode == CurrentVOINode)
      {
      initialSelectedIndex = voiIndex;
      }
    this->Internal->SurfaceMesherOptionDlg->insertVOIName( voiIndex++, voiNode->getName() );
    }

  // we better have at least one VOI!
  if (voiIndex == 0)
    {
    qDebug() << "Must have at least one VOI to create surface mesh!";
    return 0;
    }

  this->Internal->SurfaceMesherOptionDlg->setCurrentVOINameIndex( initialSelectedIndex );
  return 1;
}

//-----------------------------------------------------------------------------
int pqCMBSceneBuilderMainWindowCore::setupModifierDialog(pqCMBSceneObjectBase::enumObjectType dt)
{
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );

  // any selected nodes that are surfaces will be initially selected pieces
  QList<pqCMBSceneNode*> selectedNodes;
  if (this->Tree->getSelected().size() > 0)
  {
    std::vector<pqCMBSceneNode *>::const_iterator selectedNodesIter;
    for (selectedNodesIter = this->Tree->getSelected().begin();
         selectedNodesIter != this->Tree->getSelected().end(); selectedNodesIter++)
    {
      if ((*selectedNodesIter)->getDataObject() &&
          (*selectedNodesIter)->getDataObject()->getType() == dt)
      {
        selectedNodes.push_back( *selectedNodesIter );
      }
    }
  }

  // setup list for surfaces (LIDAR data)
  this->Internal->ArcModOptionDlg->removeAllSourceNames();
  nodeIter.setTypeFilter( dt );
  nodeIter.reset();
  pqCMBSceneNode *surfaceNode;
  int surfaceIndex = 0;
  QList<int> selectedSurfaceIndices;
  while ((surfaceNode = nodeIter.next()) != 0)
  {
    if (selectedNodes.contains(surfaceNode))
    {
      selectedSurfaceIndices.push_back( surfaceIndex );
    }
    this->Internal->ArcModOptionDlg->insertSourceName( surfaceIndex++, surfaceNode->getName() );
  }
  // if only one surface, make sure it is selected
  if (this->Internal->ArcModOptionDlg->getNumberOfSourceNames() == 1 &&
      selectedSurfaceIndices.isEmpty())
  {
    selectedSurfaceIndices.push_back(0);
  }

  this->Internal->ArcModOptionDlg->setSelectedSourceNames( selectedSurfaceIndices );

  // if current selected node is VOI, have it be the initially selected VOI in the dialog
  pqCMBSceneNode *CurrentVOINode = NULL;
  if ((this->Tree->getSelected().size() == 1) &&
      this->Tree->getSelected()[0]->getDataObject() &&
      this->Tree->getSelected()[0]->getDataObject()->getType() ==
      pqCMBSceneObjectBase::VOI)
  {
    CurrentVOINode = this->Tree->getSelected()[0];
  }

  return 1;
}

//-----------------------------------------------------------------------------
int pqCMBSceneBuilderMainWindowCore::setupModelMesherDialog()
{
  SceneObjectNodeIterator nodeIter( this->Tree->getRoot() );

  // any selected nodes that are surfaces will be initially selected pieces
  QList<pqCMBSceneNode*> selectedNodes;
  if (this->Tree->getSelected().size() > 0)
    {
    std::vector<pqCMBSceneNode *>::const_iterator selectedNodesIter;
    for (selectedNodesIter = this->Tree->getSelected().begin();
      selectedNodesIter != this->Tree->getSelected().end(); selectedNodesIter++)
      {
      if ((*selectedNodesIter)->getDataObject() &&
        (*selectedNodesIter)->getDataObject()->getType() == pqCMBSceneObjectBase::Points)
        {
        selectedNodes.push_back( *selectedNodesIter );
        }
      }
    }

  // setup list for surfaces (LIDAR data)
  this->Internal->MesherOptionDlg->removeAllSurfaceNames();
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::Points );
  nodeIter.reset();
  pqCMBSceneNode *surfaceNode;
  int surfaceIndex = 0;
  QList<int> selectedSurfaceIndices;
  while ((surfaceNode = nodeIter.next()) != 0)
    {
    if (selectedNodes.contains(surfaceNode))
      {
      selectedSurfaceIndices.push_back( surfaceIndex );
      }
    this->Internal->MesherOptionDlg->insertSurfaceName( surfaceIndex++, surfaceNode->getName() );
    }
  // if only one surface, make sure it is selected
  if (this->Internal->MesherOptionDlg->getNumberOfSurfaceNames() == 1 &&
    selectedSurfaceIndices.isEmpty())
    {
    selectedSurfaceIndices.push_back(0);
    }

  this->Internal->MesherOptionDlg->setSelectedSurfaceNames( selectedSurfaceIndices );

  // generate a full path for the initial "NewPts" file
  if (this->Internal->MesherOptionDlg->getTemporaryPtsFileName().isEmpty())
    {
    std::string fullPathName = vtksys::SystemTools::GetRealPath( "defaultName.pts" );
    this->Internal->MesherOptionDlg->setTemporaryPtsFileName(fullPathName.c_str());
    }

  // if current selected node is VOI, have it be the initially selected VOI in the dialog
  if ((this->Tree->getSelected().size() == 1) &&
      this->Tree->getSelected()[0]->getDataObject() &&
      this->Tree->getSelected()[0]->getDataObject()->getType() ==
      pqCMBSceneObjectBase::VOI)
    {
    this->Internal->VOINodeForMesher = this->Tree->getSelected()[0];
    }

  // setup dropdown for the VOI's... can have same name multiple times if VOI's
  // given same name (though I don't think that is possible)
  // previously selected VOI is the initial VOI (if it is still in the list)
  this->Internal->MesherOptionDlg->removeAllVOINames();
  nodeIter.setTypeFilter( pqCMBSceneObjectBase::VOI );
  nodeIter.reset();
  pqCMBSceneNode *voiNode;
  int voiIndex = 0;
  int initialSelectedIndex = 0;
  while ((voiNode = nodeIter.next()) != 0)
    {
    if (voiNode == this->Internal->VOINodeForMesher)
      {
      initialSelectedIndex = voiIndex;
      }
    this->Internal->MesherOptionDlg->insertVOIName( voiIndex++, voiNode->getName() );
    }

  // we better have at least one VOI!
  if (voiIndex == 0)
    {
    qDebug() << "Must have at least one VOI to create surface mesh!";
    return 0;
    }

  this->Internal->MesherOptionDlg->setCurrentVOINameIndex( initialSelectedIndex );
  return 1;
}


//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::previewMesherOutput()
{
  // Normal exit... see if our expected output file exists... and if so,
  // query if we should load it
  QString testMeshName = this->getProcessExecDirectory() + "/Test_Mesh.vtk";
  QString criticalMessage("The surface mesher was unable to generate a mesh based on your inputs.  Please examine both the model and meshing settings for errors");
  QString warningMessage("The scene that you created had intersecting faces around one or multiple objects that intersect the surface.\n\nSelect surface with edges in the dropdown list and zoom in on the objects to look for crossed edges on the object surface at the surface intersection line.\n\nTry turning or slightly moving the object/objects that have crossed edges and remesh.");

  this->PreviewMeshOutput = true;

  QFileInfo info(testMeshName);
  bool problem = false;
  if (info.exists() == false)
    {
    problem = true;
    testMeshName =  this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + ".vtk";
    info.setFile(testMeshName);
    }

  if (info.exists() == false)
    {
    QMessageBox::critical(this->parentWidget(), "Critical Meshing Error",criticalMessage);
    emit enableExternalProcesses(true);
    return;
    }

  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();
  pqRenderView *tmpView = qobject_cast<pqRenderView*>(
    builder->createView(pqRenderView::renderViewType(),
                        this->getActiveServer() ));

  QStringList fileList;
  fileList << testMeshName;
  builder->blockSignals(true);
  pqPipelineSource* reader = builder->createReader("sources", "LegacyVTKFileReader",
      fileList, this->getActiveServer());
  builder->blockSignals(false);
  if (!reader)
    {
    qCritical() << "Unknown file extension " << info.suffix();
    if (!problem)
      {
      QFile::remove(this->getProcessExecDirectory() + "/Test_Mesh.vtk");
      }
    return;
    }
  pqDataRepresentation* repr =
    builder->createDataRepresentation(
    reader->getOutputPort(0), tmpView, "GeometryRepresentation");

  this->previewDialog()->setRepresentationAndView(repr, tmpView);
  if (problem)
    {
    QMessageBox::warning(this->parentWidget(), "Meshing Error",warningMessage);
    }
  this->previewDialog()->enableErrorView(problem);
  this->previewDialog()->show();

  if (!problem)
    {
    QFile::remove(this->getProcessExecDirectory() + "/Test_Mesh.vtk");
    }
}

//----------------------------------------------------------------------------
bool pqCMBSceneBuilderMainWindowCore::exportSolidsCMBModel(
  QList<pqOutputPort*>& inputs, const QString& cmbFileName, bool is2D)
{
  // TODO: implement.
  return false;
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::exportSelectedSolids()
{
  // see what we have for inputs (must be at least 1 solid); use NodeInfo to
  // store solid name and color info which is added after running the builder
  QList<pqOutputPort*> inputs;
  QList<pqCMBSceneNode *>::iterator iter;
  pqCMBSceneObjectBase* dataObj;
  pqCMBFacetedObject *fobj;
  this->Internal->NodeInfo.clear();
  vtkInternal::NodeInfoForBuilder nodeInfo;
  double color[4];
  bool lineObjectsPresent = false;
  bool contourObjectsPresent = false;

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  for (iter = this->SelectedLeaves.begin(); iter != this->SelectedLeaves.end(); iter++)
    {
    dataObj = (*iter)->getDataObject();
    fobj = dynamic_cast<pqCMBFacetedObject*>(dataObj);
    if((*iter)->isVisible() && fobj && fobj->getSurfaceType() == pqCMBSceneObjectBase::Solid)
      {
      inputs.push_back( fobj->getTransformedSource(this->getActiveServer())->getOutputPort(0) );
      (*iter)->getColor(color);
      memcpy(nodeInfo.Color, color, sizeof(double) * 4);
      nodeInfo.UserName = (*iter)->getName();
      this->Internal->NodeInfo.push_back(nodeInfo);
      }
    else if ((*iter)->isVisible() && dataObj->getType() == pqCMBSceneObjectBase::Line)
      {
      lineObjectsPresent = true;
      }
    else if ((*iter)->isVisible() && dataObj->getType() == pqCMBSceneObjectBase::Arc)
      {
      contourObjectsPresent = true;
      }
    }

  if (inputs.size() == 0 && contourObjectsPresent == false)
    {
    QMessageBox::warning(this->parentWidget(), "No Solids or Contours Selected", "Must selected at least 1 solid or contour to export!");
    return;
    }

  // get output filename
  QString filters = "CMB Files(*.cmb)";
  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Save Model:"), QString(), filters);
  file_dialog.setObjectName("CMBExportDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  QString cmbFileName;
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() == 0)
      {
      return;
      }
    cmbFileName = files[0];
    }
  else
    {
    return;
    }

  if(contourObjectsPresent)
    {
    vtkSmartPointer<vtkCMBModelWriterBase> writerBase =
      vtkSmartPointer<vtkCMBModelWriterBase>::New();

    // temporary hack because don't have method to push LineSegment to server (yet)
    vtkDiscreteModelWrapper *hackWrapper = vtkDiscreteModelWrapper::New();
    if ( this->AddContourObjectsToModel(hackWrapper) )
      {
      writerBase->SetFileName( cmbFileName.toAscii().constData() );
      writerBase->Operate(hackWrapper);
      }

    hackWrapper->Delete();
    return;
    }
  bool success = this->exportSolidsCMBModel(inputs, cmbFileName, false);

  if (success && lineObjectsPresent)
    {
    // temporary hack because don't have method to push LineSegment to server (yet)
    vtkDiscreteModelWrapper *hackWrapper = vtkDiscreteModelWrapper::New();

    vtkSmartPointer<vtkCMBModelReadOperator> readerOperator =
      vtkSmartPointer<vtkCMBModelReadOperator>::New();
    readerOperator->SetFileName( cmbFileName.toAscii().constData() );

    readerOperator->Operate(hackWrapper);
    this->AddLineSegmentObjectsToModel(hackWrapper, false);

    vtkSmartPointer<vtkCMBModelWriterBase> writerBase =
      vtkSmartPointer<vtkCMBModelWriterBase>::New();
    writerBase->SetFileName( cmbFileName.toAscii().constData() );
    writerBase->Operate(hackWrapper);

    hackWrapper->Delete();
    }
}
//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::exportSelectedPolygons()
{
  // see what we have for inputs (must be at least 1 solid); use NodeInfo to
  // store solid name and color info which is added after running the builder
  QList<pqOutputPort*> inputs;
  QList<pqCMBSceneNode *>::iterator iter;
  pqCMBSceneObjectBase* dataObj;
  pqCMBPolygon *polygonObj;
  this->Internal->NodeInfo.clear();
  vtkInternal::NodeInfoForBuilder nodeInfo;
  double color[4];

  for (iter = this->SelectedLeaves.begin(); iter != this->SelectedLeaves.end(); iter++)
    {
    dataObj = (*iter)->getDataObject();
    polygonObj = dynamic_cast<pqCMBPolygon*>(dataObj);
    if((*iter)->isVisible() && polygonObj)
      {
      inputs.push_back( polygonObj->getSource()->getOutputPort(0) );
      (*iter)->getColor(color);
      memcpy(nodeInfo.Color, color, sizeof(double) * 4);
      nodeInfo.UserName = (*iter)->getName();
      this->Internal->NodeInfo.push_back(nodeInfo);
      }
    }

  if (inputs.size() == 0)
    {
    QMessageBox::warning(this->parentWidget(), "No Polygons Selected",
      "Must selected at least 1 Polygon to export!");
    return;
    }

  // get output filename
  QString filters = "CMB Files(*.cmb)";
  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Export Polygon CMB Model:"), QString(), filters);
  file_dialog.setObjectName("CMBExportDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  QString cmbFileName;
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() == 0)
      {
      return;
      }
    cmbFileName = files[0];
    }
  else
    {
    return;
    }

  /*bool success = */this->exportSolidsCMBModel(inputs, cmbFileName, true);
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::exportCMBFile()
{
  emit enableExternalProcesses(true);

  QString filters = "CMB Files(*.cmb)";
  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Save Model:"), QString(), filters);
  file_dialog.setObjectName("CMBExportDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  QString cmbFileName;
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() == 0)
      {
      return;
      }
    cmbFileName = files[0];
    }
  else
    {
    return;
    }

  // for now, just going to do this on the client... the basics would be easy
  // to do on the server... passing the scene information to the CMB would be
  // a little more difficult
  vtkSmartPointer<vtkUnstructuredGridReader> reader =
    vtkSmartPointer<vtkUnstructuredGridReader>::New();
  QString mesherOutputFileName = this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + ".vtk";
  reader->SetFileName( mesherOutputFileName.toAscii().constData() );

  vtkSmartPointer<vtkDataSetSurfaceFilter> surface =
    vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surface->SetInputConnection( reader->GetOutputPort() );

  // execute to this point, and then add model face info to the data
  surface->Update();
  this->addOmicronModelFaceData( surface->GetOutput() );

  vtkSmartPointer<vtkAddCellDataFilter> cellDataFilter =
    vtkSmartPointer<vtkAddCellDataFilter>::New();
  cellDataFilter->SetInputConnection( surface->GetOutputPort() );

  vtkSmartPointer<vtkMasterPolyDataNormals> normals =
    vtkSmartPointer<vtkMasterPolyDataNormals>::New();
  normals->SetInputConnection( cellDataFilter->GetOutputPort() );

  vtkSmartPointer<vtkCompleteShells> fixHoles =
    vtkSmartPointer<vtkCompleteShells>::New();
  fixHoles->SetInputConnection(normals->GetOutputPort());


  vtkSmartPointer<vtkCMBModelBuilder> builder =
    vtkSmartPointer<vtkCMBModelBuilder>::New();
  vtkSmartPointer<vtkDiscreteModelWrapper> wrapper =
    vtkSmartPointer<vtkDiscreteModelWrapper>::New();

  builder->Operate(wrapper, fixHoles);

  // omicron has surface as BC 1, and then each solid increasing from there

  QMap<int, pqCMBSolidMesh*> solidMeshes;
  vtkModelItemIterator* regions =
    wrapper->GetModel()->NewIterator(vtkModelRegionType);
  for(regions->Begin(); !regions->IsAtEnd(); regions->Next())
    {
    int regionIndex;
    vtkDiscreteModelRegion* region =
      vtkDiscreteModelRegion::SafeDownCast(regions->GetCurrentItem());

    std::string temp = vtkModelUserName::GetUserName(region);
    if (sscanf(temp.c_str(), "Region%d", &regionIndex) > 0)
      {
      // regionIndex-1 because default region names are 1 based

      // user name
      vtkModelUserName::SetUserName(region,
        this->Internal->NodeInfo[regionIndex-1].UserName.c_str());

      // color
      region->SetColor(
        this->Internal->NodeInfo[regionIndex-1].Color[0],
        this->Internal->NodeInfo[regionIndex-1].Color[1],
        this->Internal->NodeInfo[regionIndex-1].Color[2],
        this->Internal->NodeInfo[regionIndex-1].Color[3]);
      // object
      pqCMBSceneObjectBase* sceneObj = this->Internal->NodeInfo[regionIndex-1].Object;
      if(!sceneObj)
        {
        continue;
        }
      pqCMBSolidMesh* solMesh = dynamic_cast<pqCMBSolidMesh*>(sceneObj);
      if(solMesh)
        {
        vtkIdType regionId = region->GetUniquePersistentId();
        solidMeshes[regionId]=solMesh;
        }
      }
    else
      {
      vtkGenericWarningMacro("Unable to set region color and name!");
      }
    }
  regions->Delete();
  if(solidMeshes.size()>0)
    {
    vtkNew<vtkCMBIncorporateMeshOperator> solidMeshOperator;
    std::vector<vtkSmartPointer<vtkHydroModelPolySource> > solids;
    foreach(vtkIdType regId, solidMeshes.keys())
      {
      vtkSmartPointer<vtkHydroModelPolySource> solSource =
        vtkSmartPointer<vtkHydroModelPolySource>::New();
      solids.push_back(solSource);
      vtkNew<vtkTransform> transform;
      solidMeshes[regId]->getTransform(transform.GetPointer());
      if(this->clientTransformSource(solidMeshes[regId]->getSource(),
        solSource, transform.GetPointer()))
        {
        solidMeshOperator->AddSolidMesh(regId,
          vtkPolyData::SafeDownCast(solSource->GetSource()));
        }
      }
    solidMeshOperator->Operate(wrapper);
    solids.clear();
    if(!solidMeshOperator->GetOperateSucceeded())
      {
      QMessageBox::critical(NULL, "SceneBuilder",
        "Failed to incorporate solid mesh to cmb model!");
      return ;
      }
    }
  // Adding the line segment to the model
  this->AddLineSegmentObjectsToModel(wrapper, true);
  // Adding the contours to the model
  this->AddContourObjectsToModel(wrapper);

  vtkSmartPointer<vtkCMBModelWriterBase> modelWriter =
    vtkSmartPointer<vtkCMBModelWriterBase>::New();
  modelWriter->SetFileName( cmbFileName.toAscii().constData() );
  modelWriter->Operate(wrapper);
}


//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::rejectMesh()
{
  emit enableExternalProcesses(true);

  QFile::remove( this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + ".vtk" );
  QFile::remove( this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + "_Surf3D.vtk" );
  QFile::remove( this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + "_Box_Shell.vtk" );
  QFile::remove( this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + "_BCfaces.vtk" );
  QFile::remove( this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + "_nodes.bc" );
  QFile::remove( this->getProcessExecDirectory() + "/" +
    this->Internal->MesherOutputPrefix + "_faces.bc" );
  QFile::remove( this->Internal->OmicronInputFileName );
  if (this->Internal->MesherOptionDlg->getDeleteCreatedPtsFile())
    {
    QFileInfo finfo(this->Internal->MesherOptionDlg->getTemporaryPtsFileName());
    QFile::remove(this->getProcessExecDirectory() + "/" + finfo.fileName());
    }
}


//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::addOmicronModelFaceData(vtkPolyData *outputPD)
{
  vtkIntArray *regionIDs =
  vtkIntArray::SafeDownCast( outputPD->GetCellData()->GetArray(
                            vtkMultiBlockWrapper::GetShellTagName()) );
  if (regionIDs)
    {
    // depending on whether BCS specified, create model faces either on shells or the BCS file
    QString bcFacesFile = this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + "_BCfaces.vtk";
    vtkSmartPointer<vtkPolyDataReader> faceReader =
      vtkSmartPointer<vtkPolyDataReader>::New();
    faceReader->SetFileName( bcFacesFile.toAscii().constData() );
    faceReader->Update();

    vtkIntArray *modelFaceIDs =
      vtkIntArray::SafeDownCast( faceReader->GetOutput()->GetFieldData()->
      GetArray( "BCtags" ) );

    if (modelFaceIDs &&
      regionIDs->GetNumberOfTuples() == modelFaceIDs->GetNumberOfTuples())
      {
      vtkSmartPointer<vtkIntArray> newModelFaceIDs =
        vtkSmartPointer<vtkIntArray>::New();
      newModelFaceIDs->SetNumberOfValues( modelFaceIDs->GetNumberOfTuples() );
      // want model face ID array to be 0 based
      int i, shellID = 1, modelFaceID, mfID, nextID = 0;
      std::map<int, int> uniqueMF;
      std::map<int, int>::const_iterator uniqueMFIter;
      while(1)
        {
        uniqueMF.clear();
        bool shellIDMatch = false;
        for (i = 0; i < modelFaceIDs->GetNumberOfTuples(); i++)
          {
          if (regionIDs->GetValue(i) == shellID)
            {
            shellIDMatch = true;
            modelFaceID = modelFaceIDs->GetValue(i);
            uniqueMFIter = uniqueMF.find( modelFaceID );
            if (uniqueMFIter == uniqueMF.end())
              {
              uniqueMF[modelFaceID] = nextID;
              mfID = nextID++;
              }
            else
              {
              mfID = uniqueMFIter->second;
              }

            newModelFaceIDs->SetValue(i, mfID);
            }
          }
        if (!shellIDMatch)
          {
          break;
          }
        shellID++;
        }

      newModelFaceIDs->SetName( vtkMultiBlockWrapper::GetModelFaceTagName() );
      outputPD->GetCellData()->AddArray( newModelFaceIDs );
      }
    else if (modelFaceIDs)
      {
      qDebug() << "ERROR: ReaLLLy expected # of cells in shell and model data to match";
      }
    else
      {
      qDebug() << "ERROR: ReaLLLy expected there to be model face data!";
      }
    }
  else
    {
    qDebug() << "ERROR: ReaLLLy expected there to be region/shell data!";
    }
}
//-----------------------------------------------------------------------------
bool pqCMBSceneBuilderMainWindowCore::clientTransformSource(pqPipelineSource* serverSource,
  vtkHydroModelPolySource* clientSource, vtkTransform* transform)
{
  vtkAlgorithm* inAlgorithm = vtkAlgorithm::SafeDownCast(
    serverSource->getProxy()->GetClientSideObject());
  if(!inAlgorithm)
    {
    return false;
    }
  vtkMatrix4x4 *matrix = transform->GetMatrix();
  // if non-identity transform... need to transform the data
  if (matrix->Element[0][0] != 1 || matrix->Element[0][1] != 0 ||
    matrix->Element[0][2] != 0 || matrix->Element[0][3] != 0 ||
    matrix->Element[1][0] != 0 || matrix->Element[1][1] != 1 ||
    matrix->Element[1][2] != 0 || matrix->Element[1][3] != 0 ||
    matrix->Element[2][0] != 0 || matrix->Element[2][1] != 0 ||
    matrix->Element[2][2] != 1 || matrix->Element[2][3] != 0 ||
    matrix->Element[3][0] != 0 || matrix->Element[3][1] != 0 ||
    matrix->Element[3][2] != 0 || matrix->Element[3][3] != 1)
    {
    vtkNew<vtkTransformFilter> transFilter;
    transFilter->SetInputConnection(inAlgorithm->GetOutputPort(0));
    transFilter->SetTransform(transform);
    transFilter->Update();

    clientSource->CopyData(vtkPolyData::SafeDownCast(
      transFilter->GetOutputDataObject(0)));
    }
  else
    {
    clientSource->CopyData(vtkPolyData::SafeDownCast(
      inAlgorithm->GetOutputDataObject(0)));
    }
  return true;
}

//----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onRubberBandSelect(bool checked)
{
  if (checked)
    {
    int isShiftKeyDown = this->activeRenderView()->
    getRenderViewProxy()->GetInteractor()->GetShiftKey();

    if (!isShiftKeyDown && this->Tree->getSelected().size())
      {
      // Clear all existing selections
      this->Tree->clearSelection();
      }

    this->renderViewSelectionHelper()->beginSurfaceSelection();
    }
  else
    {
    this->renderViewSelectionHelper()->endSelection();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::closeData()
{
  this->ColorNode = NULL;
  if (this->Tree->isEmpty())
    {
    return;
    }
  this->Tree->empty();
  this->Internal->CurrentSceneFileName.clear();
  this->Internal->OutputFileName = "";
  this->Superclass::closeData();
}

//-----------------------------------------------------------------------------
/// Called when a new reader is created by the GUI.
void pqCMBSceneBuilderMainWindowCore::onReaderCreated(pqPipelineSource* reader,
  const QString& filename)
{
  if (this->checkForPreviewDialog())
    {
    return;
    }

  if (!reader)
    {
    return;
    }
  pqFileDialogModel model(this->getActiveServer());
  QString fullpath;

  if (!model.fileExists(filename, fullpath))
    {
    QMessageBox::information(NULL, "SceneGen", "The File does not exist!");
    return ;
    }

  /* If this is an old format file - call this->ProcessOldFormat(reader, filename) which is the original SetCurrentSource that expects a multiblock
     else call the version of processOSDLInfo that does not -
     actually this should be called addSource for the non-multiblock and
     addSources for the other
  */
  this->processScene(filename, reader);
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::resetCenterOfRotationToCenterOfCurrentData()
{
  this->Superclass::resetCenterOfRotationToCenterOfCurrentData();
/*
  pqRenderView* rm = qobject_cast<pqRenderView*>(
    pqActiveView::instance().current());

  if(!rm)
    {
    qDebug() << "No active render module. Cannot reset center of rotation.";
  return;
    }
  if(rm->getNumberOfRepresentations()<=0)
    {
  qDebug() << "No active source. Cannot reset center of rotation.";
  return;
    }
  double bounds[6];
  double prev_bounds_volume_sqrd;
  pqDataRepresentation* repr_tmp = qobject_cast<pqDataRepresentation*>(
      rm->getRepresentation(0));

  repr_tmp->getDataBounds(bounds);
  prev_bounds_volume_sqrd = (bounds[0]-bounds[1])*(bounds[2]-bounds[3])*(bounds[4]-bounds[5]);
  prev_bounds_volume_sqrd = prev_bounds_volume_sqrd*prev_bounds_volume_sqrd;

  //Find the object with the largest volume and set the center to its center
  for(int i = 1; i<rm->getNumberOfRepresentations();++i)
    {
    pqDataRepresentation* repr = qobject_cast<pqDataRepresentation*>(
    rm->getRepresentation(i));
  double bounds_tmp[6];
  repr->getDataBounds(bounds_tmp);

    double curr_bounds_volume_sqrd = (bounds_tmp[0]-bounds_tmp[1])*(bounds_tmp[2]-bounds_tmp[3])*(bounds_tmp[4]-bounds_tmp[5]);
  curr_bounds_volume_sqrd = curr_bounds_volume_sqrd*curr_bounds_volume_sqrd;
  if (curr_bounds_volume_sqrd > prev_bounds_volume_sqrd)
    {
    bounds[0] = bounds_tmp[0];
    bounds[1] = bounds_tmp[1];
    bounds[2] = bounds_tmp[2];
    bounds[3] = bounds_tmp[3];
    bounds[4] = bounds_tmp[4];
    bounds[5] = bounds_tmp[5];
    prev_bounds_volume_sqrd = curr_bounds_volume_sqrd;
    }
    }
  double center[3];
  center[0] = (bounds[1]+bounds[0])/2.0;
  center[1] = (bounds[3]+bounds[2])/2.0;
  center[2] = (bounds[5]+bounds[4])/2.0;
  rm->setCenterOfRotation(center);
  rm->render();
*/
}

//-----------------------------------------------------------------------------
// update the state of the \c node if node is not an ancestor of any of the
// non-blockable widgets. If so, then it recurses over all its children.
static void selectiveEnabledInternal(QWidget* node,
  QList<QPointer<QObject> >& nonblockable, bool enable)
{
  if (!node)
    {
    return;
    }
  if (nonblockable.size() == 0)
    {
    node->setEnabled(enable);
    return;
    }

  foreach (QObject* objElem, nonblockable)
    {
    QWidget* elem = qobject_cast<QWidget*>(objElem);
    if (elem)
      {
      if (node == elem)
        {
        // this is a non-blockable wiget. Don't change it's enable state.
        nonblockable.removeAll(elem);
        return;
        }

      if (node->isAncestorOf(elem))
        {
        // iterate over all children and selectively disable each.
        QList<QObject*> children = node->children();
        for (int cc=0; cc < children.size(); cc++)
          {
          QWidget* child = qobject_cast<QWidget*>(children[cc]);
          if (child)
            {
            ::selectiveEnabledInternal(child, nonblockable, enable);
            }
          }
        return;
        }
      }
    }

  // implies node is not an ancestor of any of the nonblockable widgets,
  // we can simply update its enable state.
  node->setEnabled(enable);
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onServerCreationFinished(pqServer *server)
{
  if(this->Tree)
    {
    this->Tree->blockSignals(true);
    }
  this->Superclass::onServerCreationFinished(server);

  //import in the cmb model plugin after the common plugin(s) has been loaded
  //incase it depends on any symbols of the common plugins(s)
  // FIXME: where should this come from?
  //PV_PLUGIN_IMPORT(CMBModel_Plugin)

  if(this->Tree)
    {
    this->Tree->blockSignals(false);
    this->Tree->setCurrentView(this->activeRenderView());
    this->Tree->setCurrentServer(this->getActiveServer());
    }
  this->renderViewSelectionHelper()->setView(this->activeRenderView());

  QObject::connect(this->renderViewSelectionHelper(),
    SIGNAL(selectionFinished(int, int, int, int)),
    this->renderViewSelectionHelper(),
    SLOT(endSelection()));

  pqSMAdaptor::setElementProperty(this->activeRenderView()->
    getProxy()->GetProperty("LODThreshold"),  50);

  this->setupBoxWidget();

}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::setpqCMBSceneTree(pqCMBSceneTree * tree)
{
  this->Tree = tree;
  this->Tree->setCurrentView(this->activeRenderView());
  this->Tree->setCurrentServer(this->getActiveServer());

  if( Internal->ArcModManager != NULL )
    {
    delete Internal->ArcModManager;
    }

  Internal->ArcModManager = new pqCMBModifierArcManager(NULL, this->getActiveServer(),
                                                   this->activeRenderView());


  //make sure the context menu has the new scene tree
  this->Internal->RenderWindowMenuBehavior->setTree(this->Tree);

  QObject::connect(this->Tree,
                   SIGNAL(requestSceneUpdate()),
                   this,
                   SLOT(requestRender()));
  QObject::connect(this->Tree,
                   SIGNAL(selectionUpdated(const QList<pqCMBSceneNode*> *,
                                           const QList<pqCMBSceneNode*> *)),
                   this,
                   SLOT(updateSelection(const QList<pqCMBSceneNode*> *,
                                        const QList<pqCMBSceneNode*> *)));
  QObject::connect(this->Tree,
                   SIGNAL(requestTINStitch()),
                   this,
                   SLOT(stitchTINs()));
  QObject::connect(this->Tree,
                   SIGNAL(requestSolidExport()),
                   this,
                   SLOT(exportSelectedSolids()));
  QObject::connect(this->Tree,
                   SIGNAL(requestPolygonsExport()),
                    this,
                    SLOT(exportSelectedPolygons()));

  this->Internal->VTKBoxConnect->Connect(this->Internal->BoxWidget,
                                         vtkCommand::InteractionEvent,
                                         this->Tree, SLOT(collapseAllDataInfo()));


  this->Internal->MesherOptionDlg = new qtCMBSceneMesherDialog(this->Tree);
  this->Internal->SurfaceMesherOptionDlg =
    new qtCMBSceneSurfaceMesherDialog(this->Tree);
  this->Internal->ArcModOptionDlg = new qtCMBArcModifierInputDialog(this->Tree);
  QObject::connect(this->Internal->ArcModOptionDlg, SIGNAL(sourceTypeChanged()),
                   this, SLOT(modifierInputSelectionType()));
  if (this->Internal->DefaultSurfaceMesher.size() == 0)
    {
    // internal mesher NOT an option
    this->Internal->MesherOptionDlg->removeMesherPath(0);
    }
  this->Internal->MesherOptionDlg->setMeshLength( 0.1, true );
  this->Internal->MesherOptionDlg->setInterpolatingRadius( 0.4 );

  QObject::connect(this->Internal->MesherOptionDlg,
    SIGNAL(mesherSelectionChanged(int)),
    this, SLOT(checkForChooseMesher(int)));
}
//-----------------------------------------------------------------------------
// I'm leaving this here when we need to deal with multiselection

void pqCMBSceneBuilderMainWindowCore::boxWidgetInteraction()
{
  if (!this->Tree->getSelected().size())
    {
    return;
    }

  pqCMBSceneNode * node = this->Tree->getSelected()[0];
  if (!(node || node->isTypeNode()))
    {
    return;
    }

  vtkSMProxy *srcProxy = node->getDataObject()->getRepresentation()->getProxy();
  srcProxy->GetProperty("Position")->Copy(this->Internal->BoxWidget->GetProperty("Position"));
  srcProxy->GetProperty("Orientation")->Copy(this->Internal->BoxWidget->GetProperty("Rotation"));
  srcProxy->GetProperty("Scale")->Copy(this->Internal->BoxWidget->GetProperty("Scale"));
  srcProxy->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::newScene()
{
  if (this->checkForPreviewDialog())
    {
    return;
    }

  // ToDo:: Need to check to see if there are unsaved changed!!

#if 0
  cmbSceneUnits::Enum sceneUnits;
  if (qtCMBNewSceneUnitsDialog::getUnits(cmbSceneUnits::Unknown, sceneUnits))
    {
    this->closeData();
    this->Tree->createRoot("Scene");
    this->Tree->setUnits(sceneUnits);
    this->Internal->OutputFileName = "";
    emit this->newSceneLoaded();
    }
#else
  this->closeData();
  this->Tree->createRoot("Scene");
  this->Tree->setUnits(cmbSceneUnits::Unknown);
  this->Internal->OutputFileName = "";
  emit this->newSceneLoaded();
#endif
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateNodeColor()
{
  if (!this->ColorNode)
    {
    return;
    }

  double color[4];
  this->ColorNode->getDataObject()->getColor(color);

  size_t i, n = this->Tree->getSelected().size();
  pqCMBSceneNode *node;
  for (i = 0; i < n; i++)
    {
    node = this->Tree->getSelected()[i];
    node->setExplicitColor(color);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateBoxInteraction()
{
  // We only have to do anything  iff there is more than 1 leaf
  // selected or if the 1 leaf is a glyph
  if (this->Internal->selGlyphs.count() == 0)
    {
    if (this->SelectedLeaves.size() == 0)
      {
      return;
      }

    if (this->SelectedLeaves.size() < 2 &&
        (this->SelectedLeaves[0]->getDataObject()->getType()
         != pqCMBSceneObjectBase::Glyph))

      {
      return;
      }
    }
  double newPos[3], newOri[3], newScale[3];
  double tdelta[3], odelta[3], sdelta[3];
    vtkSMPropertyHelper(this->Internal->BoxWidget,
                        "Position").Get(newPos, 3);
    vtkSMPropertyHelper(this->Internal->BoxWidget,
                        "Rotation").Get(newOri, 3);
    vtkSMPropertyHelper(this->Internal->BoxWidget,
                        "Scale").Get(newScale, 3);
    int i;
    for (i = 0; i < 3; i++)
      {
      // Calculate the new translation
      tdelta[i] = newPos[i] - this->BoxTranslation[i];
      this->BoxTranslation[i] = newPos[i];
      odelta[i] = newOri[i] - this->BoxOrientation[i];
      this->BoxOrientation[i] = newOri[i];
      sdelta[i] = newScale[i] / this->BoxScale[i];
      this->BoxScale[i] = newScale[i];
      }
/*
    std::cout << "Translation delta = (" << tdelta[0]
              << ", " << tdelta[1] << ", " << tdelta[2] << ")\n";
    std::cout << "Orientation delta = (" << odelta[0]
              << ", " << odelta[1] << ", " << odelta[2] << ")\n";
    std::cout << "Scale factor = (" << sdelta[0]
              << ", " << sdelta[1] << ", " << sdelta[2] << ")\n";
       */
    QList<pqCMBSceneNode *>::iterator iter;
    pqCMBSceneObjectBase* dataObj;
    for (iter = this->SelectedLeaves.begin();
         iter != this->SelectedLeaves.end();
         ++iter)
      {
      dataObj = (*iter)->getDataObject();
      if(dataObj->getType() == pqCMBSceneObjectBase::Line ||
         dataObj->getType() == pqCMBSceneObjectBase::Arc ||
         dataObj->isFullyConstrained())
        {
        continue;
        }
      dataObj->applyTransform(sdelta, odelta, tdelta);
      }

  if(this->Internal->selGlyphs.count())
    {
    this->updateBoxInteractionForGlyph(sdelta, odelta, tdelta);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::AddLineSegmentObjectsToModel(
  vtkDiscreteModelWrapper* wrapper, bool voiPresent)
{
  pqCMBSceneNode* parentNode = this->Tree->getLineTypeNode(false);
  if(!parentNode || !parentNode->getChildren().size())
    {
    return;
    }

  vtkBoundingBox bbox;
  vtkSmartPointer<vtkTransform> voiTransform;
  if (voiPresent)
    {
    // get bounds and transform of voi so we can confirm the line is within the VOI
    // before considering including it
    double voiBounds[6];
    this->Internal->VOINodeForMesher->getDataObject()->getRepresentation()->
      getOutputPortFromInput()->getDataInformation()->GetBounds(voiBounds);
    voiTransform = vtkSmartPointer<vtkTransform>::New();
    this->Internal->VOINodeForMesher->getDataObject()->getTransform(voiTransform);
    voiTransform->Inverse();

    bbox.SetBounds(voiBounds);
    }

  pqCMBSceneNodeIterator iter(parentNode);
  pqCMBSceneNode *n;
  double point1[3], point2[3];


  vtkSmartPointer<vtkEnclosingModelEntityOperator> enclosingEntityOperator =
    vtkSmartPointer<vtkEnclosingModelEntityOperator>::New();
  enclosingEntityOperator->BuildLinks(wrapper);

  while((n = iter.next()))
    {
    pqCMBLine* lineObj = static_cast<pqCMBLine*>(n->getDataObject());
    if(!lineObj || !n->isVisible())
      {
      continue;
      }

    lineObj->getPoint1Position(point1[0], point1[1], point1[2]);
    lineObj->getPoint2Position(point2[0], point2[1], point2[2]);

    if (voiPresent)
      {
      // skip if both endpts not in voi
      double transformedPt[3];
      voiTransform->TransformPoint(point1, transformedPt);
      if (!bbox.ContainsPoint(transformedPt))
        {
        QMessageBox::warning(this->parentWidget(), "Not in VOI", "Not in VOI");
        continue;
        }
      voiTransform->TransformPoint(point2, transformedPt);
      if (!bbox.ContainsPoint(transformedPt))
        {
        QMessageBox::warning(this->parentWidget(), "Not in VOI", "Not in VOI");
        continue;
        }
      }

    // test the end points
    enclosingEntityOperator->Operate(wrapper, point1);
    vtkModelEntity *entity0 = enclosingEntityOperator->GetEnclosingEntity();
    enclosingEntityOperator->Operate(wrapper, point2);
    vtkModelEntity *entity1 = enclosingEntityOperator->GetEnclosingEntity();

    // make sure they are in the same entity (region) && not NULL
    if (entity0 == entity1 && entity0)
      {
      vtkIdType regionId = entity0->GetUniquePersistentId();
      wrapper->GetModel()->BuildFloatingRegionEdge(point1, point2, 1, regionId);
      }
    else
      {
      QMessageBox::warning(this->parentWidget(), "Line not added to model!",
        "The line is not completely enclosed by a single region and thus is not being added to the model.");
      }
    }
}

//-----------------------------------------------------------------------------
bool pqCMBSceneBuilderMainWindowCore::AddContourObjectsToModel(
  vtkDiscreteModelWrapper* wrapper)
{
  SceneObjectNodeIterator iter(this->Tree->getRoot());
  iter.setTypeFilter(pqCMBSceneObjectBase::Arc);
  pqCMBSceneNode *n;

  vtkPolyData* Geometry = vtkPolyData::New();
  vtkPoints* Points = vtkPoints::New();
  Geometry->SetPoints(Points);
  Points->Delete();
  Geometry->Allocate();

  bool validContourFound = false;
  while((n = iter.next()))
    {
    pqCMBArc* contourObj = static_cast<pqCMBArc*>(n->getDataObject());
    if(!contourObj || !n->isVisible())
      {
      continue;
      }
    vtkPVArcInfo* arcInfo = contourObj->getArcInfo();
    if(arcInfo && arcInfo->GetNumberOfPoints() && arcInfo->IsClosedLoop())
      {
      validContourFound = true;
      // first go through and create the polydata
      vtkDoubleArray* coords = arcInfo->GetPointLocations();
      double point[3];
      if(coords->GetNumberOfTuples() > 1)
        { // assume that there is more than 1 point so that we can make a loop
        coords->GetTupleValue(0, point);
        vtkIdType FirstPointId = Points->InsertNextPoint(point);
        vtkIdType PreviousPointId = FirstPointId;
        for(vtkIdType i=1;i<coords->GetNumberOfTuples();i++)
          {
          coords->GetTupleValue(i, point);
          vtkIdType CurrentPointId = Points->InsertNextPoint(point);
          vtkIdType LinePointIds[] = {PreviousPointId, CurrentPointId};
          Geometry->InsertNextCell(VTK_LINE, 2, LinePointIds);
          PreviousPointId = CurrentPointId;
          }
        vtkIdType LinePointIds[] = {PreviousPointId, FirstPointId};
        Geometry->InsertNextCell(VTK_LINE, 2, LinePointIds);

        vtkDiscreteModel* model = wrapper->GetModel();
        // make sure that model entities don't have a unique persistent id of 0
        model->GetNextUniquePersistentId();

        DiscreteMesh mesh(Geometry);
        model->SetMesh(mesh);


        std::vector<vtkModelEdge*> edges;
        vtkIdTypeArray* selectedNodes = arcInfo->GetEndNodeIds();
        if(selectedNodes->GetNumberOfTuples())
          {
          std::vector<vtkIdType> sortedNodes(
            selectedNodes->GetPointer(0),
            selectedNodes->GetPointer(selectedNodes->GetNumberOfTuples()));;
          // sort the ids since we are not sure
          std::sort(sortedNodes.begin(), sortedNodes.end());
          vtkModelVertex* firstVertex =
            model->BuildModelVertex(sortedNodes[0]);
          vtkModelVertex* prevVertex = firstVertex;
          for(size_t i=1;i<sortedNodes.size();i++)
            {
            vtkModelVertex* nextVertex =
              model->BuildModelVertex(sortedNodes[i]);
            vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
              model->BuildModelEdge(prevVertex, nextVertex));
            vtkIdList* cellIds = vtkIdList::New();
            for(vtkIdType j=sortedNodes[i-1];j<sortedNodes[i];j++)
              {
              cellIds->InsertNextId(j);
              }
            edge->AddCellsToGeometry(cellIds);
            cellIds->Delete();
            edges.push_back(edge);
            prevVertex = nextVertex;
            }
          // build last model edge
          vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
            model->BuildModelEdge(prevVertex, firstVertex));
          vtkIdList* cellIds = vtkIdList::New();
          for(vtkIdType j=0;j<sortedNodes[0];j++)
            {
            cellIds->InsertNextId(j);
            }
          for(vtkIdType j=sortedNodes[sortedNodes.size()-1];j<Geometry->GetNumberOfCells();j++)
            {
            cellIds->InsertNextId(j);
            }
          edge->AddCellsToGeometry(cellIds);
          cellIds->Delete();
          edges.push_back(edge);
          }
        else
          { // a loop model edge with no vertices
          vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(
            model->BuildModelEdge(0, 0));
          vtkIdList* cellIds = vtkIdList::New();
          for(vtkIdType j=0;j<Geometry->GetNumberOfCells();j++)
            {
            cellIds->InsertNextId(j);
            }
          edge->AddCellsToGeometry(cellIds);
          cellIds->Delete();
          edges.push_back(edge);
          }

        Geometry->Delete();
        int one = 1;
        std::vector<int> edgeDirections(edges.size(), one);
        vtkModelMaterial* material = model->BuildMaterial();
        model->BuildModelFace(static_cast<int>(edges.size()), &edges[0], &edgeDirections[0], material);
        }
      }
    }

  if ( !validContourFound && Geometry)
    {
    Geometry->Delete();
    }

  return validContourFound;
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::stitchTINs()
{
  if (this->previewDialog()->isVisible())
    {
    QMessageBox::warning(NULL, "TIN Stitch", "Preview dialog already open; must close before stitching!");
    return;
    }

  if (this->Internal->TINStitcherDlg->exec() != QDialog::Accepted)
    {
    return;
    }

  emit enableExternalProcesses(false);

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqCMBFacetedObject *fobj;
  QList<pqOutputPort*> inputs;
  size_t numSelected = this->Tree->getSelected().size();
  for (size_t i = 0; i < numSelected; i++)
    {
    fobj = dynamic_cast<pqCMBFacetedObject*>( this->Tree->getSelected()[i]->getDataObject());
    if (fobj)
      {
      inputs.push_back(fobj->
                       getTransformedSource(this->getActiveServer())->getOutputPort(0) );
      }
    }
  QMap<QString, QList<pqOutputPort*> > namedInputs;
  namedInputs["Input"] = inputs;

  // TINStitcher should be NULL before creation
  this->TINStitcher = builder->createFilter( "filters",
    "MultiLayerTINStitcher", namedInputs, this->getActiveServer());
  if(this->TINStitcher == NULL)
    {
    QMessageBox::critical(NULL, "TIN Stitch", "Unable to locate TIN Stitching support!");
    return;
    }

  pqSMAdaptor::setElementProperty(
    this->TINStitcher->getProxy()->GetProperty("MinimumAngle"),
    this->Internal->TINStitcherDlg->getMinimumAngle());
  pqSMAdaptor::setElementProperty(
    this->TINStitcher->getProxy()->GetProperty("UseQuads"),
    this->Internal->TINStitcherDlg->getUseQuads());
  pqSMAdaptor::setElementProperty(
    this->TINStitcher->getProxy()->GetProperty("AllowInteriorPointInsertion"),
    this->Internal->TINStitcherDlg->getAllowInteriorPointInsertion());
  pqSMAdaptor::setElementProperty(
    this->TINStitcher->getProxy()->GetProperty("Tolerance"),
    this->Internal->TINStitcherDlg->getTolerance());
  pqSMAdaptor::setElementProperty(
    this->TINStitcher->getProxy()->GetProperty("TIN Type"),
    this->Internal->TINStitcherDlg->getUserSpecifiedTINType());
  this->TINStitcher->getProxy()->UpdateVTKObjects();

  vtkSMSourceProxy::SafeDownCast( this->TINStitcher->getProxy() )->UpdatePipeline();

  // verify we have ouput from TINStitcher before firing up the preview
  vtkPVDataInformation* dataInfo =
    this->TINStitcher->getOutputPort(0)->getDataInformation();
  if (dataInfo->GetPolygonCount() > 0)
    {
    pqRenderView *tmpView = qobject_cast<pqRenderView*>(
      builder->createView(pqRenderView::renderViewType(),
      this->getActiveServer() ));
    pqDataRepresentation* repr =
      builder->createDataRepresentation(
      this->TINStitcher->getOutputPort(0), tmpView, "GeometryRepresentation");

    this->PreviewMeshOutput = false;

    this->previewDialog()->setRepresentationAndView(repr, tmpView);
    this->previewDialog()->show();
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onPreviewAccepted()
{
  if (this->PreviewMeshOutput)
    {
    this->exportCMBFile();
    QFile::remove( this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + "_BCfaces.vtk" );
    QFile::remove( this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + "_nodes.bc" );
    QFile::remove( this->getProcessExecDirectory() + "/" +
      this->Internal->MesherOutputPrefix + "_faces.bc" );
    if (this->Internal->MesherOptionDlg->getDeleteCreatedPtsFile())
      {
      QFileInfo finfo(this->Internal->MesherOptionDlg->getTemporaryPtsFileName());
      QFile::remove(this->getProcessExecDirectory() + "/" + finfo.fileName());
      }
    }
  else // previewing stitched TIN
    {
    // ask for file to save to...
    QString filters = "VTK Files(*.vtk)";
    pqFileDialog file_dialog(
      this->getActiveServer(),
      this->parentWidget(), tr("Save Stitched TIN:"), QString(), filters);
    file_dialog.setObjectName("SaveStitchedTIN");
    file_dialog.setFileMode(pqFileDialog::AnyFile);
    QString outputFileName;
    if (file_dialog.exec() == QDialog::Accepted)
      {
      QStringList files = file_dialog.getSelectedFiles();
      if (files.size() != 0)
        {
        outputFileName = files[0];
        }
      }

    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
    if (outputFileName.size() != 0)
      {
      QFileInfo origFInfo(outputFileName);
      if(this->TINStitcher == NULL)
        {
        //This can happen with the ctests
        qDebug("TIN Stitcher is not set");
        emit enableExternalProcesses(false);
        return;
        }

      vtkSMOutputPort *outputPort = vtkSMSourceProxy::SafeDownCast(
        this->TINStitcher->getProxy() )->GetOutputPort(static_cast<unsigned int>(0)) ;
      vtkPVCompositeDataInformation* compositeInformation =
        outputPort->GetDataInformation()->GetCompositeDataInformation();
      int numBlocks = compositeInformation->GetNumberOfChildren();
      // extract, write, and create node, for each block
      std::vector<pqCMBSceneNode*> newNodes;
      cmbSceneNodeReplaceEvent *event = NULL;
      // Are we recording events?
      if (this->Tree->recordingEvents())
        {
        event = new cmbSceneNodeReplaceEvent(numBlocks,0);
        this->Tree->insertEvent(event);
        }

      for(int i = 0; i < numBlocks; i++)
        {
        pqPipelineSource* extract = builder->createFilter("filters",
          "ExtractLeafBlock", this->TINStitcher);

        pqSMAdaptor::setElementProperty(
          extract->getProxy()->GetProperty("BlockIndex"), i);
        extract->getProxy()->UpdateVTKObjects();
        vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

        pqPipelineSource *pdSource = builder->createSource("sources",
          "HydroModelPolySource", this->getActiveServer());
        vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
          vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
        pdSource->updatePipeline();
        builder->destroy(extract);

        pqPipelineSource *writer = builder->createFilter("writers",
          "DataSetWriter", pdSource);
        if (numBlocks > 1)
          {
          char fileNameWithIndex[256];
          sprintf(fileNameWithIndex, "%s/%s_%d.vtk",
            origFInfo.path().toStdString().c_str(),
            origFInfo.baseName().toStdString().c_str(), i);
          outputFileName = fileNameWithIndex;
          }
        pqSMAdaptor::setElementProperty(
          writer->getProxy()->GetProperty("FileName"),
          outputFileName.toStdString().c_str());
        writer->getProxy()->UpdateVTKObjects();
        vtkSMSourceProxy::SafeDownCast(writer->getProxy())->UpdatePipeline();
        builder->destroy(writer);

        pqCMBFacetedObject *fobj =
          new pqCMBFacetedObject(pdSource, this->activeRenderView(),
          this->getActiveServer(), outputFileName.toStdString().c_str());
        fobj->setSurfaceType(pqCMBSceneObjectBase::Solid);
        fobj->setUserDefinedType("-Solid");
        QFileInfo fInfo(outputFileName);
        pqCMBSceneNode *node =
          this->Tree->createNode(
                                 fInfo.baseName().toStdString().c_str(),
                                 this->Tree->getRoot(), fobj,
                                 event);
        newNodes.push_back(node);
        }

      this->Tree->blockSignals(true);
      for (unsigned int i = 0; i < this->Tree->getSelected().size(); i++)
        {
        this->Tree->getSelected()[i]->getWidget()->setSelected(false);
        }
      for (unsigned int i = 0; i < newNodes.size(); i++)
        {
        newNodes[i]->getWidget()->setSelected(true);
        }
      this->Tree->blockSignals(false);
      this->Tree->nodesSelected();
      }

    builder->destroy(this->TINStitcher);
    this->TINStitcher = NULL;
    emit enableExternalProcesses(true);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onPreviewRejected()
{
  if (this->PreviewMeshOutput)
    {
    this->rejectMesh();
    }
  else
    {
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
    builder->destroy(this->TINStitcher);
    this->TINStitcher = NULL;
    emit enableExternalProcesses(true);
    }
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::clearColorNode()
{
  this->Internal->VTKColorConnect->Disconnect();
  this->Internal->VTKOpacityConnect->Disconnect();
  this->setAppearanceEditor(NULL);
  this->ColorNode  = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onSelectGlyph(bool checked)
{
  if (checked)
    {
    int isShiftKeyDown = this->activeRenderView()->
      getRenderViewProxy()->GetInteractor()->GetShiftKey();

    if (!isShiftKeyDown && this->Tree->getSelected().size())
      {
      // Clear all existing selections
      this->Tree->clearSelection();
      }
    this->renderViewSelectionHelper()->beginSurfacePointsSelection();
    }
  else
    {
    this->renderViewSelectionHelper()->endSelection();
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::selectGlyph(QList<pqCMBGlyphObject*> &glyphList)
{
  this->Internal->selGlyphs.clear();
  for(QList<pqCMBGlyphObject*>::iterator it=glyphList.begin();
    it != glyphList.end(); ++it)
    {
    vtkSMCMBGlyphPointSourceProxy* glyphProxy =
      vtkSMCMBGlyphPointSourceProxy::SafeDownCast((*it)->getSource()->getProxy());
    if(!glyphProxy)
      {
      continue;
      }
    vtkSMSourceProxy* selSource = glyphProxy->GetSelectionInput(0);
    vtkSMVectorProperty* vp = vtkSMVectorProperty::SafeDownCast(
      selSource->GetProperty("IDs"));
    if(!vp)
      {
      continue;
      }
    QList<QVariant> ids = pqSMAdaptor::getMultipleElementProperty(vp);
    int numElemsPerCommand = vp->GetNumberOfElementsPerCommand();
    if(ids.count() == 0 || numElemsPerCommand == 0)
      {
      continue;
      }
    QList<vtkIdType> selIds;
    int numSelPoints = (ids.count()/numElemsPerCommand);
    vtkIdType pointId;
    double color[4]={1.0, 0.0, 1.0, 1.0};
    for (int cc=0; cc < numSelPoints; cc++)
      {
      pointId = ids.value(numElemsPerCommand*cc+1).value<vtkIdType>();
      selIds.append(pointId);
      glyphProxy->SetColor(pointId,color);
      }
    glyphProxy->MarkModified(glyphProxy);
    (*it)->setSelectionInput(NULL);
    this->Internal->selGlyphs[(*it)] = selIds;
    }
  this->updateBoxWidgetForGlyph();
  this->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateBoxWidgetForGlyph()
{
  // If there are no selected glyph objects, then turn off widget
  if (!this->Internal->selGlyphs.count())
    {
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
      GetProperty("Visibility"), false);
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
      GetProperty("Enabled"), false);
    this->Internal->BoxWidget->UpdateVTKObjects();
    return;
    }

  // Always remove existing links
  this->RemoveBoxWidgetPropertyLinks();

  // Calculate the bounds of the selected leaves
  vtkBoundingBox bb;
  QMap<pqCMBGlyphObject*, QList<vtkIdType> >::iterator iter;
  // Check to see if there are any moveable leaf nodes
  for (iter = this->Internal->selGlyphs.begin();
      iter != this->Internal->selGlyphs.end(); ++iter)
    {
    pqCMBGlyphObject* obj = iter.key();
    if (obj && obj->getSource() && (!obj->isFullyConstrained())
      && !obj->isDefaultConstrained())
      {
      vtkSMCMBGlyphPointSourceProxy* glyphProxy =
        vtkSMCMBGlyphPointSourceProxy::
        SafeDownCast(obj->getSource()->getProxy());
      if(!glyphProxy)
        {
        continue;
        }
      double selBounds[6];
      foreach(vtkIdType selId, iter.value())
        {
        glyphProxy->GetBounds(selId, selBounds);
        vtkBoundingBox tmpbb(selBounds);
        bb.AddBox(tmpbb);
        }
      }
    }

  double bounds[6];
  // We need to "adjust" the bounding box so that the center is at
  // the origin
  bb.GetBounds(bounds);
  bb.GetCenter(this->BoxTranslation);

  // The default orientation and scale :
  this->BoxOrientation[0] = this->BoxOrientation[1] =
    this->BoxOrientation[2] = 0.0;
  this->BoxScale[0] = this->BoxScale[1] =
    this->BoxScale[2] = 1.0;

  QList<QVariant> values;
  values << (bounds[0] - this->BoxTranslation[0])
    << (bounds[1] - this->BoxTranslation[0])
    << (bounds[2] - this->BoxTranslation[1])
    << (bounds[3] - this->BoxTranslation[1])
    << (bounds[4] - this->BoxTranslation[2])
    << (bounds[5] - this->BoxTranslation[2]);
  pqSMAdaptor::setMultipleElementProperty(this->Internal->BoxWidget->
    GetProperty("PlaceWidget"),
    values);
  vtkSMPropertyHelper(this->Internal->BoxWidget,
    "Position").Set(this->BoxTranslation, 3);
  double ori[3], scale[3];
  ori[0] = ori[1] = ori[2] = 0.0;
  scale[0] = scale[1] = scale[2] = 1.0;
  vtkSMPropertyHelper(this->Internal->BoxWidget,
    "Rotation").Set(ori, 3);
  vtkSMPropertyHelper(this->Internal->BoxWidget,
    "Scale").Set(scale, 3);

  pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
    GetProperty("Visibility"), true);
  pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
    GetProperty("Enabled"), true);
  this->Internal->BoxWidget->UpdateVTKObjects();

}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::clearSelectedGlyphList()
{
  QMap<pqCMBGlyphObject*, QList<vtkIdType> >::iterator iter;
  // Check to see if there are any moveable leaf nodes
  for (iter = this->Internal->selGlyphs.begin();
    iter != this->Internal->selGlyphs.end(); ++iter)
    {
    pqCMBGlyphObject* obj = iter.key();
    if (obj && obj->getSource())
      {
      obj->clearSelectedPointsColor();
      }
    }
  this->Internal->selGlyphs.clear();
  this->updateBoxWidgetForGlyph();
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::updateBoxInteractionForGlyph(
  double sdelta[3], double odelta[3], double tdelta[3])
{
  QMap<pqCMBGlyphObject*, QList<vtkIdType> >::iterator iter;
  // Check to see if there are any moveable leaf nodes
  for (iter = this->Internal->selGlyphs.begin();
    iter != this->Internal->selGlyphs.end(); ++iter)
    {
    pqCMBGlyphObject* obj = iter.key();
    if (obj && obj->getSource() && (!obj->isFullyConstrained())
      && !obj->isDefaultConstrained())
      {
      vtkSMCMBGlyphPointSourceProxy* glyphProxy =
        vtkSMCMBGlyphPointSourceProxy::
        SafeDownCast(obj->getSource()->getProxy());
      if(!glyphProxy)
        {
        continue;
        }
      foreach(vtkIdType selId, iter.value())
        {
        glyphProxy->ApplyTransform(
         selId, odelta, tdelta, sdelta);
        }
      glyphProxy->UpdateVTKObjects();
      glyphProxy->UpdatePipeline();
      obj->updateRepresentation();
      obj->getRepresentation()->renderView(true);
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBSceneBuilderMainWindowCore::onEditSettings()
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
void pqCMBSceneBuilderMainWindowCore::applyAppSettings()
{
  this->Superclass::applyAppSettings();
  QColor initColor = this->Internal->AppOptions->initialNewObjectColor();
  this->Tree->setInitialSceneObjectColor(initColor);
}
