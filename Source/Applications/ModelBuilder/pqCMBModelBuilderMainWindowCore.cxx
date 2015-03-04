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

#include "pqScalarBarWidget.h"
#include "pqCMBEnumPropertyWidget.h"
#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBLegacyMesherDialog.h"
#include "qtRemusVolumeMesherSubmitter.h"
#include "qtRemusVolumeMesherSelector.h"

#include "pqCMBRubberBandHelper.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <QDir>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QShortcut>
#include <QFileInfo>
#include <QTimer>
#include <QTreeWidget>
#include <QMap>

#include "pqActionGroupInterface.h"

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
#include "pqRenderView.h"
#include "pqRecentlyUsedResourcesList.h"

#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
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

#include <pqFileDialog.h>
#include "pqCMBProcessWidget.h"
#include "qtCMBProgressWidget.h"
#include "pqCMBPreviewDialog.h"

#include <pqSetData.h>
#include <pqSetName.h>
#include <pqUndoStack.h>
#include <pqWriterDialog.h>
#include <pqWaitCursor.h>

#include <QVTKWidget.h>
#include <vtkClientServerStream.h>
#include "vtkCMBWriter.h"
#include "vtkDataObject.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkGeometryRepresentation.h"
#include "vtkHydroModelCreator.h"
#include "vtkImageData.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include <vtkPVDisplayInformation.h>
#include <vtkPVOptions.h>
#include <vtkPVXMLElement.h>
#include <vtkPVXMLParser.h>
#include <vtkSMDoubleRangeDomain.h>
#include <vtkSMDoubleVectorProperty.h>
#include "vtkSMIdTypeVectorProperty.h"
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include <vtkSMProxyIterator.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMProxyManager.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSession.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMStringVectorProperty.h>
#include "vtkSMProxyProperty.h"

#include <vtkToolkits.h>
#include <algorithm>

#include "assert.h"
#include <vtksys/Process.h>
#include "vtkCollection.h"
#include "vtkHydroModelPolySource.h"
#include "vtkHydroModelMultiBlockSource.h"
#include "vtkOmicronMeshInputFilter.h"
#include "vtkOmicronMeshInputWriter.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMProxyLink.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkGenericDataObjectWriter.h"
#include "vtkStringArray.h"
#include "vtkDataSetAlgorithm.h"
#include "vtkExtractLeafBlock.h"
#include "vtkPVSelectionInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSMSelectionHelper.h"

///////////////////////////////////////////////////////////////////////////
#include "SimBuilder/SimBuilderCore.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneReader.h"
#include "vtkPVSceneGenFileInformation.h"
#include "qtCMBSceneObjectFilterDialog.h"
#include "SimBuilder/pqSMTKUIManager.h"
#include "pqCMBModelBuilderOptions.h"
#include "qtCMBApplicationOptionsDialog.h"
#include "pqCMBImportShapefile.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/model/Operator.h"
#include "smtk/extension/qt/qtModelView.h"

#include "remus/proto/Job.h"
#include <QLayout>
#include <QPushButton>
#include "pqCMBLoadDataReaction.h"
#include "pqCMBFileExtensions.h"
#include "pqSMTKModelPanel.h"
#include <QMainWindow>
#include "pqActiveObjects.h"
#include "pqCMBModelManager.h"
#include "vtkSMModelManagerProxy.h"
#include "pqModelBuilderViewContextMenuBehavior.h"
#include "vtkPVSMTKModelInformation.h"
#include "pqMultiBlockInspectorPanel.h"

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
      this->Is3DMeshCreated = false;
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
    QPointer<QAction> ColorEdgeWidget;
    QPointer<QAction> ColorEdgeLabel;
    QPointer<QAction> ColorEdgeDomainWidget;
    QPointer<QComboBox> ColorFaceCombo;
    QPointer<QComboBox> ColorEdgeCombo;
    QPointer<QComboBox> ColorEdgeDomainCombo;

    int RubberSelectionMode;

    QString MesherOutputFileName;

    QString OmicronMeshInputFileName;
    QString DefaultSurfaceMesher;

    QString OmicronInputFileName;

    ///////////////////////////////////////////////////////////////////////////
    // The Model related variables
    pqCMBRubberBandHelper cmbRenderViewSelectionHelper;

    ///////////////////////////////////////////////////////////////////////////
    // The SimBuilder variables

    SimBuilderCore* SimBuilder;
    QPointer<pqCMBSceneTree> SceneGeoTree;
    QString CurrentSceneFileName;
    bool Is3DMeshCreated;

    QPointer<pqCMBModelBuilderOptions> AppOptions;

    // For coloring faces by Attributes
    QPointer<QAction> AttFaceColorAction;
    QPointer<QAction> AttFaceCategoryAction;
    QPointer<QComboBox> AttFaceColorCombo;
    QPointer<QComboBox> AttFaceCategoryCombo;

    // For coloring edges by Attributes
    QPointer<QAction> AttEdgeColorAction;
    QPointer<QAction> AttEdgeCategoryAction;
    QPointer<QComboBox> AttEdgeColorCombo;
    QPointer<QComboBox> AttEdgeCategoryCombo;

    // For coloring edge faces by Attributes
    QPointer<QAction> AttColorEdgeDomainAction;
    QPointer<QAction> AttCategoryEdgeDomainAction;
    QPointer<QComboBox> AttColorEdgeDomainCombo;
    QPointer<QComboBox> AttCategoryEdgeDomainCombo;

    // Legends for coloring faces or edges by Attributes
    QPointer<QAction> AttEdgeColorLegendAction;
    QPointer<pqScalarBarWidget> AttEdgeScalarBarWidget;
    QPointer<QAction> AttFaceColorLegendAction;
    QPointer<pqScalarBarWidget> AttFaceScalarBarWidget;

    QPointer<pqSMTKModelPanel> ModelDock;
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

  // default surface mesher
  QDir dir(QApplication::applicationDirPath());
  dir.makeAbsolute();
  // unix?
  this->Internal->DefaultSurfaceMesher = dir.path() + "/model";
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (!fModel.fileExists(this->Internal->DefaultSurfaceMesher, fullpath))
    {
    // Windows
    this->Internal->DefaultSurfaceMesher += ".exe";
    if (!fModel.fileExists(this->Internal->DefaultSurfaceMesher, fullpath))
      {
      // Mac install
      this->Internal->DefaultSurfaceMesher = dir.path() + "/../bin/model";
      if (!fModel.fileExists(this->Internal->DefaultSurfaceMesher, fullpath))
        {
        // Mac build
        this->Internal->DefaultSurfaceMesher = dir.path() + "/../../../model";
        if (!fModel.fileExists(this->Internal->DefaultSurfaceMesher, fullpath))
          {
          this->Internal->DefaultSurfaceMesher.clear();
          }
        }
      }
    }
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
void pqCMBModelBuilderMainWindowCore::setupVariableToolbar(QToolBar* toolbar)
{
  toolbar->clear();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::updateVariableToolbar(QToolBar* /*toolbar*/)
{
  this->Internal->ColorFaceWidget->setVisible(0);
  this->Internal->ColorEdgeDomainWidget->setVisible(0);
  bool hasEdges = false; //this->getCMBModel()->has2DEdges();
  this->Internal->ColorEdgeWidget->setVisible(hasEdges);
  this->Internal->ColorEdgeLabel->setVisible(hasEdges);
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
void pqCMBModelBuilderMainWindowCore::updateColorByAttributeMode(int mode, bool isEdge)
{
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
  bool attVisible = false;
  if(isEdge)
    {
    this->Internal->AttEdgeColorAction->setVisible(attVisible);
    this->Internal->AttEdgeCategoryAction->setVisible(attVisible);
    this->Internal->AttEdgeColorLegendAction->setVisible(attVisible);
    }
  else // face
    {
    this->Internal->AttFaceColorAction->setVisible(attVisible);
    this->Internal->AttFaceCategoryAction->setVisible(attVisible);
    this->Internal->AttFaceColorLegendAction->setVisible(attVisible);
    }

  if(attVisible)
    {
    this->onNumOfAttriubtesChanged();
    }
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::updateColorDomainByAttributeMode(
  int mode)
{
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
  bool attVisible = false;
  this->Internal->AttColorEdgeDomainAction->setVisible(attVisible);
  this->Internal->AttCategoryEdgeDomainAction->setVisible(attVisible);
  this->Internal->AttFaceColorLegendAction->setVisible(attVisible);
  if(attVisible)
    {
    this->onNumOfAttriubtesChanged();
    }
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setColorByFaceMode(int mode)
{
  this->updateColorByAttributeMode(mode);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setColorByEdgeMode(int mode)
{
  this->updateColorByAttributeMode(mode, true);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setColorByEdgeDomainMode(int mode)
{
  this->updateColorDomainByAttributeMode(mode);
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
void pqCMBModelBuilderMainWindowCore::loadMesherOutput(remus::proto::JobResult)
{
  //for now we totally ignore the values coming from the JobResult

  // Normal exit... preview the output, and if happy with it, give the user the
  // option to load it
  if (this->previewWindow( this->Internal->MesherOutputFileName ))
    {
    if (QMessageBox::question(this->parentWidget(), "Load mesher output?",
          "Mesher completed normally.  Load output?",
          QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
      {
/*
      // If there is any data loaded, should close it...
      // Yumin:  how should I be testing to see if we have changed data?
      if (this->Internal->CMBModel->getCurrentModelFile().size() > 0)
        {
        // hmmm... onCloseData/closeData should probably be doing this
        if (QMessageBox::question(this->parentWidget(), "Save Changes?",
              "Any changes to the currently loaded data will be lost.  Save before closing?",
              QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
          {
//          this->getCMBModel()->onSaveData();
          }
        this->onCloseData(true);
        }
*/
      pqCMBLoadDataReaction::openFiles(QStringList( this->Internal->MesherOutputFileName ),
        QStringList(), pqCMBFileExtensions::ModelBuilder_ReadersMap());
      }
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::hideDisplayPanelPartialComponents()
{
  QCheckBox* visibleCheck = this->getAppearanceEditorContainer()->
    findChild<QCheckBox*>("ViewData");
  if(visibleCheck)
    {
    visibleCheck->hide();
    }
  QGroupBox* sliceGroup = this->getAppearanceEditorContainer()->
    findChild<QGroupBox*>("SliceGroup");
  if(sliceGroup)
    {
    sliceGroup->hide();
    }
  QGroupBox* colorGroup = this->getAppearanceEditorContainer()->
    findChild<QGroupBox*>("ColorGroup");
  if(colorGroup)
    {
    colorGroup->hide();
    }

  //the .ui should use a proper name such as "labelStyleRepresentation"
  //QLabel* repLabel = this->getAppearanceEditorContainer()->
  //  findChild<QLabel*>("label_2");
  //if(repLabel)
  //  {
  //  repLabel->hide();
  //  }
  //pqDisplayRepresentationWidget* repWidget = this->getAppearanceEditorContainer()->
  //  findChild<pqDisplayRepresentationWidget*>("StyleRepresentation");
  //if(repWidget)
  //  {
  //  repWidget->hide();
  //  }

  // Material
  QLabel* mLabel = this->getAppearanceEditorContainer()->
    findChild<QLabel*>("label_13");
  if(mLabel)
    {
    mLabel->hide();
    }
  QComboBox* mBox = this->getAppearanceEditorContainer()->
    findChild<QComboBox*>("StyleMaterial");
  if(mBox)
    {
    mBox->hide();
    }

  // volume mapper
  QLabel* vLabel = this->getAppearanceEditorContainer()->
    findChild<QLabel*>("label_19");
  if(vLabel)
    {
    vLabel->hide();
    }
  QComboBox* vMapperBox = this->getAppearanceEditorContainer()->
    findChild<QComboBox*>("SelectedMapperIndex");
  if(vMapperBox)
    {
    vMapperBox->hide();
    }
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

    this->Internal->SelectionModeBox->blockSignals(false);

    // set 2d face and edge colors from app settings
    this->applyColorSettings();

    // new attribute legends
/*
    this->Internal->AttEdgeScalarBarWidget = new pqScalarBarWidget(
      this->Internal->CMBModel->activeModelRepresentation(),NULL);
    this->Internal->AttEdgeScalarBarWidget->setPositionToRight();
    this->Internal->AttFaceScalarBarWidget = new pqScalarBarWidget(
      this->Internal->CMBModel->activeModelRepresentation(),NULL);
    this->Internal->AttFaceScalarBarWidget->setPositionToLeft();
*/

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
void pqCMBModelBuilderMainWindowCore::onSaveBCSs()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSaveData()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSaveAsData()
{
  this->onSaveData();
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
//     this->Internal->ModelDock = NULL;
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

  if (this->Internal->AttEdgeColorLegendAction)
    this->Internal->AttEdgeColorLegendAction->setVisible(false);
  if (this->Internal->AttFaceColorLegendAction)
    this->Internal->AttFaceColorLegendAction->setVisible(false);

  if(this->Internal->AttFaceScalarBarWidget)
    {
    this->Internal->AttFaceScalarBarWidget->setVisible(false);
    delete this->Internal->AttFaceScalarBarWidget;
    this->Internal->AttFaceScalarBarWidget = NULL;
    }
  if(this->Internal->AttEdgeScalarBarWidget)
    {
    this->Internal->AttEdgeScalarBarWidget->setVisible(false);
    delete this->Internal->AttEdgeScalarBarWidget;
    this->Internal->AttEdgeScalarBarWidget = NULL;
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

/*
  this->Internal->AttFaceColorAction->setVisible(false);
  this->Internal->AttFaceCategoryAction->setVisible(false);
  this->Internal->AttEdgeColorAction->setVisible(false);
  this->Internal->AttEdgeCategoryAction->setVisible(false);
  this->Internal->AttCategoryEdgeDomainAction->setVisible(false);
  this->Internal->AttColorEdgeDomainAction->setVisible(false);
  this->Internal->AttEdgeColorLegendAction->setChecked(false);
  this->Internal->AttFaceColorLegendAction->setChecked(false);
  this->Internal->AttEdgeColorLegendAction->setVisible(false);
  this->Internal->AttFaceColorLegendAction->setVisible(false);
*/
}

//-----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::loadOmicronModelInputData()
{
  QString filters = "Omicron \"model\": Input files (*.dat)";
  pqFileDialog fileDialog(this->getActiveServer(),
      this->parentWidget(), tr("Specify Omicron \"model\" Input:"),
      QString(), filters);
  fileDialog.setObjectName("FileOpenDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFile);
  if (fileDialog.exec() != QDialog::Accepted)
    {
    return false;
    }

  QStringList files = fileDialog.getSelectedFiles();
  if (files.size() == 0)
    {
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
FileBasedMeshingParameters
pqCMBModelBuilderMainWindowCore::generateLegacyVolumeMesherInput()
{
  FileBasedMeshingParameters dummy; // This originally didn't return anything!
  return dummy;
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onGenerateOmicronInput()
{
  FileBasedMeshingParameters fmp = this->generateLegacyVolumeMesherInput();

  if(fmp.valid)
    {
    //for legacy reasons we need to set this state variable
    this->Internal->OmicronMeshInputFileName = fmp.inputFilePath;
    }
}

//----------------------------------------------------------------------------
QString pqCMBModelBuilderMainWindowCore::saveBCSFileForOmicronInput(
                                                    QString meshInputFilePath)
{
  // first let's save out the bcs file for the omicroninput file
  QFileInfo outFileInfo( meshInputFilePath );
  QString outPath = outFileInfo.absolutePath();
  QString outBaseName = outFileInfo.baseName();
  QString outBCSName = "mesh_" + outBaseName + ".bcs";
  QString outputBCSFileName =  outPath + "/" + outBCSName;
  // get a unique file name for the new bcs file
  QFileInfo info(outputBCSFileName);
  int suffix=0;
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  while(fModel.fileExists(outputBCSFileName, fullpath))
    {
    outBCSName = "mesh_" + outBaseName +
      QString::number(suffix++) + ".bcs";
    outputBCSFileName =  outPath + "/" + outBCSName;
    //info.setFile(outputBCSFileName);
    }
  //this->getCMBModel()->saveBCSs(outputBCSFileName);
  return outBCSName;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSpawnSurfaceMesher()
{
  // come up with the mesher we're going to use
  if (this->Internal->DefaultSurfaceMesher.size() == 0)
    {
    QMessageBox::warning(this->parentWidget(),
        "Unable to find the default surface mesher!",
        "Unable to find default surface mesher!  Must select alternate mesher.");
    }

  // Not "perfect", but add some ability to select a different mesher
  // (currently expecting omicron, but may not be in application directory)
  if (this->Internal->DefaultSurfaceMesher.size() == 0 ||
      QMessageBox::question(this->parentWidget(),
        "Select alternate surface mesher?",
        "Select alternate mesher (Yes) or use default (No) at \n" +
        this->Internal->DefaultSurfaceMesher,
        QMessageBox::Yes|QMessageBox::No, QMessageBox::No) ==
      QMessageBox::Yes)
    {
    QString alternateExec;
    if (this->getExistingFileName("All files (*);;Windows Executable (*.exe)",
          "Select surface mesher", alternateExec) == true)
      {
      this->Internal->DefaultSurfaceMesher = alternateExec;
      }
    else
      {
      // warning
      return;
      }
    }

  // now determine the input file... perhaps (in future) the type will
  // depend on the mesher selected?

  // get input filename...
  QString inputFilename;
  if (this->getExistingFileName("Omicron \"model\" Input files (*.dat)",
        "Specify Omicron \"model\" Input:", inputFilename) == true)
    {
    }
  else
    {
    //warning
    return;
    }

  // change to directoy where the data is
  QFileInfo inputFinfo(inputFilename);
  this->setProcessExecDirectory(inputFinfo.absoluteDir().absolutePath());

  // read the 1st line of the input file to see what the output file will be
  ifstream fin(inputFinfo.filePath().toAscii().constData());
  if(!fin)
    {
    QMessageBox::warning(this->parentWidget(), "Unable to read input file!",
        "Unable to read input file!  Cancelling \"model\".");
    return;
    }
  char inputPtsFileName[1024];
  fin >> inputPtsFileName;
  fin.close();
  QFileInfo ptsFInfo(inputPtsFileName);
  this->Internal->MesherOutputFileName =
    this->getProcessExecDirectory() + "/" + ptsFInfo.baseName() + ".vtk";

  const QString executablePath = this->Internal->DefaultSurfaceMesher;
  const QString executableArgs = inputFinfo.fileName();
  const QString commandString = executablePath + ";" + executableArgs;

  QObject::connect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)),
      this, SLOT(loadMesherOutput(remus::proto::JobResult)));

  QObject::connect(this, SIGNAL(cleanupOutputFiles()),
      this, SLOT(cleanupSurfaceMeshFiles()));

  this->submitRemusSurfaceJob(commandString);
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSpawnVolumeMesher()
{
  this->launchLocalRemusServer();

  //need to propagate the local remus server enpoint to the mesh selector
  //so that we use the correct transport proto, ip and port.
  QPointer<qtRemusVolumeMesherSelector> mesherSelector(
                  new qtRemusVolumeMesherSelector( this->remusServerEndpoint(),
                                                this->parentWidget() ) );

  const bool mesher_chosen = mesherSelector->chooseMesher();
  const bool use_legacy_mesher = mesherSelector->useLegacyMesher();

  //copy the mesherData so we can delete the mesherSelector
  remus::proto::JobRequirements mesherData = mesherSelector->mesherData();

  //delete the mesherSelector
  delete mesherSelector;

  //it is easier to reason about the following if we remove the use case
  //of not meshing now
  if(!mesher_chosen)
    {
    return;
    }

  if( use_legacy_mesher )
    {
    //query the user for the what legacy mesher they want to use
    const FileBasedMeshingParameters fmp =
                                  this->generateLegacyVolumeMesherInput();

    //submit the legacy mesh job to remus, and return the expected output
    //file name
    const QString expectedOutputFileName = this->submitRemusVolumeJob(fmp);

    //set the output file name that was returned by remus
    this->Internal->MesherOutputFileName = expectedOutputFileName;
    const bool jobSubmissionIsValid = (expectedOutputFileName.size() > 0);

    if(jobSubmissionIsValid)
      {
      QObject::connect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)),
                       this, SLOT(loadRemusOutput(remus::proto::JobResult)));
      }
    }
  else
    {
/*
    //need to propagate the local remus server enpoint to the submitter
    //so that we use the correct transport proto, ip and port.
    QPointer<qtRemusVolumeMesherSubmitter> jobSubmitter(
                          new qtRemusVolumeMesherSubmitter(this->remusServerEndpoint(),
                                                 this->parentWidget()) );

    //fetch the job from the server
    remus::proto::Job submittedJob =
            jobSubmitter->submitRequirements(this->getCMBModel(),
                                             this->getCMBModel()->getCurrentModelFile(),
                                             mesherData);

    //clear the MesherOutputFileName so that loadRemusOutput can determine
    //if the output file name is held be client side state or by the job result
    this->Internal->MesherOutputFileName = QString();

    const bool jobSubmissionIsValid = this->monitorRemusJob(submittedJob);

    if(jobSubmissionIsValid)
      {
      QObject::connect(this, SIGNAL(remusCompletedNormally(remus::proto::JobResult)),
                       this, SLOT(loadRemusOutput(remus::proto::JobResult)));
      }
*/
    }
}

//-----------------------------------------------------------------------------
int pqCMBModelBuilderMainWindowCore::loadRemusOutput(remus::proto::JobResult result)
{
  //The results are the file name that should be MesherOutputFileName
  //if MesherOutputFileName is empty we know that the JobResult's contain
  //the new MesherOutputFileName, otherwise we have arrived here by using
  //the legacy omicron mesher, and MesherOutputFileName already contains
  //the correct output path
  if(this->Internal->MesherOutputFileName.size() == 0)
    {
    //We need to set MesherOutputFileName now as slots that
    //are connected to previewWindow expect MesherOutputFileName to be set
    const std::string meshOutputFName(result.data(), result.dataSize());
    this->Internal->MesherOutputFileName =
                            QString::fromStdString(meshOutputFName);
    }

  //now preview the output mesh to the user
  return this->previewWindow(this->Internal->MesherOutputFileName);
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

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::acceptMeshResult()
{
  emit enableExternalProcesses(true);

  const QFileInfo finfo(this->Internal->MesherOutputFileName);
  const QString baseName(finfo.completeBaseName());
  const QString path(finfo.absolutePath());
  const QString bc_path(path + "/" + baseName + ".bc");
  this->loadBCFile(bc_path);
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::cleanupVolumeMeshFiles()
{
  emit enableExternalProcesses(true);
  if ( QFile::exists( this->Internal->MesherOutputFileName ) )
    {
    QFile::remove( this->Internal->MesherOutputFileName );
    }

  if ( QFile::exists( this->Internal->OmicronMeshInputFileName ) )
    {
    QFile::remove( this->Internal->OmicronMeshInputFileName );
    }


  const QFileInfo finfo(this->Internal->MesherOutputFileName);
  const QString baseName(finfo.completeBaseName());
  const QString path(finfo.absolutePath());

  const QString bc_path(path + "/" + baseName + ".bc");
  const QString bcs_path(path + "/" + baseName + ".bcs");

  if(QFile::exists(bc_path))
    {
    QFile::remove(bc_path);
    }
  if(QFile::exists(bcs_path))
    {
    QFile::remove(bcs_path);
    }
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::cleanupSurfaceMeshFiles()
{
  emit enableExternalProcesses(true);

  QFileInfo info(this->Internal->MesherOutputFileName);

  if ( QFile::exists( this->Internal->MesherOutputFileName ) )
    {
    QFile::remove( this->Internal->MesherOutputFileName );
    QFile::remove( info.path() + "/Surf3d_" + info.fileName() );
    QFile::remove( info.path() + "/Box_Shell.vtk" );
    QFile::remove( info.path() + "/BCfaces.vtk" );
    }
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
    SIGNAL(operationFinished(const smtk::model::OperatorResult&, bool)),
    this, SLOT(processModelInfo( const smtk::model::OperatorResult& , bool)));

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
int pqCMBModelBuilderMainWindowCore::loadBCFile(const QString& filename)
{
  this->Internal->Is3DMeshCreated = false;
  pqFileDialogModel fModel(this->getActiveServer());
  QString fullpath;
  if (!fModel.fileExists(filename, fullpath))
    {
    qCritical() << "File does not exist: " << filename;
    return 0;
    }
/*
  // load the BC file
  vtkCMBImportBCFileOperatorClient* importBCOperator =
    vtkCMBImportBCFileOperatorClient::New();
  importBCOperator->SetFileName(filename.toStdString().c_str());
  int retVal = importBCOperator->Operate(this->getCMBModel()->getModel(),
                                         this->getCMBModel()->getModelWrapper());//serverModelProxy);
  importBCOperator->Delete();
  this->Internal->Is3DMeshCreated = retVal ? true : false;
  return retVal;
*/
  return 0;
}

//-----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::checkAnalysisMesh()
{
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::clearCurrentEntityWidgets()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setFaceMeshRepresentationType(
  const char* strType, vtkCollection* selFaces)
{
  if(selFaces->GetNumberOfItems()==0)
    {
    return;
    }

  this->activeRenderView()->render();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::setMeshRepresentationColor(
  const QColor& repColor, vtkCollection* selFaces, vtkCollection* selEdges)
{
  this->activeRenderView()->render();
}

//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onSaveMeshToModelInfo()
{
}
//----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onLoadMeshToModelInfo()
{
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::createRectangleModel(
  double* boundingBox, int baseResolution, int heightResolution)
{
  this->onCloseData(true);

}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::createEllipseModel(double* values, int resolution)
{
  this->onCloseData(true);
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
void pqCMBModelBuilderMainWindowCore::applyColorSettings()
{
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorByAttribute()
{
  if(this->Internal->AttFaceColorAction->isVisible())
    {
    this->onColorFaceByAttribute();
    }
  if(this->Internal->AttEdgeColorAction->isVisible())
    {
    this->onColorEdgeByAttribute();
    }
  if(this->Internal->AttColorEdgeDomainAction->isVisible())
    {
    this->onEdgeDomainColorByAttribute();
    }
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorFaceByAttribute()
  {
  if(!this->canColorByAttribute() ||
    this->Internal->AttFaceColorCombo->count()==0)
    {
    return;
    }
  QString currentDefType = this->Internal->AttFaceColorCombo->currentText();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
//  this->Internal->CMBModel->setCurrentFaceAttributeColorInfo(
//    attSystem, currentDefType);
  if(this->Internal->AttFaceColorLegendAction->isVisible() &&
    this->Internal->AttFaceColorLegendAction->isChecked())
    {
    this->toggleAttFaceColorLegend(true);
    }

//  this->Internal->CMBModel->onLookupTableModified();
  }
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onFaceAttCategoryChanged()
{
  this->Internal->AttFaceColorCombo->blockSignals(true);
  QString currentDefType = this->Internal->AttFaceColorCombo->currentText();
  QString currentCat = this->Internal->AttFaceCategoryCombo->currentText();
  this->Internal->AttFaceColorCombo->clear();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  QStringList attDefTypes;
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();

  QMap<QString, QList<smtk::attribute::DefinitionPtr> > attDefMap;
  simUIManager->getAttributeDefinitions(attDefMap);
  QMap<QString, QList<smtk::attribute::DefinitionPtr> >::iterator dit=
    attDefMap.begin();
  for(; dit != attDefMap.end(); ++dit)
    {
    if(currentCat == dit.key())
      {
      foreach(smtk::attribute::DefinitionPtr pDef, dit.value())
        {
        std::vector<smtk::attribute::AttributePtr> result;
        attSystem->findDefinitionAttributes(pDef->type(), result);
        // if there is no attribute with the def, don't show it.
        if(result.size() > 0)
          {
          attDefTypes << pDef->type().c_str();
          }
        }
      }
    }
  this->Internal->AttFaceColorCombo->addItems(attDefTypes);
  int index = this->Internal->AttFaceColorCombo->findText(currentDefType);
  index = (index==-1) ? 0 : index;
  this->Internal->AttFaceColorCombo->setCurrentIndex(index);
  this->Internal->AttFaceColorCombo->blockSignals(false);
  this->onColorFaceByAttribute();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onColorEdgeByAttribute()
{
  if(!this->canColorByAttribute() ||
    this->Internal->AttEdgeColorCombo->count()==0)
    {
    return;
    }
  QString currentDefType = this->Internal->AttEdgeColorCombo->currentText();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
//  this->Internal->CMBModel->setCurrentEdgeAttributeColorInfo(
//    attSystem, currentDefType);
  if(this->Internal->AttEdgeColorLegendAction->isVisible() &&
    this->Internal->AttEdgeColorLegendAction->isChecked())
    {
    this->toggleAttEdgeColorLegend(true);
    }

//  this->Internal->CMBModel->onLookupTableModified();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onEdgeAttCategoryChanged()
{
  this->Internal->AttEdgeColorCombo->blockSignals(true);
  QString currentDefType = this->Internal->AttEdgeColorCombo->currentText();
  QString currentCat = this->Internal->AttEdgeCategoryCombo->currentText();
  this->Internal->AttEdgeColorCombo->clear();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  QStringList attDefTypes;
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();

  QMap<QString, QList<smtk::attribute::DefinitionPtr> > attDefMap;
  simUIManager->getAttributeDefinitions(attDefMap);
  QMap<QString, QList<smtk::attribute::DefinitionPtr> >::iterator dit=
    attDefMap.begin();
  for(; dit != attDefMap.end(); ++dit)
    {
    if(currentCat == dit.key())
      {
      foreach(smtk::attribute::DefinitionPtr pDef, dit.value())
        {
        std::vector<smtk::attribute::AttributePtr> result;
        attSystem->findDefinitionAttributes(pDef->type(), result);
        // if there is no attribute with the def, don't show it.
        if(result.size() > 0)
          {
          attDefTypes << pDef->type().c_str();
          }
        }
      }
    }
  this->Internal->AttEdgeColorCombo->addItems(attDefTypes);
  int index = this->Internal->AttEdgeColorCombo->findText(currentDefType);
  index = (index==-1) ? 0 : index;
  this->Internal->AttEdgeColorCombo->setCurrentIndex(index);
  this->Internal->AttEdgeColorCombo->blockSignals(false);
  this->onColorEdgeByAttribute();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onEdgeDomainColorByAttribute()
{
  if(!this->canColorByAttribute() ||
    this->Internal->AttColorEdgeDomainCombo->count()==0)
    {
    return;
    }
  QString currentDefType = this->Internal->AttColorEdgeDomainCombo->currentText();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();
//  this->Internal->CMBModel->setCurrentDomainAttributeColorInfo(
//    attSystem, currentDefType);
  if(this->Internal->AttFaceColorLegendAction->isVisible() &&
    this->Internal->AttFaceColorLegendAction->isChecked())
    {
    this->toggleAttFaceColorLegend(true);
    }

//  this->Internal->CMBModel->onLookupTableModified();
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onEdgeDomainAttCategoryChanged()
{
  this->Internal->AttColorEdgeDomainCombo->blockSignals(true);
  QString currentDefType = this->Internal->AttColorEdgeDomainCombo->currentText();
  QString currentCat = this->Internal->AttCategoryEdgeDomainCombo->currentText();
  this->Internal->AttColorEdgeDomainCombo->clear();
  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();
  QStringList attDefTypes;
  smtk::attribute::SystemPtr attSystem = simUIManager->attSystem();

  QMap<QString, QList<smtk::attribute::DefinitionPtr> > attDefMap;
  simUIManager->getAttributeDefinitions(attDefMap);
  QMap<QString, QList<smtk::attribute::DefinitionPtr> >::iterator dit=
    attDefMap.begin();
  for(; dit != attDefMap.end(); ++dit)
    {
    if(currentCat == dit.key())
      {
      foreach(smtk::attribute::DefinitionPtr pDef, dit.value())
        {
        std::vector<smtk::attribute::AttributePtr> result;
        attSystem->findDefinitionAttributes(pDef->type(), result);
        // if there is no attribute with the def, don't show it.
        if(result.size() > 0)
          {
          // only show those defs that can be associated with domain
          if(pDef->associatesWithVolume())
            {
            attDefTypes << pDef->type().c_str();
            }
          }
        }
      }
    }
  this->Internal->AttColorEdgeDomainCombo->addItems(attDefTypes);
  int index = this->Internal->AttColorEdgeDomainCombo->findText(currentDefType);
  index = (index==-1) ? 0 : index;
  this->Internal->AttColorEdgeDomainCombo->setCurrentIndex(index);
  this->Internal->AttColorEdgeDomainCombo->blockSignals(false);
  this->onEdgeDomainColorByAttribute();
}

//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::onNumOfAttriubtesChanged()
{
  if(!this->canColorByAttribute())
    {
    return;
    }

  pqSMTKUIManager* simUIManager = this->getSimBuilder()->attributeUIManager();

  QStringList categories;
  QMap<QString, QList<smtk::attribute::DefinitionPtr> > attDefMap;
  simUIManager->getAttributeDefinitions(attDefMap);
  QMap<QString, QList<smtk::attribute::DefinitionPtr> >::iterator dit=
    attDefMap.begin();
  for(; dit != attDefMap.end(); ++dit)
    {
    categories << dit.key();
    }
  if(this->Internal->AttFaceCategoryAction->isVisible())
    {
    this->Internal->AttFaceCategoryCombo->blockSignals(true);
    QString currentText = this->Internal->AttFaceCategoryCombo->currentText();
    this->Internal->AttFaceCategoryCombo->clear();
    this->Internal->AttFaceCategoryCombo->addItems(categories);
    int index = this->Internal->AttFaceCategoryCombo->findText(currentText);
    index = (index==-1) ? 0 : index;
    this->Internal->AttFaceCategoryCombo->setCurrentIndex(index);
    this->Internal->AttFaceCategoryCombo->blockSignals(false);
    this->onFaceAttCategoryChanged();
    }
  if(this->Internal->AttCategoryEdgeDomainAction->isVisible())
    {
    this->Internal->AttCategoryEdgeDomainCombo->blockSignals(true);
    QString currentText = this->Internal->AttCategoryEdgeDomainCombo->currentText();
    this->Internal->AttCategoryEdgeDomainCombo->clear();
    this->Internal->AttCategoryEdgeDomainCombo->addItems(categories);
    int index = this->Internal->AttCategoryEdgeDomainCombo->findText(currentText);
    index = (index==-1) ? 0 : index;
    this->Internal->AttCategoryEdgeDomainCombo->setCurrentIndex(index);
    this->Internal->AttCategoryEdgeDomainCombo->blockSignals(false);
    this->onEdgeDomainAttCategoryChanged();
    }
  if(this->Internal->AttEdgeCategoryAction->isVisible())
    {
    this->Internal->AttEdgeCategoryCombo->blockSignals(true);
    QString currentText = this->Internal->AttEdgeCategoryCombo->currentText();
    this->Internal->AttEdgeCategoryCombo->clear();
    this->Internal->AttEdgeCategoryCombo->addItems(categories);
    int index = this->Internal->AttEdgeCategoryCombo->findText(currentText);
    index = (index==-1) ? 0 : index;
    this->Internal->AttEdgeCategoryCombo->setCurrentIndex(index);
    this->Internal->AttEdgeCategoryCombo->blockSignals(false);
    this->onEdgeAttCategoryChanged();
    }
}
//-----------------------------------------------------------------------------
bool pqCMBModelBuilderMainWindowCore::canColorByAttribute()
{
  return false; // This originally didn't return anything
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::toggleAttFaceColorLegend(bool show)
{
  QString currentDefType;
  if(this->Internal->AttFaceCategoryAction->isVisible())
    {
    currentDefType = this->Internal->AttFaceColorCombo->currentText();
    }
  else if(this->Internal->AttCategoryEdgeDomainAction->isVisible())
    {
    currentDefType = this->Internal->AttColorEdgeDomainCombo->currentText();
    }

  this->updateScalarBarWidget(
    this->Internal->AttFaceScalarBarWidget,
    currentDefType, show);
}
//-----------------------------------------------------------------------------
void pqCMBModelBuilderMainWindowCore::toggleAttEdgeColorLegend(bool show)
{
  QString currentDefType = this->Internal->AttEdgeColorCombo->currentText();
  this->updateScalarBarWidget(
    this->Internal->AttEdgeScalarBarWidget,
    currentDefType, show);
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
  const smtk::model::OperatorResult& result, bool hasNewModels)
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
    result->findModelEntity("entities");

  QList<unsigned int> visBlocks;
  QList<unsigned int> colorBlocks;
  QColor color;
  bool visible = true;
  if(resultEntities && resultEntities->numberOfValues() > 0)
    {
    std::cout << " client associated entities " << resultEntities->numberOfValues() << std::endl;

    smtk::model::EntityRefArray::const_iterator it;
    for(it = resultEntities->begin(); it != resultEntities->end(); ++it)
      {
      unsigned int flatIndex;
      //cmbSMTKModelInfo* minfo = this->Internal->smtkModelManager->modelInfo((*it).entity());

      //if(minfo && minfo->Representation && (*it).hasIntegerProperty("block_index"))
      //   minfo->Info->GetBlockId((*it).entity(), flatIndex))
      if((*it).hasIntegerProperty("block_index"))
        {
        const smtk::model::IntegerList& prop((*it).integerProperty("block_index"));
        if(!prop.empty())
          {
          flatIndex = prop[0];
          if((*it).hasVisibility())
            {
            visBlocks << flatIndex+1;
            visible = (*it).visible();
            }

          colorBlocks << flatIndex+1;
          if((*it).hasColor())
            {
            smtk::model::FloatList rgba = (*it).color();
            if ((rgba.size() == 3 || rgba.size() ==4) &&
            !(rgba[0]+rgba[1]+rgba[2] == 0))
              color.setRgbF(rgba[0], rgba[1], rgba[2]);
            }
          }
        }
      }
    }

  //this->modelPanel()->setIgnorePropertyChange(true);
  pqDataRepresentation* repr = pqActiveObjects::instance().activeRepresentation();
  if(repr)
    {
    cmbSMTKModelInfo* minfo = this->Internal->smtkModelManager->modelInfo(repr);
    if(minfo)
      {
      if(visBlocks.count())
        this->Internal->ViewContextBehavior->setBlockVisibility(
          visBlocks, visible, minfo->Info->GetUUID2BlockIdMap().size());
      if(colorBlocks.count())
        this->Internal->ViewContextBehavior->setBlockColor(
          colorBlocks, color);
      }
    }

  //this->modelPanel()->setIgnorePropertyChange(false);
  smtk::attribute::ModelEntityItem::Ptr newEntities =
    result->findModelEntity("new entities");
  smtk::attribute::ModelEntityItem::Ptr remEntities =
    result->findModelEntity("expunged");
  bool hasNewEntities = newEntities && newEntities->numberOfValues() > 0;
  bool entitiesRemoved = remEntities && remEntities->numberOfValues() > 0;
  if(hasNewModels || hasNewEntities || entitiesRemoved)
    {
    this->modelPanel()->resetUI();
    if(entitiesRemoved)
      {
      smtk::model::EntityRefs remEnts;
      smtk::model::EntityRefArray::const_iterator it;
      for(it = remEntities->begin(); it != remEntities->end(); ++it)
        {
        remEnts.insert(*it);
        }
      this->modelPanel()->onEntitiesExpunged(remEnts);
      }

    this->activeRenderView()->resetCamera();
    emit this->newModelCreated();
    }
  else
    {
    this->activeRenderView()->render();
    this->modelPanel()->update();
    }
  return true;
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
 /*
    //this->getAppearanceEditorContainer()->layout()->addWidget(
    //    displayEditor);
    // Hide unrelated GUI components on the display property
    this->hideDisplayPanelPartialComponents();
    QPushButton* zoomOnData = this->getAppearanceEditorContainer()->
      findChild<QPushButton*>("ViewZoomToData");
    if(zoomOnData)
      {
      QObject::connect(zoomOnData, SIGNAL(clicked()),
        this, SLOT(zoomOnSelection()));
      }
 */
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
