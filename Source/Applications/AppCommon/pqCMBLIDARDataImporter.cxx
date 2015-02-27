/*=========================================================================

  Program:   CMB
  Module:    pqCMBLIDARDataImporter.cxx

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
// .NAME Represents a class for importing LIDAR objects.
// .SECTION Description
// .SECTION Caveats

#include "pqCMBLIDARDataImporter.h"
#include "ui_qtLIDARDataImportDialog.h"

#include <QFileInfo>
#include <QDebug>
#include <QStringList>
#include <QToolButton>
#include <QTableWidgetItem>
#include <QCoreApplication>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QComboBox>

#include "pqCMBLIDARPieceObject.h"
#include "pqCMBLIDARPieceTable.h"
#include "pqCMBDisplayProxyEditor.h"
#include "pqCMBEnumPropertyWidget.h"
#include "qtCMBProgressWidget.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include <vtkSMPropertyHelper.h>
#include "vtkSMPropertyLink.h"
#include "vtkSMDataSourceProxy.h"
#include <vtkSMDoubleVectorProperty.h>
#include "vtkSMRepresentationProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkTransform.h"

//-----------------------------------------------------------------------------
pqCMBLIDARDataImporter::pqCMBLIDARDataImporter()
{
  this->MainDialog = new QDialog();

  this->ImportDialog = new Ui::qtLIDARDataImportDialog;
  this->ImportDialog->setupUi(this->MainDialog);

  //QObject::connect(this->ImportDialog->saveButtonAsDisplayed, SIGNAL(clicked()),
  //                 this, SLOT(onSavePiecesAsDisplayed()));
  QObject::connect(this->ImportDialog->loadButton, SIGNAL(clicked()),
                   this, SLOT(onAcceptToLoad()));
  //QObject::connect(this->ImportDialog->saveButtonAsOriginal, SIGNAL(clicked()),
  //                 this, SLOT(onSavePiecesAsOriginal()));
  //QObject::connect(this->ImportDialog->loadButtonAsOriginal, SIGNAL(clicked()),
  //                 this, SLOT(onAcceptToLoadAsOriginal()));
  QObject::connect(this->ImportDialog->cancelButton, SIGNAL(clicked()),
                   this->MainDialog, SLOT(reject()));
  QObject::connect(this->ImportDialog->showSelectedButton, SIGNAL(clicked()),
                   this, SLOT(OnPreviewSelected()));
  QObject::connect(this->ImportDialog->updateButton, SIGNAL(clicked()),
                   this, SLOT(onUpdateSelectedPieces()));

  QObject::connect(this->ImportDialog->CheckAllButton, SIGNAL(clicked()),
                   this, SLOT(selectAll()));
  QObject::connect(this->ImportDialog->UncheckAllButton, SIGNAL(clicked()),
                   this, SLOT(unselectAll()));
  //QObject::connect(this->ImportDialog->PieceSlider, SIGNAL(sliderMoved(int)),
  //                 this, SLOT(onSetSliderPosition(int)));
  //QObject::connect(this->ImportDialog->PieceRatioBox, SIGNAL(editingFinished()),
  //                 this, SLOT(onCurrentPieceRatioChanged()));

  QObject::connect(this->ImportDialog->zoomSelection, SIGNAL(clicked()),
                   this, SLOT(zoomSelection()));
  QObject::connect(this->ImportDialog->clearSelection, SIGNAL(clicked()),
                   this, SLOT(clearSelection()));

  // enableClip defaults to not checked and the clip group is disabled
  QObject::connect(this->ImportDialog->clipGroupBox, SIGNAL(toggled(bool)),
                   this, SLOT(onEnableClip()));
  QObject::connect(this->ImportDialog->minXClip, SIGNAL(valueChanged(double)),
    this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->ImportDialog->maxXClip, SIGNAL(valueChanged(double)),
    this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->ImportDialog->minYClip, SIGNAL(valueChanged(double)),
    this, SLOT(onClippingBoxChanged()));
  QObject::connect(this->ImportDialog->maxYClip, SIGNAL(valueChanged(double)),
    this, SLOT(onClippingBoxChanged()));

  QObject::connect(this->ImportDialog->applyTargetNumberOfPoints, SIGNAL(clicked()),
                   this, SLOT(applyTargetNumberOfPoints()));

  QPalette p = this->ImportDialog->displayPointsLabel->palette();
  p.setColor(QPalette::Highlight, Qt::red);
  this->ImportDialog->displayPointsLabel->setPalette(p);
  this->ImportDialog->savePointsLabel->setPalette(p);

  this->RepresentationWidget = NULL;

  // Set up the data table
  this->PieceMainTable =
    new pqCMBLIDARPieceTable(this->ImportDialog->PieceVisibilityTable);
  QObject::connect(PieceMainTable,
    SIGNAL(currentObjectChanged(pqCMBLIDARPieceObject*)),
    this, SLOT(onPiecesSelectionChanged(pqCMBLIDARPieceObject*)), Qt::QueuedConnection);

  //this->PieceOnRatioTable =
  //  new pqCMBLIDARPieceTable(this->ImportDialog->PieceOnRatioTable, true);

  //this->PieceMainTable->setLinkedTable( this->PieceOnRatioTable );
  //this->PieceOnRatioTable->setLinkedTable( this->PieceMainTable );
  QObject::connect(this->ImportDialog->advancedCheckBox, SIGNAL(stateChanged(int)),
    this, SLOT(onAdvancedCheckBox(int)));

  QObject::connect(this->PieceMainTable, SIGNAL(
    objectOnRatioChanged(pqCMBLIDARPieceObject*, int)),
    this, SLOT(onObjectOnRatioChanged(pqCMBLIDARPieceObject*, int)));

  // if change state in one table, should change in the other
  QObject::connect(this->PieceMainTable, SIGNAL(
    checkedObjectsChanged(QList<int>, QList<int>)),
    this, SLOT(onObjectsCheckStateChanged(QList<int>, QList<int>)));
  // if change state in one table, should change in the other
  //QObject::connect(this->PieceOnRatioTable, SIGNAL(
  //  checkedObjectsChanged(QList<int>, QList<int>)),
  //  this, SLOT(onObjectsCheckStateChanged(QList<int>, QList<int>)));


  // RSB!!!! should be the smae object slected in each table!!!
  QObject::connect(this->PieceMainTable, SIGNAL(
    currentObjectChanged(pqCMBLIDARPieceObject*)),
    this, SLOT(onCurrentObjectChanged(pqCMBLIDARPieceObject*)));
  //QObject::connect(this->PieceOnRatioTable, SIGNAL(
  //  currentObjectChanged(pqCMBLIDARPieceObject*)),
  //  this, SLOT(onCurrentObjectChanged(pqCMBLIDARPieceObject*)));
  //QObject::connect(this->PieceMainTable, SIGNAL(checkedObjectsChanged()),
  //  this, SLOT(onCheckedObjectsChanged(pqCMBLIDARPieceObject*)));

  this->MinimumNumberOfPointsPerPiece = 50;

  this->RenderNeeded = false;
  //QObject::connect(this->PieceOnRatioTable, SIGNAL(requestRender()),
  //  this, SLOT(onRequestRender()));
  QObject::connect(this->PieceMainTable, SIGNAL(requestRender()),
    this, SLOT(onRequestRender()));
  QObject::connect(this, SIGNAL(requestRender()),
    this, SLOT(onRequestRender()));

  QObject::connect(this, SIGNAL(renderRequested()),
    this, SLOT(onRenderRequested()), Qt::QueuedConnection);

  this->CurrentServer = NULL;
  this->CurrentRenderView = NULL;
  this->RepPropLink = NULL;
  this->ReaderSource = NULL;
  this->OutlineSource = NULL;
  this->CurrentWriter = NULL;
  this->LastSelectedObject = NULL;
  this->OutlineRepresentation = NULL;
  this->setupProgressBar();
  this->AppearanceEditor = NULL;
  this->setupAppearanceEditor(this->ImportDialog->tabDisplay);

  QObject::connect(this->ImportDialog->tabWidget,
    SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

}

//-----------------------------------------------------------------------------
pqCMBLIDARDataImporter::~pqCMBLIDARDataImporter()
{
  if(this->PieceMainTable)
    {
    delete this->PieceMainTable;
    }
  if(this->AppearanceEditor)
    {
    delete this->AppearanceEditor;
    }
  if(this->RepresentationWidget)
    {
    delete this->RepresentationWidget;
    }
  if (this->ImportDialog)
    {
    delete this->ImportDialog;
    }
  if (this->MainDialog)
    {
    delete this->MainDialog;
    }
}

//-----------------------------------------------------------------------------
const char *pqCMBLIDARDataImporter::importLIDARFile(const char* filename)
{
  if(!this->isValidFile(filename))
    {
    return NULL;
    }

  this->clearCurrentLIDARData();
  this->PieceMainTable->clear();
  //this->PieceOnRatioTable->clear();

  this->MainDialog->setModal(true);
  this->MainDialog->show();

  int imported = this->ImportLIDARData(filename);
  this->ReaderSource->getProxy()->UpdatePropertyInformation();

  if(imported)
    {
    double *bounds = vtkSMDoubleVectorProperty::SafeDownCast(
      this->ReaderSource->getProxy()->GetProperty("DataBounds"))->GetElements();
    // not sure we really need to save anything other than Z bounds...
    this->DataBounds[0] = bounds[0];
    this->DataBounds[1] = bounds[1];
    this->DataBounds[2] = bounds[2];
    this->DataBounds[3] = bounds[3];
    this->DataBounds[4] = bounds[4];
    this->DataBounds[5] = bounds[5];

    // setup the min and max values for clipping: 3 times the range we read in
    double xDelta = (bounds[1] - bounds[0]);
    this->ImportDialog->minXClip->setMinimum( bounds[0] - xDelta );
    this->ImportDialog->maxXClip->setMinimum( bounds[0] - xDelta );
    this->ImportDialog->minXClip->setMaximum( bounds[1] + xDelta );
    this->ImportDialog->maxXClip->setMaximum( bounds[1] + xDelta );
    double yDelta = (bounds[3] - bounds[2]);
    this->ImportDialog->minYClip->setMinimum( bounds[2] - yDelta );
    this->ImportDialog->maxYClip->setMinimum( bounds[2] - yDelta );
    this->ImportDialog->minYClip->setMaximum( bounds[3] + yDelta );
    this->ImportDialog->maxYClip->setMaximum( bounds[3] + yDelta );
    // set initial values for clipping... 1% larger in each direction
    this->ImportDialog->minXClip->setValue( bounds[0] - xDelta * 0.01 );
    this->ImportDialog->maxXClip->setValue( bounds[1] + xDelta * 0.01 );
    this->ImportDialog->minYClip->setValue( bounds[2] - yDelta * 0.01 );
    this->ImportDialog->maxYClip->setValue( bounds[3] + yDelta * 0.01 );
    //this->setupSliderBar();
    //this->updateRepresentationLink();

    this->initialClippingSetup();
    }
  else
    {
    int aborted = pqSMAdaptor::getElementProperty(
      this->ReaderSource->getProxy()->GetProperty("AbortExecute")).toInt();
    if(!aborted)
      {
      qCritical() << "Failed to import the LIDAR File:!\n" << filename;
      }
    return NULL;
    }

  if(this->MainDialog->exec() == QDialog::Accepted)
    {
    this->MainDialog->hide();
    }

  if (this->OutputFileName.size() > 0)
    {
    return this->OutputFileName.c_str();
    }
  return 0;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::isValidFile(const char *filename)
{
  if (this->FileName.empty() && filename == NULL)
    {
    return false;
    }
  if (!this->FileName.empty() && filename &&
    !strcmp(this->FileName.c_str(), filename))
    {
    return false;
    }

  this->FileName = filename ? filename : "";

  return true;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::clearCurrentLIDARData()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBLIDARPieceObject* dataObj = NULL;
  int pieceIdx;
  for (int i=0; i<this->PieceIdObjectMap.keys().count(); i++)
    {
    pieceIdx = this->PieceIdObjectMap.keys().value(i);
    dataObj = this->PieceIdObjectMap[pieceIdx];
    if(dataObj)
      {
      delete dataObj;
      }
    }
  this->PieceIdObjectMap.clear();

  if(this->RepPropLink)
    {
    this->RepPropLink->RemoveAllLinks();
    this->RepPropLink->Delete();
    this->RepPropLink = NULL;
    }
}

//-----------------------------------------------------------------------------
int pqCMBLIDARDataImporter::getPieceInfo(QList<int> &pieceInfo)
{
  vtkSMSourceProxy* source =
    vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy());
  source->UpdatePropertyInformation();

  int numberOfPieces = pqSMAdaptor::getElementProperty(
    source->GetProperty("KnownNumberOfPieces")).toInt();

  int numberOfPointsInPiece, totalNumberOfPoints = 0;
  for (int i = 0; i < numberOfPieces; i++)
    {
    pqSMAdaptor::setElementProperty(source->GetProperty("PieceIndex"), i);
    source->UpdateVTKObjects();
    source->UpdatePropertyInformation();

    numberOfPointsInPiece = pqSMAdaptor::getElementProperty(
      source->GetProperty("NumberOfPointsInPiece")).toInt();
    totalNumberOfPoints += numberOfPointsInPiece;
    pieceInfo.push_back( numberOfPointsInPiece );
    }

  return totalNumberOfPoints;
}

//-----------------------------------------------------------------------------
int pqCMBLIDARDataImporter::calculateMainOnRatio(int totalNumberOfPoints)
{
  double tmpOnRatio = static_cast<double>(this->ImportDialog->targetNumberOfPoints->value()) /
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

//-----------------------------------------------------------------------------
int pqCMBLIDARDataImporter::calculateOnRatioForPiece(int onRatio,
                                                    int numberOfPointsInPiece)
{
  if (numberOfPointsInPiece / onRatio < this->MinimumNumberOfPointsPerPiece)
    {
    onRatio = numberOfPointsInPiece / this->MinimumNumberOfPointsPerPiece;
    }
  if (onRatio < 1)
    {
    onRatio = 1;
    }
  return onRatio;
}


//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::initialClippingSetup()
{
  pqObjectBuilder* const builder = pqApplicationCore::instance()->getObjectBuilder();
  this->OutlineSource =
    builder->createSource("sources", "OutlineSource", this->CurrentServer);

  double zPad = (this->DataBounds[5] - this->DataBounds[4]) * 0.5;
  QList<QVariant> values;
  values << this->ImportDialog->minXClip->value() <<
    this->ImportDialog->maxXClip->value() <<
    this->ImportDialog->minYClip->value() <<
    this->ImportDialog->maxYClip->value() <<
    this->DataBounds[4] - zPad << this->DataBounds[5] + zPad;

  this->ClipBBox.SetBounds(this->ImportDialog->minXClip->value(),
    this->ImportDialog->maxXClip->value(),
    this->ImportDialog->minYClip->value(),
    this->ImportDialog->maxYClip->value(),
    VTK_DOUBLE_MIN, VTK_DOUBLE_MAX);
  this->PieceMainTable->setClipBBox( this->ClipBBox );

  pqSMAdaptor::setMultipleElementProperty(
    this->OutlineSource->getProxy()->GetProperty("Bounds"), values);
  this->OutlineSource->getProxy()->UpdateVTKObjects();
  this->OutlineRepresentation =
    builder->createDataRepresentation(this->OutlineSource->getOutputPort(0),
    this->CurrentRenderView, "GeometryRepresentation");
  // initially not visible
  this->OutlineRepresentation->setVisible( false );
  //this->OutlineRepresentation->getProxy()->UpdateVTKObjects();
  //this->OutlineRepresentation->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
int pqCMBLIDARDataImporter::ImportLIDARData(const char *filename)
{
  if(!this->CurrentServer)
    {
    //qCritical() << "The server is not set!";
    return 0;
    }
  QFileInfo info(filename);
  if (info.exists() == false)
    {
    //qCritical() << "The LIDAR File does not exist!\n" << fileNameStr;
    return 0;
    }

  pqApplicationCore* const core = pqApplicationCore::instance();
  pqObjectBuilder* const builder = core->getObjectBuilder();

  QStringList fileList;
  fileList << filename;
  builder->blockSignals(true);
  this->ReaderSource = builder->createReader(
    "sources", "LIDARReader", fileList, this->CurrentServer );
  builder->blockSignals(false);

  if (!this->ReaderSource)
    {
    //qCritical() << "Failed to create LIDAR reader for file:\n " << fileNameStr;
    return 0;
    }

  this->CurrentRenderView = qobject_cast<pqRenderView*>(
    builder->createView(pqRenderView::renderViewType(),
                       this->CurrentServer ));
   if (!this->CurrentRenderView)
    {
    //qCritical() << "Failed to create the render window for LIDAR preview. ";
    return 0;
    }
//  this->CurrentRenderView->getRenderViewProxy()->SetUseImmediateMode(0);
  this->CurrentRenderView->setCenterAxesVisibility(false);
  pqSMAdaptor::setElementProperty(this->CurrentRenderView->
    getProxy()->GetProperty("LODThreshold"),  0);

  this->setRenderView(this->CurrentRenderView);

  this->PieceMainTable->getWidget()->blockSignals(true);
  //this->PieceOnRatioTable->getWidget()->blockSignals(true);


  int aborted;
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy());
  //source->UpdateVTKObjects();
  //source->UpdatePropertyInformation();

  QList<QVariant> pieceOnratioList;
  pieceOnratioList << 0 << VTK_INT_MAX;
  //pqSMAdaptor::setElementProperty(
  //  source->GetProperty("PieceIndex"), 0);

  pqSMAdaptor::setMultipleElementProperty(
    source->GetProperty("RequestedPiecesForRead"), pieceOnratioList);
  pqSMAdaptor::setElementProperty(
    source->GetProperty("AbortExecute"), 0);
  this->enableAbort(true); //make sure progressbar is enabled
  source->UpdateVTKObjects();
  source->UpdatePipeline();
  aborted = pqSMAdaptor::getElementProperty(
    source->GetProperty("AbortExecute")).toInt();
  if(aborted)
    {
    this->enableAbort(false);
    return 0;
    }

  QList<int> pieceInfo;
  int totalNumberOfPoints = this->getPieceInfo( pieceInfo );
  int onRatio, mainOnRatio = this->calculateMainOnRatio( totalNumberOfPoints );

  for (int pieceId = 0; pieceId < pieceInfo.count(); pieceId++)
    {
    onRatio = this->calculateOnRatioForPiece(mainOnRatio, pieceInfo[pieceId]);

    QList<QVariant> pieceOnratioList;
    pieceOnratioList << pieceId << onRatio;

    pqPipelineSource *pdSource = this->readPieces(
      this->ReaderSource, pieceOnratioList);
    aborted = pqSMAdaptor::getElementProperty(
      this->ReaderSource->getProxy()->GetProperty("AbortExecute")).toInt();
    if(aborted)
     {
     break;
     }

    if(!pdSource)
      {
      if(QMessageBox::question(this->MainDialog,
                 "LIDAR File Reading",
                 tr("Failed to load LIDAR piece: ").append(QString::number(pieceId)).append(
                 ". Continue to load other pieces?"),
                 QMessageBox::Yes|QMessageBox::No, QMessageBox::No) ==
                 QMessageBox::Yes)
        {
        continue;
        }
      else
        {
        break;
        }
      }
    int visible = pieceInfo[pieceId] > this->MinimumNumberOfPointsPerPiece ? 1 : 0;

    source->UpdatePropertyInformation();
    int numberOfPointsRead = pqSMAdaptor::getElementProperty(
      source->GetProperty("RealNumberOfOutputPoints")).toInt();
    QList<QVariant> values = pqSMAdaptor::getMultipleElementProperty(
      source->GetProperty("DataBounds"));
    double bounds[6] = { values[0].toDouble(), values[1].toDouble(), values[2].toDouble(),
      values[3].toDouble(), values[4].toDouble(), values[5].toDouble() };

    pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
      pdSource, bounds, this->CurrentServer, this->CurrentRenderView, visible);
    dataObj->setPieceIndex(pieceId);
    dataObj->setDisplayOnRatio(onRatio);
    dataObj->setReadOnRatio( onRatio );
    dataObj->setNumberOfPoints( pieceInfo[pieceId] );
    dataObj->setNumberOfReadPoints( numberOfPointsRead );
    dataObj->setNumberOfDisplayPointsEstimate( numberOfPointsRead );
    dataObj->setNumberOfSavePointsEstimate( pieceInfo[pieceId] );


    this->PieceMainTable->AddLIDARPiece(dataObj, visible);
    //this->PieceOnRatioTable->AddLIDARPiece(dataObj, visible);

    this->PieceIdObjectMap[pieceId] = dataObj;
    // The following effect is like OnPreviewSelected();
    if(visible) // update the render window
      {
      this->CurrentRenderView->resetCamera();
      this->CurrentRenderView->render();
      }
    }

  this->PieceMainTable->getWidget()->blockSignals(false);
  if (this->PieceMainTable->getWidget()->rowCount() < 2)
    {
    this->ImportDialog->saveAsSinglePiece->setEnabled(false);
    }
  else
    {
    this->ImportDialog->saveAsSinglePiece->setEnabled(true);
    }
  //this->PieceOnRatioTable->getWidget()->blockSignals(false);

  this->enableAbort(false);
  return (this->PieceIdObjectMap.count()>0);
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARDataImporter::readPieces(
  pqPipelineSource* reader,  QList<QVariant> pieces)
{
  if(reader && pieces.count()>1)
    {
    this->readData(reader, pieces);
    int aborted = pqSMAdaptor::getElementProperty(
      reader->getProxy()->GetProperty("AbortExecute")).toInt();
    if(aborted)
     {
     return NULL;
     }

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* const builder = core->getObjectBuilder();
    pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", this->CurrentServer);
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(reader->getProxy()));
    return pdSource;
    }

  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::readData(pqPipelineSource* reader,
                                     QList<QVariant> pieces)
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(
    reader->getProxy());
  pqSMAdaptor::setMultipleElementProperty(
    source->GetProperty("RequestedPiecesForRead"), pieces);
  pqSMAdaptor::setElementProperty(
    source->GetProperty("AbortExecute"), 0);

  this->enableAbort(true); //make sure progressbar is enabled
  source->UpdateVTKObjects();
  source->UpdatePipeline();
}

//-----------------------------------------------------------------------------
//void pqCMBLIDARDataImporter::reReadSelectedPieces()
//{
//  pqCMBLIDARPieceObject* dataObj;
//  QList<QVariant> pieceOnRatioList;
//
//  QList<pqCMBLIDARPieceObject*> visiblePieces =
//    this->PieceMainTable->getVisiblePieceObjects();
//  for(int i = 0; i < visiblePieces.count(); i++)
//    {
//    dataObj = visiblePieces[i];
//    pieceOnRatioList << dataObj->getPieceIndex() << dataObj->getDisplayOnRatio();
//    }
//  this->readData(this->ReaderSource, pieceOnRatioList);
//}

//-----------------------------------------------------------------------------
//void pqCMBLIDARDataImporter::onSavePiecesAsDisplayed()
//{
//  // use Display onRatio
//  this->onSavePieces(0);
//}
//
////-----------------------------------------------------------------------------
//void pqCMBLIDARDataImporter::onAcceptToLoadAsDisplayed()
//{
//  // use Display onRatio
//  this->onAcceptToLoad(0);
//}
//
////-----------------------------------------------------------------------------
//void pqCMBLIDARDataImporter::onSavePiecesAsOriginal()
//{
//  // use Original onRatio
//  this->onSavePieces(1);
//}
//
////-----------------------------------------------------------------------------
//void pqCMBLIDARDataImporter::onAcceptToLoadAsOriginal()
//{
//  // use Original onRatio
//  this->onAcceptToLoad(1);
//}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::isNewFileNeeded()
{
  // if all the files are not selected or clipping or displayOnRatio != 1

  if (this->ImportDialog->clipGroupBox->isChecked())
    {
    return true;
    }

  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() != this->PieceMainTable->getWidget()->rowCount())
    {
    return true;
    }

  bool loadAsDisplayed = this->ImportDialog->loadAsDisplayed->isChecked();
  for (int i = 0; i < visiblePieces.count(); i++)
    {
    if ((loadAsDisplayed && visiblePieces[i]->getDisplayOnRatio() != 1) ||
      (!loadAsDisplayed && visiblePieces[i]->getSaveOnRatio() != 1) ||
      visiblePieces[i]->isPieceTransformed())
      {
      return true;
      }
    }

  // if we have more than one piece but want to write as one piece
  if (visiblePieces.count() > 1 && this->ImportDialog->saveAsSinglePiece->isChecked())
    {
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
int pqCMBLIDARDataImporter::onSavePieces(int onRatio,bool askMultiOutput)
{
  //QList<pqCMBLIDARPieceObject*> visiblePieces =
  //  this->PieceMainTable->getVisiblePieceObjects();
  //if(visiblePieces.count()>0)
  //  {
  //  bool multiOutput = false;
  //  if(askMultiOutput && visiblePieces.count() > 1 &&
  //     QMessageBox::question(this->MainDialog,
  //               "Save each piece seperately?",
  //               "Do you want to save each selected piece as a seperate file?",
  //               QMessageBox::Yes|QMessageBox::No, QMessageBox::No) ==
  //               QMessageBox::Yes)
  //    {
  //    multiOutput = true;
  //    }

  //  QString filters = "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);;";
  //  pqFileDialog file_dialog(
  //    this->CurrentServer, this->MainDialog,
  //    tr("Save LIDAR Pieces:"), QString(), filters);
  //  file_dialog.setObjectName("LIDARFileSaveDialog");
  //  file_dialog.setFileMode(pqFileDialog::AnyFile);
  //  if (file_dialog.exec() == QDialog::Accepted)
  //    {
  //    QStringList files = file_dialog.getSelectedFiles();
  //    if (files.size() > 0)
  //      {
  //      this->enableAbort(true);
  //      this->savePieces(visiblePieces, files[0], multiOutput);
  //      this->enableAbort(false);
  //      return 1;
  //      }
  //    }
  //  }
  //
  return 0;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onAcceptToLoad()
{
  this->OutputFileName.clear();
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() > 0 && this->isNewFileNeeded())
    {
    QString filters = "LIDAR ASCII (*.pts);; LIDAR binary (*.bin.pts);; VTK PolyData (*.vtp);;";
    pqFileDialog file_dialog(
      this->CurrentServer, this->MainDialog,
      tr("Save LIDAR Pieces:"), QString(), filters);
    file_dialog.setObjectName("LIDARFileSaveDialog");
    file_dialog.setFileMode(pqFileDialog::AnyFile);
    if (file_dialog.exec() == QDialog::Accepted)
      {
      QStringList files = file_dialog.getSelectedFiles();
      if (files.size() > 0)
        {
        this->enableAbort(true);
        if ( !this->savePieces(visiblePieces, files[0]) )
          {
          // write cancelled?
          this->enableAbort(false);
          return;
          }
        this->OutputFileName = files[0].toAscii().constData();
        this->enableAbort(false);
        }
      }
    else
      {
      return;
      }
    }
  else
    {
    this->OutputFileName = this->FileName;
    }
  this->MainDialog->accept();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onUpdateSelectedPieces()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  if(visiblePieces.count()>0)
    {
    this->enableAbort(true);

    this->updatePieceRepresentations(visiblePieces);

    this->enableAbort(false);
    this->updateLoadAndUpdateButtons();
    }
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::isUpdateNeeded()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  for(int i = 0; i < visiblePieces.count(); i++)
    {
    if ( !this->isObjectUpToDate(visiblePieces[i]) )
      {
      return true;
      }
    }

  return false;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARDataImporter::loadPieces(
  QList<pqCMBLIDARPieceObject*> pieces, int onRatio)
{
// HERE we need some logic to make it user-friendly
// 1. If the pts file already saved with the onRatio, ask user for that file?
// 2. If no pts file specified, go ahead and save it
// 3. qtCMBSceneObjectImporter needs a way to get the new pts file name, so that
//    it can set the filename to the LIDAR object created.
//  if(!this->onSavePieces(onRatio, false))
//    {
//    }

  pqPipelineSource* appendPoly = NULL;
  if(onRatio > 0)
    {

    // HERE Could be smarter. For example, we could figure out what pieces' onRatios
    // are different from the input onRatio, and only read those with the new onRatio,
    // then we do appendPoly.

    bool needRead = false;
    QList<QVariant> pieceOnratioList;
    pqCMBLIDARPieceObject* dataObj = NULL;
    for(int i=0; i<pieces.count();  i++)
      {
      dataObj = pieces[i];
      if(dataObj)
        {
        pieceOnratioList << dataObj->getPieceIndex() << onRatio;
        }
      if(!needRead)
        {
        needRead = (dataObj->getDisplayOnRatio() != onRatio) ? true : false;
        }
      }

    appendPoly = needRead ? this->readPieces(this->ReaderSource, pieceOnratioList) :
                 this->appendPieces(pieces);
    }
  else // loading as currently displayed
    {
    if(pieces.count() == 1)
      {
      appendPoly = pieces[0]->getSource();
      }
    else if( pieces.count() > 1)
      {
      appendPoly = this->appendPieces(pieces);
      }
    }

  return appendPoly;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARDataImporter::appendPieces(
  QList<pqCMBLIDARPieceObject*> pieces)
{
  pqPipelineSource* appendPoly = NULL;
  QList<pqOutputPort*> inputs;
  for(int i=0; i<pieces.count();  i++)
    {
    pqCMBLIDARPieceObject* dataObj = pieces[i];
    if(dataObj)
      {
      inputs.push_back(dataObj->getSource()->getOutputPort(0));
      }
    }

  if(inputs.count() > 0)
    {
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = inputs;

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    appendPoly = builder->createFilter(
      "filters", "AppendPolyData", namedInputs, this->CurrentServer );

    // Since the vtkAppendPolyData only supports Progress, not "Abort" yet,
    // we have to disable AbortButton
    this->ProgressBar->enableProgress(true);
    this->enableButtons(false);
    this->ProgressBar->setProgress(QString("Appending pieces ..."), 0.5);

    appendPoly->updatePipeline();
    }

  return appendPoly;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::savePieces(
  QList<pqCMBLIDARPieceObject*> pieces, const QString& filename,
  bool multiOutput)
{
  QList<QString> outFileNames;
  if(pieces.count() >1 && multiOutput &&
    !this->generateAndValidateOutFileNames(pieces, filename, outFileNames))
    {
    return false;
    }

  QFileInfo info(filename);
  QString extension(info.completeSuffix()), WriterName;
  if(extension == "pts" || extension == "bin.pts")
    {
    WriterName = "LIDARWriter";
    }
  else if(extension == "vtp")
    {
    WriterName = "XMLPolyDataWriter";
    }
  else
    {
    QString message("The file extension, ");
    message.append(extension).append(", is not supported for writing LIDAR file.");
    QMessageBox::warning(this->MainDialog,
      tr("File Save Warning"),message, QMessageBox::Ok);

    return false;
    }



  bool loadAsDisplayed = this->ImportDialog->loadAsDisplayed->isChecked();

  int onRatio;

   // update pieces if necessary
  QList<pqPipelineSource*> savePieces;
  pqCMBLIDARPieceObject* dataObj = NULL;
  pqPipelineSource* source = NULL;
  vtkSMSourceProxy* readerSource =
    vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy());

  for(int i=0; i<pieces.count();  i++)
    {
    dataObj = pieces[i];
    source = dataObj->getSource();

    if (loadAsDisplayed)
      {
      onRatio = dataObj->getDisplayOnRatio();
      }
    else
      {
      onRatio = dataObj->getSaveOnRatio();
      }

    if (dataObj->isPieceTransformed())
      {
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      dataObj->getTransform( transform );
      vtkSMPropertyHelper(this->ReaderSource->getProxy(),
        "Transform").Set(transform->GetMatrix()->Element[0], 16);
      // want to transform the data for output from the reader
      vtkSMPropertyHelper(this->ReaderSource->getProxy(),
        "TransformOutputData").Set(true);
      }

    // if piece is transformed, need to transform the output of the reader,
    // which we "normally" don't do (the actor maintains the transformation)
    if(onRatio != dataObj->getReadOnRatio() || dataObj->isPieceTransformed())
      {
      // read the piece again with new onRatio
      QList<QVariant> pieceOnratioList;
      pieceOnratioList << dataObj->getPieceIndex() << onRatio;
      source = this->readPieces(this->ReaderSource, pieceOnratioList);
      if (!source)
        {
        return false;  // read must have been cancelled
        }
      readerSource->UpdatePropertyInformation();
      int numberOfPointsRead = pqSMAdaptor::getElementProperty(
        readerSource->GetProperty("RealNumberOfOutputPoints")).toInt();
      dataObj->setNumberOfReadPoints( numberOfPointsRead );
      dataObj->setReadOnRatio( onRatio );
      }
    if (dataObj->getNumberOfReadPoints() > 0)
      {
      savePieces.push_back(source);
      }
    }

  // to get rid of lingering events that may undo our enabling of the ProgressBar
  QCoreApplication::processEvents();

  this->ProgressBar->setProgress(
    QString("Writing pieces ..."), 0);
  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();
  if(savePieces.count() == 1)
    {
    source = savePieces[0];
    return this->WritePiece(source, WriterName, filename);
    }
  else if( savePieces.count() > 1)
    {
    return this->WritePieces(savePieces, WriterName, filename,
      this->ImportDialog->saveAsSinglePiece->isChecked());
    //source = this->appendPieces(savePieces);
    }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::WritePiece(
  pqPipelineSource* source, const QString& writerName,
  const QString& fileName)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->CurrentWriter = builder->createFilter("writers",
      writerName.toStdString().c_str(), source);
  return this->WriteFile(fileName);
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::WritePieces(QList<pqPipelineSource*> pieces,
  const QString& writerName, const QString& fileName, bool writeAsSinglePiece)
{
  QList<pqOutputPort*> inputs;
  for(int i=0; i<pieces.count();  i++)
    {
    if(pieces[i])
      {
      inputs.push_back(pieces[i]->getOutputPort(0));
      }
    }

  if(inputs.count() > 0)
    {
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = inputs;

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->CurrentWriter = builder->createFilter(
      "writers", writerName.toStdString().c_str(),
      namedInputs, this->CurrentServer );
    if (writeAsSinglePiece)
      {
      vtkSMPropertyHelper(this->CurrentWriter->getProxy(),
        "WriteAsSinglePiece").Set(true);
      }

    return this->WriteFile(fileName);
    }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::WriteFile(const QString& fileName)
{
  this->enableAbort(true);
  if (!this->CurrentWriter)
    {
    qCritical() << "Failed to create LIDAR points writer! ";
    return false;
    }
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqSMAdaptor::setElementProperty(
    this->CurrentWriter->getProxy()->GetProperty("FileName"),
    fileName.toStdString().c_str());
  pqSMAdaptor::setElementProperty(
    this->CurrentWriter->getProxy()->GetProperty("AbortExecute"), 0);

  this->CurrentWriter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->CurrentWriter->getProxy())->UpdatePipeline();
  int aborted = pqSMAdaptor::getElementProperty(
    this->CurrentWriter->getProxy()->GetProperty("AbortExecute")).toInt();
  if(aborted)
    {
    return false;
    }

  builder->destroy(this->CurrentWriter);
  this->CurrentWriter = NULL;
  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::generateAndValidateOutFileNames(
  QList<pqCMBLIDARPieceObject*> pieces,
  const QString& filename, QList <QString>& outFiles)
{
  QFileInfo info(filename);
  QString outputPath(info.absolutePath()), outputFileName;
  outputPath.append("/").append(info.baseName()).append("_piece");
  QFileInfo outFile;
  pqCMBLIDARPieceObject* dataObj = NULL;
  bool yesToAll = false;
  outFiles.clear();
  for(int i=0; i<pieces.count(); i++)
    {
    dataObj = pieces[i];
    outputFileName = outputPath +
      QString::number(dataObj->getPieceIndex()) + "." + info.completeSuffix();
    outFile.setFile(outputFileName);
    if(outFile.exists() && !yesToAll)
      {
      int answer = QMessageBox::question(this->MainDialog,
                 "Overwrite Existing File?",
                 "The following file already exists. Do you want to overwrite it? \n"
                 + QString(outputFileName),
                 QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No, QMessageBox::No) ;
      if(answer == QMessageBox::No)
        {
        return false;
        }
      else if( answer == QMessageBox::YesToAll)
        {
        yesToAll = true;
        }
      }
    outFiles.push_back(outputFileName);
    }

  if(pieces.count() != outFiles.count())
    {
    QMessageBox::warning(this->MainDialog,
      tr("File Save Error"),
      tr("The file names for multi-output are not valid for all pieces"),
      QMessageBox::Ok);
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARDataImporter::isObjectUpToDate(pqCMBLIDARPieceObject* dataObj)
{
  if (!dataObj)
    {
    return true; // well, no object... but
    }

  return dataObj->isObjectUpToDate(this->ImportDialog->clipGroupBox->isChecked(),
    this->ClipBBox);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updatePieceRepresentations(
  QList<pqCMBLIDARPieceObject*> pieces)
{
  pqCMBLIDARPieceObject* dataObj = NULL;

  // setup clipping
  double zClip[2];
  if (this->ImportDialog->clipGroupBox->isChecked())
    {
    //double zPad = (this->DataBounds[5] - this->DataBounds[4]) * 0.5;
    //zClip[0] = this->DataBounds[4] - zPad;
    //zClip[1] = this->DataBounds[5] + zPad;
    zClip[0] = VTK_DOUBLE_MIN;
    zClip[1] = VTK_DOUBLE_MAX;

    QList<QVariant> values;
    values << this->ImportDialog->minXClip->value() <<
      this->ImportDialog->maxXClip->value() <<
      this->ImportDialog->minYClip->value() <<
      this->ImportDialog->maxYClip->value() <<
      zClip[0] << zClip[1];
    pqSMAdaptor::setMultipleElementProperty(this->ReaderSource->
      getProxy()->GetProperty("ReadBounds"), values);
    }

  vtkSMSourceProxy* readerSource =
    vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy());
  for(int i=0; i<pieces.count();  i++)
    {
    dataObj = pieces[i];
    if(!this->isObjectUpToDate(dataObj))
      {
      QList<QVariant> pieceOnratioList;
      pieceOnratioList << dataObj->getPieceIndex() <<
        dataObj->getDisplayOnRatio();
      // only need to do trasnformation in the reader if we're clipping the data
      if (dataObj->isPieceTransformed() && this->ImportDialog->clipGroupBox->isChecked())
        {
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        dataObj->getTransform( transform );
        vtkSMPropertyHelper(this->ReaderSource->getProxy(),
                      "Transform").Set(transform->GetMatrix()->Element[0], 16);
        // don't want to transform the data for output from the reader, but do
        // want to transform the to see if in the ReadBounds
        vtkSMPropertyHelper(this->ReaderSource->getProxy(),
                      "TransformOutputData").Set(false);
        this->ReaderSource->getProxy()->UpdateVTKObjects();
        }
      else
        {
        this->ReaderSource->getProxy()->InvokeCommand("ClearTransform");
        }

      this->readData(this->ReaderSource, pieceOnratioList);
      int aborted = pqSMAdaptor::getElementProperty(
        this->ReaderSource->getProxy()->GetProperty("AbortExecute")).toInt();
      if(aborted)
        {
        break;
        }
      dataObj->setReadOnRatio( dataObj->getDisplayOnRatio() );
      if ( this->ImportDialog->clipGroupBox->isChecked() )
        {
        dataObj->setClipBounds( this->ImportDialog->minXClip->value(),
          this->ImportDialog->maxXClip->value(),
          this->ImportDialog->minYClip->value(),
          this->ImportDialog->maxYClip->value(), zClip[0], zClip[1]);
        dataObj->setClipState( true );
        dataObj->saveClipPosition();
        dataObj->saveClipOrientation();
        dataObj->saveClipScale();
        dataObj->saveClipOrigin();
        }
      else
        {
        dataObj->setClipState( false );
        }

      readerSource->UpdatePropertyInformation();
      int numberOfPointsRead = pqSMAdaptor::getElementProperty(
        readerSource->GetProperty("RealNumberOfOutputPoints")).toInt();
      dataObj->setNumberOfReadPoints( numberOfPointsRead );

      vtkSMDataSourceProxy* pdSource =
        vtkSMDataSourceProxy::SafeDownCast(
        dataObj->getSource()->getProxy());
      pdSource->CopyData(readerSource);
      pdSource->UpdatePipeline();

      this->PieceMainTable->computeDisplayNumberOfPointsEstimate(dataObj);
      this->PieceMainTable->computeSaveNumberOfPointsEstimate(dataObj);
      this->PieceMainTable->updateWithPieceInfo(dataObj);
      }
    }

  this->updatePointTotals();
  this->CurrentRenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::setupProgressBar()
{
  this->ProgressBar = new qtCMBProgressWidget();
  this->ProgressBar->setObjectName("fileReadingProgress");

  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();

  //QObject::connect(progress_manager, SIGNAL(enableProgress(bool)),
  //                 this->ProgressBar,     SLOT(enableProgress(bool)));

  //QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
  //                 this->ProgressBar,     SLOT(setProgress(const QString&, int)));
  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
                   this, SLOT(updateProgress(const QString&, int)));

  QObject::connect(progress_manager, SIGNAL(enableAbort(bool)),
                   this->ProgressBar,      SLOT(enableAbort(bool)));

  QObject::connect(this->ProgressBar, SIGNAL(abortPressed()),
                   progress_manager, SLOT(triggerAbort()));
  QObject::connect(progress_manager, SIGNAL(abort()),
                   this, SLOT(abort()));


 // progress_manager->addNonBlockableObject(this->ImportDialog);
  progress_manager->addNonBlockableObject(this->ProgressBar);
  progress_manager->addNonBlockableObject(this->ProgressBar->getAbortButton());

  //this->ProgressBar->enableAbort(true);
  this->enableAbort(false);
  this->addProgressWidget(this->ProgressBar);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::enableAbort(bool abortEnabled)
{
  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();
  progress_manager->setEnableAbort(abortEnabled);
  this->ProgressBar->enableProgress(abortEnabled);
  QCoreApplication::processEvents();

  this->enableButtons(!abortEnabled);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::abort()
{
  vtkSMSourceProxy* source = NULL;
  if(this->CurrentWriter)
    {
    source = vtkSMSourceProxy::SafeDownCast(
          this->CurrentWriter->getProxy());
    }
  else if(this->ReaderSource)
    {
    source = vtkSMSourceProxy::SafeDownCast(
          this->ReaderSource->getProxy());
    }

  if(source)
    {
    pqSMAdaptor::setElementProperty(
      source->GetProperty("AbortExecute"), 1);
    source->UpdateVTKObjects();
    }

  //this->enableAbort(false);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateProgress(const QString& text, int progress)
{
  this->ProgressBar->setProgress(text, progress);
  QCoreApplication::processEvents();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::addProgressWidget(QWidget* progressBar)
{
  if(!progressBar)
    {
    return;
    }

  progressBar->setParent(this->ImportDialog->ToolbarFrame);
  if(this->ImportDialog->ToolbarFrame->layout())
    {
    delete this->ImportDialog->ToolbarFrame->layout();
    }
  QHBoxLayout* toollayout = new QHBoxLayout(this->ImportDialog->ToolbarFrame);
  toollayout->setMargin(0);
  toollayout->addWidget(progressBar);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateRepresentationWidget(
  pqDataRepresentation* dataRep)
  {
  if(!dataRep)
    {
    return;
    }
  if(this->RepresentationWidget)
    {
    delete this->RepresentationWidget;
    this->RepresentationWidget = NULL;
    }

  this->RepresentationWidget =
    new pqCMBEnumPropertyWidget(this->ImportDialog->ToolbarFrame)
    << pqSetName("previewRepresentation");
  this->RepresentationWidget->setPropertyName("Representation");
  this->RepresentationWidget->setLabelText("Change representation style");
  this->RepresentationWidget->setRepresentation(dataRep);

  if(this->ImportDialog->ToolbarFrame->layout())
    {
    delete this->ImportDialog->ToolbarFrame->layout();
    }
  QVBoxLayout* toollayout = new QVBoxLayout(this->ImportDialog->ToolbarFrame);
  toollayout->setMargin(0);
  toollayout->addWidget(this->RepresentationWidget);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::setRenderView(pqRenderView* renderView)
  {
  if(!renderView)
    {
    return;
    }

  if(this->ImportDialog->RenderViewFrame->layout())
    {
    delete this->ImportDialog->RenderViewFrame->layout();
    }

  QVBoxLayout* vboxlayout = new QVBoxLayout(this->ImportDialog->RenderViewFrame);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(renderView->getWidget());
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::OnPreviewSelected()
{
  //this->ImportDialog->PieceSlider->setEnabled(!checked);
  //this->ImportDialog->PieceRatioBox->setEnabled(!checked);
  //this->ImportDialog->DataPieceInfoTable->setSelectionMode(
  //  checked ? QAbstractItemView::NoSelection :
  //  QAbstractItemView::SingleSelection);
  //QList<pqCMBLIDARPieceObject*> pieces =
  //  this->PieceMainTable->getVisiblePieceObjects();
  //if(pieces.count()==0)
  //  {
  //  return;
  //  }
//  this->setObjectsVisibility(pieces, 1);
  this->updateFocus(true);
  this->CurrentRenderView->resetCamera();
  this->CurrentRenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::setObjectsVisibility(
  QList<pqCMBLIDARPieceObject*> pieces, int visible)
{
  for(int i=0; i<this->PieceIdObjectMap.count(); i++)
    {
    this->PieceIdObjectMap[i]->setVisibility(
      pieces.contains(this->PieceIdObjectMap[i]) ? visible : !visible);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::enableButtons(bool enabled)
{
  this->ImportDialog->PieceFrame->setEnabled(enabled);
  this->ImportDialog->LoadCancelFrame->setEnabled(enabled);
  this->ImportDialog->clipGroupBox->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onSetSliderPosition(
  int sliderPos)
{
  //int pieceIdx = sliderPos-1;
  //int onRatio = this->PieceIdObjectMap[pieceIdx]->getDisplayOnRatio();
  //this->ImportDialog->PieceLabel->setText(
  //  QString("Piece ").append(QString::number(pieceIdx)));
  //if(this->ImportDialog->PieceRatioBox->value() != onRatio)
  //  {
  //  this->ImportDialog->PieceRatioBox->blockSignals(true);
  //  this->ImportDialog->PieceRatioBox->setValue(onRatio);
  //  this->ImportDialog->PieceRatioBox->blockSignals(false);
  //  }

  //pqCMBLIDARPieceObject* currObj = this->PieceOnRatioTable->getCurrentObject();
  //if(!currObj || currObj->getPieceIndex() != pieceIdx)
  //  {
  //  this->PieceOnRatioTable->setCurrentPiece(pieceIdx);
  //  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onCurrentPieceRatioChanged()
{
  //this->PieceOnRatioTable->setCurrentOnRatio(
  //  this->ImportDialog->PieceRatioBox->value());
  //this->PieceMainTable->updateOnRatioOfPiece(
  //  this->ImportDialog->PieceSlider->value(), onRatio);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onEnableClip()
{
  bool clipEnabled = this->ImportDialog->clipGroupBox->isChecked();
  this->PieceMainTable->setClipEnabled( clipEnabled );
  pqSMAdaptor::setElementProperty(
    vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy())->
    GetProperty("LimitReadToBounds"), clipEnabled);
  //if (clipEnabled)
  //  {
  //  if (!this->OutlineSource)
  //    {
  //    this->initialClippingSetup();
  //    }
  //  }

  // update all, even those unselected (invisible)
  for (int i = 0; i < this->PieceMainTable->getWidget()->rowCount(); i++)
    {
    this->PieceMainTable->updateWithPieceInfo(i);
    }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons();


  //pqSMAdaptor::setElementProperty(
  //  this->OutlineRepresentation->getProxy()->GetProperty("Visibility"),
  //  this->ImportDialog->clipGroupBox->isChecked() );
  this->OutlineRepresentation->setVisible( clipEnabled );
  //rep->getRepresentationProxy()->SetSelectionVisibility(visible);
  //this->OutlineRepresentation->getProxy()->UpdateVTKObjects();

  this->CurrentRenderView->render();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onClippingBoxChanged()
{
  if (this->OutlineSource)
    {
    double zPad = (this->DataBounds[5] - this->DataBounds[4]) * 0.5;
    QList<QVariant> values;
    values << this->ImportDialog->minXClip->value() <<
      this->ImportDialog->maxXClip->value() <<
      this->ImportDialog->minYClip->value() <<
      this->ImportDialog->maxYClip->value() <<
      this->DataBounds[4] - zPad << this->DataBounds[5] + zPad;
    pqSMAdaptor::setMultipleElementProperty(this->OutlineSource->
      getProxy()->GetProperty("Bounds"), values);
    this->OutlineSource->getProxy()->UpdateVTKObjects();
    this->CurrentRenderView->render();

    this->ClipBBox.SetBounds(this->ImportDialog->minXClip->value(),
      this->ImportDialog->maxXClip->value(),
      this->ImportDialog->minYClip->value(),
      this->ImportDialog->maxYClip->value(),
      VTK_DOUBLE_MIN, VTK_DOUBLE_MAX);
    this->PieceMainTable->setClipBBox( this->ClipBBox );

    for (int i = 0; i < this->PieceMainTable->getWidget()->rowCount(); i++)
      {
      this->PieceMainTable->updateWithPieceInfo(i);
      }
    }
  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(false); // don't change focus
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::applyTargetNumberOfPoints()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  if (visiblePieces.count() < 1)
    {
    this->updateFocus();
    return;
    }

  vtkSMSourceProxy* source =
    vtkSMSourceProxy::SafeDownCast(this->ReaderSource->getProxy());
  int totalNumberOfPoints = 0;
  for (int i = 0; i < visiblePieces.count(); i++)
    {
    totalNumberOfPoints += visiblePieces[i]->getNumberOfPoints();
    }

  int mainOnRatio = this->calculateMainOnRatio( totalNumberOfPoints );

  int oldRatio;
  for (int i = 0; i < visiblePieces.count(); i++)
    {
    oldRatio = visiblePieces[i]->getDisplayOnRatio();
    visiblePieces[i]->setDisplayOnRatio(
      this->calculateOnRatioForPiece(mainOnRatio, visiblePieces[i]->getNumberOfPoints()) );
    if (visiblePieces[i]->getReadOnRatio() != visiblePieces[i]->getDisplayOnRatio() ||
      oldRatio != visiblePieces[i]->getDisplayOnRatio())
      {
      this->PieceMainTable->updateWithPieceInfo(visiblePieces[i]);
      }
    }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::zoomSelection()
{
  pqCMBLIDARPieceObject *currentObject = this->PieceMainTable->getCurrentObject();
  if (currentObject)
    {
    currentObject->zoomOnObject();
    this->CurrentRenderView->render();
    }
  this->updateFocus(true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::clearSelection(bool updateFocusFlag/*=true*/)
{
  this->ImportDialog->tabDisplay->setEnabled(0);
  pqCMBLIDARPieceObject *currentObject =
    this->PieceMainTable->getCurrentObject(false);
  if (currentObject && currentObject->getHighlight())
    {
    currentObject->setHighlight(0);
    this->CurrentRenderView->render();
    }
  this->PieceMainTable->onClearSelection();
  if (updateFocusFlag)
    {
    this->updateFocus();
    }
  this->LastSelectedObject = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::selectAll()
{
  this->PieceMainTable->checkAll();
  this->updateFocus(true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::unselectAll()
{
  this->PieceMainTable->uncheckAll();
  this->clearSelection();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onObjectsCheckStateChanged(QList<int>, QList<int>)
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();

  int hasSelection = visiblePieces.count() > 0 ? 1 : 0;
  this->updateSelectionButtons(hasSelection);

  // go through all the selected pieces and make sure they are up-to-date in the table
  for (int i = 0; i < visiblePieces.count(); i++)
    {
    this->PieceMainTable->updateWithPieceInfo(visiblePieces[i]);
    }

  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(true, true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onAdvancedCheckBox(int advancedState)
{
  this->PieceMainTable->configureTable(advancedState);
  this->updateFocus(true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onObjectOnRatioChanged(
  pqCMBLIDARPieceObject* dataObj, int onRatio)
{
  //if(this->ImportDialog->PieceRatioBox->value() != onRatio)
  //  {
  //  this->ImportDialog->PieceRatioBox->setValue(onRatio);
  //  }

  this->PieceMainTable->updateWithPieceInfo(dataObj);

  // enable the Apply button
  this->updatePointTotals();
  this->updateLoadAndUpdateButtons(true, true);
//  this->ImportDialog->updateButton->setEnabled(true);
  //QList<pqCMBLIDARPieceObject*> pieces;
  //pieces.push_back(dataObj);

 //this->enableAbort(true);
 //this->updatePieceRepresentations(pieces, onRatio);
 //this->enableAbort(false);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onCurrentObjectChanged(
  pqCMBLIDARPieceObject* dataObj)
{
  if(this->LastSelectedObject != dataObj)
    {
    this->PieceMainTable->getWidget()->setFocus();
    bool renderNeeded = false;
    if (this->LastSelectedObject)
      {
      this->LastSelectedObject->setHighlight(0);
      renderNeeded = true;
      }
    if(dataObj)
      {
      //this->ImportDialog->PieceSlider->setValue(dataObj->getPieceIndex()+1);
      //dataObj->setVisibility(1);
      dataObj->setHighlight(1);
      renderNeeded = true;
      //dataObj->zoomOnObject();
      //this->ImportDialog->showSelectedButton->setChecked(false);
//      int sliderPos = dataObj->getPieceIndex()+1;
      //if(sliderPos != this->ImportDialog->PieceSlider->sliderPosition())
      //  {
      //  this->ImportDialog->PieceSlider->setSliderPosition(dataObj->getPieceIndex()+1);
      //  this->onSetSliderPosition(dataObj->getPieceIndex()+1);
      //  }
      }
    this->updateZoomAndClearState();
    if (renderNeeded)
      {
      // if we are doing a render we MAY have made a piece visible / invisible...
      // see if we have at least 2 visible pieces, in which case
      // "Save As Single Piece" needs to be enabled
      QList<pqCMBLIDARPieceObject*> visiblePieces =
        this->PieceMainTable->getVisiblePieceObjects();
      if (visiblePieces.count() < 2)
        {
        this->ImportDialog->saveAsSinglePiece->setEnabled(false);
        }
      else
        {
        this->ImportDialog->saveAsSinglePiece->setEnabled(true);
        }

      emit this->requestRender();
      //this->CurrentRenderView->render();
      }
    this->LastSelectedObject = dataObj;
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::setupSliderBar()
{
  //this->ImportDialog->PieceSlider->setRange(1,
  //  this->PieceIdObjectMap.count());
  //this->onSetSliderPosition(this->PieceIdObjectMap.keys()[0]+1);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateSelectionButtons(int hasSelection)
{
//  this->ImportDialog->applyRatioButton->setEnabled(hasSelection);
  //this->ImportDialog->showSelectedButton->setEnabled(hasSelection);
  //this->ImportDialog->saveButtonAsDisplayed->setEnabled(hasSelection);
  //this->ImportDialog->saveButtonAsOriginal->setEnabled(hasSelection);
  //this->ImportDialog->loadButtonAsDisplayed->setEnabled(hasSelection);
  //this->ImportDialog->loadButtonAsOriginal->setEnabled(hasSelection);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateLoadAndUpdateButtons(
  bool shouldUpdateFocus/*=true*/, bool focusOnTableIfRowIsSelected/*=false*/)
{
  bool updateNeeded = this->isUpdateNeeded();

  this->ImportDialog->updateButton->setEnabled( updateNeeded );
  this->ImportDialog->loadButton->setEnabled( !updateNeeded );

  this->ImportDialog->updateButton->setBackgroundRole(
    updateNeeded ? QPalette::Highlight : QPalette::Button);
  if (shouldUpdateFocus)
    {
    this->updateFocus(focusOnTableIfRowIsSelected);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateFocus(bool focusOnTableIfRowIsSelected/*=false*/)
{
  if ((focusOnTableIfRowIsSelected && this->PieceMainTable->getCurrentRow() >= 0) ||
    !this->isUpdateNeeded())
    {
    this->PieceMainTable->getWidget()->setFocus();
    }
  else
    {
    this->clearSelection(false);
    this->ImportDialog->updateButton->setFocus();
    }

  this->updateZoomAndClearState();

}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updateZoomAndClearState()
{
  // enable/disable zoom/clear selection buttons depending on whether
  // a piece is selected
  if (this->PieceMainTable->getCurrentObject() &&
    this->PieceMainTable->getCurrentObject()->getVisibility())
    {
    this->ImportDialog->clearSelection->setEnabled(true);
    this->ImportDialog->zoomSelection->setEnabled(true);
    }
  else
    {
    this->ImportDialog->zoomSelection->setEnabled(false);
    this->ImportDialog->clearSelection->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::updatePointTotals()
{
  QList<pqCMBLIDARPieceObject*> visiblePieces =
    this->PieceMainTable->getVisiblePieceObjects();
  int totalDisplayPoints = 0;
  int totalSavePoints = 0;
  bool allObjectsUpToDate = true;
  bool clipEnabled = this->ImportDialog->clipGroupBox->isChecked();
  bool allObjectsClipUpToDate = true;
  bool allSaveOnRatioEqualOne = true;
  for (int i = 0; i < visiblePieces.count(); i++)
    {
    totalDisplayPoints += visiblePieces[i]->getNumberOfDisplayPointsEstimate();
    totalSavePoints += visiblePieces[i]->getNumberOfSavePointsEstimate();

    if (allObjectsUpToDate &&
      !visiblePieces[i]->isObjectUpToDate(clipEnabled, this->ClipBBox))
      {
      allObjectsUpToDate = false;
      }
    if (allObjectsClipUpToDate &&
      !visiblePieces[i]->isClipUpToDate(clipEnabled, this->ClipBBox))
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
    this->ImportDialog->displayPointsLabel->setForegroundRole(QPalette::Highlight);
    }
  else
    {
    this->ImportDialog->displayPointsLabel->setForegroundRole(QPalette::WindowText);
    }

  // will be pretty good estimate for Save... unless clip not up-to-date
  if (clipEnabled && !allObjectsClipUpToDate)
    {
    this->ImportDialog->savePointsLabel->setForegroundRole(QPalette::Highlight);
    }
  else
    {
    this->ImportDialog->savePointsLabel->setForegroundRole(QPalette::WindowText);
    }

  this->ImportDialog->displayPointsLabel->setText( QString::number(totalDisplayPoints) );
  this->ImportDialog->savePointsLabel->setText( QString::number(totalSavePoints) );
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onRequestRender()
{
  this->RenderNeeded = true;
  emit this->renderRequested();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onRenderRequested()
{
  if (this->RenderNeeded)
    {
    this->RenderNeeded = false;
    this->CurrentRenderView->render();
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::setupAppearanceEditor (QWidget *parent)
{
  QVBoxLayout* vboxlayout = new QVBoxLayout(parent);
  vboxlayout->setMargin(0);
  QWidget* container = new QWidget();
  container->setObjectName("scrollWidget");
  container->setSizePolicy(QSizePolicy::Ignored,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(parent);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(container);
  vboxlayout->addWidget(s);

  (new QVBoxLayout(container))->setMargin(0);
  this->AppearanceEditorContainer = container;

}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onPiecesSelectionChanged(
  pqCMBLIDARPieceObject* selObject)
{
  if(selObject && selObject->getRepresentation())
    {
    this->ImportDialog->tabDisplay->setEnabled(1);
    this->onVTKConnectionChanged(selObject->getRepresentation());
    }
  else
    {
    this->ImportDialog->tabDisplay->setEnabled(0);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onVTKConnectionChanged(
  pqDataRepresentation* connRep)
{
  if(this->AppearanceEditor &&
     this->AppearanceEditor->displayRepresentation() == connRep)
    {
    return;
    }
  else
    {
    if(this->AppearanceEditor)
      {
      delete this->AppearanceEditor;
      }
    this->AppearanceEditor = new pqCMBDisplayProxyEditor(
      connRep, this->AppearanceEditorContainer->layout()->parentWidget());
    this->AppearanceEditorContainer->layout()->addWidget(
      this->AppearanceEditor);

    this->AppearanceEditor->setView(this->CurrentRenderView);
    this->AppearanceEditor->setApplyChangesImmediately(true);
    this->AppearanceEditor->filterWidgets();
    pqActiveObjects::instance().disconnect(this->AppearanceEditor);

    QObject::connect(this->AppearanceEditor, SIGNAL(changeFinished()),
      this, SLOT(onRenderRequested()));

    // Hide unrelated GUI components on the display property
    this->hideDisplayPanelPartialComponents();
    }

}

//----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::hideDisplayPanelPartialComponents()
{
  QCheckBox* visibleCheck = this->AppearanceEditorContainer->
    findChild<QCheckBox*>("ViewData");
  if(visibleCheck)
    {
    visibleCheck->hide();
    }
  QGroupBox* sliceGroup = this->AppearanceEditorContainer->
    findChild<QGroupBox*>("SliceGroup");
  if(sliceGroup)
    {
    sliceGroup->hide();
    }
  QGroupBox* colorGroup = this->AppearanceEditorContainer->
    findChild<QGroupBox*>("ColorGroup");
  if(colorGroup)
    {
    colorGroup->hide();
    }

  // Material
  QLabel* mLabel = this->AppearanceEditorContainer->
    findChild<QLabel*>("label_13");
  if(mLabel)
    {
    mLabel->hide();
    }
  QComboBox* mBox = this->AppearanceEditorContainer->
    findChild<QComboBox*>("StyleMaterial");
  if(mBox)
    {
    mBox->hide();
    }

  // volume mapper
  QLabel* vLabel = this->AppearanceEditorContainer->
    findChild<QLabel*>("label_19");
  if(vLabel)
    {
    vLabel->hide();
    }
  QComboBox* vMapperBox = this->AppearanceEditorContainer->
    findChild<QComboBox*>("SelectedMapperIndex");
  if(vMapperBox)
    {
    vMapperBox->hide();
    }
}

//----------------------------------------------------------------------------
void pqCMBLIDARDataImporter::onTabChanged(int tabIndex)
{
  // switched to main from Display... update info about the piece because
  // we may have changed the transformation
  if (tabIndex == 0 && this->LastSelectedObject)
    {
    this->PieceMainTable->updateWithPieceInfo(this->LastSelectedObject);

    this->updatePointTotals();
    this->updateLoadAndUpdateButtons(true, true);
    }
}
