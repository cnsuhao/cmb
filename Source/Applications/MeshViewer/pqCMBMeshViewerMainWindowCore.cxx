//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBMeshViewerMainWindowCore.h"
#include "pqCMBAppCommonConfig.h" // for CMB_TEST_DATA_ROOT

#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBProcessWidget.h"
#include "qtCMBProgressWidget.h"
#include "pqCMBPreviewDialog.h"
#include "qtCMBApplicationOptions.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
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
#include <QSpacerItem>
#include <QScrollArea>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QMap>

#include "pq3DWidget.h"
#include "pqActionGroupInterface.h"
#include "pqActiveObjects.h"
#include "pqPVApplicationCore.h"
#include "pqCameraDialog.h"
#include "pqCustomFilterDefinitionModel.h"
#include "pqCustomFilterDefinitionWizard.h"
#include "pqCustomFilterManager.h"
#include "pqCustomFilterManagerModel.h"
#include "pqDataInformationWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqDockWindowInterface.h"
#include "pqFileDialogModel.h"
#include "pqLinksManager.h"
#include "pqObjectBuilder.h"
#include "pqOutputWindow.h"

#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqPipelineModel.h"
#include "pqDataRepresentation.h"
#include "pqProgressManager.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqRenderView.h"
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
#include "pq3DWidgetFactory.h"
#include <pqFileDialog.h>
#include <pqSetData.h>
#include <pqSetName.h>
#include <pqUndoStack.h>
#include <pqWaitCursor.h>
#include <pqSelectionManager.h>
#include "pqSpreadSheetViewModel.h"

#include <QVTKWidget.h>

#include <vtkAddCellDataFilter.h>
#include "vtkBoxWidget2.h"
#include <vtkClientServerStream.h>
#include <vtkDataObject.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDoubleArray.h>
#include "vtkEvent.h"
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkImageData.h>
#include <vtkMultiBlockWrapper.h>
#include <vtkPolyDataReader.h>
#include <vtkProcessModule.h>
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include <vtkPVDisplayInformation.h>
#include <vtkPVOptions.h>
#include <vtkPVXMLElement.h>
#include <vtkPVXMLParser.h>
#include <vtkSmartPointer.h>
#include "vtkSMDataSourceProxy.h"
#include <vtkSMDoubleRangeDomain.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMPropertyHelper.h"
#include <vtkSMProxyIterator.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSession.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMOutputPort.h>
#include <vtkSMStringVectorProperty.h>
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSelectionHelper.h"
#include "vtkSelectionNode.h"

#include <vtkToolkits.h>
#include <vtkUnstructuredGridReader.h>
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "pqOmicronModelWriter.h"

#include "vtkSGXMLBCSWriter.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

#include <algorithm>
#include <vector>

#include "assert.h"
#include <vtksys/Process.h>
#include "vtkCMBMeshWriter.h"
#include "vtkHydroModelPolySource.h"
#include "vtkOmicronModelInputReader.h"
#include "vtkOmicronMeshInputFilter.h"
#include "vtkOmicronMeshInputWriter.h"
#include "vtkSMPropertyLink.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkGenericDataObjectWriter.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include "vtkEventQtSlotConnect.h"
#include <vtksys/SystemTools.hxx>
#include "vtkRenderer.h"

#include "pqProxyWidget.h"
#include "pqXYBarChartView.h"
#include "pqContourWidget.h"
#include "vtkPVContourRepresentationInfo.h"
#include "vtkBoundingBox.h"
#include "vtkCollection.h"
#include "vtkPVDataInformation.h"
#include "vtkPVMeshDataInformation.h"
#include "vtkMatrix4x4.h"
#include "vtkSMMeshSourceProxy.h"
#include "vtkSMContextViewProxy.h"
#include "vtkGeometryRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "qtCMBConeDialog.h"
#include "vtkNew.h"
#include "vtkCMBMeshContourSelector.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

namespace
{
  QFrame* newHLine(QWidget* pw)
    {
    QFrame* line = new QFrame(pw);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    line->setLineWidth(2);
    line->setMidLineWidth(1);

 QPalette p(line->palette());
p.setColor(QPalette::Background, Qt::black);
line->setAutoFillBackground(true);
line->setPalette(p);
    return line;
    }

  QWidget* newGroupSeparator(QWidget* pw)
    {
    QWidget* widget = new QWidget(pw);
    QVBoxLayout* vbox = new QVBoxLayout(widget);
    vbox->setContentsMargins(0,0,0,6);
    vbox->setSpacing(0);
    vbox->addWidget(newHLine(widget));
    pw->layout()->addWidget(widget);
    return widget;
    }
}

///////////////////////////////////////////////////////////////////////////
// pqCMBMeshViewerMainWindowCore::vtkInternal

/// Private implementation details for pqCMBMeshViewerMainWindowCore
class pqCMBMeshViewerMainWindowCore::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/) :
    PositionLink(0),
    OrientationLink(0),
    ScaleLink(0)
  {
  this->VTKColorConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->VTKOpacityConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->is2DMesh = false;
  this->HistogramMode = 1; // 0, "Quality"; 1, "Region"
  this->CurrentInputSource = NULL;
  this->MeshSculptingSource = NULL;
  this->MeshScultpingRep = NULL;
  this->MeshThresholdSource = NULL;
  this->MeshThreshPanel = NULL;
  this->MeshContourSelector = NULL;
  this->MeshSelectorSelection = NULL;
  this->MeshSurfaceFilter = NULL;
  this->MeshInfo = vtkSmartPointer<vtkPVMeshDataInformation>::New();
  this->MeshSmoother = NULL;
  this->MeshSmootherPanel = NULL;
  this->ConeSource = NULL;
  this->ConeSourceDialog = NULL;
  this->ConeRepresentation = NULL;
  this->ConeWidget = NULL;
  this->VTKConeConnect = NULL;
  this->MeshConeSelector=NULL;
  this->ShapeSelectionOption = vtkCMBMeshContourSelector::ALL_IN;
  }

  ~vtkInternal()
  {
  }

  QString CurrentMeshFileName;
  QString OutputFileName;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKColorConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKOpacityConnect;

  vtkSmartPointer<vtkSMPropertyLink> PositionLink;
  vtkSmartPointer<vtkSMPropertyLink> OrientationLink;
  vtkSmartPointer<vtkSMPropertyLink> ScaleLink;
  vtkSmartPointer<vtkSMPropertyLink> ConePositionLink;
  vtkSmartPointer<vtkSMPropertyLink> ConeOrientationLink;
  vtkSmartPointer<vtkSMPropertyLink> ConeScaleLink;

  QPointer<pqPipelineSource> ExtractMesh;
  QPointer<pqPipelineSource> MeshSource;
  QPointer<pqPipelineSource> FilterSource;
  QPointer<pqDataRepresentation> MeshRepresentation;
  QPointer<pqDataRepresentation> FullMeshRepresentation;

  QPointer<pqPipelineSource> MeshQualitySource;
  QPointer<pqProxyWidget> MeshQualityPanel;

  QPointer<pqPipelineSource> MeshThresholdSource;
  QPointer<pqProxyWidget> MeshThreshPanel;

  QPointer<pqPipelineSource> QualityThreshSource;
  QPointer<pqProxyWidget> QualityThreshPanel;

  QPointer<pqPipelineSource> ElevationFilter;
  QPointer<pqProxyWidget> ElevationPanel;

  QList<pqProxyWidget*> ModifiedFilters;


  QPointer<QScrollArea> InspectorContainer;
  QPointer<QScrollArea> SelectionContainer;

  QPointer<pqPipelineSource> MeshHistogram;
  QPointer<pqPipelineSource> ExtractSelection;
  QPointer<pqPipelineSource> SelectionHistogram;
  QPointer<pqXYBarChartView> MeshHistogramView;
  QPointer<pqXYBarChartView> SelectionHistogramView;
  QPointer<QLabel> SelectionHistLabel;

  bool is2DMesh;
  int HistogramMode;// 0, "Quality"; 1, "Region"
  QPointer<pqPipelineSource> CurrentInputSource;
  QMap<pqDataRepresentation*, vtkSMSourceProxy*> InputRepMap;
  QPointer<pqSpreadSheetViewModel> SpreadSheetViewModel;
  vtkSmartPointer<vtkSMViewProxy> SpreadSheetViewProxy;
  vtkSmartPointer<vtkSMProxy> SpreadSheetRepProxy;

  QPointer<pqPipelineSource> MeshSculptingSource;
  QPointer<pqDataRepresentation> MeshScultpingRep;
  vtkSmartPointer<vtkSMProxy> ContourTransform;
  QPointer<pqPipelineSource> TransFormFilter;
  QPointer<pqPipelineSource> MeshContourSelector;
  QPointer<pqPipelineSource> MeshSurfaceFilter;
  QPointer<pqPipelineSource> MeshSelectorSelection;

  QPointer<pqPipelineSource> MeshSmoother;
  QPointer<pqProxyWidget> MeshSmootherPanel;
  QPointer<QWidget> MeshSmootherPanelParent;

  vtkSmartPointer<vtkPVMeshDataInformation> MeshInfo;
  double SelSurfaceNodesOrietation[3];
  double LastPlaneOrigin[3];

  QPointer<pqPipelineSource> ConeSource;
  QPointer<qtCMBConeDialog> ConeSourceDialog;
  QPointer<pqDataRepresentation> ConeRepresentation;
  vtkSMNewWidgetRepresentationProxy* ConeWidget;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConeConnect;
  QPointer<pqPipelineSource> MeshConeSelector;
  int ShapeSelectionOption;
};


///////////////////////////////////////////////////////////////////////////
// pqCMBMeshViewerMainWindowCore

pqCMBMeshViewerMainWindowCore::pqCMBMeshViewerMainWindowCore(QWidget* parent_widget) :
  pqCMBCommonMainWindowCore(parent_widget), Internal(new vtkInternal(parent_widget))
{
  this->buildRenderWindowContextMenuBehavior(parent_widget);
  this->setObjectName("MeshViewerMainWindowCore");
}

//-----------------------------------------------------------------------------
pqCMBMeshViewerMainWindowCore::~pqCMBMeshViewerMainWindowCore()
{
  this->selectionManager()->blockSignals(true);
  this->closeData();
  delete Internal;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::destroyInputRepresentations()
{
  foreach(pqDataRepresentation* subsetRep, this->Internal->InputRepMap.keys())
    {
    this->destroyRepresentation(subsetRep);
    }
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::processMesh(const QString& filename,
                                             pqPipelineSource* source)
{
  if(filename.compare(this->Internal->CurrentMeshFileName,
    Qt::CaseInsensitive) == 0)
    {
    return;
    }

  pqFileDialogModel model(this->getActiveServer());
  QString fullpath;

  if (this->isMeshLoaded())
    {
    this->onCloseData();
    }

  pqWaitCursor cursor;
  pqPVApplicationCore* core = pqPVApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Internal->CurrentMeshFileName = filename;
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    source->getProxy() );
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();
  smSource->UpdatePropertyInformation();

  smSource->GatherInformation(this->Internal->MeshInfo);

  vtkCellTypes* cellTypes = this->Internal->MeshInfo->GetCellTypes();
  if (cellTypes->GetNumberOfTypes() > 1)
    {
    QMessageBox::warning(this->parentWidget(), "Loading Mesh",
      "This mesh has mixed type of cells, which may not be fully supported.");
    }
  QFileInfo finfo (filename);
  if (finfo.suffix().toLower() == "3dm" || finfo.suffix().toLower() == "2dm")
    {
    vtkSMIntVectorProperty* prop2DMesh =
      vtkSMIntVectorProperty::SafeDownCast(smSource->GetProperty("MeshDimension"));
    if (prop2DMesh->GetElement(0) == 1)
      {
      QMessageBox::warning(this->parentWidget(), "Processing Error",
        "The file contains a 1D mesh and will be ignored.");
      return;
      }
    this->Internal->is2DMesh = (prop2DMesh->GetElement(0) == 2);
    }
  else
    {
    this->Internal->is2DMesh =
      (cellTypes->IsType(VTK_TETRA) || cellTypes->IsType(VTK_VOXEL) ||
      cellTypes->IsType(VTK_HEXAHEDRON) || cellTypes->IsType(VTK_WEDGE) ||
      cellTypes->IsType(VTK_PYRAMID) || cellTypes->IsType(VTK_PENTAGONAL_PRISM) ||
      cellTypes->IsType(VTK_HEXAGONAL_PRISM)) ? false : true;
    }

  this->Internal->MeshSource = builder->createSource("sources",
    "GMSMeshSource", this->getActiveServer());
  this->Internal->FilterSource = builder->createSource("sources",
    "GMSMeshSource", this->getActiveServer());
//  if(!this->Internal->MeshInfo->GetRegionArray())
//    {
    pqPipelineSource* tmpSource = this->createFilter(
      "AddMeshDataArray", source);
    vtkSMSourceProxy* tmpSMSource = vtkSMSourceProxy::SafeDownCast(
      tmpSource->getProxy());
    tmpSMSource->UpdateVTKObjects();
    tmpSMSource->UpdatePipeline();
    smSource = tmpSMSource;
//    }

  vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->MeshSource->getProxy())->CopyData(smSource);
  this->Internal->MeshSource->getProxy()->MarkModified(NULL);
  vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshSource->getProxy() )->UpdatePipeline();
  this->Internal->ExtractMesh = this->createFilter("ExtractSelection",
    this->Internal->MeshSource);
  vtkSMSourceProxy::SafeDownCast(
    this->Internal->ExtractMesh->getProxy() )->UpdatePipeline();

  vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->FilterSource->getProxy())->CopyData(smSource);
  this->updateSource(this->Internal->FilterSource);

  this->Internal->CurrentInputSource =  this->Internal->MeshSource;

  //vtkSMDataSourceProxy::SafeDownCast(
  //  this->Internal->FilterSource->getProxy())->CopyData(smSource);
  //this->updateSource(this->Internal->FilterSource);
  //this->Internal->FilterSource = this->Internal->MeshSource;

  this->initScrollArea(this->Internal->InspectorContainer);
  this->initScrollArea(this->Internal->SelectionContainer);

  this->createFilters(this->Internal->FilterSource);
  this->createFilterPanels();
  this->acceptFilterPanels(true);
  this->createSmoothMeshPanel();

  this->Internal->MeshRepresentation =
    builder->createDataRepresentation(
    this->activeSource()->getOutputPort(0),
    this->activeRenderView());
  pqSMAdaptor::setEnumerationProperty(
    this->Internal->MeshRepresentation->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

  this->Internal->FullMeshRepresentation =
    builder->createDataRepresentation(
    this->meshSource()->getOutputPort(0),
    this->activeRenderView());
  this->Internal->FullMeshRepresentation->setVisible(0);
  pqSMAdaptor::setEnumerationProperty(
    this->Internal->FullMeshRepresentation->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->Internal->FullMeshRepresentation->getProxy(),
    vtkMultiBlockWrapper::GetShellTagName(), vtkDataObject::CELL);

  pqSMAdaptor::setElementProperty(
    this->Internal->FullMeshRepresentation->getProxy()->GetProperty("Pickable"), 0);
  this->Internal->FullMeshRepresentation->getProxy()->UpdateVTKObjects();
  this->Internal->InputRepMap[this->Internal->FullMeshRepresentation] = NULL;

  this->createHistogramViews();

  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();

  if(this->Internal->MeshRepresentation)
    {
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
      this->Internal->MeshRepresentation->getProxy(),
      vtkMultiBlockWrapper::GetShellTagName(), vtkDataObject::CELL);
    this->setDisplayRepresentation(this->Internal->MeshRepresentation);
    }

  emit this->newMeshLoaded();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::createHistogramViews()
{
  this->Internal->MeshHistogram = this->createFilter("ExtractHistogram",
    this->activeSource());
  pqSMAdaptor::setElementProperty(this->Internal->MeshHistogram->getProxy()->
    GetProperty("CalculateAverages"), 0);
  //this->setInputArray(this->Internal->MeshHistogram,
  //  "SelectInputArray", "Quality");
  this->Internal->ExtractSelection = this->createFilter("ExtractSelection",
    this->activeSource());

  this->Internal->SelectionHistogram = this->createFilter("ExtractHistogram",
    this->Internal->ExtractSelection);
  pqSMAdaptor::setElementProperty(this->Internal->SelectionHistogram->getProxy()->
    GetProperty("CalculateAverages"), 0);
  //this->setInputArray(this->Internal->SelectionHistogram,
  //  "SelectInputArray", "Quality");

  QFont font;
  font.setBold(true);

  QLabel* histLabel = new QLabel("Current Mesh Cells Histogram:",
    this->Internal->SelectionContainer);
  histLabel->setFont(font);
  this->Internal->SelectionContainer->widget()->layout()->addWidget(histLabel);

  this->Internal->MeshHistogramView = this->createHistogramView(
    this->Internal->MeshHistogram);

  this->Internal->SelectionHistLabel = new QLabel("Current Selection Cells Histogram:",
    this->Internal->SelectionContainer);
  this->Internal->SelectionHistLabel->setFont(font);
  this->Internal->SelectionContainer->widget()->layout()->addWidget(
    this->Internal->SelectionHistLabel);
  this->Internal->SelectionHistLabel->setVisible(0);

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  vtkSMProxy* activeSelection = this->getActiveSelection();
  if(smSource && activeSelection)
    {
    this->Internal->SelectionHistLabel->setVisible(1);
    vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
      this->Internal->ExtractSelection->getProxy()->GetProperty("Selection"));
    pp->RemoveAllProxies();
    pp->AddProxy(activeSelection);
    this->Internal->SelectionHistogramView = this->createHistogramView(
      this->Internal->SelectionHistogram);
    }
}

//-----------------------------------------------------------------------------
pqXYBarChartView* pqCMBMeshViewerMainWindowCore::createHistogramView(
    pqPipelineSource* source)
{
  pqObjectBuilder* builder = pqPVApplicationCore::instance()->getObjectBuilder();
  pqXYBarChartView* view = qobject_cast<pqXYBarChartView*>(
    builder->createView("XYBarChartView", this->getActiveServer()));

  if(view)
    {
    this->Internal->SelectionContainer->widget()->layout()->addWidget(
      view->widget());
    const char* arrayName = NULL;
    if(this->Internal->HistogramMode == 0)  // by Quality
      {
      arrayName = "Quality";
      }
    else  // by Region
      {
      arrayName = vtkMultiBlockWrapper::GetShellTagName();
      }

    this->setInputArray(source, "SelectInputArray", arrayName);
    this->updateSource(source);

    pqDataRepresentation* repr =
      builder->createDataRepresentation(source->getOutputPort(0), view);
    }
  return view;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::removeFiltertPanel(
  pqProxyWidget* panel)
{
  if(panel)
    {
    //this->Internal->InspectorContainer->widget()->layout()->removeWidget(panel);
    delete panel;
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::createFilters(pqPipelineSource* source)
{
  this->Internal->MeshThresholdSource = this->createFilter(
    "Threshold", source);
  this->setInputArray(this->Internal->MeshThresholdSource,
    "SelectInputScalars", vtkMultiBlockWrapper::GetShellTagName());

  this->Internal->MeshQualitySource = this->createFilter("GMSMeshQuality",
    this->Internal->MeshThresholdSource);

  this->Internal->QualityThreshSource = this->createFilter("Threshold",
    this->Internal->MeshQualitySource);
  this->setInputArray(this->Internal->QualityThreshSource,
    "SelectInputScalars","Quality");

  this->Internal->ElevationFilter = this->createFilter(
    "LIDARElevationFilter", this->Internal->QualityThreshSource);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setInputArray(
  pqPipelineSource* filter, const char* propName, const char* arrayname)
{
  if(filter)
    {
    int type = static_cast<int>( vtkDataObject::FIELD_ASSOCIATION_CELLS );
    vtkSMStringVectorProperty* strproperty = vtkSMStringVectorProperty::SafeDownCast(
      filter->getProxy()->GetProperty(propName));
    if (strproperty == NULL)
      {
      return;
      }
    strproperty->SetNumberOfElements(5);
    strproperty->SetElement(0, "0"); // idx
    strproperty->SetElement(1, "0"); //port
    strproperty->SetElement(2, "0"); //connection
    strproperty->SetElement(3, QString::number(type).toAscii().constData()); //type
    strproperty->SetElement(4, arrayname);
    filter->getProxy()->MarkModified(NULL);
    filter->getProxy()->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBMeshViewerMainWindowCore::createFilter(
  const char* filterxmlname, pqPipelineSource* source)
{
  pqObjectBuilder* builder = pqPVApplicationCore::instance()->getObjectBuilder();
  source->getProxy()->MarkModified(NULL);
  source->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(
    source->getProxy())->UpdatePipeline();
  return builder->createFilter("filters", filterxmlname, source);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::createFilterPanels()
{
  QFont font;
  font.setBold(true);

  // Threshold by Material panel
  QLabel* materialLabel = new QLabel("Threshold by Material:",
    this->Internal->InspectorContainer);
  materialLabel->setFont(font);
  materialLabel->setFrameStyle(QFrame::Raised);
  this->Internal->InspectorContainer->widget()->layout()->addWidget(materialLabel);

  this->Internal->MeshThresholdSource->getProxy()->GetProperty(
    "SelectInputScalars")->SetPanelVisibility("never");
  this->updateSource(this->Internal->MeshThresholdSource);
  this->Internal->MeshThreshPanel = new pqProxyWidget(
    this->Internal->MeshThresholdSource->getProxy(), this->Internal->InspectorContainer);
  this->Internal->MeshThreshPanel->setObjectName("MeshThresholdEditor");
  this->Internal->InspectorContainer->widget()->layout()->addWidget(
    this->Internal->MeshThreshPanel);
  newGroupSeparator(this->Internal->InspectorContainer->widget());

  this->Internal->MeshThreshPanel->setView(this->activeRenderView());
  this->Internal->MeshThresholdSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();
  this->Internal->MeshThreshPanel->apply();
  this->Internal->MeshThreshPanel->updatePanel();

  // Mesh Quality panel
  QLabel* qualityLabel = new QLabel("Mesh Quality:",
    this->Internal->InspectorContainer);
  qualityLabel->setFont(font);
  qualityLabel->setFrameStyle(QFrame::Raised);
  this->Internal->InspectorContainer->widget()->layout()->addWidget(qualityLabel);

  this->updateSource(this->Internal->MeshQualitySource);
  this->Internal->MeshQualityPanel = new pqProxyWidget(
    this->Internal->MeshQualitySource->getProxy(), this->Internal->InspectorContainer);
  this->Internal->MeshQualityPanel->setObjectName("MeshQualityEditor");
  this->Internal->InspectorContainer->widget()->layout()->addWidget(
    this->Internal->MeshQualityPanel);
  newGroupSeparator(this->Internal->InspectorContainer->widget());

  if(this->is2DMesh())
    {
    this->Internal->MeshQualitySource->getProxy()->GetProperty(
      "TetQualityMeasure")->SetPanelVisibility("never");
    this->Internal->MeshQualitySource->getProxy()->GetProperty(
      "TriangleQualityMeasure")->SetPanelVisibility("default");
    }
  else
    {
    this->Internal->MeshQualitySource->getProxy()->GetProperty(
      "TetQualityMeasure")->SetPanelVisibility("default");
    this->Internal->MeshQualitySource->getProxy()->GetProperty(
      "TriangleQualityMeasure")->SetPanelVisibility("never");
    }

  // Threshold by Mesh Quality panel
  QLabel* threshLabel = new QLabel("Threshold by Mesh Quality:",
    this->Internal->InspectorContainer);
  threshLabel->setFont(font);
  threshLabel->setFrameStyle(QFrame::Raised);
  this->Internal->InspectorContainer->widget()->layout()->addWidget(threshLabel);

  this->Internal->MeshQualityPanel->setView(this->activeRenderView());
  this->Internal->MeshQualityPanel->apply();
  this->Internal->MeshQualityPanel->updatePanel();

  this->Internal->QualityThreshSource->getProxy()->GetProperty(
    "SelectInputScalars")->SetPanelVisibility("never");
  this->updateSource(this->Internal->QualityThreshSource);
  this->Internal->QualityThreshPanel = new pqProxyWidget(
    this->Internal->QualityThreshSource->getProxy(), this->Internal->InspectorContainer);
  this->Internal->QualityThreshPanel->setObjectName("QualityThreshEditor");
  this->Internal->InspectorContainer->widget()->layout()->addWidget(
    this->Internal->QualityThreshPanel);
  newGroupSeparator(this->Internal->InspectorContainer->widget());

  this->Internal->QualityThreshPanel->setView(this->activeRenderView());
  this->Internal->QualityThreshSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();
  this->Internal->QualityThreshPanel->apply();
  this->Internal->QualityThreshPanel->updatePanel();

  // Elevation filter panel
  QLabel* eleLabel = new QLabel("Elevation Filter:",
    this->Internal->InspectorContainer);
  eleLabel->setFont(font);
  eleLabel->setFrameStyle(QFrame::Raised);
  this->Internal->InspectorContainer->widget()->layout()->addWidget(eleLabel);

  this->updateSource(this->Internal->ElevationFilter);
  this->Internal->ElevationPanel = new pqProxyWidget(
    this->Internal->ElevationFilter->getProxy(), this->Internal->InspectorContainer);
  this->Internal->ElevationPanel->setObjectName("ElevationEditor");
  this->Internal->InspectorContainer->widget()->layout()->addWidget(
    this->Internal->ElevationPanel);
//  newGroupSeparator(this->Internal->InspectorContainer->widget());
  this->Internal->InspectorContainer->widget()->layout()->addItem(
    new QSpacerItem(20, 100, QSizePolicy::Minimum, QSizePolicy::Maximum));

  this->Internal->ElevationPanel->setView(this->activeRenderView());
  this->Internal->ElevationFilter->getProxy()->GetProperty(
    "LowPoint")->ResetToDomainDefaults();
  this->Internal->ElevationFilter->getProxy()->GetProperty(
    "HighPoint")->ResetToDomainDefaults();
  this->Internal->ElevationPanel->apply();
  this->Internal->ElevationPanel->updatePanel();
  this->Internal->ElevationFilter->getProxy()->InvokeEvent(
    "UserEvent", static_cast<void*>(const_cast<char*>("HideWidget")));

  QObject::connect(this->Internal->MeshThreshPanel,
      SIGNAL(changeAvailable()), this, SLOT(filterModified()));
  QObject::connect(this->Internal->MeshQualityPanel,
    SIGNAL(changeAvailable()), this, SLOT(filterModified()));
  QObject::connect(this->Internal->QualityThreshPanel,
    SIGNAL(changeAvailable()), this, SLOT(filterModified()));
  QObject::connect(this->Internal->ElevationPanel,
    SIGNAL(changeAvailable()), this, SLOT(filterModified()));
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setFiltersSource(
  pqPipelineSource* selSource)
{
  if(!selSource || selSource == this->currentInputSource())
    {
    return;
    }
  pqWaitCursor cursor;

  pqDataRepresentation* previousRep = this->getRepresentationFromSource(
    this->Internal->CurrentInputSource);
  if(previousRep)
    {
    previousRep->setVisible(this->Internal->MeshRepresentation->isVisible());
    vtkSMRepresentationProxy* preRep =vtkSMRepresentationProxy::SafeDownCast(
      previousRep->getProxy());
    preRep->UpdateVTKObjects();
    preRep->UpdatePropertyInformation();
    preRep->UpdatePipeline();
    }
  pqDataRepresentation* subRep = this->getRepresentationFromSource(
    selSource);
  if(subRep) // we need to turn off the original sub rep
    {
    subRep->setVisible(0);
    }

  this->updateSource(selSource);
  vtkSMSourceProxy* smFiltersInput = vtkSMSourceProxy::SafeDownCast(
    selSource->getProxy());
  vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->FilterSource->getProxy())->CopyData(
    smFiltersInput);

  this->updateSource(this->Internal->FilterSource);

  this->clearSelection();

  //pqSMAdaptor::setInputProperty(
  //  this->Internal->MeshThresholdSource->getProxy()->GetProperty("Input"),
  //  smFiltersInput, 0);

  this->resetFilterInputArrays();
  this->Internal->CurrentInputSource = selSource;

  this->updateFilters();

  this->updateSelection();

  this->acceptFilterPanels(true);
  this->updateMeshHistogram();

  this->Internal->MeshRepresentation->setVisible(1);
  vtkSMRepresentationProxy* selRep =vtkSMRepresentationProxy::SafeDownCast(
    this->Internal->MeshRepresentation->getProxy());
  selRep->UpdateVTKObjects();
  selRep->UpdatePropertyInformation();
  selRep->UpdatePipeline();

  this->setDisplayRepresentation(
    this->Internal->MeshRepresentation);
//  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateMeshHistogram()
{
  this->updateSource(this->Internal->MeshHistogram);
  if(!this->Internal->MeshHistogramView)
    {
    this->Internal->MeshHistogramView =
      this->createHistogramView(this->Internal->MeshHistogram);
    }
  this->Internal->MeshHistogramView->getContextViewProxy()->Update();
  this->Internal->MeshHistogramView->resetDisplay();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::filterModified()
{
  pqProxyWidget* const proxyWidget = qobject_cast<pqProxyWidget*>(
    QObject::sender());

  if(!proxyWidget)
    {
    return;
    }

  if(!this->Internal->ModifiedFilters.contains(proxyWidget))
    {
    this->Internal->ModifiedFilters.push_back(proxyWidget);
    }
  this->updateApplyState(true);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateMeshThreshold()
{

}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateMeshQuality()
{

}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateQualityThreshold()
{

}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateApplyState(bool changesAvailable)
{
  // watch for modified state changes
  emit this->filterPropertiesChanged(changesAvailable);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::resetFilterPanels()
{
  this->Internal->MeshThreshPanel->reset();
  this->Internal->MeshQualityPanel->reset();
  this->Internal->QualityThreshPanel->reset();
  this->Internal->ElevationPanel->reset();
  this->Internal->MeshSmootherPanel->reset();
  this->Internal->ModifiedFilters.clear();
  //this->Internal->QualityThreshPanel->setEnabled(1);

  this->updateApplyState(false);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::acceptFilterPanels(bool ignoreList)
{
  if(ignoreList)
    {
    this->Internal->MeshThreshPanel->apply();
    this->Internal->MeshQualityPanel->apply();
    this->Internal->QualityThreshPanel->apply();
    this->Internal->ElevationPanel->apply();
    }
  else
    {
    if(this->Internal->MeshThreshPanel && this->Internal->ModifiedFilters.contains(
      this->Internal->MeshThreshPanel))
      {
      this->Internal->MeshThreshPanel->apply();
//      this->updateSource(this->Internal->MeshQualitySource);
      this->Internal->MeshQualityPanel->apply();

//      this->updateSource(this->Internal->QualityThreshSource);
      this->Internal->QualityThreshPanel->apply();
      }
    else if(this->Internal->ModifiedFilters.contains(
      this->Internal->MeshQualityPanel))
      {
      this->Internal->MeshQualityPanel->apply();

//      this->updateSource(this->Internal->QualityThreshSource);
      this->Internal->QualityThreshPanel->apply();
      }
    else if(this->Internal->ModifiedFilters.contains(
      this->Internal->QualityThreshPanel))
      {
      this->Internal->QualityThreshPanel->apply();
      this->Internal->ElevationPanel->apply();
      }
    else if(this->Internal->ModifiedFilters.contains(
            this->Internal->ElevationPanel))
      this->Internal->ElevationPanel->apply();
    }

  this->Internal->ModifiedFilters.clear();
  //this->Internal->QualityThreshPanel->setEnabled(1);
  this->clearSelection();
  this->activeRenderView()->render();
  this->updateApplyState(false);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateSource(pqPipelineSource* source)
{
  if(!source)
    {
    return;
    }
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  smSource->MarkModified(NULL);
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateFilters()
{
  this->updateSource(this->Internal->MeshThresholdSource);
  this->updateSource(this->Internal->MeshQualitySource);
  this->updateSource(this->Internal->QualityThreshSource);
  this->updateSource(this->Internal->ElevationFilter);
//  this->updateSource(this->Internal->ExtractSelection);
  this->updateSource(this->Internal->MeshHistogram);
//  this->updateSource(this->Internal->SelectionHistogram);
}
//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::is2DMesh()
{
  //QFileInfo fInfo(this->Internal->CurrentMeshFileName);
  //return (fInfo.suffix().toLower()=="2dm");
  return this->Internal->is2DMesh;
}
//-----------------------------------------------------------------------------
QString pqCMBMeshViewerMainWindowCore::getCurrentMeshFile() const
{
  return this->Internal->CurrentMeshFileName;
}
//-----------------------------------------------------------------------------
QScrollArea* pqCMBMeshViewerMainWindowCore::createScrollArea (QWidget *parent)
{
  QWidget* container = new QWidget();
  container->setObjectName("scrollWidget");
  container->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(parent);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(container);
  parent->layout()->addWidget(s);

  QVBoxLayout* layout = new QVBoxLayout(container);
  layout->setMargin(0);

  return s;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setupSelectionPanel (QWidget *parent)
{
  this->Internal->SelectionContainer = this->createScrollArea(parent);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setupInspectorPanel (QWidget *parent)
{
  this->Internal->InspectorContainer = this->createScrollArea(parent);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::initScrollArea(QScrollArea* area)
{
  QWidget* container = new QWidget();
  container->setObjectName("scrollWidget");
  container->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);
  QVBoxLayout* layout = new QVBoxLayout(container);
  layout->setMargin(0);

  area->setWidget(container);
}


//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::zoomOnSelection()
{
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setHistogramMode(int mode)
{
  if(this->Internal->HistogramMode == mode)
    {
    return;
    }
  this->Internal->HistogramMode = mode;
  const char* arrayName = NULL;
  if(mode == 0)  // by Quality
    {
    arrayName = "Quality";
    }
  else  // by Region
    {
    arrayName = vtkMultiBlockWrapper::GetShellTagName();
    }
  if(this->Internal->MeshHistogramView)
    {
    this->setInputArray(this->Internal->MeshHistogram,
      "SelectInputArray", arrayName);
    this->updateSource(this->Internal->MeshHistogram);
    this->Internal->MeshHistogramView->getContextViewProxy()->Update();
    this->Internal->MeshHistogramView->resetDisplay();
    }

  if(this->Internal->SelectionHistogramView)
    {
//    this->updateSource(this->Internal->ExtractSelection);
    this->setInputArray(this->Internal->SelectionHistogram,
      "SelectInputArray", arrayName);
    this->updateSource(this->Internal->SelectionHistogram);
    this->Internal->SelectionHistogramView->getContextViewProxy()->Update();
    this->Internal->SelectionHistogramView->resetDisplay();
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setSelectionMode(int mode)
{

  vtkSMRepresentationProxy* selRep = vtkSMRepresentationProxy::SafeDownCast(
    this->Internal->MeshRepresentation->getProxy());
  if (selRep)
    {
    if (!mode)
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
        mode-1);
      }
    selRep->UpdateVTKObjects();
    selRep->UpdatePipeline();
    this->activeRenderView()->render();
    }
}

//-----------------------------------------------------------------------------
QString pqCMBMeshViewerMainWindowCore::getOutputFileName() const
{
  return this->Internal->OutputFileName;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onSaveAsData()
{
  if (!this->isMeshLoaded() || !this->activeSource())
    {
    this->Internal->OutputFileName.clear();
    return;
    }
  QString filename;
  if(this->getMeshSaveAsFileName(filename))
    {
    this->saveMesh(filename);
    }
  this->Internal->OutputFileName.clear();
}
//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::getMeshSaveAsFileName(QString& filename)
{
  QString filters = this->is2DMesh() ?
    "GMS 2D Mesh (*.2dm);;GAMBIT  Mesh (*.neu)" : "GMS 3D Mesh (*.3dm);;GAMBIT  Mesh (*.neu)";

  pqFileDialog file_dialog(
    this->getActiveServer(),
    this->parentWidget(), tr("Save Mesh:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      filename = files[0];
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onSaveData()
{
  if (this->Internal->OutputFileName == "")
    {
    this->onSaveAsData();
    }
  else
    {
    this->saveMesh(this->Internal->OutputFileName);
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::exportMesh(pqPipelineSource* meshSource)
{
  if (!this->isMeshLoaded() || !this->activeSource())
    {
    return;
    }
  QString filename;
  if(this->getMeshSaveAsFileName(filename))
    {
    this->saveMesh(filename, meshSource);
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::saveMesh(const QString& filename,
  pqPipelineSource* meshSource)
{
  if (!this->isMeshLoaded() || !this->meshSource())
    {
    this->Internal->OutputFileName.clear();
    return;
    }
  this->Internal->OutputFileName = filename;

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  smSource->MarkModified(NULL);
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();

  pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();

  QFileInfo finfo(filename);
  std::string writerName;
  if (finfo.suffix().toLower() == "neu")
    {
    writerName = "GAMBITWriter";
    }
  else
    {
    writerName = "CMBMeshWriter";
    }
  pqPipelineSource* inSource = meshSource ? meshSource : this->meshSource();
  pqPipelineSource *meshWriter =
    builder->createFilter("writers", writerName.c_str(), inSource);

  this->setInputArray(meshWriter, "SelectInputScalars", vtkMultiBlockWrapper::GetShellTagName());

  vtkSMPropertyHelper(meshWriter->getProxy(), "FileName").Set(
    filename.toAscii().constData() );

  if (writerName.compare("CMBMeshWriter") == 0)
    {
    vtkSMPropertyHelper(meshWriter->getProxy(), "FileFormat").Set(
      vtkCMBMeshWriter::XMS);
    vtkSMPropertyHelper(meshWriter->getProxy(), "MeshDimension").Set(
      this->is2DMesh() ? vtkCMBMeshWriter::MESH2D : vtkCMBMeshWriter::MESH3D);
    vtkSMPropertyHelper(meshWriter->getProxy(), "ValidateDimension").Set(true);
    vtkSMPropertyHelper(meshWriter->getProxy(), "WriteMetaInfo").Set(true);
    vtkSMPropertyHelper(meshWriter->getProxy(), "FloatPrecision").Set(10);
    vtkSMPropertyHelper(meshWriter->getProxy(), "UseScientificNotation").Set(true);

    QDialog writerDlg(this->parentWidget());
    writerDlg.setObjectName("MeshViewerMeshWriterDlg");

    QVBoxLayout* layout = new QVBoxLayout(&writerDlg);
    pqProxyWidget* wrtWidget =  new pqProxyWidget(meshWriter->getProxy(), &writerDlg);
    wrtWidget->setObjectName("meshViewerWriterProxyWidget");
    wrtWidget->setApplyChangesImmediately(true);
    wrtWidget->filterWidgets();
    layout->addWidget(wrtWidget);

    QDialogButtonBox* buttonBox=new QDialogButtonBox(&writerDlg);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    layout->addWidget(buttonBox);
    QObject::connect(buttonBox, SIGNAL(accepted()), &writerDlg, SLOT(accept()));
    writerDlg.setModal(true);
    writerDlg.show();
    writerDlg.exec();
    }

  meshWriter->getProxy()->MarkModified(NULL);
  meshWriter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(meshWriter->getProxy())->UpdatePipeline();

  builder->destroy(meshWriter);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onCloseData()
{
  if (this->checkForPreviewDialog())
    {
    return;
    }

  if(this->getAppearanceEditor())
    {
    this->getAppearanceEditorContainer()->layout()->
      removeWidget(this->getAppearanceEditor());
    this->setAppearanceEditor(NULL);
    }

  this->closeData();
  emit this->newMeshLoaded();
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onFrustumSelect(bool checked)
{
  if (checked)
    {
    int isShiftKeyDown = this->activeRenderView()->
      getRenderViewProxy()->GetInteractor()->GetShiftKey();
    this->renderViewSelectionHelper()->beginFrustumSelection();
    }
  else
    {
    this->renderViewSelectionHelper()->endSelection();
    }
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onRubberBandSelectCell(bool checked)
{
  if (checked)
    {
    int isShiftKeyDown = this->activeRenderView()->
      getRenderViewProxy()->GetInteractor()->GetShiftKey();
    this->renderViewSelectionHelper()->beginSurfaceSelection();
    }
  else
    {
    this->renderViewSelectionHelper()->endSelection();
    }
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onRubberBandSelectPoints(bool checked)
{
  if (checked)
    {
    int isShiftKeyDown = this->activeRenderView()->
      getRenderViewProxy()->GetInteractor()->GetShiftKey();
    this->renderViewSelectionHelper()->beginSurfacePointsSelection();
    }
  else
    {
    this->renderViewSelectionHelper()->endSelection();
    }
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateSelection(bool isShapeSel)
{
  if(!this->activeSource())
    {
    return;
    }
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  vtkSMProxy* activeCellSelection = this->getActiveSelection();
  if(smSource && ((!isShapeSel && activeCellSelection)
    || (activeCellSelection && isShapeSel &&
       (this->hasContourSelection() || this->hasConeSelection()))))
    {
    vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
      this->Internal->ExtractSelection->getProxy()->GetProperty("Selection"));
    pp->RemoveAllProxies();
    pp->AddProxy(activeCellSelection);
    this->updateSource(this->Internal->ExtractSelection);

    this->Internal->SelectionHistLabel->setVisible(1);

    if(!this->Internal->SelectionHistogramView)
      {
      this->Internal->SelectionHistogramView =
        this->createHistogramView(this->Internal->SelectionHistogram);
      }
    else
      {
      this->Internal->SelectionHistogramView->widget()->setVisible(1);
      this->Internal->SelectionHistogramView->getContextViewProxy()->Update();
      this->Internal->SelectionHistogramView->resetDisplay();
      }
    //this->setSelectionMode(mode);
    }
  else
    {
    if(this->Internal->SelectionHistLabel)
      {
      this->Internal->SelectionHistLabel->setVisible(0);
      }
    if(this->Internal->SelectionHistogramView)
      {
      this->Internal->SelectionHistogramView->widget()->setVisible(0);
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::clearSelection()
{
  if(this->activeSource())
    {
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      this->activeSource()->getProxy());
    smSource->SetSelectionInput(0, NULL, 0);
    smSource->UpdateVTKObjects();
    smSource->UpdatePipeline();
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setActiveSelection(
  vtkSMSourceProxy* selSource)
{
  if(this->activeSource())
    {
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      this->activeSource()->getProxy());
    smSource->SetSelectionInput(0, selSource, 0);
    smSource->UpdateVTKObjects();
    smSource->UpdatePipeline();
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::selectAll()
{
  if(this->activeSource())
    {
    vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
      this->getActiveServer()->proxyManager()->NewProxy("sources", "cmbIDSelectionSource"));

    selSource->UpdateProperty("RemoveAllIDs", 1);
    pqSMAdaptor::setElementProperty(
      selSource->GetProperty("InsideOut"), 1);
    selSource->UpdateVTKObjects();

    this->setActiveSelection(selSource);
    selSource->Delete();
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::closeData()
{
  pqActiveObjects::instance().setActiveSource(NULL);
  this->Internal->CurrentMeshFileName.clear();
  this->Internal->CurrentInputSource = NULL;
  this->Internal->is2DMesh = false;
  this->Internal->HistogramMode = 1;

  this->removeFiltertPanel(this->Internal->ElevationPanel);
  this->removeFiltertPanel(this->Internal->QualityThreshPanel);
  this->removeFiltertPanel(this->Internal->MeshQualityPanel);
  this->removeFiltertPanel(this->Internal->MeshThreshPanel);
  this->removeFiltertPanel(this->Internal->MeshSmootherPanel);

  if(this->Internal->InspectorContainer)
    {
    QWidget* container = this->Internal->InspectorContainer->takeWidget();
    if(container)
      {
      delete container;
      }
    this->Internal->InspectorContainer->setWidget(NULL);
    }

  this->removeHistogramView();
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if(this->Internal->MeshContourSelector)
    {
    builder->destroy(this->Internal->MeshContourSelector);
    this->Internal->MeshContourSelector = NULL;
    }
  if(this->Internal->MeshSmoother)
    {
    builder->destroy(this->Internal->MeshSmoother);
    this->Internal->MeshSmoother = NULL;
    }
  if(this->Internal->MeshSelectorSelection)
    {
    builder->destroy(this->Internal->MeshSelectorSelection);
    this->Internal->MeshSelectorSelection = NULL;
    }
  if(this->Internal->MeshSurfaceFilter)
    {
    builder->destroy(this->Internal->MeshSurfaceFilter);
    this->Internal->MeshSurfaceFilter = NULL;
    }

  if(this->Internal->MeshSculptingSource)
    {
    builder->destroy(this->Internal->MeshScultpingRep);
    builder->destroy(this->Internal->MeshSculptingSource);
    this->Internal->MeshSculptingSource = NULL;
    this->Internal->MeshScultpingRep = NULL;
    }
  if(this->Internal->TransFormFilter)
    {
    builder->destroy(this->Internal->TransFormFilter);
    this->Internal->TransFormFilter = NULL;
    }
  if(this->coneRepresentation())
    {
    this->stopConeSelection();
    }
  if(this->Internal->MeshConeSelector)
    {
    builder->destroy(this->Internal->MeshConeSelector);
    this->Internal->MeshConeSelector = NULL;
    }
  if(this->Internal->ConeWidget)
    {
    vtkSMProxyManager* pxm=vtkSMProxyManager::GetProxyManager();
    pxm->UnRegisterProxy("3d_widgets_prototypes",
      pxm->GetProxyName("3d_widgets_prototypes",
      this->Internal->ConeWidget),this->Internal->ConeWidget);
    this->Internal->ConeWidget = NULL;
    }

  if(this->Internal->ConeSource)
    {
    builder->destroy(this->Internal->ConeRepresentation);
    builder->destroy(this->Internal->ConeSource);
    this->Internal->ConeRepresentation = NULL;
    this->Internal->ConeSource = NULL;
    }
  if(this->Internal->ConeSourceDialog)
    {
    delete this->Internal->ConeSourceDialog;
    this->Internal->ConeSourceDialog = NULL;
    }

  this->destroySources();
  this->Internal->ModifiedFilters.clear();
  this->destroyInputRepresentations();
  this->Superclass::closeData();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::removeHistogramView()
{
  pqObjectBuilder* const builder =
    pqPVApplicationCore::instance()->getObjectBuilder();

  if(this->Internal->SelectionContainer)
    {
    QWidget* container = this->Internal->InspectorContainer->takeWidget();
    if(container)
      {
      delete container;
      }
    this->Internal->SelectionContainer->setWidget(NULL);
    }

  if(this->Internal->MeshHistogramView)
    {
    builder->destroy(this->Internal->MeshHistogramView);
    this->Internal->MeshHistogramView = NULL;
    }
  if(this->Internal->SelectionHistogramView)
    {
    builder->destroy(this->Internal->SelectionHistogramView);
    this->Internal->SelectionHistogramView = NULL;
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::destroySources()
{
  pqObjectBuilder* const builder =
    pqPVApplicationCore::instance()->getObjectBuilder();
  if (this->Internal->MeshRepresentation)
    {
    builder->destroy(this->Internal->MeshRepresentation);
    this->Internal->MeshRepresentation = NULL;
    }
  if (this->Internal->FullMeshRepresentation)
    {
      this->Internal->InputRepMap.remove(this->Internal->FullMeshRepresentation);
    builder->destroy(this->Internal->FullMeshRepresentation);
    this->Internal->FullMeshRepresentation = NULL;
    }

  this->destroySource(builder, this->Internal->MeshHistogram);
  this->destroySource(builder, this->Internal->SelectionHistogram);
  this->destroySource(builder, this->Internal->ExtractSelection);

  this->destroySource(builder, this->Internal->ElevationFilter);
  this->destroySource(builder, this->Internal->QualityThreshSource);
  this->destroySource(builder, this->Internal->MeshQualitySource);
  this->destroySource(builder, this->Internal->MeshThresholdSource);

  this->destroySource(builder, this->Internal->FilterSource);
  this->destroySource(builder, this->Internal->ExtractMesh);
  this->destroySource(builder, this->Internal->MeshSource);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::destroySource(
  pqObjectBuilder* builder, pqPipelineSource* source)
{
  if(source)
    {
    builder->destroy(source);
    }
}

//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::isMeshLoaded()
{
  return !this->Internal->CurrentMeshFileName.isEmpty();
}
//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBMeshViewerMainWindowCore::meshSource()
{
  return this->Internal->MeshSource;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::activeRepresentation()
{
  return this->Internal->MeshRepresentation;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::fullMeshRepresentation()
{
  return this->Internal->FullMeshRepresentation;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBMeshViewerMainWindowCore::currentInputSource()
{
  return this->Internal->CurrentInputSource;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::meshSculptingRepresentation()
{
  return this->Internal->MeshScultpingRep;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBMeshViewerMainWindowCore::meshSculptingSource()
{
  return this->Internal->MeshSculptingSource;
}
//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBMeshViewerMainWindowCore::activeSource()
{
  return this->Internal->ElevationFilter;
}

//-----------------------------------------------------------------------------
/// Called when a new reader is created by the GUI.
void pqCMBMeshViewerMainWindowCore::onReaderCreated(pqPipelineSource* reader,
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
  QFileInfo finfo (filename);
  if (finfo.suffix().toLower() == "3dm" ||
      finfo.suffix().toLower() == "2dm")
    {
    vtkSMPropertyHelper(reader->getProxy(),
                        "CreateMeshElementIdArray").Set(1);
    vtkSMPropertyHelper(reader->getProxy(),
                        "RenameMaterialAsRegion").Set(1);
    vtkSMPropertyHelper(reader->getProxy(),
                        "CreateMeshMaterialIdArray").Set(1);
    vtkSMPropertyHelper(reader->getProxy(),
                        "CreateMeshNodeIdArray").Set(1);
    //reader->getProxy()->UpdateVTKObjects();
    //vtkSMSourceProxy::SafeDownCast(reader->getProxy())->UpdatePipeline();
    }

  pqFileDialogModel model(this->getActiveServer());
  QString fullpath;

  if (!model.fileExists(filename, fullpath))
    {
    QMessageBox::information(NULL, "Mesh Viewer", "The File does not exist!");
    return ;
    }

  this->processMesh(filename, reader);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::resetCenterOfRotationToCenterOfCurrentData()
{
  this->Superclass::resetCenterOfRotationToCenterOfCurrentData();
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
void pqCMBMeshViewerMainWindowCore::changeSelectionMaterialId(int newId)
{
  this->changeMeshMaterialId(newId);
  // we need to update all the subset sources and reps
  foreach(pqDataRepresentation* subsetRep, this->Internal->InputRepMap.keys())
    {
    vtkSMSourceProxy* selectionProxy = this->Internal->InputRepMap[subsetRep];
    if(selectionProxy)
      {
            // we need to update the ExtractMesh filter if the input is a subset
            vtkSMSourceProxy* subsetInput = vtkSMSourceProxy::SafeDownCast(
              this->Internal->ExtractMesh->getProxy());
            vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
              subsetInput->GetProperty("Selection"));
            pp->RemoveAllProxies();
            pp->AddProxy(selectionProxy);

           this->updateSource(this->Internal->ExtractMesh);
        vtkSMDataSourceProxy::SafeDownCast(
          subsetRep->getInput()->getProxy())->CopyData(subsetInput);
      this->updateSource(subsetRep->getInput());
      vtkSMRepresentationProxy::SafeDownCast(subsetRep->getProxy())->UpdatePipeline();
      }
    }

  vtkSMSourceProxy* smFiltersInput = vtkSMSourceProxy::SafeDownCast(
      this->Internal->CurrentInputSource->getProxy());
  vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->FilterSource->getProxy())->CopyData(
    smFiltersInput);
  this->updateSource(this->Internal->FilterSource);

  this->Internal->MeshThresholdSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();

//  this->resetFilterInputArrays();
  this->updateFilters();
  this->Internal->MeshThreshPanel->apply();
  this->Internal->MeshQualityPanel->apply();
  //this->acceptFilterPanels(true);
  this->updateMeshHistogram();
  //this->updateSelection();
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->Internal->MeshRepresentation->getProxy(),
    vtkMultiBlockWrapper::GetShellTagName(), vtkDataObject::CELL);

  this->Internal->MeshRepresentation->getProxy()->MarkModified(NULL);
  vtkSMRepresentationProxy::SafeDownCast(
    this->Internal->MeshRepresentation->getProxy())->UpdatePipeline();

  this->clearSelection();
  this->activeRenderView()->render();
  emit this->meshModified();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::changeMeshMaterialId(int newId)
{
  vtkSMSourceProxy* currentSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  currentSource->MarkModified(NULL);
  currentSource->UpdateVTKObjects();
  currentSource->UpdatePipeline();

  pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();
  //pqPipelineSource* tmpSource = builder->createSource("sources",
  //  "GMSMeshSource", this->getActiveServer());
  //vtkSMDataSourceProxy::SafeDownCast(
  //  tmpSource->getProxy())->CopyData(meshSource);

  QPointer<pqPipelineSource> ModifyMaterialFilter = this->createFilter(
    "GMSMeshSelectionRegion", this->activeSource());
  this->setInputArray(ModifyMaterialFilter,
    "SelectInputScalars", vtkMultiBlockWrapper::GetShellTagName());

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    ModifyMaterialFilter->getProxy());

  vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
    smSource->GetProperty("Selection"));
  pp->RemoveAllProxies();
  pp->AddProxy(this->getActiveSelection());

  vtkSMProxyProperty* ppMesh = vtkSMProxyProperty::SafeDownCast(
    smSource->GetProperty("Mesh"));
  ppMesh->RemoveAllProxies();
  ppMesh->AddProxy(this->meshSource()->getProxy());

/*
// we need to convert the current selection to ID based selection
// so that the filter can map the cells to original mesh.
  vtkSMProxy* activeSel = this->getActiveSelection();
  vtkSMSourceProxy* idSelection = vtkSMSourceProxy::SafeDownCast(
    vtkSMSelectionHelper::ConvertSelection(vtkSelectionNode::INDICES,
    activeSel, selSource, 0));

  if (idSelection)
    {
    if (idSelection != activeSel)
      {
      idSelection->UpdateVTKObjects();
      }
    pp->AddProxy(idSelection);
    }
*/
  vtkSMPropertyHelper(smSource, "SelectionRegionId").Set(newId);
  smSource->MarkModified(NULL);
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();

  vtkSMDataSourceProxy::SafeDownCast(
    this->meshSource()->getProxy())->CopyData(smSource);
  this->updateSource(this->meshSource());
  //if(idSelection)
  //  {
  //  idSelection->Delete();
  //  }

  builder->destroy(ModifyMaterialFilter);
  //builder->destroy(tmpSource);
}
//-----------------------------------------------------------------------------
vtkSMProxy* pqCMBMeshViewerMainWindowCore::getActiveSelection(int selFieldType)
{
  if(this->activeSource())
    {
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
      this->activeSource()->getProxy());
    if(smSource && smSource->GetSelectionInput(0))
      {
      vtkSMSourceProxy* selInput = smSource->GetSelectionInput(0);
      int fieldType = pqSMAdaptor::getElementProperty(
        selInput->GetProperty("FieldType")).toInt();
      return fieldType==selFieldType ? selInput : NULL;
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::extractSelectionAsInput()
{
  // make a copy of active selection
  vtkSMProxy* activeSelection = this->getActiveSelection();
  if (!activeSelection)
    {
    return NULL;
    }

  pqWaitCursor cursor;

  pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();
  pqPipelineSource* newSource = builder->createSource("sources",
    "GMSMeshSource", this->getActiveServer());

  vtkSMProxyProperty* selpp = vtkSMProxyProperty::SafeDownCast(
    this->Internal->ExtractSelection->getProxy()->GetProperty("Selection"));
  selpp->RemoveAllProxies();
  selpp->AddProxy(activeSelection);
  this->updateSource(this->Internal->ExtractSelection);
  vtkSMSourceProxy* currentSource = vtkSMSourceProxy::SafeDownCast(
    this->Internal->ExtractSelection->getProxy());
  vtkSMDataSourceProxy::SafeDownCast(
    newSource->getProxy())->CopyData(currentSource);

  this->updateSource(newSource);

  // TODO:: create representation for this source.
  pqDataRepresentation* repr =
    builder->createDataRepresentation(newSource->getOutputPort(0),
    this->activeRenderView());
  pqSMAdaptor::setElementProperty(
    repr->getProxy()->GetProperty("Pickable"), 0);
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(repr->getProxy(),
    vtkMultiBlockWrapper::GetShellTagName(), vtkDataObject::CELL);
  pqSMAdaptor::setEnumerationProperty(
    repr->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

  repr->getProxy()->UpdateVTKObjects();

  // Create selection proxy
  QPointer<pqPipelineSource> SelectionConverter = this->createFilter(
    "CmbMeshSelectionConverter", this->activeSource());

  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    SelectionConverter->getProxy());

  vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
    smSource->GetProperty("Selection"));
  pp->RemoveAllProxies();
  pp->AddProxy(activeSelection);

  vtkSMProxyProperty* ppMesh = vtkSMProxyProperty::SafeDownCast(
    smSource->GetProperty("Mesh"));
  ppMesh->RemoveAllProxies();
  ppMesh->AddProxy(this->meshSource()->getProxy());
  smSource->MarkModified(NULL);
  smSource->UpdateVTKObjects();
  smSource->UpdatePipeline();

  vtkSMDataSourceProxy* newSelSource = vtkSMDataSourceProxy::SafeDownCast(
    smSource->GetSessionProxyManager()->NewProxy("sources", "HydroModelSelectionSource"));
  newSelSource->UpdateVTKObjects();
  newSelSource->UpdatePipeline();

  newSelSource->CopyData(smSource);

  //newSource->Copy(smSource, 0,
  //  vtkSMProxy::COPY_PROXY_PROPERTY_VALUES_BY_CLONING);
  newSelSource->UpdateVTKObjects();
  newSelSource->UpdatePipeline();

  this->Internal->InputRepMap[repr] = newSelSource;
  builder->destroy(SelectionConverter);

  return repr;

}
//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::invertCurrentSelection()
{
  vtkSMSourceProxy* currentSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
    this->getActiveSelection());
  // for 2D mesh also check point selection
  if(!selSource && this->is2DMesh())
    {
    selSource = vtkSMSourceProxy::SafeDownCast(
      this->getActiveSelection(vtkSelectionNode::POINT));
    }
  if(!selSource)
    {
    return false;
    }
  int inverse = pqSMAdaptor::getElementProperty(
    selSource->GetProperty("InsideOut")).toInt();
  pqSMAdaptor::setElementProperty(
    selSource->GetProperty("InsideOut"), !inverse);
  selSource->MarkModified(NULL);
  selSource->UpdateVTKObjects();
  selSource->UpdatePipeline();
  this->updateSource(this->activeSource());

  //this->updateSelection();
  this->Internal->MeshRepresentation->getProxy()->MarkModified(NULL);
  vtkSMRepresentationProxy::SafeDownCast(
    this->Internal->MeshRepresentation->getProxy())->UpdatePipeline();
  this->activeRenderView()->render();
  return true;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::resetFilterInputArrays()
{
  const char* arrayName = NULL;
  if(this->Internal->HistogramMode == 0)  // by Quality
    {
    arrayName = "Quality";
    }
  else  // by Region
    {
    arrayName = vtkMultiBlockWrapper::GetShellTagName();
    }

  this->setInputArray(this->Internal->MeshHistogram,
    "SelectInputArray", arrayName);
  this->setInputArray(this->Internal->SelectionHistogram,
    "SelectInputArray", arrayName);
  this->setInputArray(this->Internal->MeshThresholdSource,
    "SelectInputScalars", vtkMultiBlockWrapper::GetShellTagName());
  this->setInputArray(this->Internal->QualityThreshSource,
    "SelectInputScalars","Quality");
  // update the threshold range property
  this->Internal->MeshThresholdSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();
  this->Internal->QualityThreshSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();

  this->Internal->MeshThreshPanel->apply();
  this->Internal->MeshThreshPanel->updatePanel();
  this->Internal->MeshQualityPanel->apply();
  this->Internal->MeshQualityPanel->updatePanel();
  this->Internal->QualityThreshPanel->apply();
  this->Internal->QualityThreshPanel->updatePanel();
  this->Internal->ElevationPanel->apply();
  this->Internal->ElevationPanel->updatePanel();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::destroyRepresentation(
  pqDataRepresentation* selRep)
{
  if(selRep && selRep != this->Internal->FullMeshRepresentation)
    {
    if(this->Internal->InputRepMap[selRep])
      {
      this->Internal->InputRepMap[selRep]->Delete();
      }
    this->Internal->InputRepMap.remove(selRep);
    pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();
    pqPipelineSource* source = selRep->getInput();
    builder->destroy(selRep);
    builder->destroy(source);
    }
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::getRepresentationFromSource(
  pqPipelineSource* selSource)
{
  // if the selSource is meshSource, it will return current mesh representation,
  if(selSource)
    {
    foreach(pqDataRepresentation* subsetRep, this->Internal->InputRepMap.keys())
      {
      if(subsetRep->getInput() == selSource)
        {
        return subsetRep;
        }
      }
    }
  return NULL;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onServerCreationFinished(pqServer *server)
{
  this->Superclass::onServerCreationFinished(server);

  // Set up connection with selection helpers for all views.
  this->renderViewSelectionHelper()->setView(this->activeRenderView());
  this->selectionManager()->setActiveView(this->activeRenderView());

  QObject::connect(this->renderViewSelectionHelper(),
    SIGNAL(selectionFinished(int, int, int, int)),
    this->renderViewSelectionHelper(),
    SLOT(endSelection()));

  this->activeRenderView()->getProxy()->UpdateVTKObjects();
  // Create representation to show the producer.
  vtkSMProxy* repr = server->proxyManager()->NewProxy("representations",
    "SpreadSheetRepresentation");

  // we always want to show all the blocks in the dataset
  vtkSMPropertyHelper(repr, "CompositeDataSetIndex").Set(0);
  vtkSMPropertyHelper(repr, "FieldAssociation").Set(1);//Cell
  repr->UpdateVTKObjects();

  vtkSMViewProxy* view = vtkSMViewProxy::SafeDownCast(
    server->proxyManager()->NewProxy("views", "SpreadSheetView"));
  vtkSMPropertyHelper(view, "SelectionOnly").Set(1);
//  vtkSMPropertyHelper(view, "Representations").Set(repr);
  vtkSMPropertyHelper(view, "ViewSize").Set(0, 1);
  vtkSMPropertyHelper(view, "ViewSize").Set(1, 1);
  view->UpdateVTKObjects();
  view->StillRender();
  this->Internal->SpreadSheetViewProxy.TakeReference(view);
  this->Internal->SpreadSheetRepProxy.TakeReference(repr);

  this->Internal->SpreadSheetViewModel = new pqSpreadSheetViewModel(
    this->Internal->SpreadSheetViewProxy, this);
}
//-----------------------------------------------------------------------------
pqSpreadSheetViewModel* pqCMBMeshViewerMainWindowCore::spreadSheetViewModel()
{
  //  vtkSMPropertyHelper(view, "Representations").Set(repr);
  return this->Internal->SpreadSheetViewModel;
}
//-----------------------------------------------------------------------------
vtkSMViewProxy* pqCMBMeshViewerMainWindowCore::spreadSheetView()
{
  //  vtkSMPropertyHelper(view, "Representations").Set(repr);
  return this->Internal->SpreadSheetViewProxy;
}
//-----------------------------------------------------------------------------
vtkSMProxy* pqCMBMeshViewerMainWindowCore::spreadSheetRepresentation()
{
  //  vtkSMPropertyHelper(view, "Representations").Set(repr);
  return this->Internal->SpreadSheetRepProxy;
}
//-----------------------------------------------------------------------------
pqContourWidget* pqCMBMeshViewerMainWindowCore::defineContourWidget()
{
  pqWaitCursor cursor;

  this->clearSelection();

  int orthoPlane;
  pqContourWidget* contourWidget = this->createPqContourWidget(orthoPlane);
  return contourWidget;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::clearContourSelection(
  pqContourWidget* contourWidget)
{
  if(this->Internal->MeshSculptingSource)
    {
    if(this->Internal->PositionLink)
      {
      this->Internal->PositionLink->RemoveAllLinks();
      this->Internal->ScaleLink->RemoveAllLinks();
      this->Internal->OrientationLink->RemoveAllLinks();
      }
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->Internal->MeshSculptingSource);
    this->Internal->MeshSculptingSource = NULL;
    this->Internal->MeshScultpingRep = NULL;
    }
  this->deleteContourWidget(contourWidget);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::contourSelectSurface(
  pqContourWidget* contourWidget, bool isCell, int selectContourType)
{
  pqWaitCursor cursor;
  // Now we need to do a Surface-point-selection on the active source with
  // the bounds of the contour widget, then pass the selection to
  // vtkCMBMeshContourSelector filter to get the selected points inside
  // the contour
  vtkNew<vtkPVContourRepresentationInfo> contourInfo;
  contourWidget->getWidgetProxy()->GetRepresentationProxy()->GatherInformation(
        contourInfo.GetPointer());

  double disBounds[6];
  this->getContourDisplayBounds(contourInfo.GetPointer(),disBounds);
  if(vtkBoundingBox::IsValid(disBounds))
    {
    vtkSMNewWidgetRepresentationProxy* widget = contourWidget->getWidgetProxy();
    if (widget)
      {
      vtkSMProxyProperty* proxyProp = vtkSMProxyProperty::SafeDownCast(
        widget->GetProperty("PointPlacer"));
      if (proxyProp && proxyProp->GetProxy(0))
        {
        int region[4]={0,0,0,0};
        region[0]=disBounds[0];region[1]=disBounds[2];
        region[2]=disBounds[1];region[3]=disBounds[3];
        vtkSmartPointer<vtkCollection> selectedRepresentations =
          vtkSmartPointer<vtkCollection>::New();
        vtkSmartPointer<vtkCollection> selectionSources =
          vtkSmartPointer<vtkCollection>::New();
        bool selectiondone = false;
        if(isCell)
          {
          selectiondone = this->activeRenderView()->getRenderViewProxy()->
            SelectSurfaceCells(region, selectedRepresentations, selectionSources,false);
          }
        else
          {
          selectiondone = this->activeRenderView()->getRenderViewProxy()->
            SelectSurfacePoints(region, selectedRepresentations, selectionSources,false);
          }
        if(!selectiondone || selectedRepresentations->GetNumberOfItems()==0)
          {
          return;
          }
        vtkSMRepresentationProxy* repr = vtkSMRepresentationProxy::SafeDownCast(
          selectedRepresentations->GetItemAsObject(0));
        vtkSmartPointer<vtkSMSourceProxy> selectionSource = vtkSMSourceProxy::SafeDownCast(
          selectionSources->GetItemAsObject(0));

        pqServerManagerModel* smmodel =
          pqApplicationCore::instance()->getServerManagerModel();
        pqDataRepresentation* pqRepr = smmodel->findItem<pqDataRepresentation*>(repr);
        if (!repr)
          {
          // No data display was selected (or none that is registered).
          return;
          }
        int fieldType = isCell ? vtkSelectionNode::CELL : vtkSelectionNode::POINT;
        this->updateMeshContourSelection(contourWidget, 0,
          selectionSource, selectContourType,fieldType);
        vtkSMSourceProxy* contourSelSource = this->hasContourSelection() ?
          vtkSMSourceProxy::SafeDownCast(
            this->Internal->MeshSelectorSelection->getProxy()) : NULL;
        this->setActiveSelection(contourSelSource);
        }
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::contourSelectThrough(
  pqContourWidget* contourWidget, int selectContourType)
{
  pqWaitCursor cursor;

  this->updateMeshContourSelection(contourWidget, 1,
    NULL, selectContourType, vtkSelectionNode::CELL);

  vtkSMSourceProxy* contourSelSource = this->hasContourSelection() ?
    vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshSelectorSelection->getProxy()) : NULL;
  this->setActiveSelection(contourSelSource);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setShapeSelectionOption(
  pqContourWidget* contourWidget,int selectCellThrough, int selectShapeType)
{
  if(this->shapeSelectionOption()==selectShapeType)
    {
    return;
    }
  this->Internal->ShapeSelectionOption = selectShapeType;
  vtkSMProxy* selProxy = this->getActiveSelection();
  if(selProxy &&
     selProxy != this->Internal->MeshSelectorSelection->getProxy())
    {
    return;
    }
  if(contourWidget && this->Internal->MeshContourSelector)
    {
    if(selectCellThrough)
      {
      this->contourSelectThrough(contourWidget, selectShapeType);
      }
    else
      {
      this->contourSelectSurface(contourWidget, true, selectShapeType);
      }
    }
  else if(this->Internal->MeshConeSelector &&
          this->Internal->ConeRepresentation)
    {
    bool visible = pqSMAdaptor::getElementProperty(
      this->Internal->ConeRepresentation->
      getProxy()->GetProperty("Visibility")).toBool();
    if(visible)
      {
      this->onUpdateConeInteraction();
      }
    }
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::getContourDisplayBounds(
  vtkPVContourRepresentationInfo* contourInfo, double bounds[6])
{
  vtkSMRenderViewProxy* rm = this->activeRenderView()->getRenderViewProxy();
  vtkBoundingBox bb;
  vtkViewport* viewport = rm->GetRenderer();
  if(contourInfo && contourInfo->GetNumberOfAllNodes()
    && contourInfo->GetClosedLoop() == 1)
    {
    vtkDoubleArray* coords = contourInfo->GetAllNodesWorldPositions();
    double point[3], transPt[3];
    for(vtkIdType i=0;i<coords->GetNumberOfTuples();i++)
      {
      coords->GetTypedTuple(i, point);
      viewport->SetWorldPoint(point[0], point[1], point[2], 1.0);
      viewport->WorldToDisplay();
      viewport->GetDisplayPoint(transPt);
      bb.AddPoint(transPt);
      }
    }
  bb.GetBounds(bounds);
}

//-----------------------------------------------------------------------------
vtkSMNewWidgetRepresentationProxy* pqCMBMeshViewerMainWindowCore::createPlaneWidget()
{
  if(!this->Internal->MeshScultpingRep ||
    !this->Internal->MeshSculptingSource)
    {
    return NULL;
    }
  vtkSMNewWidgetRepresentationProxy* planeWidget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("CmbPlaneWidgetRepresentation", this->getActiveServer());
  pqSMAdaptor::setElementProperty(planeWidget->GetProperty("Visibility"), false);
  pqSMAdaptor::setElementProperty(planeWidget->GetProperty("DrawPlane"), false);

  planeWidget->UpdateVTKObjects();

  pqSMAdaptor::addProxyProperty(this->activeRenderView()->getProxy()->
    GetProperty("Representations"), planeWidget);
  this->activeRenderView()->getProxy()->UpdateVTKObjects();

  // Lets change the event mapping of the box widget
  // vtkImplicitPlaneWidget2 *aw = vtkImplicitPlaneWidget2::SafeDownCast(planeWidget->GetWidget());

  return planeWidget;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::linkContourPlaneWidget(
  vtkSMNewWidgetRepresentationProxy* planeWidget)
{
  pqWaitCursor cursor;
  vtkSMProxy *srcProxy = this->Internal->MeshScultpingRep->getProxy();

  double bounds[6];
  this->meshSculptingRepresentation()->getOutputPortFromInput()->
    getDataInformation()->GetBounds(bounds);
  vtkBoundingBox bb(bounds);
  if(bb.IsValid())
    {
    QList<QVariant> values;
    values << bounds[0] << bounds[1] << bounds[2]
    << bounds[3] << bounds[4] << bounds[5];
    pqSMAdaptor::setMultipleElementProperty(planeWidget->
      GetProperty("PlaceWidget"),
      values);
    planeWidget->UpdateVTKObjects();
    double center[3];
    bb.GetCenter(center);
    vtkSMPropertyHelper(planeWidget, "Origin").Set(center, 3);
    }
  vtkSMPropertyHelper(planeWidget, "Normal").Set(
    this->Internal->SelSurfaceNodesOrietation, 3);

  pqSMAdaptor::setElementProperty(planeWidget->
    GetProperty("Visibility"), true);
  pqSMAdaptor::setElementProperty(planeWidget->
    GetProperty("Enabled"), true);
  planeWidget->UpdateVTKObjects();
  planeWidget->UpdatePropertyInformation();
  QList<QVariant> origin;
  origin = pqSMAdaptor::getMultipleElementProperty(
    planeWidget->GetProperty("OriginInfo"));
  for(int i=0; i<3 ; i++)
    {
    this->Internal->LastPlaneOrigin[i]=origin[i].toDouble();
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updatePlaneInteraction(
  vtkSMNewWidgetRepresentationProxy* planeWidget)
{
  double origPos[3];
  vtkSMProxy *srcProxy = this->Internal->MeshScultpingRep->getProxy();
  vtkSMPropertyHelper(srcProxy, "Position").Get(origPos, 3);
  double newOrigin[3];
  vtkSMPropertyHelper(planeWidget, "OriginInfo").Get(newOrigin, 3);
  double newPos[3];
  for(int i=0; i<3 ; i++)
    {
    newPos[i] = origPos[i] + (newOrigin[i]-this->Internal->LastPlaneOrigin[i]);
    this->Internal->LastPlaneOrigin[i]=newOrigin[i];
    }
  vtkSMPropertyHelper(srcProxy, "Position").Set(newPos, 3);
  srcProxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::createSelectedNodesRepresentation()
{
  int fieldType = this->getActiveSelection(vtkSelectionNode::CELL) ?
    vtkSelectionNode::CELL : vtkSelectionNode::POINT;
  vtkSMProxy* selProxy = this->getActiveSelection(fieldType);
  if(!selProxy)
    {
    return;
    }

  pqPVApplicationCore* core = pqPVApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource* currentSelCopy = builder->createSource("sources",
    "HydroModelSelectionSource", this->getActiveServer());
  vtkSMDataSourceProxy* meshSelSource = vtkSMDataSourceProxy::SafeDownCast(
    currentSelCopy->getProxy());
  meshSelSource->CopyData(vtkSMSourceProxy::SafeDownCast(selProxy));
  meshSelSource->MarkModified(NULL);
  meshSelSource->UpdateVTKObjects();
  meshSelSource->UpdatePipeline();

  this->updateMeshContourSelection(NULL, 0, meshSelSource, 0,fieldType, 1);
  builder->destroy(currentSelCopy);
  if(!this->Internal->MeshContourSelector)
    {
    return;
    }

  // --------------------------------------------------
  vtkSMSourceProxy* contourSelSource = vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshContourSelector->getProxy());

  // Copy the Selected output poly from port 1.
  if(!this->Internal->MeshSculptingSource)
    {
    this->Internal->MeshSculptingSource = builder->createSource("sources",
      "HydroModelPolySource", this->getActiveServer());
    }
  vtkSMSourceProxy* pdSource = vtkSMSourceProxy::SafeDownCast(
  this->Internal->MeshSculptingSource->getProxy());
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(contourSelSource) << "GetSelectionPolyData"
    << vtkClientServerStream::End;
  contourSelSource->GetSession()->ExecuteStream(contourSelSource->GetLocation(), stream);

  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(pdSource)
    << "CopyData"
    << vtkClientServerStream::LastResult
    << vtkClientServerStream::End;
  pdSource->GetSession()->ExecuteStream(pdSource->GetLocation(), stream);
  pdSource->MarkModified(NULL);
  pdSource->UpdateVTKObjects();
  pdSource->UpdatePipeline();

  if(!this->Internal->MeshScultpingRep)
    {
    this->Internal->MeshScultpingRep =
    builder->createDataRepresentation(
    this->Internal->MeshSculptingSource->getOutputPort(0),
    this->activeRenderView());
    pqSMAdaptor::setElementProperty(
    this->Internal->MeshScultpingRep->getProxy()->GetProperty("Pickable"), 0);
    pqSMAdaptor::setElementProperty(
    this->Internal->MeshScultpingRep->getProxy()->GetProperty("PointSize"), 3);

    double rgb[3]={0.0,1.0,0.0};
    vtkSMPropertyHelper(this->Internal->MeshScultpingRep->getProxy(),
    "DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(this->Internal->MeshScultpingRep->getProxy(),
    "AmbientColor").Set(rgb, 3);
    }
  this->Internal->MeshScultpingRep->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::updateMeshContourSelection(
  pqContourWidget* contourWidget, int selectCellThrough,
  vtkSMProxy* selectionSource, int selectContourType,
  int fieldType, int GenerateSelectedOutput)
{
  this->clearSelection();

  if(!this->Internal->MeshContourSelector)
    {
    this->Internal->MeshContourSelector = this->createFilter(
      "CmbMeshContourSelector", this->activeSource());
    }
  if(!this->Internal->MeshSurfaceFilter && !this->is2DMesh())
    {
    this->Internal->MeshSurfaceFilter = this->createFilter(
      "DataSetSurfaceFilter", this->activeSource());
    }

  vtkSMSourceProxy* contourSelSource = vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshContourSelector->getProxy());
  pqSMAdaptor::setElementProperty(contourSelSource->
    GetProperty("SelectCellThrough"), selectCellThrough);
  pqSMAdaptor::setElementProperty(contourSelSource->
    GetProperty("SelectContourType"), selectContourType);
  pqSMAdaptor::setElementProperty(contourSelSource->
    GetProperty("FieldType"), fieldType);
  pqSMAdaptor::setElementProperty(contourSelSource->
    GetProperty("GenerateSelectedOutput"), GenerateSelectedOutput);

  vtkSMProxyProperty* pSel = vtkSMProxyProperty::SafeDownCast(
    contourSelSource->GetProperty("Selection"));
  pSel->RemoveAllProxies();
  if(selectionSource)
    {
    pSel->AddProxy(selectionSource);
    }
  vtkSMProxyProperty* pSurface = vtkSMProxyProperty::SafeDownCast(
    contourSelSource->GetProperty("SurfaceInput"));
  pSurface->RemoveAllProxies();
  if(this->Internal->MeshSurfaceFilter)
    {
    pSurface->AddProxy(this->Internal->MeshSurfaceFilter->getProxy());
    }
  vtkSMProxyProperty* contourFunc = vtkSMProxyProperty::SafeDownCast(
    contourSelSource->GetProperty("Contour"));
  contourFunc->RemoveAllProxies();
  if(contourWidget)
    {
    vtkSMProxy* implicitLoop = vtkSMProxyManager::GetProxyManager()->NewProxy(
      "implicit_functions", "ImplicitSelectionLoop");
    this->updateContourLoop(implicitLoop, contourWidget);
    contourFunc->AddProxy(implicitLoop);
    implicitLoop->Delete();
    }
  contourSelSource->MarkModified(NULL);
  contourSelSource->UpdateVTKObjects();
  contourSelSource->UpdatePipeline();
  contourSelSource->UpdatePropertyInformation();
  vtkSMPropertyHelper(contourSelSource, "OrientationOfSelectedNodes").Get(
    this->Internal->SelSurfaceNodesOrietation, 3);

  if(!this->Internal->MeshSelectorSelection)
    {
    pqPVApplicationCore* core = pqPVApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->Internal->MeshSelectorSelection = builder->createSource("sources",
        "HydroModelSelectionSource", this->getActiveServer());
    }
  vtkSMDataSourceProxy* meshSelSource = vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->MeshSelectorSelection->getProxy());
  meshSelSource->CopyData(contourSelSource);
  pqSMAdaptor::setElementProperty(meshSelSource->
    GetProperty("FieldType"), fieldType);
  meshSelSource->MarkModified(NULL);
  meshSelSource->UpdateVTKObjects();
  meshSelSource->UpdatePipeline();
}

//-----------------------------------------------------------------------------
vtkSMNewWidgetRepresentationProxy* pqCMBMeshViewerMainWindowCore::createBoxWidget()
{
  vtkSMNewWidgetRepresentationProxy* boxWidget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("BoxWidgetRepresentation", this->getActiveServer());
  pqSMAdaptor::setElementProperty(boxWidget->GetProperty("Visibility"), false);
  vtkSMPropertyHelper(boxWidget,
    "PlaceFactor").Set(1.05);

  boxWidget->UpdateVTKObjects();

  pqSMAdaptor::addProxyProperty(this->activeRenderView()->getProxy()->
    GetProperty("Representations"), boxWidget);

  this->activeRenderView()->getProxy()->UpdateVTKObjects();

  // Lets change the event mapping of the box widget
  vtkBoxWidget2 *aw = vtkBoxWidget2::SafeDownCast(boxWidget->GetWidget());
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
  return boxWidget;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::linkContourBoxWidget(
  vtkSMNewWidgetRepresentationProxy* boxWidget, bool enable)
{
  if(this->Internal->PositionLink)
    {
    this->Internal->PositionLink->RemoveAllLinks();
    }
  else
    {
    this->Internal->PositionLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    }
  if(this->Internal->ScaleLink)
    {
    this->Internal->ScaleLink->RemoveAllLinks();
    }
  else
    {
    this->Internal->ScaleLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    }
  if(this->Internal->OrientationLink)
    {
    this->Internal->OrientationLink->RemoveAllLinks();
    }
  else
    {
    this->Internal->OrientationLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    }
  this->linkBoxWidget(
    this->Internal->PositionLink,
    this->Internal->OrientationLink,
    this->Internal->ScaleLink,
    boxWidget, this->Internal->MeshScultpingRep, enable);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::linkBoxWidget(
  vtkSMPropertyLink* positionLink,
  vtkSMPropertyLink* rotationLink, vtkSMPropertyLink* scaleLink,
  vtkSMNewWidgetRepresentationProxy* boxWidget,
  pqDataRepresentation* dataRep, bool enable)
{
  pqWaitCursor cursor;

  vtkSMProxy *srcProxy = dataRep->getProxy();
  boxWidget->GetProperty("Position")->
    Copy(srcProxy->GetProperty("Position"));
  positionLink->AddLinkedProperty(srcProxy, "Position",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);
  positionLink->AddLinkedProperty(boxWidget, "Position",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);

  boxWidget->GetProperty("Rotation")->
    Copy(srcProxy->GetProperty("Orientation"));
  rotationLink->AddLinkedProperty(srcProxy, "Orientation",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);
  rotationLink->AddLinkedProperty(boxWidget, "Rotation",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);

  boxWidget->GetProperty("Scale")->
    Copy(srcProxy->GetProperty("Scale"));
  scaleLink->AddLinkedProperty(srcProxy, "Scale",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);
  scaleLink->AddLinkedProperty(boxWidget, "Scale",
    vtkSMLink::INPUT|vtkSMLink::OUTPUT);

  double bounds[6];
  dataRep->getOutputPortFromInput()->
    getDataInformation()->GetBounds(bounds);
  if(vtkBoundingBox::IsValid(bounds))
    {
    QList<QVariant> values;
    values << bounds[0] << bounds[1] << bounds[2]
    << bounds[3] << bounds[4] << bounds[5];
    pqSMAdaptor::setMultipleElementProperty(boxWidget->
      GetProperty("PlaceWidget"),
      values);
    }
  if(enable)
    {
    pqSMAdaptor::setElementProperty(boxWidget->
      GetProperty("Visibility"), true);
    pqSMAdaptor::setElementProperty(boxWidget->
      GetProperty("Enabled"), true);
    }
  boxWidget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::moveMeshScultpingPoints()
{
  if(!this->Internal->MeshScultpingRep ||
    !this->Internal->MeshSculptingSource)
    {
    return false;
    }

  pqWaitCursor cursor;
  pqPVApplicationCore* core = pqPVApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  if(!this->Internal->ContourTransform)
    {
    this->Internal->ContourTransform = builder->createProxy(
      "transforms", "Transform", this->getActiveServer(), "transforms");
    }

  double position[3], scale[3], orientation[3], origin[3];
  vtkSMProxy* contourRepProxy = this->Internal->MeshScultpingRep->getProxy();
  vtkSMPropertyHelper(contourRepProxy, "Position").Get(position, 3);
  vtkSMPropertyHelper(contourRepProxy, "Orientation").Get(orientation, 3);
  vtkSMPropertyHelper(contourRepProxy, "Scale").Get(scale, 3);
  vtkSMPropertyHelper(contourRepProxy, "Origin").Get(origin, 3);

  // build the transformation
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Identity();
  transform->PreMultiply();
  transform->Translate( position[0] + origin[0],
    position[1] + origin[1],
    position[2] + origin[2] );
  transform->RotateZ( orientation[2] );
  transform->RotateX( orientation[0] );
  transform->RotateY( orientation[1] );
  transform->Scale( scale );
  transform->Translate( -origin[0], -origin[1], -origin[2] );

  vtkMatrix4x4 *matrix = transform->GetMatrix();
  QList<QVariant> values;
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      values << matrix->Element[i][j];
      }
    }
  pqSMAdaptor::setMultipleElementProperty(
    this->Internal->ContourTransform->GetProperty("Matrix"), values);
  this->Internal->ContourTransform->UpdateVTKObjects();
  return this->moveMeshPoints(this->Internal->MeshSculptingSource,
    this->Internal->ContourTransform);
}
//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::moveMeshPoints(
  pqPipelineSource* movedsource, vtkSMProxy* transformProxy)
{
  if(!movedsource)
    {
    return false;
    }
  bool meshmoved = vtkSMMeshSourceProxy::SafeDownCast(
    this->Internal->FilterSource->getProxy())->MovePoints(
    movedsource->getProxy(), transformProxy);
  if(meshmoved)
    {
    this->updateSource(this->Internal->FilterSource);
    this->updateSource(this->Internal->MeshQualitySource);
    this->updateSource(this->Internal->QualityThreshSource);
    this->updateSource(this->Internal->ElevationFilter);
    this->Internal->QualityThreshPanel->apply();
    this->Internal->ElevationPanel->apply();
    this->updateMeshHistogram();
    this->Internal->MeshRepresentation->getProxy()->MarkModified(NULL);
    vtkSMRepresentationProxy::SafeDownCast(
      this->Internal->MeshRepresentation->getProxy())->UpdatePipeline();
    this->activeRenderView()->render();
    }
  return meshmoved;
}
//----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::hasContourSelection()
{
  if(!this->Internal->MeshContourSelector)
    {
    return false;
    }
  vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
      this->Internal->MeshContourSelector->getProxy());
  if(selSource)
    {
    vtkSMIntVectorProperty* vp = vtkSMIntVectorProperty::SafeDownCast(
      selSource->GetProperty("IsSelectionEmpty"));
    selSource->UpdatePropertyInformation(vp);
    int isSelectionEmpty = pqSMAdaptor::getElementProperty(vp).toInt();
    return isSelectionEmpty ? false : true;
    }
  return false;
}
//----------------------------------------------------------------------------
int pqCMBMeshViewerMainWindowCore::shapeSelectionOption()
{
  return this->Internal->ShapeSelectionOption;
}
//----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::hasConeSelection()
{
  if(!this->Internal->MeshConeSelector)
    {
    return false;
    }
  vtkSMSourceProxy* selSource = vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshConeSelector->getProxy());
  if(selSource)
    {
    vtkSMIntVectorProperty* vp = vtkSMIntVectorProperty::SafeDownCast(
      selSource->GetProperty("IsSelectionEmpty"));
    selSource->UpdatePropertyInformation(vp);
    int isSelectionEmpty = pqSMAdaptor::getElementProperty(vp).toInt();
    return isSelectionEmpty ? false : true;
    }
  return false;
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::applySmoothing()
{
  if(!this->activeSource())
    {
    return;
    }
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    this->activeSource()->getProxy());
  if(!smSource || !smSource->GetSelectionInput(0))
    {
    return;
    }
  this->createSmoothMeshPanel();
  if(!this->Internal->MeshSurfaceFilter && !this->is2DMesh())
    {
    this->Internal->MeshSurfaceFilter = this->createFilter(
      "DataSetSurfaceFilter", this->activeSource());
    }
  vtkSMSourceProxy* smoothFilter = vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshSmoother->getProxy());
  vtkSMProxyProperty* pSel = vtkSMProxyProperty::SafeDownCast(
    smoothFilter->GetProperty("Selection"));
  pSel->RemoveAllProxies();
  pSel->AddProxy(smSource->GetSelectionInput(0));

  vtkSMProxyProperty* pSurface = vtkSMProxyProperty::SafeDownCast(
    smoothFilter->GetProperty("SurfaceInput"));
  pSurface->RemoveAllProxies();
  if(this->Internal->MeshSurfaceFilter)
    {
    pSurface->AddProxy(this->Internal->MeshSurfaceFilter->getProxy());
    }
  this->updateSource(this->Internal->MeshSmoother);
  this->Internal->MeshSmootherPanel->apply();
  this->Internal->QualityThreshSource->getProxy()->GetProperty(
    "ThresholdBetween")->ResetToDomainDefaults();
  this->Internal->QualityThreshPanel->apply();
  this->Internal->ElevationPanel->apply();
  this->moveMeshPoints(this->Internal->MeshSmoother);
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::createSmoothMeshPanel()
{
  if(!this->Internal->MeshSmoother)
    {
    this->Internal->MeshSmoother = this->createFilter(
      "CmbSmoothMeshFilter", this->activeSource());
    }
  if(!this->Internal->MeshSmootherPanel)
    {
    this->Internal->MeshSmootherPanel = new pqProxyWidget(
      this->Internal->MeshSmoother->getProxy(), this->Internal->MeshSmootherPanelParent);
    this->Internal->MeshSmootherPanel->setObjectName("MeshSmootherPanel");
    this->Internal->MeshSmootherPanelParent->layout()->addWidget(
      this->Internal->MeshSmootherPanel);
    this->Internal->MeshSmootherPanel->setView(this->activeRenderView());
    }
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::setSmoothMeshPanelParent(QWidget* parent)
{
  this->Internal->MeshSmootherPanelParent = parent;
}
pqDataRepresentation* pqCMBMeshViewerMainWindowCore::coneRepresentation()
{
  return this->Internal->ConeRepresentation;
}
//----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindowCore::startConeSelection(bool showDialog)
{
  if(!this->meshSource())
    {
    return false;
    }
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if(!this->Internal->ConeSource)
    {
    this->Internal->ConeSource = builder->createSource(
      "sources", "CmbConeSource", this->getActiveServer());
    double baseCenter[3];
    double direction[3]={0.0,0.0,-1.0};
    vtkSMPropertyHelper(this->Internal->ConeSource->getProxy(),
      "Direction").Set(direction, 3);
    double meshbounds[6];
    this->Internal->FullMeshRepresentation->getOutputPortFromInput()->
      getDataInformation()->GetBounds(meshbounds);
    if(vtkBoundingBox::IsValid(meshbounds))
      {
      baseCenter[0]=(meshbounds[0]+meshbounds[1])/2.0;
      baseCenter[1]=(meshbounds[2]+meshbounds[3])/2.0;
      baseCenter[2]=meshbounds[5];
      vtkSMPropertyHelper(this->Internal->ConeSource->getProxy(),
        "BaseCenter").Set(baseCenter, 3);
      double height = fabs(meshbounds[5]-meshbounds[4])*1.05;
      height = height==0 ? 0.5 : height;
      pqSMAdaptor::setElementProperty(this->Internal->ConeSource->
        getProxy()->GetProperty("Height"), height);
      double baseRadius = fabs(meshbounds[1]-meshbounds[0])*0.2;
      baseRadius = baseRadius==0 ? 0.5 : baseRadius;
      pqSMAdaptor::setElementProperty(this->Internal->ConeSource->
        getProxy()->GetProperty("BaseRadius"), baseRadius);
      }

    this->Internal->ConeSource->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(
      this->Internal->ConeSource->getProxy())->UpdatePipeline();
    }
  bool firsttime = false;
  if(!this->Internal->ConeSourceDialog)
    {
    this->Internal->ConeSourceDialog = new qtCMBConeDialog(
      this->Internal->ConeSource, this->activeRenderView());
    firsttime = true;
    }

  // if this is the first time, we need to show the dialog no matter
  // where the request come from.
  if(firsttime || showDialog || !this->Internal->ConeRepresentation)
    {
    if(this->Internal->ConeSourceDialog->exec()==QDialog::Rejected)
      {
      return false;
      }
    }
  if(!this->Internal->ConeRepresentation)
    {
    this->Internal->ConeRepresentation =
      builder->createDataRepresentation(
      this->Internal->ConeSource->getOutputPort(0),
      this->activeRenderView(), "GeometryRepresentation");
    vtkSMProxy* coneRepProxy = this->Internal->ConeRepresentation->getProxy();
    pqSMAdaptor::setElementProperty(coneRepProxy->GetProperty("Pickable"), 0);
    pqSMAdaptor::setElementProperty(coneRepProxy->GetProperty("Opacity"), 0.3);
    double rgb[3]={0.0,1.0,0.0};
    vtkSMPropertyHelper(coneRepProxy,"DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(coneRepProxy,"AmbientColor").Set(rgb, 3);

    coneRepProxy->UpdateVTKObjects();
    }
//  this->Internal->ConeRepresentation->setDefaultPropertyValues();
  pqSMAdaptor::setElementProperty(this->Internal->ConeRepresentation->
    getProxy()->GetProperty("Visibility"), true);
  this->Internal->ConeRepresentation->
    getProxy()->UpdateVTKObjects();
  this->Internal->ConeRepresentation->
    getProxy()->UpdatePropertyInformation();
  if(!this->Internal->ConeWidget)
    {
    this->Internal->ConeWidget = this->createBoxWidget();
    if(this->Internal->ConePositionLink)
      {
      this->Internal->ConePositionLink->RemoveAllLinks();
      }
    this->Internal->ConePositionLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    if(this->Internal->ConeScaleLink)
      {
      this->Internal->ConeScaleLink->RemoveAllLinks();
      }
    this->Internal->ConeScaleLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    if(this->Internal->ConeOrientationLink)
      {
      this->Internal->ConeOrientationLink->RemoveAllLinks();
      }
    this->Internal->ConeOrientationLink = vtkSmartPointer<vtkSMPropertyLink>::New();
    if(!this->Internal->VTKConeConnect)
      {
      this->Internal->VTKConeConnect =
        vtkSmartPointer<vtkEventQtSlotConnect>::New();
      }

    this->Internal->VTKConeConnect->Disconnect();
    //this->Internal->VTKConeConnect->Connect(this->Internal->ConeWidget,
    //  vtkCommand::InteractionEvent,
    //  this, SLOT(onUpdateConeInteraction()));
    this->Internal->VTKConeConnect->Connect(this->Internal->ConeWidget,
      vtkCommand::EndInteractionEvent,
      this, SLOT(onUpdateConeInteraction()));
    }
  this->linkBoxWidget(
    this->Internal->ConePositionLink,
    this->Internal->ConeOrientationLink,
    this->Internal->ConeScaleLink,
    this->Internal->ConeWidget,
    this->Internal->ConeRepresentation, true);
  // do a cone selection on startup
  this->onUpdateConeInteraction();
  this->activeRenderView()->render();
  return true;
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::stopConeSelection()
{
  if(this->Internal->ConeWidget)
    {
    pqSMAdaptor::setElementProperty(this->Internal->ConeWidget->
      GetProperty("Visibility"), false);
    pqSMAdaptor::setElementProperty(this->Internal->ConeWidget->
      GetProperty("Enabled"), false);
    this->Internal->ConeWidget->UpdateVTKObjects();
    }
  if(this->Internal->ConeRepresentation)
    {
    pqSMAdaptor::setElementProperty(this->Internal->ConeRepresentation->
      getProxy()->GetProperty("Visibility"), false);
    this->Internal->ConeRepresentation->
      getProxy()->UpdateVTKObjects();
    }
  this->activeRenderView()->render();
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindowCore::onUpdateConeInteraction()
{
  pqWaitCursor cursor;
  this->clearSelection();

  if(!this->Internal->MeshConeSelector)
    {
    this->Internal->MeshConeSelector = this->createFilter(
      "CmbMeshConeSelector", this->activeSource());
    }

  vtkSMSourceProxy* coneSelSource = vtkSMSourceProxy::SafeDownCast(
    this->Internal->MeshConeSelector->getProxy());
  pqSMAdaptor::setElementProperty(coneSelSource->
                                  GetProperty("SelectConeType"),
                                  this->shapeSelectionOption());
  pqSMAdaptor::setElementProperty(coneSelSource->
                                  GetProperty("FieldType"), 0);//cell

  vtkSMProxyProperty* coneSourceProp = vtkSMProxyProperty::SafeDownCast(
    coneSelSource->GetProperty("ConeSource"));
  coneSourceProp->RemoveAllProxies();
  coneSourceProp->AddProxy(this->Internal->ConeSource->getProxy());

  double position[3], scale[3], orientation[3], origin[3];
  vtkSMProxy* coneRepProxy = this->Internal->ConeRepresentation->getProxy();
  vtkSMPropertyHelper(coneRepProxy, "Position").Get(position, 3);
  vtkSMPropertyHelper(coneRepProxy, "Orientation").Get(orientation, 3);
  vtkSMPropertyHelper(coneRepProxy, "Scale").Get(scale, 3);
  vtkSMPropertyHelper(coneRepProxy, "Origin").Get(origin, 3);

  // build the transformation
  // Create a transform for transforming the test points into
  // the coordinate system of the cone
  vtkNew<vtkTransform> transform;
  transform->Identity();
  transform->PreMultiply();
  transform->Translate( position[0] + origin[0],
    position[1] + origin[1],
    position[2] + origin[2] );
  transform->RotateZ( orientation[2] );
  transform->RotateX( orientation[0] );
  transform->RotateY( orientation[1] );
  transform->Scale( scale );
  transform->Translate( -origin[0], -origin[1], -origin[2] );
  transform->Inverse();
  transform->Update();

  vtkMatrix4x4 *matrix = transform->GetMatrix();
  QList<QVariant> values;
  for (int i = 0; i < 4; i++)
    {
    for (int j = 0; j < 4; j++)
      {
      values << matrix->Element[i][j];
      }
    }
  pqSMAdaptor::setMultipleElementProperty(
    coneSelSource->GetProperty("Transform"), values);
  coneSelSource->MarkModified(NULL);
  coneSelSource->UpdateVTKObjects();
  coneSelSource->UpdatePipeline();
  coneSelSource->UpdatePropertyInformation();

  if(!this->Internal->MeshSelectorSelection)
    {
    pqPVApplicationCore* core = pqPVApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->Internal->MeshSelectorSelection = builder->createSource("sources",
      "HydroModelSelectionSource", this->getActiveServer());
    }
  vtkSMDataSourceProxy* meshSelSource = vtkSMDataSourceProxy::SafeDownCast(
    this->Internal->MeshSelectorSelection->getProxy());
  meshSelSource->CopyData(coneSelSource);
  pqSMAdaptor::setElementProperty(meshSelSource->
                                  GetProperty("FieldType"), 0); //cell
  meshSelSource->MarkModified(NULL);
  meshSelSource->UpdateVTKObjects();
  meshSelSource->UpdatePipeline();
  this->setActiveSelection(meshSelSource);
  this->selectionManager()->select(this->activeSource()->getOutputPort(0));
}
