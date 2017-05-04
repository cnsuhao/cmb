//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBPointsBuilderMainWindowCore.h"
#include "pqCMBAppCommonConfig.h" // for CMB_TEST_DATA_ROOT

#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBLIDARPieceObject.h"
#include "pqCMBLIDARPieceTable.h"
#include "pqCMBLIDARReaderManager.h"
#include "pqCMBLIDARSaveDialog.h"
#include "pqCMBModifierArcManager.h"
#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"
#include "pqDEMExporterDialog.h"
#include "qtCMBApplicationOptions.h"
#include "qtCMBLIDARFilterDialog.h"
#include "qtCMBLIDARPanelWidget.h"
#include "ui_qtCMBMainWindow.h"
#include "ui_qtFilterDialog.h"
#include "ui_qtLIDARPanel.h"
#include "vtkTransform.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QFocusEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPair>
#include <QProgressBar>
#include <QScrollArea>
#include <QShortcut>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QVector>
#include <QtDebug>

#include "pqActionGroupInterface.h"
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCameraDialog.h"
#include "pqCustomFilterDefinitionModel.h"
#include "pqCustomFilterDefinitionWizard.h"
#include "pqCustomFilterManager.h"
#include "pqCustomFilterManagerModel.h"
#include "pqDataInformationWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqDockWindowInterface.h"
#include "pqFileDialogModel.h"
#include "pqGeneralTransferFunctionWidget.h"
#include "pqLinksManager.h"
#include "pqObjectBuilder.h"
#include "pqOutputWindow.h"
#include "smtk/extension/paraview/widgets/qtArcWidget.h"

#include "pqCMBRubberBandHelper.h"
#include "pqDataRepresentation.h"
#include "pqOptions.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqPipelineModel.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"

#include "pqActiveObjects.h"
#include "pqCMBProcessWidget.h"
#include "pqSMAdaptor.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqServerManagerObserver.h"
#include "pqSettings.h"
#include "pqSpreadSheetView.h"
#include "pqTimerLogDisplay.h"
#include "pqToolTipTrapper.h"
#include "pqUndoStackBuilder.h"
#include "pqView.h"
#include "pqViewContextMenuManager.h"
#include "qtCMBArcEditWidget.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include <pqFileDialog.h>
#include <pqSetData.h>
#include <pqSetName.h>
#include <pqUndoStack.h>
#include <pqWaitCursor.h>
#include <qtCMBProgressWidget.h>

#include "pqCMBArc.h"
#include <smtk/extension/paraview/widgets/qtArcWidget.h>

#include <QVTKWidget.h>

#include "vtkAbstractWidget.h"
#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkEvent.h"
#include "vtkIdTypeArray.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVContourRepresentationInfo.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include <vtkHydroModelCreator.h>
#include <vtkImageData.h>
#include <vtkMultiBlockWrapper.h>
#include <vtkNew.h>
#include <vtkPVDisplayInformation.h>
#include <vtkPVOptions.h>
#include <vtkPVXMLElement.h>
#include <vtkPVXMLParser.h>
#include <vtkPolyDataReader.h>
#include <vtkProcessModule.h>
#include <vtkSMDoubleRangeDomain.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMOutputPort.h>
#include <vtkSMProxyIterator.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSession.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSmartPointer.h>
#include <vtkToolkits.h>
#include <vtkUnstructuredGridReader.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "assert.h"
#include "vtkBoundingBox.h"
#include "vtkCellData.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkHydroModelPolySource.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMPropertyLink.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include <vtksys/Process.h>
#include <vtksys/SystemTools.hxx>

#include "pqCMBArcTreeItem.h"
#include "pqCMBLIDARContourTree.h"
#include "pqCMBLIDARTerrainExtractionManager.h"
#include "pqCMBPreviewDialog.h"

#include "pqCMBLoadDataReaction.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVContourGroupInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkSMSceneContourSourceProxy.h"

///////////////////////////////////////////////////////////////////////////
// pqCMBPointsBuilderMainWindowCore::vtkInternal

/// Private implementation details for pqCMBPointsBuilderMainWindowCore
class pqCMBPointsBuilderMainWindowCore::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/)
  {
    this->LIDARPanel = 0;
    this->PieceMainTable = NULL;
    this->pqCMBModifierArcTable = NULL;
    this->RepPropLink = NULL;
    this->OutlineSource = NULL;
    this->CurrentWriter = NULL;
    this->OutlineRepresentation = NULL;
    this->MinimumNumberOfPointsPerPiece = 50;
    this->LastSelectedObject = NULL;
    this->ConvertFromLatLong = 0;

    this->ContourTree = 0;
    this->CurrentTotalNumPoints = 0;
    this->AddingMoreFiles = false;
  }

  ~vtkInternal()
  {
    if (this->PieceMainTable)
    {
      delete this->PieceMainTable;
    }
    if (this->LIDARPanel)
    {
      delete this->LIDARPanel;
    }
    if (this->FilterDlg)
    {
      delete this->FilterDlg;
    }
    foreach (QString filename, this->FilePieceIdObjectMap.keys())
    {
      foreach (int pieceIdx, this->FilePieceIdObjectMap[filename].keys())
      {
        pqCMBLIDARPieceObject* dataObj = this->FilePieceIdObjectMap[filename].value(pieceIdx);
        if (dataObj)
        {
          delete dataObj;
        }
      }
    }
  }

  vtkSMPropertyLink* RepPropLink;
  QPointer<pqPipelineSource> CurrentWriter;
  QPointer<pqPipelineSource> OutlineSource;
  QPointer<pqDataRepresentation> OutlineRepresentation;
  QPointer<qtCMBProgressWidget> ProgressBar;
  QPointer<qtCMBLIDARFilterDialog> FilterDlg;

  vtkIdType CurrentTotalNumPoints;
  vtkIdType MinimumNumberOfPointsPerPiece;
  double DataBounds[6];
  pqCMBLIDARPieceTable* PieceMainTable;
  pqCMBModifierArcManager* pqCMBModifierArcTable;
  vtkBoundingBox ClipBBox;
  qtCMBLIDARPanelWidget* LIDARPanel;

  // Map for <Filename, vector< pair<pieceId, lidarObject> > >
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> > FilePieceIdObjectMap;
  pqCMBLIDARPieceObject* LastSelectedObject;
  QPointer<pqCMBLIDARContourTree> ContourTree;
  QMap<qtArcWidget*, vtkSMProxy*> ContourProxyMap;
  QMap<pqCMBArcTreeItem*, QList<qtArcWidget*> > ContourGroupMap;

  bool RenderNeeded;
  QList<QTreeWidgetItem*> NeedUpdateItems;
  int ConvertFromLatLong;

  double ElevationMaxZ;
  double ElevationMinZ;
  bool AddingMoreFiles;
};

///////////////////////////////////////////////////////////////////////////
// pqCMBPointsBuilderMainWindowCore

pqCMBPointsBuilderMainWindowCore::pqCMBPointsBuilderMainWindowCore(QWidget* parent_widget)
  : pqCMBCommonMainWindowCore(parent_widget)
  , Internal(new vtkInternal(parent_widget))
{
  this->buildRenderWindowContextMenuBehavior(parent_widget);
  this->ProgramKey = qtCMBProjectServerManager::PointsBuilder;
  this->setObjectName("LIDARMainWindowCore");
  this->TerrainExtractionManager = 0;
  this->ReaderManager = 0;

  this->Internal->FilterDlg = new qtCMBLIDARFilterDialog(parent_widget);

  QObject::connect(this->Internal->FilterDlg, SIGNAL(OkPressed()), this, SLOT(onFilterChanged()));

  this->connect(pqApplicationCore::instance()->getObjectBuilder(),
    SIGNAL(readerCreated(pqPipelineSource*, const QStringList&)), this,
    SLOT(onReaderForFilesCreated(pqPipelineSource*, const QStringList&)));
}

pqCMBPointsBuilderMainWindowCore::~pqCMBPointsBuilderMainWindowCore()
{
  //  pqActiveView::instance().setCurrent(0);
  this->clearContourFilters();
  if (this->TerrainExtractionManager)
  {
    delete this->TerrainExtractionManager;
  }
  if (this->ReaderManager)
  {
    delete this->ReaderManager;
  }
  delete Internal;
}

pqCMBLIDARTerrainExtractionManager* pqCMBPointsBuilderMainWindowCore::getTerrainExtractionManager()
{
  return this->TerrainExtractionManager;
}

pqCMBLIDARPieceTable* pqCMBPointsBuilderMainWindowCore::getLIDARPieceTable()
{
  return this->Internal->PieceMainTable;
}

void pqCMBPointsBuilderMainWindowCore::onOpenMoreFiles(pqCMBLoadDataReaction* ldReaction)
{
  this->Internal->AddingMoreFiles = true;
  bool cancelled;
  QStringList files;
  ldReaction->loadData(cancelled, files);

  this->Internal->AddingMoreFiles = false;
}

void pqCMBPointsBuilderMainWindowCore::ImportLIDARFiles(
  const QStringList& files, pqPipelineSource* reader)
{
  if (!this->getActiveServer())
  {
    qCritical() << "The server is not set!";
    return;
  }

  bool hasDataLoaded = this->IsDataLoaded();
  if (!this->Internal->AddingMoreFiles && hasDataLoaded)
  {
    int answer = QMessageBox::question(this->parentWidget(), "Closing Loaded Files?",
      "Do you want to close all current files? \n" +
        QString("Answer No, if you want to keep current files open."),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer == QMessageBox::No)
    {
      this->Internal->AddingMoreFiles = true;
    }
  }
  else if ((files.size() > 1) && (!hasDataLoaded))
  {
    //  If the are no files loaded and user has selected multiple files
    // load them all in
    this->Internal->AddingMoreFiles = true;
  }

  if (!this->Internal->AddingMoreFiles && hasDataLoaded)
  {
    this->onCloseData();
  }
  QStringList newFiles;
  foreach (QString strFileName, files)
  {
    if (this->ReaderManager->isFileLoaded(strFileName))
    {
      QString message("File is already loaded, \n");
      message.append(strFileName);
      QMessageBox::warning(NULL, tr("File Open Warning"), message, QMessageBox::Ok);
      continue;
    }
    newFiles.append(strFileName);
  }

  // exit from extraction tab (if that is the current tab)
  if (this->Internal->LIDARPanel->getGUIPanel()->tabWidget->currentIndex() == 2)
  {
    this->Internal->LIDARPanel->getGUIPanel()->tabWidget->setCurrentIndex(0);
  }

  this->Internal->CurrentTotalNumPoints =
    this->ReaderManager->scanTotalNumPointsInfo(newFiles, reader);
  if (this->Internal->CurrentTotalNumPoints <= 0)
  {
    QMessageBox::warning(
      this->parentWidget(), tr("File Open Warning"), "No points info found", QMessageBox::Ok);
    return;
  }

  foreach (QString filename, newFiles)
  {
    int result = this->ImportLIDARFile(filename.toLatin1().constData());
    if (!result)
    {
      QString message("Failed to load file, \n");
      message.append(filename).append(".\n Make sure the file is a valid LIDAR file.");
      QMessageBox::warning(this->parentWidget(), tr("File Open Warning"), message, QMessageBox::Ok);
    }
  }

  this->enableAbort(false);
  if (!this->Internal->AddingMoreFiles || !hasDataLoaded)
  {
    this->initThresholdTable();
    this->initPolygonsTree();
  }
  this->initControlPanel();

  if (this->Internal->FilePieceIdObjectMap.count() > 0)
  {
    // FIXME: not sure I've translated this code correctly. Please validate.
    // Add this to the list of recent server resources ...
    vtkSMProxy* readerProxy =
      this->ReaderManager->getReaderSourceProxy(files[0].toLatin1().constData());
    pqCMBRecentlyUsedResourceLoaderImplementatation::addDataFilesToRecentResources(
      this->getActiveServer(), files, readerProxy->GetXMLGroup(), readerProxy->GetXMLName());
    if (!this->Internal->AddingMoreFiles || !hasDataLoaded)
    {
      emit this->newDataLoaded();
      this->unselectCheckBoxes();
    }
  }
}

void pqCMBPointsBuilderMainWindowCore::onSaveAsData()
{
  this->onAcceptToLoad();
}

void pqCMBPointsBuilderMainWindowCore::onSaveData()
{
  this->onSaveAsData();
}

void pqCMBPointsBuilderMainWindowCore::onCloseData()
{
  // make sure the terrain extraction "preview" tree is cleared
  if (this->TerrainExtractionManager)
  {
    this->TerrainExtractionManager->onExitExtraction(true);
  }

  // clear the "# of Points" labels
  this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->setText("");
  this->Internal->LIDARPanel->getGUIPanel()->savePointsLabel->setText("");

  this->closeData();
  emit this->newDataLoaded();
}

void pqCMBPointsBuilderMainWindowCore::onRubberBandSelect(bool checked)
{
  if (checked)
  {

    this->renderViewSelectionHelper()->beginSurfaceSelection();
  }
  else
  {
    this->renderViewSelectionHelper()->endSelection();
  }
}

void pqCMBPointsBuilderMainWindowCore::onConvertFromLatLong(bool state)
{
  this->Internal->ConvertFromLatLong = state ? 1 : 0;
  if (this->ReaderManager->hasReaderSources())
  {
    this->ReaderManager->convertFromLatLongToXYZ(state);

    // turn off clipping (if on), because we need to recompute bounds
    bool clipWasOn = false;
    if (this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->isChecked())
    {
      this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setChecked(false);

      this->Internal->PieceMainTable->setClipEnabled(false);
      this->ReaderManager->limitReadToBounds(false);
      clipWasOn = true;
    }

    // force reread of every piece, or only selected pieces?
    this->enableAbort(true);

    QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();
    this->updatePieceRepresentations(allPieces, true);
    this->enableAbort(false);

    // update the outline source
    this->setupReadClipping();
    this->setupElevationFilter(this->Internal->DataBounds[4], this->Internal->DataBounds[5]);

    this->activeRenderView()->resetCamera();
    this->activeRenderView()->render();

    if (clipWasOn)
    {
      this->updatePointTotals();
    }

    this->updateLoadAndUpdateButtons();
  }
}

void pqCMBPointsBuilderMainWindowCore::closeData()
{
  pqActiveObjects::instance().setActiveSource(NULL);
  this->clearCurrentLIDARData();
  this->initControlPanel();
  this->Internal->pqCMBModifierArcTable->clear();
  this->Superclass::closeData();
}

bool pqCMBPointsBuilderMainWindowCore::IsDataLoaded()
{
  return this->Internal->FilePieceIdObjectMap.count() > 0;
}

void pqCMBPointsBuilderMainWindowCore::abort()
{
  vtkSMSourceProxy* source = NULL;
  if (this->TerrainExtractionManager->getFullTerrainFilter())
  {
    source = vtkSMSourceProxy::SafeDownCast(
      this->TerrainExtractionManager->getFullTerrainFilter()->getProxy());
  }
  else if (this->TerrainExtractionManager->getTerrainFilter())
  {
    source = vtkSMSourceProxy::SafeDownCast(
      this->TerrainExtractionManager->getTerrainFilter()->getProxy());
  }
  else if (this->Internal->CurrentWriter)
  {
    source = vtkSMSourceProxy::SafeDownCast(this->Internal->CurrentWriter->getProxy());
  }
  else if (this->ReaderManager->activeReader())
  {
    source = this->ReaderManager->activeReader();
  }

  if (source)
  {
    pqSMAdaptor::setElementProperty(source->GetProperty("AbortExecute"), 1);
    source->UpdateVTKObjects();
  }

  //this->enableAbort(false);
}

void pqCMBPointsBuilderMainWindowCore::onReaderForFilesCreated(
  pqPipelineSource* reader, const QStringList& files)
{
  if (!reader || files.empty())
  {
    return;
  }
  QStringList newFileList;
  if (files.size() > 1)
  {
    // This is coming from MostRecentFileList, so we
    // need to trim the first file name
    QString filename = files[0];
    int trimIndex = filename.lastIndexOf((QString(".(%1 files)").arg(files.size())));
    if (trimIndex != -1)
    {
      filename = filename.left(trimIndex);
    }
    newFileList.append(filename);
    QString strDir = vtksys::SystemTools::GetFilenamePath(filename.toLatin1().constData()).c_str();
    for (int i = 1; i < files.count(); i++)
    {
      filename = strDir;
      filename.append("/").append(files.at(i));
      newFileList.append(filename);
    }
  }
  else
  {
    newFileList.append(files);
  }
  this->ImportLIDARFiles(newFileList, reader);
}

int pqCMBPointsBuilderMainWindowCore::ImportLIDARFile(const char* filename)
{
  // The reader should have been created from scanTotalNumPointsInfo
  if (!this->ReaderManager->getReaderSourceProxy(filename))
  {
    return 0;
  }

  this->Internal->PieceMainTable->getWidget()->blockSignals(true);
  int res = this->ReaderManager->importData(filename, this->Internal->PieceMainTable,
    this->Internal->pqCMBModifierArcTable, this->Internal->FilePieceIdObjectMap,
    this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->isChecked());
  if (res != 0 && this->ReaderManager->userRequestsOrigin())
  {
    if (this->ReaderManager->setOrigin())
    {
      QList<pqCMBLIDARPieceObject*> allPieces =
        this->Internal->PieceMainTable->getAllPieceObjects();
      this->updatePieceRepresentations(allPieces, true);
      this->setupReadClipping();
      this->setupElevationFilter(this->Internal->DataBounds[4], this->Internal->DataBounds[5]);
      this->activeRenderView()->forceRender();
      this->activeRenderView()->resetCamera();
      this->activeRenderView()->render();
    }
  }

  if (res != 0 && this->ReaderManager->userRequestsDoubleData())
  {
    // return true if change (to double) made
    if (this->ReaderManager->setOutputDataTypeToDouble())
    {
      QList<pqCMBLIDARPieceObject*> allPieces =
        this->Internal->PieceMainTable->getAllPieceObjects();
      QList<pqCMBLIDARPieceObject*> filePieces =
        this->ReaderManager->getFilePieceObjects(filename, allPieces);
      this->ReaderManager->updatePieces(
        filename, filePieces, true, this->Internal->PieceMainTable, false, NULL);
    }
  }
  this->Internal->PieceMainTable->getWidget()->blockSignals(false);

  // reset progress to 0
  this->updateProgress(QString(tr("Done importing file: %1")).arg(filename), 0);

  if (this->Internal->FilePieceIdObjectMap.count() > 0)
  {
    return 1;
  }

  return 0;
}

void pqCMBPointsBuilderMainWindowCore::initThresholdTable()
{
  //Add the new filter to the table
  QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;

  table->setColumnCount(2);
  table->setHorizontalHeaderLabels(QStringList() << tr("Apply") << tr("Thresholds"));

  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->verticalHeader()->hide();

  QObject::connect(
    table, SIGNAL(itemSelectionChanged()), this, SLOT(onThresholdSelectionChanged()));
  QObject::connect(table, SIGNAL(itemChanged(QTableWidgetItem*)), this,
    SLOT(onThresholdItemChanged(QTableWidgetItem*)) /*, Qt::QueuedConnection*/);
}

void pqCMBPointsBuilderMainWindowCore::initPolygonsTree()
{
  if (this->Internal->ContourTree)
  {
    return;
  }

  this->Internal->ContourTree =
    new pqCMBLIDARContourTree(this->Internal->LIDARPanel->getGUIPanel()->frameContour);
  QObject::connect(this->Internal->ContourTree,
    SIGNAL(itemChanged(QList<QTreeWidgetItem*>, int, int)), this,
    SLOT(onPolygonItemChanged(QList<QTreeWidgetItem*>, int, int)));
  QObject::connect(this->Internal->ContourTree, SIGNAL(selectionChanged(QTreeWidgetItem*)), this,
    SLOT(onPolygonTableSelectionChanged(QTreeWidgetItem*)) /*, Qt::QueuedConnection*/);
  QObject::connect(this->Internal->ContourTree,
    SIGNAL(onItemsDropped(QTreeWidgetItem*, int, QList<QTreeWidgetItem*>)), this,
    SLOT(onPolygonTreeItemsDropped(
      QTreeWidgetItem*, int, QList<QTreeWidgetItem*>)) /*, Qt::QueuedConnection*/);
  QObject::connect(this->Internal->ContourTree, SIGNAL(itemRemoved(QList<qtArcWidget*>)), this,
    SLOT(onPolygonItemRemoved(QList<qtArcWidget*>)));
}

void pqCMBPointsBuilderMainWindowCore::initControlPanel()
{
  if (!this->IsDataLoaded())
  {
    this->clearCurrentLIDARData();
    this->Internal->LIDARPanel->blockSignals(true);

    this->Internal->LIDARPanel->getGUIPanel()->advancedCheckBox->blockSignals(true);
    this->Internal->LIDARPanel->getGUIPanel()->advancedCheckBox->setChecked(0);
    this->Internal->LIDARPanel->getGUIPanel()->advancedCheckBox->blockSignals(false);

    this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->blockSignals(true);
    this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setChecked(0);
    this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->blockSignals(false);

    this->Internal->LIDARPanel->getGUIPanel()->updateButton->setEnabled(false);
    //this->Internal->LIDARPanel->getGUIPanel()->loadButton->setEnabled( true );
    this->Internal->LIDARPanel->getGUIPanel()->updateButton->setBackgroundRole(QPalette::Button);
    this->Internal->LIDARPanel->blockSignals(false);

    this->Internal->PieceMainTable->clear();
    this->Internal->PieceMainTable->configureTable(false);
    this->Internal->PieceMainTable->setClipEnabled(false);

    this->Internal->LIDARPanel->setFocus(Qt::NoFocusReason);
    this->Internal->LIDARPanel->setEnabled(false);
  }
  else
  {
    this->setupReadClipping();
    this->setupElevationFilter(this->Internal->DataBounds[4], this->Internal->DataBounds[5]);
  }
}

void pqCMBPointsBuilderMainWindowCore::setupReadClipping()
{
  this->Internal->LIDARPanel->setEnabled(true);
  double bounds[6];
  this->ReaderManager->getDataBounds(bounds);
  // not sure we really need to save anything other than Z bounds...
  this->Internal->DataBounds[0] = bounds[0];
  this->Internal->DataBounds[1] = bounds[1];
  this->Internal->DataBounds[2] = bounds[2];
  this->Internal->DataBounds[3] = bounds[3];
  this->Internal->DataBounds[4] = bounds[4];
  this->Internal->DataBounds[5] = bounds[5];

  // setup the min and max values for clipping: 3 times the range we read in
  double xDelta = (bounds[1] - bounds[0]);
  this->Internal->LIDARPanel->getGUIPanel()->minXClip->setMinimum(bounds[0] - xDelta);
  this->Internal->LIDARPanel->getGUIPanel()->maxXClip->setMinimum(bounds[0] - xDelta);
  this->Internal->LIDARPanel->getGUIPanel()->minXClip->setMaximum(bounds[1] + xDelta);
  this->Internal->LIDARPanel->getGUIPanel()->maxXClip->setMaximum(bounds[1] + xDelta);
  double yDelta = (bounds[3] - bounds[2]);
  this->Internal->LIDARPanel->getGUIPanel()->minYClip->setMinimum(bounds[2] - yDelta);
  this->Internal->LIDARPanel->getGUIPanel()->maxYClip->setMinimum(bounds[2] - yDelta);
  this->Internal->LIDARPanel->getGUIPanel()->minYClip->setMaximum(bounds[3] + yDelta);
  this->Internal->LIDARPanel->getGUIPanel()->maxYClip->setMaximum(bounds[3] + yDelta);
  // set initial values for clipping... 1% larger in each direction
  this->Internal->LIDARPanel->getGUIPanel()->minXClip->setValue(bounds[0] - xDelta * 0.01);
  this->Internal->LIDARPanel->getGUIPanel()->maxXClip->setValue(bounds[1] + xDelta * 0.01);
  this->Internal->LIDARPanel->getGUIPanel()->minYClip->setValue(bounds[2] - yDelta * 0.01);
  this->Internal->LIDARPanel->getGUIPanel()->maxYClip->setValue(bounds[3] + yDelta * 0.01);

  if (!this->Internal->OutlineSource)
  {
    pqObjectBuilder* const builder = pqApplicationCore::instance()->getObjectBuilder();
    this->Internal->OutlineSource =
      builder->createSource("sources", "OutlineSource", this->getActiveServer());
    this->Internal->OutlineRepresentation =
      builder->createDataRepresentation(this->Internal->OutlineSource->getOutputPort(0),
        this->activeRenderView(), "GeometryRepresentation");
  }

  double zPad = (this->Internal->DataBounds[5] - this->Internal->DataBounds[4]) * 0.5;
  QList<QVariant> values;
  values << this->Internal->LIDARPanel->getGUIPanel()->minXClip->value()
         << this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value()
         << this->Internal->LIDARPanel->getGUIPanel()->minYClip->value()
         << this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value()
         << this->Internal->DataBounds[4] - zPad << this->Internal->DataBounds[5] + zPad;

  this->Internal->ClipBBox.SetBounds(this->Internal->LIDARPanel->getGUIPanel()->minXClip->value(),
    this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value(),
    this->Internal->LIDARPanel->getGUIPanel()->minYClip->value(),
    this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value(), VTK_DOUBLE_MIN, VTK_DOUBLE_MAX);
  this->Internal->PieceMainTable->setClipBBox(this->Internal->ClipBBox);

  pqSMAdaptor::setMultipleElementProperty(
    this->Internal->OutlineSource->getProxy()->GetProperty("Bounds"), values);
  this->Internal->OutlineSource->getProxy()->UpdateVTKObjects();

  // initially not visible
  this->Internal->OutlineRepresentation->setVisible(false);
}

void pqCMBPointsBuilderMainWindowCore::setupElevationFilter(double minZ, double maxZ)
{
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->blockSignals(true);

  // setup min and max values
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->setMinimum(VTK_FLOAT_MIN);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->setMaximum(maxZ);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->setMinimum(minZ);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->setMaximum(VTK_FLOAT_MAX);

  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->setValue(minZ);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->setValue(maxZ);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->blockSignals(false);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->blockSignals(false);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->setSingleStep(
    (maxZ - minZ) / 100.0);
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->setSingleStep(
    (maxZ - minZ) / 100.0);

  this->Internal->ElevationMinZ = minZ;
  this->Internal->ElevationMaxZ = maxZ;

  this->updateElevationFilterExtent();

  // is up-to-date... so disable the update button
  this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(false);
}

void pqCMBPointsBuilderMainWindowCore::updateElevationFilterExtent()
{
  // loop over all pieces, update ElevationFilter settings
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  double minPt[3] = { 0, 0,
    this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->value() };
  double maxPt[3] = { 0, 0,
    this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->value() };
  for (int i = 0; i < allPieces.size(); i++)
  {
    allPieces[i]->setElevationFilterLowPoint(minPt);
    allPieces[i]->setElevationFilterHighPoint(maxPt);
  }
  this->Internal->ElevationMinZ = minPt[2];
  this->Internal->ElevationMaxZ = maxPt[2];
}

QString pqCMBPointsBuilderMainWindowCore::getLIDARFileTitle() const
{
  return this->ReaderManager->getFileTitle();
}

void pqCMBPointsBuilderMainWindowCore::clearCurrentLIDARData()
{
  if (this->IsDataLoaded())
  {
    this->clearContourFilters();
    this->clearThresholdFilters();
  }

  pqCMBLIDARPieceObject* dataObj = NULL;
  foreach (QString filename, this->Internal->FilePieceIdObjectMap.keys())
  {
    foreach (int pieceIdx, this->Internal->FilePieceIdObjectMap[filename].keys())
    {
      dataObj = this->Internal->FilePieceIdObjectMap[filename].value(pieceIdx);
      if (dataObj)
      {
        delete dataObj;
      }
    }
  }
  this->Internal->FilePieceIdObjectMap.clear();
  if (this->Internal->pqCMBModifierArcTable != NULL)
    this->Internal->pqCMBModifierArcTable->clearProxies();

  if (this->Internal->RepPropLink)
  {
    this->Internal->RepPropLink->RemoveAllLinks();
    this->Internal->RepPropLink->Delete();
    this->Internal->RepPropLink = NULL;
  }
  this->Internal->LastSelectedObject = NULL;
  this->Internal->CurrentTotalNumPoints = 0;
  this->Internal->ConvertFromLatLong = 0;
  this->ReaderManager->destroyAllReaders();

  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setChecked(0);
  this->Internal->LIDARPanel->getGUIPanel()->frame_ROI->setVisible(0);
  if (this->Internal->OutlineRepresentation)
  {
    this->Internal->OutlineRepresentation->setVisible(0);
  }
  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->blockSignals(false);

  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->setChecked(0);
  this->Internal->LIDARPanel->getGUIPanel()->tabWidget_Filters->setVisible(0);
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->blockSignals(false);

  this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->setChecked(0);
  this->Internal->LIDARPanel->getGUIPanel()->elevationFrame->setVisible(0);
  this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->blockSignals(false);
}

void pqCMBPointsBuilderMainWindowCore::unselectCheckBoxes()
{
  //Make sure all toggle events are called by selecting all checkboxes
  //before unselecting
  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setChecked(1);
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->setChecked(1);
  this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->setChecked(1);

  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setChecked(0);
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->setChecked(0);
  this->Internal->LIDARPanel->getGUIPanel()->elevationGroup->setChecked(0);
}

void pqCMBPointsBuilderMainWindowCore::resetCenterOfRotationToCenterOfCurrentData()
{
  this->Superclass::resetCenterOfRotationToCenterOfCurrentData();
}

int pqCMBPointsBuilderMainWindowCore::onSavePieces(int /*onRatio*/, bool /*askMultiOutput*/)
{
  return 0;
}

void pqCMBPointsBuilderMainWindowCore::showFilterDialog()
{
  this->onActiveFilterChanged();
  this->Internal->FilterDlg->show();
  return;
}

void pqCMBPointsBuilderMainWindowCore::onAcceptToLoad()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() > 0)
  {
    QString filename;
    bool saveAsSingle, saveAsDisplayed;

    if (!pqCMBLIDARSaveDialog::getFile(this->parentWidget(), this->getActiveServer(),
          (this->Internal->PieceMainTable->getWidget()->rowCount() >= 2), &filename, &saveAsSingle,
          &saveAsDisplayed))
    {
      // This means user canceled op
      return;
    }
    this->enableAbort(true);
    if (!this->savePieces(visiblePieces, filename, saveAsSingle, saveAsDisplayed))
    {
      // write cancelled?
      this->enableAbort(false);
      return;
    }
    this->enableAbort(false);
  }
}

bool pqCMBPointsBuilderMainWindowCore::savePieces(QList<pqCMBLIDARPieceObject*> pieces,
  const QString& filename, bool saveAsSinglePiece, bool loadAsDisplayed, bool multiOutput)
{
  if (pieces.count() == 0)
    return false;
  QList<QString> outFileNames;
  if (pieces.count() > 1 && multiOutput &&
    !this->generateAndValidateOutFileNames(pieces, filename, outFileNames))
  {
    return false;
  }

  QFileInfo info(filename);
  QString extension(info.completeSuffix()), writerName;
  if (extension == "pts" || extension == "xyz")
  {
    writerName = "LIDARWriter";
  }
  else if (extension == "dem")
  {
    writerName = "DEMWriter";
  }
  else if (extension == "vtp")
  {
    writerName = "XMLPolyDataWriter";
  }
  else
  {
    QString message("The file extension, ");
    message.append(extension).append(", is not supported for writing LIDAR file.");
    QMessageBox::warning(this->parentWidget(), tr("File Save Warning"), message, QMessageBox::Ok);

    return false;
  }

  // update pieces if necessary
  QList<pqPipelineSource*> savePieces;
  this->ReaderManager->getSourcesForOutput(loadAsDisplayed, pieces, savePieces);

  // to get rid of lingering events that may undo our enabling of the ProgressBar
  QCoreApplication::processEvents();

  this->Internal->ProgressBar->setProgress(QString("Writing pieces ..."), 0);
  bool writeResult = false;
  if (writerName.compare("DEMWriter") == 0)
  {
    writeResult = this->ExportDem(savePieces, writerName, filename);
  }
  else if (savePieces.count() == 1)
  {
    writeResult = this->WritePiece(savePieces[0], writerName, filename);
  }
  else if (savePieces.count() > 1)
  {
    writeResult = this->WritePieces(savePieces, writerName, filename, saveAsSinglePiece);
  }
  this->ReaderManager->destroyTemporarySources();
  return writeResult;
}

bool pqCMBPointsBuilderMainWindowCore::WritePiece(
  pqPipelineSource* source, const QString& writerName, const QString& fileName)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  this->Internal->CurrentWriter =
    builder->createFilter("writers", writerName.toStdString().c_str(), source);
  return this->WriteFile(fileName);
}

bool pqCMBPointsBuilderMainWindowCore::ExportDem(
  QList<pqPipelineSource*> pieces, const QString& writerName, const QString& fileName)
{

  pqPipelineSource* pdSource = NULL;
  bool distroy = false;
  if (pieces.count() == 1)
  {
    pdSource = pieces[0];
  }
  else
  {
    QList<pqOutputPort*> inputs;
    for (int i = 0; i < pieces.count(); i++)
    {
      if (pieces[i])
      {
        inputs.push_back(pieces[i]->getOutputPort(0));
      }
    }
    if (inputs.count() == 0)
      return false;
    pdSource = this->getAppendedSource(inputs);
    distroy = true;
  }

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  QPointer<pqPipelineSource> filter =
    builder->createFilter("filters", "DemDataExtractor", pdSource);
  vtkSMSourceProxy* fspoxy = vtkSMSourceProxy::SafeDownCast(filter->getProxy());
  fspoxy->Modified();
  fspoxy->UpdateVTKObjects();
  fspoxy->UpdatePipeline();
  fspoxy->UpdatePropertyInformation();

  QList<QVariant> min = pqSMAdaptor::getMultipleElementProperty(fspoxy->GetProperty("Min"));
  QList<QVariant> max = pqSMAdaptor::getMultipleElementProperty(fspoxy->GetProperty("Max"));
  QList<QVariant> spacing = pqSMAdaptor::getMultipleElementProperty(fspoxy->GetProperty("Spacing"));
  double scale = pqSMAdaptor::getElementProperty(fspoxy->GetProperty("Scale")).toDouble();
  int zone = pqSMAdaptor::getElementProperty(fspoxy->GetProperty("Zone")).toInt();
  bool isNorth = pqSMAdaptor::getElementProperty(fspoxy->GetProperty("IsNorth")).toBool();

  builder->destroy(filter);
  filter = NULL;

  double rSpacing[2] = { spacing[0].toDouble(), spacing[1].toDouble() };
  double rMin[2] = { min[0].toDouble(), min[1].toDouble() };
  double rMax[2] = { max[0].toDouble(), max[1].toDouble() };
  int rsize[2];

  if (!pqDEMExportDialog::exportToDem(this->parentWidget(), this->getActiveServer(), rMin, rMax,
        rsize, rSpacing, &zone, &isNorth, &scale))
  {
    return false;
  }

  this->Internal->CurrentWriter =
    builder->createFilter("writers", writerName.toStdString().c_str(), pdSource);
  //set things up
  {
    QList<QVariant> values;
    values << rsize[0] << rsize[1];
    pqSMAdaptor::setMultipleElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("RasterSize"), values);
    pqSMAdaptor::setElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("RadiusX"), rSpacing[0]);
    pqSMAdaptor::setElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("RadiusY"), rSpacing[1]);
    pqSMAdaptor::setElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("Zone"), zone);
    pqSMAdaptor::setElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("IsNorth"), isNorth);
    pqSMAdaptor::setElementProperty(
      this->Internal->CurrentWriter->getProxy()->GetProperty("Scale"), scale);
  }
  bool result = this->WriteFile(fileName);
  if (distroy)
    builder->destroy(pdSource);
  return result;
}

bool pqCMBPointsBuilderMainWindowCore::WritePieces(QList<pqPipelineSource*> pieces,
  const QString& writerName, const QString& fileName, bool writeAsSinglePiece)
{
  QList<pqOutputPort*> inputs;
  for (int i = 0; i < pieces.count(); i++)
  {
    if (pieces[i])
    {
      inputs.push_back(pieces[i]->getOutputPort(0));
    }
  }

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if (inputs.count() > 0)
  {
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = inputs;

    if (writerName.compare("LIDARWriter") == 0)
    {
      this->Internal->CurrentWriter = builder->createFilter(
        "writers", writerName.toStdString().c_str(), namedInputs, this->getActiveServer());
      if (writeAsSinglePiece)
      {
        vtkSMPropertyHelper(this->Internal->CurrentWriter->getProxy(), "WriteAsSinglePiece")
          .Set(true);
      }
      return this->WriteFile(fileName);
    }
    else
    {
      pqPipelineSource* pdSource = this->getAppendedSource(inputs);
      this->Internal->CurrentWriter =
        builder->createFilter("writers", writerName.toStdString().c_str(), pdSource);
      bool result = this->WriteFile(fileName);
      builder->destroy(pdSource);
      return result;
    }
  }
  return false;
}

bool pqCMBPointsBuilderMainWindowCore::WriteFile(const QString& fileName)
{
  this->enableAbort(true);
  if (!this->Internal->CurrentWriter)
  {
    qCritical() << "Failed to create LIDAR points writer! ";
    return false;
  }
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqSMAdaptor::setElementProperty(
    this->Internal->CurrentWriter->getProxy()->GetProperty("FileName"),
    fileName.toStdString().c_str());
  pqSMAdaptor::setElementProperty(
    this->Internal->CurrentWriter->getProxy()->GetProperty("AbortExecute"), 0);

  this->Internal->CurrentWriter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->Internal->CurrentWriter->getProxy())->UpdatePipeline();
  int aborted = pqSMAdaptor::getElementProperty(
                  this->Internal->CurrentWriter->getProxy()->GetProperty("AbortExecute"))
                  .toInt();
  if (aborted)
  {
    return false;
  }

  builder->destroy(this->Internal->CurrentWriter);
  this->Internal->CurrentWriter = NULL;

  return true;
}

bool pqCMBPointsBuilderMainWindowCore::generateAndValidateOutFileNames(
  QList<pqCMBLIDARPieceObject*> pieces, const QString& filename, QList<QString>& outFiles)
{
  QFileInfo info(filename);
  QString outputPath(info.absolutePath()), outputFileName;
  outputPath.append("/").append(info.baseName()).append("_piece");
  QFileInfo outFile;
  pqCMBLIDARPieceObject* dataObj = NULL;
  bool yesToAll = false;
  outFiles.clear();
  for (int i = 0; i < pieces.count(); i++)
  {
    dataObj = pieces[i];
    outputFileName =
      outputPath + QString::number(dataObj->getPieceIndex()) + "." + info.completeSuffix();
    outFile.setFile(outputFileName);
    if (outFile.exists() && !yesToAll)
    {
      int answer = QMessageBox::question(this->parentWidget(), "Overwrite Existing File?",
        "The following file already exists. Do you want to overwrite it? \n" +
          QString(outputFileName),
        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No, QMessageBox::No);
      if (answer == QMessageBox::No)
      {
        return false;
      }
      else if (answer == QMessageBox::YesToAll)
      {
        yesToAll = true;
      }
    }
    outFiles.push_back(outputFileName);
  }

  if (pieces.count() != outFiles.count())
  {
    QMessageBox::warning(this->parentWidget(), tr("File Save Error"),
      tr("The file names for multi-output are not valid for all pieces"), QMessageBox::Ok);
    return false;
  }

  return true;
}

void pqCMBPointsBuilderMainWindowCore::onVTKConnectionChanged(pqDataRepresentation* connRep)
{
  this->setDisplayRepresentation(connRep);

  if (this->getAppearanceEditor())
  {
    this->hideDisplayPanelPartialComponents();
  }
}

void pqCMBPointsBuilderMainWindowCore::setupControlPanel(QWidget* parent)
{
  this->Internal->LIDARPanel = new qtCMBLIDARPanelWidget(parent);

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->showSelectedButton, SIGNAL(clicked()),
    this, SLOT(OnPreviewSelected()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->updateButton, SIGNAL(clicked()), this,
    SLOT(onUpdateSelectedPieces()));

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->CheckAllButton, SIGNAL(clicked()),
    this, SLOT(selectAll()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->UncheckAllButton, SIGNAL(clicked()),
    this, SLOT(unselectAll()));

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->zoomSelection, SIGNAL(clicked()),
    this, SLOT(zoomSelection()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->clearSelection, SIGNAL(clicked()),
    this, SLOT(clearSelection()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->pushButtonAddFiles, SIGNAL(clicked()),
    this, SIGNAL(openMoreFiles()));

  // enableClip defaults to not checked and the clip group is disabled
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox, SIGNAL(toggled(bool)),
    this, SLOT(onEnableClip()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->minXClip,
    SIGNAL(valueChanged(double)), this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->maxXClip,
    SIGNAL(valueChanged(double)), this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->minYClip,
    SIGNAL(valueChanged(double)), this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->maxYClip,
    SIGNAL(valueChanged(double)), this, SLOT(onClippingBoxChanged()));

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->applyTargetNumberOfPoints,
    SIGNAL(clicked()), this, SLOT(applyTargetNumberOfPoints()));

  QPalette p = this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->palette();
  p.setColor(QPalette::Highlight, Qt::red);
  this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->setPalette(p);
  this->Internal->LIDARPanel->getGUIPanel()->savePointsLabel->setPalette(p);

  // Set up the data table
  this->Internal->PieceMainTable =
    new pqCMBLIDARPieceTable(this->Internal->LIDARPanel->getGUIPanel()->PieceVisibilityTable);
  QObject::connect(this->Internal->PieceMainTable,
    SIGNAL(currentObjectChanged(pqCMBLIDARPieceObject*)), this,
    SLOT(onPiecesSelectionChanged(pqCMBLIDARPieceObject*)), Qt::QueuedConnection);

  //TODO Setup connections

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->advancedCheckBox,
    SIGNAL(stateChanged(int)), this, SLOT(onAdvancedCheckBox(int)));

  QObject::connect(this->Internal->PieceMainTable,
    SIGNAL(objectOnRatioChanged(pqCMBLIDARPieceObject*, int)), this,
    SLOT(onObjectOnRatioChanged(pqCMBLIDARPieceObject*, int)));

  // if change state in one table, should change in the other
  QObject::connect(this->Internal->PieceMainTable,
    SIGNAL(checkedObjectsChanged(QList<int>, QList<int>)), this,
    SLOT(onObjectsCheckStateChanged(QList<int>, QList<int>)));
  QObject::connect(this->Internal->PieceMainTable,
    SIGNAL(currentObjectChanged(pqCMBLIDARPieceObject*)), this,
    SLOT(onCurrentObjectChanged(pqCMBLIDARPieceObject*)));

  this->Internal->RenderNeeded = false;
  QObject::connect(
    this->Internal->PieceMainTable, SIGNAL(requestRender()), this, SLOT(onRequestRender()));
  QObject::connect(this, SIGNAL(requestingRender()), this, SLOT(onRequestRender()));

  QObject::connect(
    this, SIGNAL(renderRequested()), this, SLOT(onRenderRequested()), Qt::QueuedConnection);
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->tabWidget,
    SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

  //Set up threshold controls
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters,
    SIGNAL(toggled(bool)), this, SLOT(onUseFiltersToggled(bool)));

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->addFilterButton, SIGNAL(clicked()),
    this, SLOT(onAddThreshold()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->removeFilterButton, SIGNAL(clicked()),
    this, SLOT(onRemoveFilter()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->editFilterButton, SIGNAL(clicked()),
    this, SLOT(showFilterDialog()));
  this->Internal->LIDARPanel->getGUIPanel()->editFilterButton->setEnabled(0);

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->newGroupButton, SIGNAL(clicked()),
    this, SLOT(onAddContourGroup()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->addContourButton, SIGNAL(clicked()),
    this, SLOT(onAddContourWidget()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->removeContourButton,
    SIGNAL(clicked()), this, SLOT(onRemoveContour()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->buttonUpdateContours,
    SIGNAL(clicked()), this, SLOT(onUpdateContours()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox,
    SIGNAL(valueChanged(double)), this, SLOT(onElevationMaxChanged(double)));

  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->elevationGroup, SIGNAL(toggled(bool)),
    this, SLOT(onElevationFilter(bool)));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton,
    SIGNAL(clicked()), this, SLOT(onUpdateElevationFilter()));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox,
    SIGNAL(valueChanged(double)), this, SLOT(onElevationMinChanged(double)));
  QObject::connect(this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox,
    SIGNAL(valueChanged(double)), this, SLOT(onElevationMaxChanged(double)));

  // Prep the target number of points line edit
  QIntValidator* validator = new QIntValidator(parent);
  validator->setBottom(1);
  this->Internal->LIDARPanel->getGUIPanel()->targetNumberOfPoints->setValidator(validator);
  this->Internal->LIDARPanel->getGUIPanel()->targetNumberOfPoints->setText(
    QString::number(this->cmbAppOptions()->maxNumberOfCloudPoints()));

  this->setupAppearanceEditor(this->Internal->LIDARPanel->getGUIPanel()->tabDisplay);

  this->ReaderManager =
    new pqCMBLIDARReaderManager(this, pqApplicationCore::instance()->getObjectBuilder());

  this->initControlPanel();

  this->TerrainExtractionManager =
    new pqCMBLIDARTerrainExtractionManager(this, this->Internal->LIDARPanel);

  this->Internal->pqCMBModifierArcTable =
    new pqCMBModifierArcManager(this->Internal->LIDARPanel->getGUIPanel()->EditPoints,
      this->getActiveServer(), this->activeRenderView());
  QObject::connect(
    this->Internal->pqCMBModifierArcTable, SIGNAL(requestRender()), this, SLOT(onRequestRender()));
  QObject::connect(this->Internal->pqCMBModifierArcTable, SIGNAL(addingNewArc()), this,
    SLOT(onModifierArcWidgetStart()));
  QObject::connect(this->Internal->pqCMBModifierArcTable, SIGNAL(modifyingArcDone()), this,
    SLOT(onModifierArcWidgetFinish()));

  QObject::connect(this->Internal->pqCMBModifierArcTable->arcWidgetManager(),
    SIGNAL(editingStarted()), this, SLOT(onModifierArcWidgetStart()));
  QObject::connect(this->Internal->pqCMBModifierArcTable->arcWidgetManager(), SIGNAL(Finish()),
    this, SLOT(onModifierArcWidgetFinish()));
  QObject::connect(this->cmbAppOptions(), SIGNAL(defaultMaxNumberOfPointsChanged()), this,
    SLOT(onDefaultMaxNumberOfTargetPointsChanged()));
}

void pqCMBPointsBuilderMainWindowCore::onDefaultMaxNumberOfTargetPointsChanged()
{
  this->Internal->LIDARPanel->getGUIPanel()->targetNumberOfPoints->setText(
    QString::number(this->cmbAppOptions()->maxNumberOfCloudPoints()));

  this->Internal->LIDARPanel->getGUIPanel()->targetNumberOfPoints->update();
}

QWidget* pqCMBPointsBuilderMainWindowCore::getControlPanel()
{
  return this->Internal->LIDARPanel;
}

int pqCMBPointsBuilderMainWindowCore::calculateMainOnRatio(int totalNumberOfPoints)
{
  double tmpOnRatio =
    this->Internal->LIDARPanel->getGUIPanel()->targetNumberOfPoints->text().toDouble() /
    static_cast<double>(totalNumberOfPoints);

  int onRatio;
  if (tmpOnRatio > 0.8)
  {
    onRatio = 1;
  }
  else
  {
    onRatio = (1.0 / tmpOnRatio) + 0.5;
    if (onRatio < 2)
    {
      onRatio = 2;
    }
  }

  return onRatio;
}

int pqCMBPointsBuilderMainWindowCore::calculateOnRatioForPiece(
  int onRatio, int numberOfPointsInPiece)
{
  if (numberOfPointsInPiece / onRatio < this->Internal->MinimumNumberOfPointsPerPiece)
  {
    onRatio = numberOfPointsInPiece / this->Internal->MinimumNumberOfPointsPerPiece;
  }
  if (onRatio < 1)
  {
    onRatio = 1;
  }
  return onRatio;
}

int pqCMBPointsBuilderMainWindowCore::getMinimumNumberOfPointsPerPiece()
{
  return this->Internal->MinimumNumberOfPointsPerPiece;
}

void pqCMBPointsBuilderMainWindowCore::updateSelection(pqOutputPort* selPort)
{
  if (selPort && this->Internal->PieceMainTable->getCurrentObject() &&
    selPort->getSource() == this->Internal->PieceMainTable->getCurrentObject()->getSource())
  {
    return;
  }

  //  int isShiftKeyDown = this->MainWindowCore->activeRenderView()->
  //    getRenderViewProxy()->GetInteractor()->GetShiftKey();

  //  if (!isShiftKeyDown)
  //    {
  // Clear all existing selections
  this->clearSelection();
  //    }

  if (selPort)
  {
    this->Internal->PieceMainTable->selectObject(selPort->getSource());
  }
}

void pqCMBPointsBuilderMainWindowCore::onUpdateSelectedPieces()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() > 0)
  {
    this->enableAbort(true);

    this->updatePieceRepresentations(visiblePieces);

    this->enableAbort(false);
    this->updateLoadAndUpdateButtons();
  }
}

bool pqCMBPointsBuilderMainWindowCore::isUpdateNeeded()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    if (!this->isObjectUpToDate(visiblePieces[i]))
    {
      return true;
    }
  }

  return false;
}

bool pqCMBPointsBuilderMainWindowCore::isObjectUpToDate(pqCMBLIDARPieceObject* dataObj)
{
  if (!dataObj)
  {
    return true; // well, no object... but
  }

  return dataObj->isObjectUpToDate(
    this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->isChecked(), this->Internal->ClipBBox);
}

void pqCMBPointsBuilderMainWindowCore::updatePieceRepresentations(
  QList<pqCMBLIDARPieceObject*> pieces, bool forceRead /*=false*/)
{
  double* clipbounds = NULL;
  bool bclipdata = false;
  double bounds[6];
  // setup clipping
  if (this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->isChecked())
  {
    double zClip[2] = { VTK_DOUBLE_MIN, VTK_DOUBLE_MAX };

    QList<QVariant> values;
    values << this->Internal->LIDARPanel->getGUIPanel()->minXClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->minYClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value() << zClip[0] << zClip[1];
    this->ReaderManager->setReadBounds(values);
    double tmpbounds[6] = { this->Internal->LIDARPanel->getGUIPanel()->minXClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->minYClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value(), zClip[0], zClip[1] };
    memcpy(bounds, tmpbounds, sizeof(double) * 6);
    clipbounds = bounds;
    bclipdata = true;
  }

  // Separate the whole list into lists grouped by filename
  QMap<QString, QList<pqCMBLIDARPieceObject*> > filePieces;
  foreach (pqCMBLIDARPieceObject* obj, pieces)
  {
    filePieces[obj->getFileName().c_str()].push_back(obj);
  }
  foreach (QString filename, filePieces.uniqueKeys())
  {
    this->ReaderManager->updatePieces(filename.toLatin1().constData(), filePieces[filename],
      forceRead, this->Internal->PieceMainTable, bclipdata, clipbounds);
  }

  this->updatePointTotals();
  this->activeRenderView()->render();
}

void pqCMBPointsBuilderMainWindowCore::setupProgressBar(QStatusBar* statusBar)
{
  this->Internal->ProgressBar = new qtCMBProgressWidget(statusBar);
  this->Internal->ProgressBar->setObjectName("fileReadingProgress");

  statusBar->addPermanentWidget(this->Internal->ProgressBar, 1);

  pqProgressManager* progress_manager = pqApplicationCore::instance()->getProgressManager();

  //QObject::connect(progress_manager, SIGNAL(enableProgress(bool)),
  //                 this->Internal->ProgressBar,     SLOT(enableProgress(bool)));

  //QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
  //                 this->Internal->ProgressBar,     SLOT(setProgress(const QString&, int)));
  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)), this,
    SLOT(updateProgress(const QString&, int)), Qt::QueuedConnection);

  QObject::connect(progress_manager, SIGNAL(enableAbort(bool)), this->Internal->ProgressBar,
    SLOT(enableAbort(bool)));

  QObject::connect(
    this->Internal->ProgressBar, SIGNAL(abortPressed()), progress_manager, SLOT(triggerAbort()));
  QObject::connect(progress_manager, SIGNAL(abort()), this, SLOT(abort()));

  // progress_manager->addNonBlockableObject(this->Internal->LIDARPanel->getGUIPanel());
  progress_manager->addNonBlockableObject(this->Internal->ProgressBar);
  progress_manager->addNonBlockableObject(this->Internal->ProgressBar->getAbortButton());
  progress_manager->setEnableAbort(false);
  this->Internal->ProgressBar->enableProgress(false);

  this->enableButtons(true);

  //this->Internal->ProgressBar->enableAbort(true);
  //this->enableAbort(false);
}

void pqCMBPointsBuilderMainWindowCore::enableAbort(bool abortEnabled)
{
  pqProgressManager* progress_manager = pqApplicationCore::instance()->getProgressManager();
  progress_manager->setEnableAbort(abortEnabled);
  this->Internal->ProgressBar->enableProgress(abortEnabled);
  //  QCoreApplication::processEvents();

  this->enableButtons(!abortEnabled);
}

void pqCMBPointsBuilderMainWindowCore::updateProgress(const QString& text, int progress)
{
  this->Internal->ProgressBar->setProgress(text, progress);
  QCoreApplication::processEvents();
}

void pqCMBPointsBuilderMainWindowCore::OnPreviewSelected()
{
  this->updateFocus(true);
  this->activeRenderView()->resetCamera();
  this->activeRenderView()->render();
}

void pqCMBPointsBuilderMainWindowCore::enableButtons(bool enabled)
{
  this->Internal->LIDARPanel->getGUIPanel()->PieceFrame->setEnabled(enabled);
  this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->setEnabled(enabled);
}

void pqCMBPointsBuilderMainWindowCore::onEnableClip()
{
  bool clipEnabled = this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->isChecked();
  this->Internal->PieceMainTable->setClipEnabled(clipEnabled);
  this->ReaderManager->limitReadToBounds(clipEnabled);

  // update all, even those unselected (invisible)
  for (int i = 0; i < this->Internal->PieceMainTable->getWidget()->rowCount(); i++)
  {
    this->Internal->PieceMainTable->updateWithPieceInfo(i);
  }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons();

  this->Internal->OutlineRepresentation->setVisible(clipEnabled);
  this->activeRenderView()->render();
}

void pqCMBPointsBuilderMainWindowCore::onClippingBoxChanged()
{
  if (this->Internal->OutlineSource)
  {
    double zPad = (this->Internal->DataBounds[5] - this->Internal->DataBounds[4]) * 0.5;
    QList<QVariant> values;
    values << this->Internal->LIDARPanel->getGUIPanel()->minXClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->minYClip->value()
           << this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value()
           << this->Internal->DataBounds[4] - zPad << this->Internal->DataBounds[5] + zPad;
    pqSMAdaptor::setMultipleElementProperty(
      this->Internal->OutlineSource->getProxy()->GetProperty("Bounds"), values);
    this->Internal->OutlineSource->getProxy()->UpdateVTKObjects();
    this->activeRenderView()->render();

    this->Internal->ClipBBox.SetBounds(this->Internal->LIDARPanel->getGUIPanel()->minXClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->maxXClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->minYClip->value(),
      this->Internal->LIDARPanel->getGUIPanel()->maxYClip->value(), VTK_DOUBLE_MIN, VTK_DOUBLE_MAX);
    this->Internal->PieceMainTable->setClipBBox(this->Internal->ClipBBox);

    for (int i = 0; i < this->Internal->PieceMainTable->getWidget()->rowCount(); i++)
    {
      this->Internal->PieceMainTable->updateWithPieceInfo(i);
    }
  }
  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(false); // don't change focus
}

void pqCMBPointsBuilderMainWindowCore::applyTargetNumberOfPoints()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() < 1)
  {
    this->updateFocus();
    return;
  }

  int totalNumberOfPoints = 0;
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    totalNumberOfPoints += visiblePieces[i]->getNumberOfPoints();
  }

  int mainOnRatio = this->calculateMainOnRatio(totalNumberOfPoints);

  int oldRatio;
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    oldRatio = visiblePieces[i]->getDisplayOnRatio();
    visiblePieces[i]->setDisplayOnRatio(
      this->calculateOnRatioForPiece(mainOnRatio, visiblePieces[i]->getNumberOfPoints()));
    if (visiblePieces[i]->getReadOnRatio() != visiblePieces[i]->getDisplayOnRatio() ||
      oldRatio != visiblePieces[i]->getDisplayOnRatio())
    {
      this->Internal->PieceMainTable->updateWithPieceInfo(visiblePieces[i]);
    }
  }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons();
}

void pqCMBPointsBuilderMainWindowCore::zoomSelection()
{
  pqCMBLIDARPieceObject* currentObject = this->Internal->PieceMainTable->getCurrentObject();
  if (currentObject)
  {
    currentObject->zoomOnObject();
    this->activeRenderView()->render();
  }
  this->updateFocus(true);
}

void pqCMBPointsBuilderMainWindowCore::clearSelection(bool updateFocusFlag /*=true*/)
{
  this->Internal->LIDARPanel->getGUIPanel()->tabDisplay->setEnabled(0);
  pqCMBLIDARPieceObject* currentObject = this->Internal->PieceMainTable->getCurrentObject(false);
  if (currentObject)
  {
    currentObject->setSelectionInput(NULL);
    currentObject->setHighlight(0);
    this->activeRenderView()->render();
  }
  this->Internal->PieceMainTable->onClearSelection();
  if (updateFocusFlag)
  {
    this->updateFocus();
  }
  this->Internal->LastSelectedObject = NULL;
}

void pqCMBPointsBuilderMainWindowCore::selectAll()
{
  this->Internal->PieceMainTable->checkAll();
  this->updateFocus(true);
}

void pqCMBPointsBuilderMainWindowCore::unselectAll()
{
  this->Internal->PieceMainTable->uncheckAll();
  this->clearSelection();
}

void pqCMBPointsBuilderMainWindowCore::onObjectsCheckStateChanged(QList<int>, QList<int>)
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();

  // go through all the selected pieces and make sure they are up-to-date in the table
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    this->Internal->PieceMainTable->updateWithPieceInfo(visiblePieces[i]);
  }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(true, true);
}

void pqCMBPointsBuilderMainWindowCore::onAdvancedCheckBox(int advancedState)
{
  this->Internal->PieceMainTable->configureTable(advancedState);
  this->updateFocus(true);
}

void pqCMBPointsBuilderMainWindowCore::onObjectOnRatioChanged(
  pqCMBLIDARPieceObject* dataObj, int /*onRatio*/)
{
  this->Internal->PieceMainTable->updateWithPieceInfo(dataObj);

  // enable the Apply button
  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(true, true);
}

void pqCMBPointsBuilderMainWindowCore::onCurrentObjectChanged(pqCMBLIDARPieceObject* dataObj)
{
  if (this->Internal->LastSelectedObject != dataObj)
  {
    this->Internal->PieceMainTable->getWidget()->setFocus();
    bool renderNeeded = false;
    if (this->Internal->LastSelectedObject)
    {
      this->Internal->LastSelectedObject->setHighlight(0);
      renderNeeded = true;
    }
    if (dataObj)
    {
      dataObj->setSelectionInput(0);
      dataObj->setHighlight(1);
      renderNeeded = true;
    }
    this->updateZoomAndClearState();
    if (renderNeeded)
    {
      emit this->requestingRender();
    }
    this->Internal->LastSelectedObject = dataObj;
  }
}

void pqCMBPointsBuilderMainWindowCore::updateLoadAndUpdateButtons(
  bool shouldUpdateFocus /*=true*/, bool focusOnTableIfRowIsSelected /*=false*/)
{
  bool updateNeeded = this->isUpdateNeeded();

  this->Internal->LIDARPanel->getGUIPanel()->updateButton->setEnabled(updateNeeded);
  //  this->Internal->LIDARPanel->getGUIPanel()->loadButton->setEnabled( !updateNeeded );

  this->Internal->LIDARPanel->getGUIPanel()->updateButton->setBackgroundRole(
    updateNeeded ? QPalette::Highlight : QPalette::Button);
  if (shouldUpdateFocus)
  {
    this->updateFocus(focusOnTableIfRowIsSelected);
  }
}

void pqCMBPointsBuilderMainWindowCore::updateFocus(bool focusOnTableIfRowIsSelected /*=false*/)
{
  if ((focusOnTableIfRowIsSelected && this->Internal->PieceMainTable->getCurrentRow() >= 0) ||
    !this->isUpdateNeeded())
  {
    this->Internal->PieceMainTable->getWidget()->setFocus();
  }
  else
  {
    this->clearSelection(false);
    this->Internal->LIDARPanel->getGUIPanel()->updateButton->setFocus();
  }

  this->updateZoomAndClearState();
}

void pqCMBPointsBuilderMainWindowCore::updateZoomAndClearState()
{
  // enable/disable zoom/clear selection buttons depending on whether
  // a piece is selected
  if (this->Internal->PieceMainTable->getCurrentObject() &&
    this->Internal->PieceMainTable->getCurrentObject()->getVisibility())
  {
    this->Internal->LIDARPanel->getGUIPanel()->clearSelection->setEnabled(true);
    this->Internal->LIDARPanel->getGUIPanel()->zoomSelection->setEnabled(true);
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->zoomSelection->setEnabled(false);
    this->Internal->LIDARPanel->getGUIPanel()->clearSelection->setEnabled(false);
  }
}

void pqCMBPointsBuilderMainWindowCore::updatePointTotals()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  int totalDisplayPoints = 0;
  int totalSavePoints = 0;
  bool allObjectsUpToDate = true;
  bool clipEnabled = this->Internal->LIDARPanel->getGUIPanel()->clipGroupBox->isChecked();
  bool allObjectsClipUpToDate = true;
  bool allSaveOnRatioEqualOne = true;
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    totalDisplayPoints += visiblePieces[i]->getNumberOfDisplayPointsEstimate();
    totalSavePoints += visiblePieces[i]->getNumberOfSavePointsEstimate();

    if (allObjectsUpToDate &&
      !visiblePieces[i]->isObjectUpToDate(clipEnabled, this->Internal->ClipBBox))
    {
      allObjectsUpToDate = false;
    }
    if (allObjectsClipUpToDate &&
      !visiblePieces[i]->isClipUpToDate(clipEnabled, this->Internal->ClipBBox))
    {
      allObjectsClipUpToDate = false;
    }
    if (visiblePieces[i]->getSaveOnRatio() != 1)
    {
      allSaveOnRatioEqualOne = false;
    }
  }

  if (!allObjectsUpToDate)
  {
    this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->setForegroundRole(
      QPalette::Highlight);
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->setForegroundRole(
      QPalette::WindowText);
  }

  // will be pretty good estimate for Save... unless clip not up-to-date
  if (clipEnabled && !allObjectsClipUpToDate)
  {
    this->Internal->LIDARPanel->getGUIPanel()->savePointsLabel->setForegroundRole(
      QPalette::Highlight);
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->savePointsLabel->setForegroundRole(
      QPalette::WindowText);
  }

  this->Internal->LIDARPanel->getGUIPanel()->displayPointsLabel->setText(
    QString::number(totalDisplayPoints));
  this->Internal->LIDARPanel->getGUIPanel()->savePointsLabel->setText(
    QString::number(totalSavePoints));
}

void pqCMBPointsBuilderMainWindowCore::onRequestRender()
{
  this->Internal->RenderNeeded = true;
  emit this->renderRequested();
}

void pqCMBPointsBuilderMainWindowCore::onRenderRequested()
{
  if (this->Internal->RenderNeeded)
  {
    this->Internal->RenderNeeded = false;
    this->activeRenderView()->render();
  }
}

void pqCMBPointsBuilderMainWindowCore::onPiecesSelectionChanged(pqCMBLIDARPieceObject* selObject)
{
  if (selObject && selObject->getRepresentation())
  {
    this->Internal->LIDARPanel->getGUIPanel()->tabDisplay->setEnabled(1);
    this->onVTKConnectionChanged(selObject->getRepresentation());
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->tabDisplay->setEnabled(0);
  }
}

void pqCMBPointsBuilderMainWindowCore::hideDisplayPanelPartialComponents()
{
  QCheckBox* visibleCheck = this->getAppearanceEditorContainer()->findChild<QCheckBox*>("ViewData");
  if (visibleCheck)
  {
    visibleCheck->hide();
  }
  QGroupBox* sliceGroup = this->getAppearanceEditorContainer()->findChild<QGroupBox*>("SliceGroup");
  if (sliceGroup)
  {
    sliceGroup->hide();
  }
  QGroupBox* colorGroup = this->getAppearanceEditorContainer()->findChild<QGroupBox*>("ColorGroup");
  if (colorGroup)
  {
    colorGroup->hide();
  }

  QLabel* repLabel = this->getAppearanceEditorContainer()->findChild<QLabel*>("label_2");
  if (repLabel)
  {
    repLabel->hide();
  }
  pqDisplayRepresentationWidget* repWidget =
    this->getAppearanceEditorContainer()->findChild<pqDisplayRepresentationWidget*>(
      "StyleRepresentation");
  if (repWidget)
  {
    repWidget->hide();
  }

  // Material
  QLabel* mLabel = this->getAppearanceEditorContainer()->findChild<QLabel*>("label_13");
  if (mLabel)
  {
    mLabel->hide();
  }
  QComboBox* mBox = this->getAppearanceEditorContainer()->findChild<QComboBox*>("StyleMaterial");
  if (mBox)
  {
    mBox->hide();
  }

  // volume mapper
  QLabel* vLabel = this->getAppearanceEditorContainer()->findChild<QLabel*>("label_19");
  if (vLabel)
  {
    vLabel->hide();
  }
  QComboBox* vMapperBox =
    this->getAppearanceEditorContainer()->findChild<QComboBox*>("SelectedMapperIndex");
  if (vMapperBox)
  {
    vMapperBox->hide();
  }
}

void pqCMBPointsBuilderMainWindowCore::onUseFiltersToggled(bool checked)
{
  this->updateTransformPanel(!checked);
  if (!checked)
  {
    this->resetAllPiecesWithNoFilters();
    // Reset all filters to be un-used
    this->Internal->ContourTree->clearAllUseContours();

    QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;
    table->clearSelection();
    table->blockSignals(true);
    for (int i = 0; i < table->rowCount(); i++)
    {
      table->item(i, pqCMBLIDARContourTree::UseFilterCol)->setCheckState(Qt::Unchecked);
    }
    table->blockSignals(false);
  }
}

//Called when the filter properties are changed
void pqCMBPointsBuilderMainWindowCore::onFilterChanged()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();

  for (int i = 0; i < visiblePieces.size(); ++i)
  {
    vtkSMSourceProxy* thresholdSource =
      vtkSMSourceProxy::SafeDownCast(visiblePieces[i]->getThresholdSource()->getProxy());
    this->Internal->FilterDlg->UpdateThresholdSource(thresholdSource);
    thresholdSource->Modified();
    //pqSMAdaptor::setElementProperty(
    //  thresholdSource->GetProperty("SetUseFilter"),
    //  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->isChecked());
    //thresholdSource->InvokeCommand("SetUseFilter");
    thresholdSource->UpdateVTKObjects();

    this->updateThresholdTransform(visiblePieces[i]);

    thresholdSource->UpdatePipeline();
  }
  this->activeRenderView()->render();
}

bool pqCMBPointsBuilderMainWindowCore::updateThresholdTransform(pqCMBLIDARPieceObject* dataObj)
{
  if (dataObj->isThresholdTransformationUnchanged())
  {
    return false;
  }

  if (dataObj->isPieceTransformed())
  {
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    dataObj->getTransform(transform);
    vtkSMPropertyHelper(dataObj->getThresholdSource()->getProxy(), "Transform")
      .Set(transform->GetMatrix()->Element[0], 16);

    // don't want to transform the data for output, but do want to transform
    // the data before applying the threshold
    vtkSMPropertyHelper(dataObj->getThresholdSource()->getProxy(), "TransformOutputData")
      .Set(false);
    dataObj->getThresholdSource()->getProxy()->UpdateVTKObjects();

    // set the transform for the clip polygon filter
    vtkSMPropertyHelper(dataObj->getContourSource()->getProxy(), "Transform")
      .Set(transform->GetMatrix()->Element[0], 16);
    dataObj->getContourSource()->getProxy()->UpdateVTKObjects();

    // set the transform for the elevation filter
    vtkSMPropertyHelper(dataObj->getElevationSource()->getProxy(), "Transform")
      .Set(transform->GetMatrix()->Element[0], 16);
    dataObj->getElevationSource()->getProxy()->UpdateVTKObjects();
  }
  else
  {
    dataObj->getThresholdSource()->getProxy()->InvokeCommand("ClearTransform");
    dataObj->getContourSource()->getProxy()->InvokeCommand("ClearTransform");
    dataObj->getElevationSource()->getProxy()->InvokeCommand("ClearTransform");
  }

  // save snapshot of Transform used for thresholding
  dataObj->saveThresholdOrientation();
  dataObj->saveThresholdOrigin();
  dataObj->saveThresholdPosition();
  dataObj->saveThresholdScale();

  if (dataObj->hasActiveFilters())
  {
    return true;
  }
  return false;
}

void pqCMBPointsBuilderMainWindowCore::onTabChanged(int tabIndex)
{
  // switched to main from Display... update info about the piece because
  // we may have changed the transformation
  static int lastTabIndex = -1;
  bool update = true;
  if (tabIndex == 0 && this->Internal->LastSelectedObject)
  {
    this->Internal->PieceMainTable->updateWithPieceInfo(this->Internal->LastSelectedObject);

    if (this->updateThresholdTransform(this->Internal->LastSelectedObject))
    {
      this->Internal->LastSelectedObject->updateRepresentation();
      this->activeRenderView()->render();
    }
    this->updatePointTotals();
    this->updateLoadAndUpdateButtons(true, true);
  }
  else if (tabIndex == 2 && this->TerrainExtractionManager)
  {
    //we are going to the Terrain Extraction
    this->TerrainExtractionManager->onShow();
  }
  else if (tabIndex == 3 && !(lastTabIndex == 2 && this->TerrainExtractionManager))
  {
    // NOTE: The camera manipulator should only happen when an arc is actively being edited,
    // which is now handled by slot(onArcWidgetBusy) linked Busy signal from pqArcWidgetmanager
    //    this->pushCameraInteraction();
    //    this->set2DCameraInteraction();
    //    this->resetViewDirectionNegZ();
    //    this->enableCameraInteractionModeChanges(false);
    //    emit(disableAxisChange());
  }
  if (lastTabIndex == 2 && this->TerrainExtractionManager)
  {
    update = tabIndex == 0;
    this->TerrainExtractionManager->onExitExtraction(false);
  }
  else if (lastTabIndex == 3)
  {
    //    this->enableCameraInteractionModeChanges(true);
    //    this->popCameraInteraction();
    //    emit(enableAxisChange());
  }
  if (update)
  {
    lastTabIndex = tabIndex;
  }
}

void pqCMBPointsBuilderMainWindowCore::onAddThreshold()
{
  //Add the new filter to the table
  QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;
  int row_count = table->rowCount();
  table->setRowCount(row_count + 1);

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QTableWidgetItem* objItem = new QTableWidgetItem();
  //QVariant vdata;
  //vdata.setValue((void*)dataObj);
  //objItem->setData(Qt::UserRole, vdata);
  table->setItem(row_count, pqCMBLIDARContourTree::UseFilterCol, objItem);
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(Qt::Unchecked);

  QTableWidgetItem* toAdd = new QTableWidgetItem();
  //Name the filter New Filter [index]
  QString filterText = "New Filter ";
  static int filterNum = 0;
  filterText += QString::number(filterNum++);
  toAdd->setText(filterText);
  table->setItem(row_count, pqCMBLIDARContourTree::NameCol, toAdd);

  //Add the new filter to the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); ++i)
  {
    allPieces[i]->addThreshold();
  }
}

void pqCMBPointsBuilderMainWindowCore::onRemoveFilter()
{
  QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;
  QList<QTableWidgetItem*> selectedItems = table->selectedItems();
  if (!selectedItems.count())
  {
    QMessageBox::warning(this->parentWidget(), tr("Cannot Remove Filter"),
      tr("You must select a filter first."), QMessageBox::Ok);
    return;
  }

  QTableWidgetItem* sel = selectedItems[0];

  int row = sel->row();
  int row_count = table->rowCount();
  //Unselect the row before you remove it
  table->blockSignals(true);
  //if (row == 0)
  //  {
  //  table->selectRow(1);
  //  }
  //else
  //  {
  //  table->selectRow(row-1);
  //  }
  table->removeRow(row);
  table->setRowCount(row_count - 1);

  //Remove the filter from the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); ++i)
  {
    allPieces[i]->removeThreshold();
  }
  table->blockSignals(false);
  if (row_count > 1)
  {
    table->selectRow(0);
  }
  this->onActiveFilterChanged();
  this->activeRenderView()->render();
}

//Called when the selected filter changes index
void pqCMBPointsBuilderMainWindowCore::onActiveFilterChanged()
{
  QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;
  QList<QTableWidgetItem*> selectedItems = table->selectedItems();
  if (selectedItems.size() < 1)
  {
    return;
  }

  this->Internal->LIDARPanel->getGUIPanel()->editFilterButton->setEnabled(1);
  this->Internal->LIDARPanel->getGUIPanel()->removeFilterButton->setEnabled(1);

  QTableWidgetItem* sel = selectedItems[0];
  int row = sel->row();

  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  vtkSMSourceProxy* thresholdSourceProxy = NULL;
  for (int i = 0; i < visiblePieces.size(); ++i)
  {
    thresholdSourceProxy =
      vtkSMSourceProxy::SafeDownCast(visiblePieces[i]->getThresholdSource()->getProxy());
    pqSMAdaptor::setElementProperty(thresholdSourceProxy->GetProperty("SetActiveFilterIndex"), row);
    thresholdSourceProxy->UpdateProperty("SetActiveFilterIndex", true);
    thresholdSourceProxy->UpdateVTKObjects();
    thresholdSourceProxy->UpdatePipeline();
  }
  //Update the filter dialog with the new selected filter's information
  this->Internal->FilterDlg->UpdateFilterDialog(thresholdSourceProxy);
  /*
  //Update the use filter button with the new selected filter's information
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->setChecked(
    pqSMAdaptor::getElementProperty(
    thresholdSourceProxy->GetProperty("GetUseFilter")).toBool());

  this->Internal->LIDARPanel->getGUIPanel()->groupBoxFilters->blockSignals(false);
*/
}

void pqCMBPointsBuilderMainWindowCore::onContourChanged()
{
  qtArcWidget* const contourWidget = qobject_cast<qtArcWidget*>(QObject::sender());
  if (!contourWidget || !this->Internal->ContourProxyMap.contains(contourWidget))
  {
    return;
  }
  vtkSMProxy* proxy = this->Internal->ContourProxyMap[contourWidget];
  // The contour is not done yet.
  if (!proxy)
  {
    return;
  }
  QTreeWidgetItem* contourItem = this->Internal->ContourTree->onContourChanged(contourWidget);
  if (contourItem)
  {
    if (!this->Internal->NeedUpdateItems.contains(contourItem))
    {
      this->Internal->NeedUpdateItems.append(contourItem);
    }
  }
  this->Internal->LIDARPanel->getGUIPanel()->buttonUpdateContours->setEnabled(
    this->Internal->NeedUpdateItems.count() > 0);
}

void pqCMBPointsBuilderMainWindowCore::onContourFinished()
{
  qtArcWidget* const contourWidget = qobject_cast<qtArcWidget*>(QObject::sender());
  QTreeWidgetItem* contourItem = this->Internal->ContourTree->contourFinished(contourWidget);
  if (contourItem)
  {
    this->addContourFilter(contourWidget);
    if (this->Internal->ContourTree->isContourApplied(contourItem))
    {
      QList<QTreeWidgetItem*> changedItems;
      changedItems.append(contourItem);
      this->onPolygonItemChanged(changedItems, pqCMBLIDARContourTree::UseFilterCol, 1);
    }
  }
  this->enableCameraInteractionModeChanges(true);
}

qtArcWidget* pqCMBPointsBuilderMainWindowCore::createArcWidget(int normal, double position)
{
  qtArcWidget* widget = new qtArcWidget(nullptr);
  vtkSMProxy* pointplacer = widget->pointPlacer();

  widget->setObjectName("CmbSceneContourWidget");

  vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
  vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(position);
  pointplacer->UpdateVTKObjects();

  vtkSMPropertyHelper(widget->widgetProxy(), "AlwaysOnTop").Set(1);

  widget->setView(this->activeRenderView());

  //this block is needed to create the widget in the right order
  //we need to set on the proxy enabled, not the widget
  //than we need to call Initialize
  vtkSMPropertyHelper(widget->widgetProxy(), "AlwaysOnTop").Set(1);
  widget->setEnableInteractivity(1);
  vtkSMPropertyHelper(widget->widgetProxy(), "Enabled").Set(1);
  widget->widgetProxy()->UpdateVTKObjects();
  //widget->getWidgetProxy()->GetWidget()->SetEnabled(true);
  widget->widgetProxy()->InvokeCommand("Initialize");

  return widget;
}

void pqCMBPointsBuilderMainWindowCore::onRemoveContour()
{
  this->Internal->ContourTree->deleteSelected();
  this->enableCameraInteractionModeChanges(true);
}

void pqCMBPointsBuilderMainWindowCore::onPolygonItemRemoved(QList<qtArcWidget*> remContours)
{
  pqWaitCursor cursor;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(0);
  for (int i = 0; i < remContours.size(); i++)
  {
    qtArcWidget* contourWidget = remContours.value(i);
    int gid = this->getContourGroupIdWithContour(contourWidget);
    if (gid < 0)
    {
      continue;
    }
    vtkSMProxy* proxy = this->Internal->ContourProxyMap[contourWidget];
    if (proxy)
    {
      //Remove the filter from the vtk object
      QList<pqCMBLIDARPieceObject*> allPieces =
        this->Internal->PieceMainTable->getAllPieceObjects();

      for (int j = 0; j < allPieces.size(); ++j)
      {
        allPieces[j]->setActiveContourGroup(gid);
        allPieces[j]->removeContour(proxy);
      }
      proxy->Delete();
    }
    this->Internal->ContourProxyMap.remove(contourWidget);
    this->deleteContourWidget(contourWidget);
  }
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(1);
}

pqCMBArcTreeItem* pqCMBPointsBuilderMainWindowCore::onAddContourGroup()
{
  pqCMBArcTreeItem* groupItem = this->Internal->ContourTree->createContourGroupNode();
  return groupItem;
}

void pqCMBPointsBuilderMainWindowCore::onAddContourWidget()
{
  QList<QTreeWidgetItem*> selItems = this->Internal->ContourTree->getSelectedItems();
  if (selItems.size() == 0)
  {
    this->onAddContourGroup();
  }
  int orthoPlane;
  qtArcWidget* contourWidget = this->createPqContourWidget(orthoPlane);
  this->Internal->ContourProxyMap[contourWidget] = NULL;
  QTreeWidgetItem* contourNode = this->addContourNode(contourWidget, orthoPlane);
  if (contourNode && contourNode->parent()->childCount() == 1)
  {
    int groupId = this->getContourGroupIdWithNode(contourNode->parent());
    int invert = contourNode->parent()->checkState(pqCMBLIDARContourTree::InvertCol);
    QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();
    for (int i = 0; i < allPieces.size(); i++)
    {
      allPieces[i]->updateGroupInvert(groupId, invert);
    }
  }

  QObject::connect(contourWidget, SIGNAL(contourLoopClosed()), this,
    SLOT(onContourFinished()) /*, Qt::QueuedConnection*/);
  QObject::connect(contourWidget, SIGNAL(widgetEndInteraction()), this,
    SLOT(onContourChanged()) /*, Qt::QueuedConnection*/);
}

void pqCMBPointsBuilderMainWindowCore::addContourFilter(qtArcWidget* contourWidgt)
{
  if (!contourWidgt)
  {
    return;
  }

  int gid = this->getContourGroupIdWithContour(contourWidgt);
  if (gid < 0)
  {
    return;
  }
  pqWaitCursor cursor;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(0);

  vtkSMProxy* implicitLoop =
    vtkSMProxyManager::GetProxyManager()->NewProxy("implicit_functions", "ImplicitSelectionLoop");
  this->updateContourLoop(implicitLoop, contourWidgt);

  //Add the new filter to the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int piece = 0; piece < allPieces.size(); ++piece)
  {
    allPieces[piece]->setActiveContourGroup(gid);
    allPieces[piece]->addContour(implicitLoop);
    this->updateThresholdTransform(allPieces[piece]);
  }

  this->Internal->ContourProxyMap[contourWidgt] = implicitLoop;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(1);
}

void pqCMBPointsBuilderMainWindowCore::updateContourSource(
  vtkSMSourceProxy* contourSource, vtkSMSourceProxy* currentContours, bool forceUpdate)
{
  currentContours->UpdatePropertyInformation();
  if (!forceUpdate &&
    pqSMAdaptor::getElementProperty(currentContours->GetProperty("NumberOfActivePolygons"))
        .toInt() == 0)
  {
    return;
  }

  unsigned int numGroups = this->Internal->ContourTree->getNumberOfGroups();
  vtkSMProxyProperty* clipFunc =
    vtkSMProxyProperty::SafeDownCast(contourSource->GetProperty("ClipPolygon"));
  int addGroupIndex = 0;
  for (unsigned int i = 0; i < numGroups; i++)
  {
    QTreeWidgetItem* group = this->Internal->ContourTree->getGroup(i);
    // only need to add the group if it is on
    if (group->checkState(pqCMBLIDARContourTree::UseFilterCol) == Qt::Checked)
    {
      bool groupStarted = false;
      int addFilterIndex = 0;
      for (int j = 0; j < group->childCount(); j++)
      {
        if (group->child(j)->checkState(pqCMBLIDARContourTree::UseFilterCol) == Qt::Checked)
        {
          if (!groupStarted)
          {
            pqSMAdaptor::setElementProperty(
              contourSource->GetProperty("ActiveGroupIndex"), addGroupIndex++);
            contourSource->UpdateProperty("ActiveGroupIndex", true);
            groupStarted = true;
          }
          clipFunc->AddProxy(
            this->Internal
              ->ContourProxyMap[this->Internal->ContourTree->getItemObject(group->child(j))]);
          contourSource->UpdateProperty("ClipPolygon", true);
          pqSMAdaptor::setMultipleElementProperty(
            contourSource->GetProperty("ClipApplyPolygon"), 0, addFilterIndex);
          pqSMAdaptor::setMultipleElementProperty(
            contourSource->GetProperty("ClipApplyPolygon"), 1, 1);
          contourSource->UpdateProperty("ClipApplyPolygon", true);
          if (addFilterIndex == 0)
          {
            int invertGroup =
              (group->checkState(pqCMBLIDARContourTree::InvertCol) == Qt::Unchecked) ? 1 : 0;
            pqSMAdaptor::setElementProperty(contourSource->GetProperty("GroupInvert"), invertGroup);
            contourSource->UpdateProperty("GroupInvert", true);
          }
          addFilterIndex++;
        }
      }
    }
  }

  contourSource->UpdatePipeline();
}

void pqCMBPointsBuilderMainWindowCore::updateThresholdSource(
  vtkSMSourceProxy* thresholdSource, vtkSMSourceProxy* currentThresholds, bool forceUpdate)
{
  currentThresholds->UpdatePropertyInformation();
  if (!forceUpdate &&
    pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("NumberOfActiveThresholdSets"))
        .toInt() == 0)
  {
    return;
  }

  int numFilters =
    pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("NumberOfThresholdSets"))
      .toInt();
  int addIdx = 0;
  for (int idx = 0; idx < numFilters; idx++)
  {
    // only need to do filter if it is "active" ("UseFilter" is true)
    pqSMAdaptor::setElementProperty(currentThresholds->GetProperty("SetActiveFilterIndex"), idx);
    currentThresholds->UpdateProperty("SetActiveFilterIndex", true);
    currentThresholds->UpdatePropertyInformation();

    if (!pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseFilter")).toInt())
    {
      continue;
    }

    thresholdSource->InvokeCommand("AddFilter");
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetActiveFilterIndex"), addIdx);
    thresholdSource->UpdateProperty("SetActiveFilterIndex", true);

    // use this filter
    pqSMAdaptor::setMultipleElementProperty(
      thresholdSource->GetProperty("SetUseFilter"), 0, addIdx++);
    pqSMAdaptor::setMultipleElementProperty(thresholdSource->GetProperty("SetUseFilter"), 1, 1);

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMinX"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMinX")).toInt());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMinY"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMinY")).toInt());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMinZ"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMinZ")).toInt());

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMaxX"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMaxX")).toInt());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMaxY"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMaxY")).toInt());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMaxZ"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMaxZ")).toInt());

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMinRGB"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMinRGB")).toInt());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetUseMaxRGB"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetUseMaxRGB")).toInt());

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetInvert"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetInvert")).toInt());

    QList<QVariant> minColorList;
    minColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinR")).toInt());
    minColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinG")).toInt());
    minColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinB")).toInt());
    pqSMAdaptor::setMultipleElementProperty(
      thresholdSource->GetProperty("SetMinRGB"), minColorList);

    QList<QVariant> maxColorList;
    maxColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxR")).toInt());
    maxColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxG")).toInt());
    maxColorList.push_back(
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxB")).toInt());
    pqSMAdaptor::setMultipleElementProperty(
      thresholdSource->GetProperty("SetMaxRGB"), maxColorList);

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMinX"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinX")).toDouble());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMinY"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinY")).toDouble());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMinZ"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMinZ")).toDouble());

    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMaxX"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxX")).toDouble());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMaxY"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxY")).toDouble());
    pqSMAdaptor::setElementProperty(thresholdSource->GetProperty("SetMaxZ"),
      pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("GetMaxZ")).toDouble());

    thresholdSource->UpdateVTKObjects();
  }

  thresholdSource->UpdatePipeline();
}

void pqCMBPointsBuilderMainWindowCore::onThresholdSelectionChanged()
{
  QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->FilterTable;
  QList<QTableWidgetItem*> selectedItems = table->selectedItems();
  int enable = selectedItems.size() < 1 ? 0 : 1;
  this->Internal->LIDARPanel->getGUIPanel()->editFilterButton->setEnabled(enable);
  this->Internal->LIDARPanel->getGUIPanel()->removeFilterButton->setEnabled(enable);
  if (enable)
  {
    this->onActiveFilterChanged();
  }
}

void pqCMBPointsBuilderMainWindowCore::onThresholdItemChanged(QTableWidgetItem* item)
{
  if (item->column() != pqCMBLIDARContourTree::UseFilterCol)
  {
    return;
  }
  int idx = item->row();
  int useFilter = (item->checkState() == Qt::Checked) ? 1 : 0;

  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  for (int i = 0; i < visiblePieces.size(); ++i)
  {
    visiblePieces[i]->updateThresholdUseFilter(idx, useFilter);
  }
  this->activeRenderView()->render();
}

void pqCMBPointsBuilderMainWindowCore::onPolygonTreeItemsDropped(
  QTreeWidgetItem* toGroup, int fromGroup, QList<QTreeWidgetItem*> movedContours)
{
  pqCMBArcTreeItem* parentItem = static_cast<pqCMBArcTreeItem*>(toGroup);
  pqCMBArcTreeItem* fromItem = this->getContourGroupNodeWithId(fromGroup);
  if (this->Internal->ContourGroupMap.contains(parentItem) &&
    this->Internal->ContourGroupMap.contains(fromItem))
  {
    //Add the new filter to the vtk object
    QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();
    for (int i = 0; i < movedContours.size(); ++i)
    {
      qtArcWidget* contourWidget = this->Internal->ContourTree->getItemObject(movedContours[i]);
      if (!this->Internal->ContourGroupMap[parentItem].contains(contourWidget))
      {
        // Add the moved contour to new group
        this->Internal->ContourGroupMap[parentItem].push_back(contourWidget);
      }
      if (this->Internal->ContourGroupMap[fromItem].contains(contourWidget))
      {
        // remove the moved contour from current group
        this->Internal->ContourGroupMap[fromItem].removeAt(
          this->Internal->ContourGroupMap[fromItem].indexOf(contourWidget));
      }

      vtkSMProxy* proxy = this->Internal->ContourProxyMap[contourWidget];
      for (int j = 0; j < allPieces.size(); ++j)
      {
        allPieces[j]->setActiveContourGroup(fromItem->itemId());
        allPieces[j]->removeContour(proxy);
        allPieces[j]->setActiveContourGroup(parentItem->itemId());
        allPieces[j]->addContour(proxy);
      }
    }
    this->activeRenderView()->render();
  }
}

void pqCMBPointsBuilderMainWindowCore::onPolygonItemChanged(
  QList<QTreeWidgetItem*> changedItems, int col, int val)
{
  if (col != pqCMBLIDARContourTree::UseFilterCol &&
    col != pqCMBLIDARContourTree::InvertCol) //  && col != ROICol)
  {
    return;
  }
  pqWaitCursor cursor;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(0);

  for (int i = 0; i < changedItems.size(); i++)
  {
    QTreeWidgetItem* item = changedItems.value(i);
    if (item->parent() == 0)
    {
      //      item->child(0)
      // should be invert... why isn't it?
      if (col != pqCMBLIDARContourTree::InvertCol)
      {
        continue;
      }
      int groupId = this->getContourGroupIdWithNode(item);
      if (groupId < 0)
      {
        continue;
      }
      QList<pqCMBLIDARPieceObject*> allPieces =
        this->Internal->PieceMainTable->getAllPieceObjects();
      for (int j = 0; j < allPieces.size(); j++)
      {
        allPieces[j]->updateGroupInvert(groupId, val);
      }
      continue;
    }

    int groupId = this->getContourGroupIdWithNode(item->parent());
    if (groupId < 0)
    {
      continue;
    }

    int idx = item->parent()->indexOfChild(item);
    QList<pqCMBLIDARPieceObject*> visiblePieces =
      this->Internal->PieceMainTable->getVisiblePieceObjects();
    for (int j = 0; j < visiblePieces.size(); ++j)
    {
      visiblePieces[j]->setActiveContourGroup(groupId);
      if (col == pqCMBLIDARContourTree::UseFilterCol)
      {
        visiblePieces[j]->updatePolygonUseFilter(idx, val);
      }
      //else if(col == pqCMBLIDARContourTree::InvertCol)
      //  {
      //  visiblePieces[j]->updatePolygonInvert(idx, val);
      //  }
      /*
      else if(col == ROICol)
        {
        visiblePieces[j]->updatePolygonROI(idx, val);
        QTableWidget* table = this->Internal->LIDARPanel->getGUIPanel()->tablePolygon;
        QTableWidgetItem* invertItem = table->item(item->row(), pqCMBLIDARContourTree::InvertCol);

        invertItem->setFlags(val ? Qt::ItemIsSelectable :
          Qt::ItemIsSelectable| Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        //if(!val)
        //  {
        //  invertItem->setCheckState(Qt::Unchecked);
        //  }
        }
  */
    }
  }
  this->activeRenderView()->render();
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(1);
}

void pqCMBPointsBuilderMainWindowCore::onPolygonTableSelectionChanged(QTreeWidgetItem* /*selItem*/)
{
  QList<QTreeWidgetItem*> selectedItems = this->Internal->ContourTree->getSelectedItems();
  int enable = selectedItems.size() < 1 ? 0 : 1;
  this->Internal->LIDARPanel->getGUIPanel()->removeContourButton->setEnabled(enable);
  this->Internal->LIDARPanel->getGUIPanel()->addContourButton->setEnabled(enable);

  this->activeRenderView()->render();
}

void pqCMBPointsBuilderMainWindowCore::updateTransformPanel(bool enable)
{
  QGroupBox* transfromBox =
    this->getAppearanceEditorContainer()->findChild<QGroupBox*>("TransformationGroup");
  if (transfromBox)
  {
    transfromBox->setEnabled(enable);
  }
}

void pqCMBPointsBuilderMainWindowCore::resetAllPiecesWithNoFilters()
{
  pqWaitCursor cursor;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(0);

  //Add the new filter to the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); ++i)
  {
    allPieces[i]->resetWithNoContours();
    allPieces[i]->resetWithNoThresholds();
  }
  this->activeRenderView()->render();
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(1);
}

void pqCMBPointsBuilderMainWindowCore::clearContourFilters()
{
  //Add the new filter to the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); ++i)
  {
    allPieces[i]->clearContours();
  }

  for (int i = 0; i < this->Internal->ContourProxyMap.keys().count(); i++)
  {
    qtArcWidget* widget = this->Internal->ContourProxyMap.keys().value(i);
    if (widget)
    {
      this->deleteContourWidget(widget);
      vtkSMProxy* proxy = this->Internal->ContourProxyMap[widget];
      if (proxy)
      {
        proxy->Delete();
      }
    }
  }
  this->Internal->ContourProxyMap.clear();
  if (this->Internal->ContourTree)
  {
    this->Internal->ContourTree->clear(true);
  }
}

pqCMBArcTreeItem* pqCMBPointsBuilderMainWindowCore::getContourGroupNodeWithId(int id)
{
  for (int i = 0; i < this->Internal->ContourGroupMap.keys().count(); i++)
  {
    pqCMBArcTreeItem* item = this->Internal->ContourGroupMap.keys().value(i);
    if (item->itemId() == id)
    {
      return item;
    }
  }
  return NULL;
}

int pqCMBPointsBuilderMainWindowCore::getContourGroupIdWithNode(QTreeWidgetItem* inItem)
{
  for (int i = 0; i < this->Internal->ContourGroupMap.keys().count(); i++)
  {
    pqCMBArcTreeItem* item = this->Internal->ContourGroupMap.keys().value(i);
    if (item == inItem)
    {
      return item->itemId();
    }
  }
  return -1;
}

void pqCMBPointsBuilderMainWindowCore::clearThresholdFilters()
{
  //Add the new filter to the vtk object
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); ++i)
  {
    allPieces[i]->clearThresholds();
  }

  this->Internal->LIDARPanel->getGUIPanel()->FilterTable->blockSignals(true);
  this->Internal->LIDARPanel->getGUIPanel()->FilterTable->clear();
  while (this->Internal->LIDARPanel->getGUIPanel()->FilterTable->rowCount())
  {
    this->Internal->LIDARPanel->getGUIPanel()->FilterTable->removeRow(0);
  }
  this->Internal->LIDARPanel->getGUIPanel()->FilterTable->blockSignals(false);
}

void pqCMBPointsBuilderMainWindowCore::onUpdateContours()
{
  pqWaitCursor cursor;
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(0);

  for (int i = 0; i < this->Internal->NeedUpdateItems.count(); i++)
  {
    QTreeWidgetItem* item = this->Internal->NeedUpdateItems.value(i);
    item->setBackgroundColor(
      pqCMBLIDARContourTree::NameCol, this->Internal->ContourTree->contourFinishedColor);
    qtArcWidget* widget = this->Internal->ContourTree->getItemObject(item);
    vtkSMProxy* implicitLoop = this->Internal->ContourProxyMap[widget];
    if (widget && implicitLoop)
    {
      this->updateContourLoop(implicitLoop, widget);
    }
  }

  // go through all the selected pieces and update
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->Internal->PieceMainTable->getVisiblePieceObjects();
  for (int i = 0; i < visiblePieces.count(); i++)
  {
    visiblePieces[i]->updateRepresentation();
  }
  this->activeRenderView()->render();
  this->Internal->NeedUpdateItems.clear();
  this->Internal->LIDARPanel->getGUIPanel()->tabContour->setEnabled(1);
  this->Internal->LIDARPanel->getGUIPanel()->buttonUpdateContours->setEnabled(0);
}

void pqCMBPointsBuilderMainWindowCore::onElevationFilter(bool useFilter)
{
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();

  for (int i = 0; i < allPieces.size(); i++)
  {
    allPieces[i]->useElevationFilter(useFilter);
  }
  emit this->requestingRender();
}

void pqCMBPointsBuilderMainWindowCore::onUpdateElevationFilter()
{
  this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(false);
  this->updateElevationFilterExtent();
  emit this->requestingRender();
}

void pqCMBPointsBuilderMainWindowCore::onElevationMinChanged(double minZ)
{
  this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->setMinimum(minZ);
  if (this->Internal->ElevationMinZ != minZ ||
    this->Internal->ElevationMaxZ !=
      this->Internal->LIDARPanel->getGUIPanel()->elevationMaxSpinBox->value())
  {
    this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(true);
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(false);
  }
}

void pqCMBPointsBuilderMainWindowCore::onElevationMaxChanged(double maxZ)
{
  this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->setMaximum(maxZ);
  if (this->Internal->ElevationMaxZ != maxZ ||
    this->Internal->ElevationMinZ !=
      this->Internal->LIDARPanel->getGUIPanel()->elevationMinSpinBox->value())
  {
    this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(true);
  }
  else
  {
    this->Internal->LIDARPanel->getGUIPanel()->elevationUpdateButton->setEnabled(false);
  }
}

int pqCMBPointsBuilderMainWindowCore::getContourGroupIdWithContour(qtArcWidget* contourWidget)
{
  for (int i = 0; i < this->Internal->ContourGroupMap.keys().count(); i++)
  {
    pqCMBArcTreeItem* item = this->Internal->ContourGroupMap.keys().value(i);
    if (this->Internal->ContourGroupMap[item].contains(contourWidget))
    {
      return item->itemId();
    }
  }
  return -1;
}

void pqCMBPointsBuilderMainWindowCore::onServerCreationFinished(pqServer* server)
{
  this->Superclass::onServerCreationFinished(server);
  this->renderViewSelectionHelper()->setView(this->activeRenderView());

  QObject::connect(this->renderViewSelectionHelper(), SIGNAL(selectionFinished(int, int, int, int)),
    this->renderViewSelectionHelper(), SLOT(endSelection()));

  //  this->activeRenderView()->getRenderViewProxy()->SetUseImmediateMode(0);
  pqSMAdaptor::setElementProperty(
    this->activeRenderView()->getProxy()->GetProperty("LODThreshold"), 50);
}

void pqCMBPointsBuilderMainWindowCore::onSaveContour()
{
  QString filters = "MultiGroup Contours (*.vtm);;All files (*)";
  pqFileDialog file_dialog(this->getActiveServer(), NULL, tr("Save Contours:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
  {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
    {
      return this->saveContour(files[0].toLatin1().constData());
    }
  }
}

void pqCMBPointsBuilderMainWindowCore::onLoadContour()
{
  QString filters = "MultiGroup Contours (*.vtm);;All files (*)";
  pqFileDialog file_dialog(this->getActiveServer(), NULL, tr("Load Contours:"), QString(), filters);

  //file_dialog.setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("FileOpenDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  if (file_dialog.exec() == QDialog::Accepted)
  {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
    {
      return this->loadContour(files[0].toLatin1().constData());
    }
  }
}

void pqCMBPointsBuilderMainWindowCore::saveContour(const char* filename)
{
  pqObjectBuilder* const builder = pqApplicationCore::instance()->getObjectBuilder();
  vtkClientServerStream stream;
  pqPipelineSource* contourSource =
    builder->createSource("sources", "ContourGroup", this->getActiveServer());
  vtkSMSourceProxy* contourFilter = vtkSMSourceProxy::SafeDownCast(contourSource->getProxy());
  unsigned int numGroups = this->Internal->ContourTree->getNumberOfGroups();
  vtkSMProxyProperty* smpContour =
    vtkSMProxyProperty::SafeDownCast(contourFilter->GetProperty("Contour"));
  for (unsigned int i = 0; i < numGroups; i++)
  {
    smpContour->RemoveAllProxies();
    pqSMAdaptor::setElementProperty(contourFilter->GetProperty("ActiveGroupIndex"), i);
    contourFilter->UpdateProperty("ActiveGroupIndex", true);
    QTreeWidgetItem* group = this->Internal->ContourTree->getGroup(i);

    // Add contours for this group
    for (int j = 0; j < group->childCount(); j++)
    {
      qtArcWidget* widget = this->Internal->ContourTree->getItemObject(group->child(j));
      if (!widget)
      {
        continue;
      }
      widget->closeLoop(true);
      pqPipelineSource* pdSource =
        builder->createSource("sources", "SceneContourSource", this->getActiveServer());
      vtkSMSceneContourSourceProxy* dwProxy =
        vtkSMSceneContourSourceProxy::SafeDownCast(pdSource->getProxy());
      dwProxy->CopyData(widget->widgetProxy());
      pdSource->updatePipeline();
      smpContour->AddProxy(dwProxy);
      contourFilter->UpdateProperty("Contour", true);
      int orthoPlane;
      if (this->getContourProjectionNormal(orthoPlane, widget))
      {
        // Set projection normal for this contour
        pqSMAdaptor::setMultipleElementProperty(
          contourFilter->GetProperty("ProjectionNormal"), 0, j);
        pqSMAdaptor::setMultipleElementProperty(
          contourFilter->GetProperty("ProjectionNormal"), 1, orthoPlane);
        contourFilter->UpdateProperty("ProjectionNormal", true);
      }
      double projPos;
      if (this->getContourProjectionPosition(projPos, widget))
      {
        // Set projection position for this contour
        stream.Reset();
        stream << vtkClientServerStream::Invoke << VTKOBJECT(contourFilter)
               << "SetContourProjectionPosition" << j << projPos << vtkClientServerStream::End;

        contourFilter->GetSession()->ExecuteStream(contourFilter->GetLocation(), stream);
        contourFilter->UpdateProperty("ProjectionPosition", true);
      }
    }
    int invertGroup =
      (group->checkState(pqCMBLIDARContourTree::InvertCol) == Qt::Unchecked) ? 1 : 0;
    pqSMAdaptor::setElementProperty(contourFilter->GetProperty("GroupInvert"), invertGroup);
    contourFilter->UpdateProperty("GroupInvert", true);
  }

  contourFilter->UpdatePipeline();
  pqPipelineSource* writer = builder->createFilter("writers", "XMLContourWriter", contourSource);
  pqSMAdaptor::setElementProperty(writer->getProxy()->GetProperty("FileName"), filename);
  writer->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(writer->getProxy())->UpdatePipeline();
  builder->destroy(writer);
  builder->destroy(contourSource);
}

void pqCMBPointsBuilderMainWindowCore::loadContour(const char* filename)
{
  // Load the file and extract the contour groups
  QStringList files;
  files << filename;
  pqObjectBuilder* const builder = pqApplicationCore::instance()->getObjectBuilder();
  builder->blockSignals(true);
  pqPipelineSource* reader =
    builder->createReader("sources", "XMLMultiBlockDataReader", files, this->getActiveServer());
  builder->blockSignals(false);
  if (!reader)
  {
    QMessageBox::warning(this->parentWidget(), tr("Loading Contour Warning"),
      tr("Failed to create the reader for: ").append(filename), QMessageBox::Ok);
    return;
  }
  vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  sourceProxy->UpdatePipeline();
  vtkSMOutputPort* outputPort = sourceProxy->GetOutputPort(static_cast<unsigned int>(0));
  vtkPVCompositeDataInformation* compositeInformation =
    outputPort->GetDataInformation()->GetCompositeDataInformation();
  int numBlocks = compositeInformation->GetNumberOfChildren();
  this->Internal->ContourTree->TreeWidget->blockSignals(true);
  QList<pqCMBLIDARPieceObject*> allPieces = this->Internal->PieceMainTable->getAllPieceObjects();
  for (int i = 0; i < numBlocks; i++)
  {
    pqPipelineSource* extractBlock = builder->createFilter("filters", "ExtractLeafBlock", reader);
    vtkSMSourceProxy* extractSource = vtkSMSourceProxy::SafeDownCast(extractBlock->getProxy());
    pqSMAdaptor::setElementProperty(extractSource->GetProperty("BlockIndex"), i);
    extractSource->UpdateVTKObjects();
    extractSource->UpdatePipeline();
    extractSource->UpdatePipelineInformation();
    extractSource->UpdatePropertyInformation();

    vtkPVDataSetAttributesInformation* contourInfo =
      extractSource->GetDataInformation(0)->GetAttributeInformation(
        vtkDataObject::FIELD_ASSOCIATION_NONE);
    vtkNew<vtkPVContourGroupInformation> groupinfo;
    extractSource->GatherInformation(groupinfo.GetPointer());
    vtkIntArray* planeArray = groupinfo->GetProjectionPlaneArray();
    vtkDoubleArray* posArray = groupinfo->GetProjectionPositionArray();
    /*
    if(!planeArray || planeArray->GetNumberOfTuples()==0 ||
       !posArray || posArray->GetNumberOfTuples()==0)
      {
      builder->destroy(extractBlock);
      QMessageBox::warning(this->parentWidget(), "Loading Contour",
        "One of the contour groups missing projection normal, position or invert info. Skipped!");
      continue;
      }
*/
    pqPipelineSource* extractContour =
      builder->createFilter("filters", "CmbExtractContours", extractBlock);
    vtkSMSourceProxy* smContours = vtkSMSourceProxy::SafeDownCast(extractContour->getProxy());
    smContours->UpdateVTKObjects();
    smContours->UpdatePipeline();
    smContours->UpdatePropertyInformation();
    int max =
      pqSMAdaptor::getElementProperty(smContours->GetProperty("NumberOfContoursInfo")).toInt();
    /*
    if(max != planeArray->GetNumberOfTuples() ||
       max != posArray->GetNumberOfTuples())
      {
      builder->destroy(extractContour);
      builder->destroy(extractBlock);
      QMessageBox::warning(this->parentWidget(), "Loading Contour",
        "One of the contour groups has mismatched projection normal or position info. Skipped!");
      continue;
      }
*/
    double* pRange = NULL;
    int groupInvert = 0;
    vtkPVArrayInformation* groupInfo = contourInfo->GetArrayInformation("GroupInvert");
    if (groupInfo && groupInfo->GetComponentRange(0))
    {
      pRange = groupInfo->GetComponentRange(0);
      groupInvert = static_cast<int>(pRange[0]);
    }
    // for each block, we create a group of contours
    pqCMBArcTreeItem* groupNode = this->onAddContourGroup();
    groupNode->setBackgroundColor(
      pqCMBLIDARContourTree::NameCol, this->Internal->ContourTree->contourFinishedColor);
    Qt::CheckState checkstate = groupInvert ? Qt::Unchecked : Qt::Checked;
    groupNode->setCheckState(pqCMBLIDARContourTree::InvertCol, checkstate);

    // add each contour to the group
    for (int w = 0; w < max; ++w)
    {
      pqSMAdaptor::setElementProperty(smContours->GetProperty("ContourIndex"), w);
      smContours->UpdateProperty("ContourIndex");
      smContours->UpdatePipeline();
      int projNormal =
        (planeArray && max == planeArray->GetNumberOfTuples()) ? planeArray->GetValue(w) : 2;
      double projPos =
        (posArray && max == posArray->GetNumberOfTuples()) ? posArray->GetValue(w) : 0.0;
      qtArcWidget* contourW = this->createContourWidgetFromSource(projNormal, projPos, smContours);
      if (contourW)
      {
        QTreeWidgetItem* item = this->addContourNode(contourW, projNormal);
        item->setBackgroundColor(
          pqCMBLIDARContourTree::NameCol, this->Internal->ContourTree->contourFinishedColor);
        this->addContourFilter(contourW);
        QObject::connect(contourW, SIGNAL(widgetEndInteraction()), this, SLOT(onContourChanged()));
      }
    }

    for (int a = 0; a < allPieces.size(); a++)
    {
      allPieces[a]->updateGroupInvert(groupNode->itemId(), !groupInvert);
    }

    builder->destroy(extractContour);
    builder->destroy(extractBlock);
  }
  this->Internal->ContourTree->TreeWidget->blockSignals(false);
  this->Internal->ContourTree->TreeWidget->clearSelection();
}

QTreeWidgetItem* pqCMBPointsBuilderMainWindowCore::addContourNode(
  qtArcWidget* contourWidget, int orthoPlane)
{
  QTreeWidgetItem* contourNode = this->Internal->ContourTree->addNewContourNode(contourWidget);
  if (contourNode)
  {
    pqCMBArcTreeItem* groupNode = static_cast<pqCMBArcTreeItem*>(contourNode->parent());
    this->Internal->ContourGroupMap[groupNode].push_back(contourWidget);
    if (orthoPlane == 2) // z axis
    {
      contourNode->setText(pqCMBLIDARContourTree::InvertCol, "XY Plane");
    }
    else if (orthoPlane == 0) // x axis
    {
      contourNode->setText(pqCMBLIDARContourTree::InvertCol, "YZ Plane");
    }
    else if (orthoPlane == 1) // y axis;
    {
      contourNode->setText(pqCMBLIDARContourTree::InvertCol, "XZ Plane");
    }
  }
  return contourNode;
}

void pqCMBPointsBuilderMainWindowCore::onModifierArcWidgetStart()
{
  this->set2DCameraInteraction();
  this->resetViewDirectionNegZ();
}

void pqCMBPointsBuilderMainWindowCore::onModifierArcWidgetFinish()
{
  this->set3DCameraInteraction();
}

void pqCMBPointsBuilderMainWindowCore::startMultiFileRead()
{
  bool hasDataLoaded = this->IsDataLoaded();
  if (!this->Internal->AddingMoreFiles && hasDataLoaded)
  {
    int answer = QMessageBox::question(this->parentWidget(), "Closing Loaded Files?",
      "Do you want to close all current files? \n" +
        QString("Answer No, if you want to keep current files open."),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer == QMessageBox::Yes)
    {
      onCloseData();
    }
  }
  this->Internal->AddingMoreFiles = true;
}

void pqCMBPointsBuilderMainWindowCore::endMultiFileRead()
{
  this->Internal->AddingMoreFiles = false;
}
