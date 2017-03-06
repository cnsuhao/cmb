//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBMeshViewerMainWindow.h"

#include "pqCMBMeshViewerMainWindowCore.h"
#include "cmbMeshViewerConfig.h"
#include "qtCMBMeshViewerPanelWidget.h"
#include "pqCMBDisplayProxyEditor.h"
#include "ui_qtMeshViewerPanel.h"
#include "ui_qtCMBMainWindow.h"

#include <QComboBox>
#include <QDir>
#include <QEventLoop>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QStringList>
#include <QtDebug>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "qtCMBAboutDialog.h"
#include "pqPVApplicationCore.h"
#include "pqDisplayColorWidget.h"
#include "pqProxyWidget.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPluginManager.h"
#include "pqProxyInformationWidget.h"
#include "pqPropertyLinks.h"
#include "pqRenderView.h"
#include "pqFindDataDialog.h"
#include "pqCMBRubberBandHelper.h"
#include "pqScalarBarRepresentation.h"

#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqSpreadSheetViewModel.h"
#include "pqWaitCursor.h"
#include "pqContourWidget.h"

#include "vtkCellData.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkConvertSelection.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkFacesConnectivityFilter.h"
#include "vtkHydroModelPolySource.h"
#include "vtkIdTypeArray.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkMapper.h"
#include "vtkMergeFacesFilter.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionSource.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVariant.h"
#include "qtCMBHelpDialog.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkCommand.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkCMBMeshContourSelector.h"
#include "vtkSelectionNode.h"

#include "pqCMBLoadDataReaction.h"
#include "pqCMBFileExtensions.h"

#define MESH_SOURCE_DATA 1000

class pqCMBMeshViewerMainWindow::vtkInternal
{
public:
  vtkInternal()
    {
    this->NumberOfExtractedSubsets = 0;
    this->MeshViewerPanel = 0;
    this->CurrentContour = 0;
    this->BoxWidget = 0;
    this->PlaneWidget = 0;
    this->VTKBoxConnect = 0;
    this->VTKPlaneConnect = 0;
    }

  ~vtkInternal()
    {
    if(this->MeshViewerPanel)
      {
      delete this->MeshViewerPanel;
      }
    }
  int NumberOfExtractedSubsets;
  qtCMBMeshViewerPanelWidget* MeshViewerPanel;

  pqPropertyLinks GUILinker;
  QLabel *SelectionLabel;
  QComboBox *SelectionMenu;
  QList<QAction*> EditMenuActions;
  QPointer<pqProxyInformationWidget> InformationWidget;
  pqContourWidget* CurrentContour;
  vtkSMNewWidgetRepresentationProxy* BoxWidget;
  vtkSMNewWidgetRepresentationProxy* PlaneWidget;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKBoxConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKPlaneConnect;
};

//----------------------------------------------------------------------------
pqCMBMeshViewerMainWindow::pqCMBMeshViewerMainWindow():
  Internal(new vtkInternal())
{
  this->initializeApplication();
  this->setupMenuActions();
  this->updateEnableState();
  this->MainWindowCore->applyAppSettings();
}

//----------------------------------------------------------------------------
pqCMBMeshViewerMainWindow::~pqCMBMeshViewerMainWindow()
{
  this->clearAllInputsList();
  if(this->Internal->CurrentContour)
    {
    this->getThisCore()->deleteContourWidget(
      this->Internal->CurrentContour);
    }
  if(this->Internal->BoxWidget)
    {
    vtkSMProxyManager* pxm=vtkSMProxyManager::GetProxyManager();
    pxm->UnRegisterProxy("3d_widgets_prototypes",
      pxm->GetProxyName("3d_widgets_prototypes",
      this->Internal->BoxWidget),this->Internal->BoxWidget);
    }
  if(this->Internal->PlaneWidget)
    {
    vtkSMProxyManager* pxm=vtkSMProxyManager::GetProxyManager();
    pxm->UnRegisterProxy("3d_widgets_prototypes",
      pxm->GetProxyName("3d_widgets_prototypes",
      this->Internal->PlaneWidget),this->Internal->PlaneWidget);
    }
  delete this->Internal;

  delete this->MainWindowCore;
  this->MainWindowCore = NULL;
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::initializeApplication()
{
  this->MainWindowCore = new pqCMBMeshViewerMainWindowCore(this);
  this->setWindowIcon( QIcon(QString::fromUtf8(":/cmb/MeshViewerIcon.png")) );
  this->initMainWindowCore();

  this->Internal->MeshViewerPanel = new qtCMBMeshViewerPanelWidget(
    this->getMainDialog()->dockWidgetContents_2->layout()->parentWidget());
  this->Superclass::addControlPanel(this->Internal->MeshViewerPanel);

  QScrollArea* scr = new QScrollArea;
  scr->setWidgetResizable(true);
  scr->setFrameShape(QFrame::NoFrame);
  this->Internal->InformationWidget = new pqProxyInformationWidget();
  scr->setWidget(this->Internal->InformationWidget);
  this->Internal->MeshViewerPanel->getGUIPanel()->tab_Info->layout()->addWidget(scr);

  this->MainWindowCore->setupMousePositionDisplay(this->statusBar());

  pqCMBMeshViewerMainWindowCore* mainWinCore =
    qobject_cast<pqCMBMeshViewerMainWindowCore*>(this->MainWindowCore);

//  mainWinCore->selectionManager()->setActiveView(
//    mainWinCore->activeRenderView());
  QObject::connect(mainWinCore->pvSelectionManager(),
    SIGNAL(selectionChanged(pqOutputPort*)),
    this, SLOT(updateSelection()));

  QObject::connect(
    mainWinCore, SIGNAL(newMeshLoaded()),
    this, SLOT(onMeshLoaded()));
  QObject::connect(
    mainWinCore, SIGNAL(meshModified()),
    this, SLOT(onMeshModified()));
  QObject::connect(
    mainWinCore, SIGNAL(filterPropertiesChanged(bool)),
    this, SLOT(onFilterPropertiesModified(bool)));

  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_Select,
    this->getMainDialog()->action_SelectThrough);
  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_Select,
    this->getMainDialog()->action_SelectPoints);

  QObject::connect(this->getMainDialog()->action_SelectThrough,
    SIGNAL(triggered(bool)),
    mainWinCore, SLOT(onFrustumSelect(bool)));
  QObject::connect(this->getMainDialog()->action_SelectPoints,
    SIGNAL(triggered(bool)),
    mainWinCore, SLOT(onRubberBandSelectPoints(bool)));

  // set some tooltips
  this->getMainDialog()->action_Select->setToolTip("Select Surface Cells");
  this->getMainDialog()->action_SelectPoints->setToolTip("Select Surface Points");

  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionMove_Selected_Contour_Points);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(0);

  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionPush_Selected_Contour_Points);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionExtract_Subset);
  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionSelect_Contour_Through_Cells);
  this->getMainDialog()->toolBar_Selection->insertSeparator(
    this->getMainDialog()->actionSelect_Contour_Through_Cells);
  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->actionSelect_Contour_Through_Cells,
    this->getMainDialog()->actionSelect_Cone_Cells);
  this->getMainDialog()->toolBar_Selection->insertSeparator(
    this->getMainDialog()->actionExtract_Subset);
  QObject::connect(this->getMainDialog()->actionExtract_Subset,
    SIGNAL(triggered()),
    this, SLOT(extractSelection()));
  this->getMainDialog()->actionExtract_Subset->setEnabled(0);

  QObject::connect(this->getMainDialog()->action_Select,
    SIGNAL(triggered(bool)),
    mainWinCore, SLOT(onRubberBandSelectCell(bool)));
  QObject::connect(this->getMainDialog()->action_FindData,
    SIGNAL(triggered()), this, SLOT(showQueryDialog()));
  QObject::connect(this->getMainDialog()->actionSelect_Contour_Surface_Point,
    SIGNAL(triggered()), this, SLOT(onDefineContourWidget()));
  QObject::connect(this->getMainDialog()->actionMove_Selected_Contour_Points,
    SIGNAL(triggered()), this, SLOT(onInvokeBoxWidget()));
  QObject::connect(this->getMainDialog()->actionPush_Selected_Contour_Points,
    SIGNAL(triggered()), this, SLOT(onInvokePlaneWidget()));
  QObject::connect(this->getMainDialog()->actionSelect_Contour_Through_Cells,
    SIGNAL(triggered()), this, SLOT(onDefineContourWidget()));
  QObject::connect(this->getMainDialog()->actionSelect_Contour_Surface_Cell,
    SIGNAL(triggered()), this, SLOT(onDefineContourWidget()));

  QObject::connect(this->getMainDialog()->actionEdit_Conical_Region,
    SIGNAL(triggered()), this, SLOT(onEditConeSelection()));
  QObject::connect(this->getMainDialog()->actionSelect_Cone_Cells,
    SIGNAL(triggered()), this, SLOT(onStartConeSelection()));


  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionSelect_Contour_Surface_Point);
  this->getMainDialog()->toolBar_Selection->insertAction(
    this->getMainDialog()->action_SelectThrough,
    this->getMainDialog()->actionSelect_Contour_Surface_Cell);
  this->getMainDialog()->toolBar_Selection->insertSeparator(
    this->getMainDialog()->action_SelectThrough);

  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->pushButton_Apply,
    SIGNAL(clicked()),
    this, SLOT(applyFilters()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->pushButton_Reset,
    SIGNAL(clicked()),
    mainWinCore, SLOT(resetFilterPanels()));

  mainWinCore->setupAppearanceEditor(
    this->Internal->MeshViewerPanel->getGUIPanel()->appearanceFrame);
  mainWinCore->setupInspectorPanel(
    this->Internal->MeshViewerPanel->getGUIPanel()->tab_Mesh);
  mainWinCore->setupSelectionPanel(
    this->Internal->MeshViewerPanel->getGUIPanel()->frame_Histogram);

  // Setup the selection widget
  this->Internal->SelectionLabel = new QLabel("Selection Style:", this->getMainDialog()->toolBar_Selection);
  this->Internal->SelectionMenu = new QComboBox(this->getMainDialog()->toolBar_Selection);
  this->getMainDialog()->toolBar_Selection->addWidget(this->Internal->SelectionLabel);
  this->getMainDialog()->toolBar_Selection->addWidget(this->Internal->SelectionMenu);
  QStringList list;
  list << "Outline" << "Points" << "Wireframe" << "Surface";
  this->Internal->SelectionMenu->addItems(list);
  this->Internal->SelectionMenu->setCurrentIndex(2); // wireframe
  QObject::connect(
    this->Internal->SelectionMenu, SIGNAL(currentIndexChanged(int)),
    this, SLOT(setSelectionMode()));

  list.clear();
  list << "Quality" << "Region";
  this->Internal->MeshViewerPanel->getGUIPanel()->comboBox_Histogram->addItems(list);
  this->Internal->MeshViewerPanel->getGUIPanel()->comboBox_Histogram->setCurrentIndex(0);
  QObject::connect(
    this->Internal->MeshViewerPanel->getGUIPanel()->comboBox_Histogram,
    SIGNAL(currentIndexChanged(int)),
    mainWinCore, SLOT(setHistogramMode(int)));

  // Setup the spreadsheet view for selection.
  this->Internal->MeshViewerPanel->getGUIPanel()->spreadsheet->setModel(
    this->getThisCore()->spreadSheetViewModel());

  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->pushButton_ChangeMaterial,
    SIGNAL(clicked()),
    this, SLOT(onChangeSelectionMaterialId()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->MaterialIdEntry,
    SIGNAL(textChanged(const QString &)),
    this, SLOT(updateChangeMaterialButton()));

  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_InvertSelection,
    SIGNAL(clicked()),
    this, SLOT(invertSelection()));
  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_InvertSelection->setEnabled(0);

  QTreeWidget* treeWidget = this->inputTreeWidget();

  QObject::connect(treeWidget, SIGNAL(itemClicked (QTreeWidgetItem*, int)),
    this, SLOT(onInputClicked(QTreeWidgetItem*, int)), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemSelectionChanged()),
    this, SLOT(onInputSelectionChanged()), Qt::QueuedConnection);
  QObject::connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
    this, SLOT(onInputNameChanged(QTreeWidgetItem*, int)), Qt::QueuedConnection);

  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->checkBoxShowCell,
    SIGNAL(toggled(bool)),
    this, SLOT(showCellLabel(bool)));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->checkBoxShowPoint,
    SIGNAL(toggled(bool)),
    this, SLOT(showPointLabel(bool)));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_ClearSelection, SIGNAL(clicked()),
    this, SLOT(onClearSelection()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_SelectAll, SIGNAL(clicked()),
    this, SLOT(onSelectAll()));

  QObject::connect(this->Internal->MeshViewerPanel->activeInputAction(),
    SIGNAL(triggered()), this, SLOT(onInputChanged()));
  QObject::connect(this->Internal->MeshViewerPanel->exportSubsetAction(),
    SIGNAL(triggered()), this, SLOT(onExportSubset()));
  QObject::connect(this->Internal->MeshViewerPanel->deleteInputAction(),
    SIGNAL(triggered()), this, SLOT(onRemoveInputSource()));

  //setup shortcuts on the render window:
  // "E" for Extract Selection as Subset
  // "A" for Activate selected subset as active input to filters
  QShortcut *eShortcut =new QShortcut(QKeySequence(tr("E")),
    this->centralWidget());
  eShortcut->setContext(Qt::WidgetShortcut);
    QObject::connect(eShortcut, SIGNAL(activated()),
    this,SLOT(onExtractShortcutActivated()));
  QShortcut *aShortcut =new QShortcut(QKeySequence(tr("A")),
    this->centralWidget());
  eShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(aShortcut, SIGNAL(activated()),
    this,SLOT(onSetActiveShortcutActivated()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->contourAllIn,
    SIGNAL(clicked()),
    this,SLOT(onShapeSelectionOptionClicked()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->contourPartiallyIn,
    SIGNAL(clicked()),
    this,SLOT(onShapeSelectionOptionClicked()));
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->contourIntersect,
   SIGNAL(clicked()),
    this,SLOT(onShapeSelectionOptionClicked()));

  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->radioButtonShowHistogram,
                 SIGNAL(toggled(bool)),
                 this,SLOT(onSwitchHistogramAndSpreadsheet(bool)));
  this->Internal->MeshViewerPanel->getGUIPanel()->scrollAreaSmooth->setVisible(0);
  this->getThisCore()->setSmoothMeshPanelParent(
    this->Internal->MeshViewerPanel->getGUIPanel()->scrollAreaWidgetContents_2);
  QObject::connect(this->Internal->MeshViewerPanel->getGUIPanel()->pushButton_ApplySmoothing,
    SIGNAL(clicked()),
    this, SLOT(onApplySmoothing()));
  this->updateEnableState();

  QString filters = pqCMBFileExtensions::MeshViewer_FileTypes();
  this->loadDataReaction()->setSupportedFileTypes(filters);
  this->loadDataReaction()->addReaderExtensionMap(
    pqCMBFileExtensions::MeshViewer_ReadersMap());
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::showPointLabel(bool showLocal)
{
  vtkSMPropertyHelper(this->getThisCore()->activeRepresentation()->getProxy(),
    "SelectionPointFieldDataArrayName").Set("Mesh Node ID");
  vtkSMPropertyHelper(this->getThisCore()->activeRepresentation()->getProxy(),
    "SelectionPointLabelVisibility").Set(showLocal);
  this->getThisCore()->activeRepresentation()->getProxy()->UpdateVTKObjects();
  this->getThisCore()->activeRenderView()->render();
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::showCellLabel(bool showLocal)
{
  vtkSMPropertyHelper(this->getThisCore()->activeRepresentation()->getProxy(),
    "SelectionCellFieldDataArrayName").Set("Mesh Cell ID");
  vtkSMPropertyHelper(this->getThisCore()->activeRepresentation()->getProxy(),
    "SelectionCellLabelVisibility").Set(showLocal);
  this->getThisCore()->activeRepresentation()->getProxy()->UpdateVTKObjects();
  this->getThisCore()->activeRenderView()->render();
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onFilterPropertiesModified(bool modifed)
{
  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_Apply->setEnabled(modifed);
  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_Reset->setEnabled(modifed);
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::setupMenuActions()
{
   // Add actions to "File" menu.
  //this->getMainDialog()->action_Save_As->setVisible(false);
  this->getMainDialog()->action_Save_Data->setText("Save Full Mesh");
  this->getMainDialog()->action_Save_As->setText("Save Full Mesh As...");
  this->getMainDialog()->menu_File->insertAction(
    this->getMainDialog()->action_Close,
    this->Internal->MeshViewerPanel->exportSubsetAction());

  // Add actions to "Edit" menu.
  this->getMainDialog()->menuEdit->addAction(
    this->getMainDialog()->action_FindData);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
     this->getMainDialog()->actionSelect_Contour_Surface_Point);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
    this->getMainDialog()->actionSelect_Contour_Surface_Cell);

  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
    this->getMainDialog()->actionMove_Selected_Contour_Points);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
    this->getMainDialog()->action_SelectThrough);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
    this->getMainDialog()->actionSelect_Contour_Through_Cells);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->action_FindData,
    this->getMainDialog()->actionExtract_Subset);
  this->getMainDialog()->menuEdit->insertAction(
    this->getMainDialog()->actionExtract_Subset,
    this->getMainDialog()->actionEdit_Conical_Region);

}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::showQueryDialog()
{
  pqFindDataDialog dialog(this);

  QObject::connect(&dialog, SIGNAL(extractSelection()),
    this, SLOT(extractSelection()));

  // We want to make the query the active application wide selection, so we
  // hookup the query action to selection manager so that the application
  // realizes a new selection has been made.
  pqSelectionManager* selManager =
    pqPVApplicationCore::instance()->selectionManager();
  if (selManager)
    {
    QObject::connect(&dialog, SIGNAL(showing(pqOutputPort*)),
      selManager, SLOT(select(pqOutputPort*)));
    }
  dialog.show();
  QEventLoop loop;
  QObject::connect(&dialog, SIGNAL(finished(int)),
    &loop, SLOT(quit()));
  loop.exec();
  //if (dialog.extractSelection())
  //  {
  //  this->extractSelection();
    //pqFiltersMenuReaction::createFilter("filters", "ExtractSelection");
  //  }
  //else if (dialog.extractSelectionOverTime())
  //  {
    //pqFiltersMenuReaction::createFilter("filters", "ExtractSelectionOverTime");
  //  }
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::applyFilters()
{
  this->getThisCore()->acceptFilterPanels();
  this->updateSelection();
  this->Internal->InformationWidget->updateInformation();
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::setSelectionMode()
{
  if(this->getThisCore()->is2DMesh())
    {
    this->getMainDialog()->action_Select->setVisible(1);
//    this->getMainDialog()->action_SelectThrough->setVisible(0);
    }
  else
    {
    this->getMainDialog()->action_SelectThrough->setVisible(1);
//    this->getMainDialog()->action_Select->setVisible(0);
    }
  this->getThisCore()->setSelectionMode(
    this->Internal->SelectionMenu->currentIndex());
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onMeshLoaded()
{
  this->clearAllInputsList();
  if(this->Internal->CurrentContour)
    {
    this->getThisCore()->clearContourSelection(
      this->Internal->CurrentContour);
    this->Internal->CurrentContour = NULL;
    }
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
  this->getMainDialog()->actionSelect_Cone_Cells->setChecked(0);
  this->hideBoxWidget();
  this->hidePlaneWidget();
  if(this->getThisCore()->coneRepresentation())
    {
    this->getThisCore()->stopConeSelection();
    }
  if(this->getThisCore()->isMeshLoaded())
    {
    this->setSelectionMode();
    pqPipelineSource* meshSource = this->getThisCore()->meshSource();
    vtkSMProxy* source = meshSource->getProxy();
    vtkSMSourceProxy::SafeDownCast( source )->UpdatePipeline();
    pqPipelineSource* activeSource = this->getThisCore()->activeSource();
    activeSource->getOutputPort(0)->renderAllViews();

    // Create representation to show the producer.
    vtkSMProxy* repr = this->getThisCore()->spreadSheetRepresentation();
    vtkSMPropertyHelper(repr, "Input").Set(activeSource->getProxy(), 0);
    repr->UpdateVTKObjects();

    vtkSMViewProxy* view = this->getThisCore()->spreadSheetView();
    vtkSMPropertyHelper(view, "Representations").Set(repr);
    view->UpdateVTKObjects();
    view->StillRender();;
    this->getThisCore()->spreadSheetViewModel()->setActiveRepresentationProxy(repr);

    this->Internal->InformationWidget->setOutputPort(
      activeSource->getOutputPort(0));
    QTreeWidget* inputList = this->Internal->MeshViewerPanel->
      getGUIPanel()->treeInputs;
    inputList->setAlternatingRowColors(true);
    inputList->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputList->setSelectionMode(QAbstractItemView::SingleSelection);

    inputList->setColumnCount(4);
    inputList->setHeaderLabels(
      QStringList() << tr("Mesh")<< tr("Active") << tr("Visibility") <<tr("") );
    inputList->header()->setResizeMode(TREE_INPUT_COL, QHeaderView::ResizeToContents);
    inputList->header()->setResizeMode(TREE_VISIBLE_COL, QHeaderView::Fixed);
    inputList->header()->setResizeMode(TREE_ACTIVE_COL, QHeaderView::Fixed);
    inputList->setColumnWidth(TREE_VISIBLE_COL, 60);
    inputList->setColumnWidth(TREE_ACTIVE_COL, 60);
    inputList->header()->setStretchLastSection(true);
    QTreeWidgetItem* parentItem = inputList->invisibleRootItem();
    this->addInputItem(parentItem,
      this->getThisCore()->fullMeshRepresentation(), true);
    this->Internal->MeshViewerPanel->getGUIPanel()->
      comboBox_Histogram->setCurrentIndex(1);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(1);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(1);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(1);
    this->getMainDialog()->actionEdit_Conical_Region->setEnabled(1);
    this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(1);
    this->getMainDialog()->action_SelectThrough->setEnabled(1);
    this->getMainDialog()->action_Select->setEnabled(1);
    this->getMainDialog()->action_SelectPoints->setEnabled(1);

    QFileInfo fInfo(this->getThisCore()->getCurrentMeshFile());
    this->appendDatasetNameToTitle(fInfo.fileName());
    }
  else
    {
    this->getThisCore()->spreadSheetViewModel()->setActiveRepresentationProxy(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(0);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(0);
    this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(0);
    this->getMainDialog()->actionEdit_Conical_Region->setEnabled(0);
    this->getMainDialog()->action_SelectThrough->setEnabled(0);
    this->getMainDialog()->action_Select->setEnabled(0);
    this->getMainDialog()->action_SelectPoints->setEnabled(0);
    this->getMainDialog()->actionExtract_Subset->setEnabled(0);
    this->appendDatasetNameToTitle("");
    }

  this->updateEnableState();
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onMeshModified()
{
  this->updateSelection();
  this->Internal->InformationWidget->updateInformation();
}

//-----------------------------------------------------------------------------
pqCMBMeshViewerMainWindowCore* pqCMBMeshViewerMainWindow::getThisCore()
{
  return qobject_cast<pqCMBMeshViewerMainWindowCore*>(this->MainWindowCore);
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::updateEnableState()
{
  bool data_loaded = this->getThisCore()->isMeshLoaded();
  this->getMainDialog()->faceParametersDock->setEnabled(data_loaded);

  if(data_loaded && this->getThisCore()->activeSource())
    {
    this->Internal->MeshViewerPanel->getGUIPanel()->spreadsheet->setEnabled(1);
    }
  else
    {
    this->Internal->MeshViewerPanel->getGUIPanel()->spreadsheet->setEnabled(0);
    }

  this->updateEnableState(data_loaded);

}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onHelpAbout()
{
  qtCMBAboutDialog* const dialog = new qtCMBAboutDialog(this);
  dialog->setWindowTitle(QApplication::translate("Mesh Viewer AboutDialog",
                                               "About Mesh Viewer",
                                               0, QApplication::UnicodeUTF8));
  dialog->setPixmap(QPixmap(QString(":/cmb/MeshViewerSplashAbout.png")));
  dialog->setVersionText(
    QString("<html><b>Version: <i>%1</i></b></html>").arg(MeshViewer_VERSION_FULL));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onHelpHelp()
{
  this->showHelpPage("qthelp://paraview.org/cmbsuite/MeshViewer_README.html");
}


//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onEnableExternalProcesses(bool vtkNotUsed(state))
{
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::updateSelection()
{
  if(!this->getThisCore()->activeSource() ||
    (this->getThisCore()->activeRepresentation() &&
    !this->getThisCore()->activeRepresentation()->isVisible()))
    {
    QMessageBox::warning(this, "Selection Warning",
      "The active mesh must be visible to select mesh elements!");
    return;
    }

// We need some logic here to handle different type of selections
  bool isShapeSel =
    this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked()  ||
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked() ||
    this->getMainDialog()->actionSelect_Contour_Surface_Point->isChecked() ||
    this->getMainDialog()->actionSelect_Cone_Cells->isChecked();
  this->getThisCore()->updateSelection(isShapeSel);
  vtkSMProxy* repr =
    this->getThisCore()->spreadSheetViewModel()->activeRepresentationProxy();

  // For 3D mesh, the move/push point actions should only be enabled
  // when the selection is NOT select-through type selection.
  bool bEnableMovePtsActions = (!this->getThisCore()->is2DMesh() &&
    (this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked() ||
    this->getMainDialog()->action_SelectThrough->isChecked() ||
    this->getMainDialog()->actionSelect_Cone_Cells->isChecked()
    )) ? false : true;

  bool hasSelection = false;
  if(this->getThisCore()->getActiveSelection(vtkSelectionNode::CELL))
    {
    vtkSMPropertyHelper(repr, "FieldAssociation").Set(1);//Cell
    hasSelection = true;
    }
  else if(this->getThisCore()->getActiveSelection(vtkSelectionNode::POINT))
    {
    vtkSMPropertyHelper(repr, "FieldAssociation").Set(0);//Point
    hasSelection = true;
    }
  else
    {
    if(!isShapeSel)
      {
      if(this->Internal->CurrentContour)
        {
        this->getThisCore()->clearContourSelection(this->Internal->CurrentContour);
        this->Internal->CurrentContour = NULL;
        }
      else if(this->getThisCore()->coneRepresentation())
        {
        this->getThisCore()->stopConeSelection();
        }

      this->getThisCore()->setActiveSelection(NULL);
      }
    bEnableMovePtsActions = false;
    }

  this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(
    bEnableMovePtsActions);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(
    bEnableMovePtsActions);

  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_InvertSelection->setEnabled(hasSelection);

  repr->UpdateVTKObjects();
  this->getThisCore()->spreadSheetViewModel()->forceUpdate();
  vtkSMViewProxy* view = this->getThisCore()->spreadSheetView();
  view->UpdateVTKObjects();
  view->StillRender();

  this->getThisCore()->activeRenderView()->render();
  this->updateChangeMaterialButton();
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::updateChangeMaterialButton()
{
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(
    this->getThisCore()->activeSource()->getProxy());
  // if there are cell selection, enable the change-material-button
  bool enable = smSource && this->getThisCore()->getActiveSelection();
  // enabling the change material button also requires there be text in the
  // line edit widget
  bool benable = enable &&
    (this->Internal->MeshViewerPanel->getGUIPanel()->MaterialIdEntry->text() != "");

  this->getMainDialog()->actionExtract_Subset->setEnabled(enable);
  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_ChangeMaterial->setEnabled(benable);
  if(enable)
    {
    this->Internal->MeshViewerPanel->getGUIPanel()->tabWidget->
      setCurrentIndex(3);// show Selection tab
    }
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onChangeSelectionMaterialId()
{
  QString vtext =  this->Internal->MeshViewerPanel->
    getGUIPanel()->MaterialIdEntry->text();
  int val = vtext.toInt();
  this->getThisCore()->changeSelectionMaterialId(val);
  this->Internal->MeshViewerPanel->getGUIPanel()->
    pushButton_ChangeMaterial->setEnabled(0);
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onInputChanged()
{
  QTreeWidget* inputList = this->inputTreeWidget();
  if(inputList->selectedItems().count()==0)
    {
    return;
    }
  QTreeWidgetItem* current = inputList->selectedItems().value(0);
  pqPipelineSource* selSource = this->getInputSourceFromItem(current);
  if(selSource == this->getThisCore()->currentInputSource())
    {
    return;
    }

  QTreeWidgetItem* previous = this->findItemFromSource(
    this->inputTreeWidget()->invisibleRootItem(),
    this->getThisCore()->currentInputSource());
  if(previous)
    {
    previous->setIcon(TREE_ACTIVE_COL,*(this->Internal->MeshViewerPanel->iconInactive()));
    }

  current->setIcon(TREE_ACTIVE_COL,*(this->Internal->MeshViewerPanel->iconActive()));
  current->setIcon(TREE_VISIBLE_COL,*(this->Internal->MeshViewerPanel->iconVisible()));
  current->setData(TREE_VISIBLE_COL, Qt::UserRole, 1);

  this->getThisCore()->setFiltersSource(selSource);

  this->updateSelection();
  this->onInputSelectionChanged();
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onExportSubset()
{
  QTreeWidget* inputList = this->Internal->MeshViewerPanel->
    getGUIPanel()->treeInputs;
  if(inputList->selectedItems().count()>0)
    {
    QTreeWidgetItem* current = inputList->selectedItems().value(0);
    pqPipelineSource* selSource = this->getInputSourceFromItem(current);
    if(selSource)
      {
      this->getThisCore()->exportMesh(selSource);
      }
    }
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::extractSelection()
{
  this->getMainDialog()->actionExtract_Subset->setEnabled(0);

  pqDataRepresentation* subsetRep =
    this->getThisCore()->extractSelectionAsInput();
  if(subsetRep)
    {
    this->Internal->MeshViewerPanel->getGUIPanel()->
      tabWidget->setCurrentIndex(0);
    QTreeWidgetItem* newparent = this->findItemFromSource(
      this->inputTreeWidget()->invisibleRootItem(),
      this->getThisCore()->currentInputSource());
    this->addInputItem(newparent, subsetRep, true);
    this->Internal->NumberOfExtractedSubsets++;
    }

}
//----------------------------------------------------------------------------
QTreeWidgetItem* pqCMBMeshViewerMainWindow::findItemFromSource(
  QTreeWidgetItem* parentItem, pqPipelineSource* currentS)
{
  if(!currentS)
    {
    return NULL;
    }

  if(this->getInputSourceFromItem(parentItem)== currentS)
    {
    return parentItem;
    }


  for(int i=0; i<parentItem->childCount(); i++)
    {
    QTreeWidgetItem* theItem = this->findItemFromSource(
      parentItem->child(i), currentS);
    if(theItem)
      {
      return theItem;
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::invertSelection()
{
  if(this->getThisCore()->invertCurrentSelection())
    {
    this->updateSelection();
    }
}
//----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::addInputItem(QTreeWidgetItem* parentLocal,
  pqDataRepresentation* extractRep,  bool select)
{
  if(!parentLocal)
    {
    return;
    }
  QTreeWidget* inputList = this->Internal->MeshViewerPanel->
    getGUIPanel()->treeInputs;

  QTreeWidgetItem* item = new QTreeWidgetItem(parentLocal);
  if(extractRep)
    {
    QVariant vdata;
    vdata.setValue(static_cast<void*>(extractRep));
    item->setData(TREE_INPUT_COL, Qt::UserRole, vdata);
    }
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  parentLocal->addChild(item);

  if(parentLocal == inputList->invisibleRootItem())
    {
    QFont advFont;
    advFont.setBold(true);
    item->setFont(TREE_INPUT_COL, advFont);
    item->setText(TREE_INPUT_COL, "Master");
    item->setIcon(TREE_ACTIVE_COL,*(this->Internal->MeshViewerPanel->iconActive()));
    }
  else
    {
    QString itemText =
      "Sel" + QString::number(this->Internal->NumberOfExtractedSubsets+1);
    item->setText(TREE_INPUT_COL, itemText);
    item->setIcon(TREE_ACTIVE_COL,*(this->Internal->MeshViewerPanel->iconInactive()));
    }
  item->setIcon(TREE_VISIBLE_COL, *(this->Internal->MeshViewerPanel->iconVisible()));
  item->setData(TREE_VISIBLE_COL, Qt::UserRole, 1);
  parentLocal->setExpanded(true);
  if(select)
    {
    item->setSelected(true);
    inputList->setCurrentItem(item);
//    this->onInputChanged();
    }
}
//-----------------------------------------------------------------------------
pqDataRepresentation *pqCMBMeshViewerMainWindow::getInputRepresentationFromItem(
  QTreeWidgetItem * item)
{
  return item ? static_cast<pqDataRepresentation*>(item->data(TREE_INPUT_COL,
    Qt::UserRole).value<void *>()) : NULL;
}

//-----------------------------------------------------------------------------
pqPipelineSource *pqCMBMeshViewerMainWindow::getInputSourceFromItem(
  QTreeWidgetItem * item)
{

//  if(item == this->inputTreeWidget()->invisibleRootItem()->child(0))
//    {
//    return this->getThisCore()->meshSource();
//    }
  pqDataRepresentation* inputRep = this->getInputRepresentationFromItem(item);
  return inputRep ? inputRep->getInput() : NULL;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onRemoveInputSource()
{
  QTreeWidget* treeInputs = this->Internal->MeshViewerPanel->
    getGUIPanel()->treeInputs;
  if(treeInputs->selectedItems().count())
    {
    treeInputs->blockSignals(true);
    QTreeWidgetItem* selItem = treeInputs->selectedItems().value(0);

    if(!this->isItemInActiveChain(selItem))
      {
      QTreeWidgetItem* parentLocal = selItem->parent();
      parentLocal->setSelected(true);
      treeInputs->setCurrentItem(parentLocal);
      this->onInputSelectionChanged();

      this->removeSubsets(selItem);
      this->removeInputItem(selItem);
      }
    treeInputs->blockSignals(false);
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onInputSelectionChanged()
{
  QTreeWidget* treeInputs = this->inputTreeWidget();
  // we can't remove current selection and original
  if(treeInputs->selectedItems().count()>0)
    {
    QTreeWidgetItem* current = treeInputs->selectedItems().value(0);
    pqPipelineSource* selSource = this->getInputSourceFromItem(current);
    if(this->isItemInActiveChain(current))
      {
      this->Internal->MeshViewerPanel->deleteInputAction()->setEnabled(0);
      }
    else
      {
      this->Internal->MeshViewerPanel->deleteInputAction()->setEnabled(1);
      }
    pqDataRepresentation* selRep = NULL;
    if(selSource==this->getThisCore()->currentInputSource())
      {
      this->Internal->MeshViewerPanel->activeInputAction()->setEnabled(0);
      selRep = this->getThisCore()->activeRepresentation();
      }
    else
      {
      this->Internal->MeshViewerPanel->activeInputAction()->setEnabled(1);
      selRep = this->getInputRepresentationFromItem(current);
      }

    this->getThisCore()->setDisplayRepresentation(selRep);
    this->getThisCore()->getAppearanceEditor()->setEnabled(1);
    this->Internal->MeshViewerPanel->exportSubsetAction()->setEnabled(1);
    }
  else
    {
    this->Internal->MeshViewerPanel->deleteInputAction()->setEnabled(0);
    this->Internal->MeshViewerPanel->activeInputAction()->setEnabled(0);
    this->Internal->MeshViewerPanel->exportSubsetAction()->setEnabled(0);
    this->getThisCore()->getAppearanceEditor()->setEnabled(0);
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::removeSubsets(QTreeWidgetItem* parentLocal)
{
  foreach(QTreeWidgetItem* child, parentLocal->takeChildren())
    {
    if(child->childCount()>0)
      {
      this->removeSubsets(child);
      }
    this->removeInputItem(child, false);
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onInputClicked(QTreeWidgetItem* item, int col)
{
  // Change visibility
  int visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
  // Change visible icon
  int itemVisible = !visible;
  if(col == TREE_VISIBLE_COL)
    {
    QIcon *visIcon = itemVisible ? this->Internal->MeshViewerPanel->iconVisible() :
      this->Internal->MeshViewerPanel->iconInvisible();
    item->setIcon(TREE_VISIBLE_COL, *visIcon);
    item->setData(TREE_VISIBLE_COL, Qt::UserRole, itemVisible);

    pqDataRepresentation* repr = this->getInputRepresentationFromItem(item);
    if(repr)
      {
      if(repr->getInput() == this->getThisCore()->currentInputSource())
        {
        this->getThisCore()->activeRepresentation()->setVisible(itemVisible);
        }
      else
        {
        repr->setVisible(itemVisible);
        }
      }
    this->getThisCore()->activeRenderView()->render();
    }
}
//-----------------------------------------------------------------------------
QTreeWidget* pqCMBMeshViewerMainWindow::inputTreeWidget()
{
  return this->Internal->MeshViewerPanel->
   getGUIPanel()->treeInputs;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::removeInputItem(
  QTreeWidgetItem* selItem, bool checkRemovable)
{
  if(!selItem)
    {
    return;
    }
  // we can't remove current selection and original
  if(checkRemovable && this->isItemInActiveChain(selItem))
    {
    return;
    }
  pqDataRepresentation* selrep = this->getInputRepresentationFromItem(selItem);
  if(selrep)
    {
    this->getThisCore()->destroyRepresentation(selrep);
    }
  delete selItem;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::clearAllInputsList()
{
  QTreeWidget* treeInputs = this->inputTreeWidget();
  treeInputs->blockSignals(true);
  this->getThisCore()->destroyInputRepresentations();
  treeInputs->clear();
  this->Internal->NumberOfExtractedSubsets=0;
  treeInputs->blockSignals(false);
}

//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindow::isItemInActiveChain(QTreeWidgetItem* item)
{
  if(!item)
    {
    return false;
    }
  QTreeWidgetItem* activeItem = this->findItemFromSource(
    this->inputTreeWidget()->invisibleRootItem(),
    this->getThisCore()->currentInputSource());
  QTreeWidgetItem* meshItem = this->findItemFromSource(
    this->inputTreeWidget()->invisibleRootItem(),
    this->getThisCore()->meshSource());
  if(item == activeItem || item == meshItem)
    {
    return true;
    }

  return this->hasChildItem(item, activeItem);
}
//-----------------------------------------------------------------------------
bool pqCMBMeshViewerMainWindow::hasChildItem(
  QTreeWidgetItem* parentLocal, QTreeWidgetItem* theChild)
{
  for(int i=0; i<parentLocal->childCount(); i++)
    {
    QTreeWidgetItem* child = parentLocal->child(i);
    if(child == theChild)
      {
      return true;
      }
    bool hasC = this->hasChildItem(child, theChild);
    if(hasC)
      {
      return hasC;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onInvokePlaneWidget()
{
  if(!this->getThisCore()->meshSculptingSource()&&
    !this->getThisCore()->getActiveSelection(vtkSelectionNode::CELL) &&
    !this->getThisCore()->getActiveSelection(vtkSelectionNode::POINT))
    {
    // No contour is defined, we need to return
    this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
    this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(0);
    return;
    }
  if(!this->getMainDialog()->actionPush_Selected_Contour_Points->isChecked())
    {
    // clear contour source ??
    this->getThisCore()->clearContourSelection(NULL);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(1);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(1);

    this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(1);
    this->getMainDialog()->action_SelectThrough->setEnabled(1);
    this->getMainDialog()->action_Select->setEnabled(1);
    this->getMainDialog()->action_SelectPoints->setEnabled(1);
    this->getMainDialog()->actionSelect_Cone_Cells->setChecked(0);
    this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(1);
    this->getMainDialog()->actionEdit_Conical_Region->setEnabled(1);

    //Abort using plane widget
    this->hidePlaneWidget();
    return;
    }
  else
    {
    if(!this->getThisCore()->meshSculptingRepresentation())
      {
      this->getThisCore()->createSelectedNodesRepresentation();
      }
    if(!this->Internal->PlaneWidget)
      {
      this->Internal->PlaneWidget = this->getThisCore()->createPlaneWidget();
      }

    // Now remove the contour widget
    this->getThisCore()->deleteContourWidget(
      this->Internal->CurrentContour);
    this->Internal->CurrentContour = NULL;
    }
  this->updatePlaneWidget();
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(0);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->actionSelect_Cone_Cells->setChecked(0);
  this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(0);
  this->getMainDialog()->actionEdit_Conical_Region->setEnabled(0);
  this->getThisCore()->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onInvokeBoxWidget()
{
  if(!this->getThisCore()->meshSculptingSource() &&
      !this->getThisCore()->getActiveSelection(vtkSelectionNode::CELL) &&
      !this->getThisCore()->getActiveSelection(vtkSelectionNode::POINT))
    {
    // No contour is defined, we need to return
    this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
    this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(0);
//    this->getMainDialog()->actionModify_Mesh_Surface_Nodes->setEnabled(0);
    return;
    }
  if(!this->getMainDialog()->actionMove_Selected_Contour_Points->isChecked())
    {
    // clear contour source ??
    this->getThisCore()->clearContourSelection(NULL);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(1);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(1);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(1);
    this->getMainDialog()->action_SelectThrough->setEnabled(1);
    this->getMainDialog()->action_Select->setEnabled(1);
    this->getMainDialog()->action_SelectPoints->setEnabled(1);
    this->getMainDialog()->actionSelect_Cone_Cells->setChecked(0);
    this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(1);
    this->getMainDialog()->actionEdit_Conical_Region->setEnabled(1);

    //Abort using Box widget
    this->hideBoxWidget();
    return;
    }
  else
    {
    if(!this->getThisCore()->meshSculptingRepresentation())
      {
      this->getThisCore()->createSelectedNodesRepresentation();
      }
    if(!this->Internal->BoxWidget)
      {
      this->Internal->BoxWidget = this->getThisCore()->createBoxWidget();
      }
    if(!this->Internal->BoxWidget)
      {
      return;
      }
    // Now remove the contour widget
    this->getThisCore()->deleteContourWidget(
      this->Internal->CurrentContour);
    this->Internal->CurrentContour = NULL;
    }

  this->updateBoxWidget();
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->actionSelect_Cone_Cells->setChecked(0);
  this->getMainDialog()->actionSelect_Cone_Cells->setEnabled(0);
  this->getMainDialog()->actionEdit_Conical_Region->setEnabled(0);

  this->getThisCore()->activeRenderView()->render();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onDefineContourWidget()
{
  if(this->Internal->CurrentContour)
    {
    this->getThisCore()->clearContourSelection(
      this->Internal->CurrentContour);
    this->Internal->CurrentContour = NULL;
    }

  if(!this->getMainDialog()->actionSelect_Contour_Surface_Point->isChecked()
    && !this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked()
    && !this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked())
    {
  // Abort creating contours.
    this->getMainDialog()->action_SelectThrough->setEnabled(1);
    this->getMainDialog()->action_Select->setEnabled(1);
    this->getMainDialog()->action_SelectPoints->setEnabled(1);
    this->getThisCore()->setCameraManipulationEnabled(true);
    return;
    }
  else
    {
    this->getThisCore()->clearSelection();
    this->updateSelection();
    }

  QAction* const senderAction = qobject_cast<QAction*>(
    QObject::sender());
  if(!senderAction)
    {
    return;
    }
  if(senderAction == this->getMainDialog()->actionSelect_Contour_Surface_Point &&
    this->getMainDialog()->actionSelect_Contour_Surface_Point->isChecked())
    {
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
    }
  else if(senderAction == this->getMainDialog()->actionSelect_Contour_Surface_Cell &&
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked())
    {
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
    }
  else if(senderAction == this->getMainDialog()->actionSelect_Contour_Through_Cells &&
    this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked())
    {
    this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
    this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
    }
  this->Internal->CurrentContour = this->getThisCore()->defineContourWidget();

  QObject::connect( this->Internal->CurrentContour, SIGNAL(contourLoopClosed()),
    this, SLOT(onContourFinished()));
  QObject::connect( this->Internal->CurrentContour, SIGNAL(contourDone()),
    this, SLOT(onContourFinished()));
  QObject::connect( this->Internal->CurrentContour, SIGNAL(widgetEndInteraction()),
    this, SLOT(onContourChanged()));
  this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(0);
  this->getMainDialog()->action_SelectThrough->setEnabled(0);
  this->getMainDialog()->action_Select->setEnabled(0);
  this->getMainDialog()->action_SelectPoints->setEnabled(0);
  this->getMainDialog()->actionExtract_Subset->setEnabled(0);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onContourFinished()
{
  if(this->getMainDialog()->actionSelect_Contour_Surface_Point->isChecked() ||
     this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked())
    {
    this->getThisCore()->contourSelectSurface(
      this->Internal->CurrentContour,
      this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked(),
      this->getShapeSelectionOption());
    this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
    this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(1);
    this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
    this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(1);
    }
  else if(this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked())
    {
    int selectContourType = this->getShapeSelectionOption();
    this->getThisCore()->contourSelectThrough(
      this->Internal->CurrentContour, selectContourType);
    }
  this->updateSelection();
  this->getThisCore()->setCameraManipulationEnabled(true);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onContourChanged()
{
  if(this->getMainDialog()->actionSelect_Contour_Surface_Point->isChecked() ||
     this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked())
    {
    this->getThisCore()->contourSelectSurface(
      this->Internal->CurrentContour,
      this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked(),
      this->getShapeSelectionOption());
    }
  else if(this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked())
    {
    this->getThisCore()->contourSelectThrough(
      this->Internal->CurrentContour, this->getShapeSelectionOption());
    }
  this->updateSelection();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::updateBoxWidget()
{
  if(this->Internal->BoxWidget)
    {
    if(!this->Internal->VTKBoxConnect)
      {
      this->Internal->VTKBoxConnect =
        vtkSmartPointer<vtkEventQtSlotConnect>::New();
      }
    this->getThisCore()->linkContourBoxWidget(this->Internal->BoxWidget);
    this->Internal->VTKBoxConnect->Disconnect();
    //this->Internal->VTKBoxConnect->Connect(this->Internal->BoxWidget,
    //  vtkCommand::InteractionEvent,
    //  this, SLOT(onUpdateBoxInteraction()));
    this->Internal->VTKBoxConnect->Connect(this->Internal->BoxWidget,
      vtkCommand::EndInteractionEvent,
      this, SLOT(onUpdateBoxInteraction()));
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::updatePlaneWidget()
{
  if(this->Internal->PlaneWidget)
    {
    if(!this->Internal->VTKPlaneConnect )
      {
      this->Internal->VTKPlaneConnect =
        vtkSmartPointer<vtkEventQtSlotConnect>::New();
      }
    this->getThisCore()->linkContourPlaneWidget(this->Internal->PlaneWidget);
    this->Internal->VTKPlaneConnect->Disconnect();
    this->Internal->VTKPlaneConnect->Connect(this->Internal->PlaneWidget,
      vtkCommand::InteractionEvent,
      this, SLOT(onUpdatePlaneInteraction()));
    this->Internal->VTKPlaneConnect->Connect(this->Internal->PlaneWidget,
      vtkCommand::EndInteractionEvent,
      this, SLOT(onEndPlaneInteraction()));
  }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onUpdateBoxInteraction()
{
  //  this->getMainDialog()->actionModify_Mesh_Surface_Nodes->setEnabled(1);
  this->getThisCore()->moveMeshScultpingPoints();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onEndPlaneInteraction()
{
  // First make sure the plane and contour is synchronized.
  this->onUpdatePlaneInteraction();
  this->getThisCore()->moveMeshScultpingPoints();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onUpdatePlaneInteraction()
{
 this->getThisCore()->updatePlaneInteraction(
    this->Internal->PlaneWidget);
}

//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::hidePlaneWidget()
{
  if(this->Internal->PlaneWidget)
    {
    pqSMAdaptor::setElementProperty(this->Internal->PlaneWidget->
      GetProperty("Visibility"), false);
    pqSMAdaptor::setElementProperty(this->Internal->PlaneWidget->
      GetProperty("Enabled"), false);
    this->Internal->PlaneWidget->UpdateVTKObjects();
    }
  this->getMainDialog()->actionPush_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionPush_Selected_Contour_Points->setEnabled(0);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::hideBoxWidget()
{
  if(this->Internal->BoxWidget)
    {
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
      GetProperty("Visibility"), false);
    pqSMAdaptor::setElementProperty(this->Internal->BoxWidget->
      GetProperty("Enabled"), false);
    this->Internal->BoxWidget->UpdateVTKObjects();
    }
  this->getMainDialog()->actionMove_Selected_Contour_Points->setChecked(0);
  this->getMainDialog()->actionMove_Selected_Contour_Points->setEnabled(0);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onClearSelection()
{
  this->getThisCore()->clearSelection();
  // clean up contour selection too
  if(this->Internal->CurrentContour)
    {
    this->getThisCore()->clearContourSelection(
      this->Internal->CurrentContour);
    this->Internal->CurrentContour = NULL;
    }
  this->updateSelection();
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setChecked(0);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setChecked(0);
  this->getMainDialog()->action_SelectThrough->setEnabled(1);
  this->getMainDialog()->action_Select->setEnabled(1);
  this->getMainDialog()->action_SelectPoints->setEnabled(1);
  this->getThisCore()->setCameraManipulationEnabled(true);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onSelectAll()
{
  this->getThisCore()->selectAll();
  this->updateSelection();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onExtractShortcutActivated()
{
  this->extractSelection();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onSetActiveShortcutActivated()
{
  this->onInputChanged();
}
//-----------------------------------------------------------------------------
int pqCMBMeshViewerMainWindow::getShapeSelectionOption()
{
  if(this->Internal->MeshViewerPanel->getGUIPanel()->contourPartiallyIn->isChecked())
    {
    return vtkCMBMeshContourSelector::PARTIAL_OR_ALL_IN;
    }
  else if(this->Internal->MeshViewerPanel->getGUIPanel()->contourIntersect->isChecked())
    {
    return vtkCMBMeshContourSelector::INTERSECT_ONLY;
    }
  return vtkCMBMeshContourSelector::ALL_IN;
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onShapeSelectionOptionClicked()
{
  int selOption = this->getShapeSelectionOption();
  if(this->getThisCore()->shapeSelectionOption() != selOption &&
    (this->getMainDialog()->actionSelect_Cone_Cells->isChecked() ||
     (this->Internal->CurrentContour &&
     (this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked()
    || this->getMainDialog()->actionSelect_Contour_Surface_Cell->isChecked()))))
    {
    this->getThisCore()->setShapeSelectionOption(
      this->Internal->CurrentContour,
      this->getMainDialog()->actionSelect_Contour_Through_Cells->isChecked(),
      selOption);
    this->updateSelection();
    }
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onApplySmoothing()
{
  this->getThisCore()->applySmoothing();
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onSwitchHistogramAndSpreadsheet(bool showHist)
{
  this->Internal->MeshViewerPanel->getGUIPanel()->stackedWidget->
    setCurrentIndex(showHist ? 0 : 1);
}
//-----------------------------------------------------------------------------
void pqCMBMeshViewerMainWindow::onStartConeSelection(bool showDialog)
{
  // showDialog==true => from "Edit" menu
  // or this is from the selection tool bar
  bool coneStop = false;
  if(showDialog ||
    this->getMainDialog()->actionSelect_Cone_Cells->isChecked())
    {
    if(!this->getThisCore()->startConeSelection(showDialog) &&
      !this->getThisCore()->coneRepresentation())
      {
      coneStop = true;
      }
    }
  else // Cancel cone selection
    {
    this->getThisCore()->stopConeSelection();
    coneStop = true;
    }
  this->getMainDialog()->actionSelect_Cone_Cells->setChecked(!coneStop);
  this->getMainDialog()->actionSelect_Contour_Surface_Point->setEnabled(coneStop);
  this->getMainDialog()->actionSelect_Contour_Surface_Cell->setEnabled(coneStop);
  this->getMainDialog()->actionSelect_Contour_Through_Cells->setEnabled(coneStop);
  this->getMainDialog()->action_SelectThrough->setEnabled(coneStop);
  this->getMainDialog()->action_Select->setEnabled(coneStop);
  this->getMainDialog()->action_SelectPoints->setEnabled(coneStop);
}
