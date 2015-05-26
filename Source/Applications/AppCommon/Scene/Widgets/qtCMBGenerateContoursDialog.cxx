//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBGenerateContoursDialog.h"
#include "ui_qtCMBGenerateContoursDialog.h"

#include "cmbSceneNodeReplaceEvent.h"
#include "pqCMBArc.h"
#include "pqCMBEnumPropertyWidget.h"
#include "pqCMBCommonMainWindowCore.h"
#include "qtCMBProgressWidget.h"
#include "pqCMBSceneNode.h"
#include "pqCMBUniformGrid.h"
#include "pqCMBSceneTree.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqSetName.h"

#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMProxyManager.h"

#include <QDoubleValidator>
#include <QFileInfo>
#include <QIntValidator>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QProgressDialog>

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

//-----------------------------------------------------------------------------
InternalDoubleValidator::InternalDoubleValidator(QObject * parent)
  :QDoubleValidator(parent)
{

}

//-----------------------------------------------------------------------------
void InternalDoubleValidator::fixup(QString &input) const
{
  if ( input.length() == 0 )
    {
    return;
    }

  double v = input.toDouble();
  double tol = 0.0001;
  if (v < this->bottom())
    {
    input = QString::number(this->bottom()+tol);
    }
  else if (v > this->top())
    {
    input = QString::number(this->top()-tol);
    }
}

//-----------------------------------------------------------------------------
qtCMBGenerateContoursDialog::qtCMBGenerateContoursDialog(
                                          pqCMBSceneNode *gridNode,
                                          QWidget *parent,
                                          Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  this->ParentNode = gridNode->getParent();
  this->MainDialog = new QDialog;
  this->InternalWidget = new Ui::qtCMBGenerateContoursDialog;
  this->InternalWidget->setupUi(this->MainDialog);
  this->ImageNodeName = gridNode->getName();
  QObject::connect(this->InternalWidget->generateContoursButton, SIGNAL(clicked()), this, SLOT(generateContours()));
  QObject::connect(this->InternalWidget->createContourNodesButton, SIGNAL(clicked()),
    this, SLOT(onCreateContourNodes()));
  QObject::connect(this->InternalWidget->cancelButton, SIGNAL(clicked()),
    this, SLOT(onCancel()));
  QObject::connect(this->InternalWidget->imageOpacitySlider, SIGNAL(valueChanged(int)),
    this, SLOT(onOpacityChanged(int)));

  this->InternalWidget->createContourNodesButton->setEnabled(false);
  this->InternalWidget->generateContoursBox->setEnabled(false);


  pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();
  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
                   this, SLOT(updateProgress(const QString&, int)));
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setWindowTitle(QString("Loading Uniform Grid"));
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();

  // validator for contour value
  this->ContourValidator = new InternalDoubleValidator( parent );
  this->ContourValidator->setNotation( QDoubleValidator::StandardNotation );
  this->ContourValidator->setDecimals(5);
  this->InternalWidget->contourValue->setValidator( this->ContourValidator );

  QDoubleValidator *minLineLengthValidator = new InternalDoubleValidator( parent );
  minLineLengthValidator->setBottom(0);
  this->InternalWidget->minimumLineLength->setValidator( minLineLengthValidator );


  this->InternalWidget->relativeLineLengthCheckbox->setChecked(true);
  this->InternalWidget->minimumLineLength->setText("5");

  // re-set after loading the image
  this->InternalWidget->contourValue->setText("0");

  // setup the render widget
  this->RenderView = qobject_cast<pqRenderView*>(
    pqApplicationCore::instance()->getObjectBuilder()->createView(pqRenderView::renderViewType(),
                        this->ParentNode->getTree()->getCurrentServer() ));

  QVBoxLayout* vboxlayout = new QVBoxLayout(this->InternalWidget->renderFrame);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(this->RenderView->widget());

  this->ContourRepresentation = 0;
  this->ContourSource = 0;
  this->CleanPolyLines = 0;

  this->ContourValue = VTK_FLOAT_MAX;
  this->MinimumLineLength = -1;
  this->UseRelativeLineLength = false;
  QObject::connect(this->InternalWidget->contourValue, SIGNAL(textChanged(const QString&)),
    this, SLOT(updateContourButtonStatus()));
  QObject::connect(this->InternalWidget->minimumLineLength, SIGNAL(textChanged(const QString&)),
    this, SLOT(updateContourButtonStatus()));
  QObject::connect(this->InternalWidget->relativeLineLengthCheckbox, SIGNAL(stateChanged(int)),
    this, SLOT(updateContourButtonStatus()));


  // setup Parallel projection and 2D manipulation
  pqSMAdaptor::setElementProperty(
    this->RenderView->getProxy()->GetProperty("CameraParallelProjection"), 1);
  const int *manipTypes = &TwoDManipulatorTypes[0];
  vtkSMProxy* viewproxy = this->RenderView->getProxy();
  vtkSMPropertyHelper(
    viewproxy->GetProperty("Camera2DManipulators")).Set(manipTypes, 9);
  viewproxy->UpdateVTKObjects();

  this->Grid = dynamic_cast<pqCMBUniformGrid *>
    (gridNode->getDataObject()->duplicate(this->ParentNode->getTree()->getCurrentServer(),
                                          this->RenderView ));
  this->InternalWidget->generateContoursBox->setEnabled(true);
  this->RenderView->resetCamera();
  this->RenderView->forceRender();
 // see pqProxyInformationWidget
  vtkPVDataInformation *imageInfo = this->Grid->getSource()->getOutputPort(0)->getDataInformation();
  int extents[6];
  imageInfo->GetExtent(extents);
  vtkPVDataSetAttributesInformation *pointInfo = imageInfo->GetPointDataInformation();
  double range[2] = {0, 0};
  if (pointInfo->GetNumberOfArrays() > 0)
    {
    vtkPVArrayInformation *arrayInfo = pointInfo->GetArrayInformation(0);
    if (arrayInfo->GetNumberOfComponents() == 1)
      {
      arrayInfo->GetComponentRange(0, range);
      }
    }

  this->InternalWidget->imageDimensionsLabel->setText(
    QString("Dimensions:   %1 x %2").arg(extents[1] - extents[0] + 1).arg(extents[3] - extents[2] + 1) );
  this->InternalWidget->imageScalarRangeLabel->setText(
    QString("Scalar Range:   %1, %2").arg(range[0]).arg(range[1]) );
  delete this->Progress;
  this->Progress = NULL;

   // not ready to enable abort
  //QObject::connect(this->ProgressWidget, SIGNAL(abortPressed()),
  //                 progress_manager, SLOT(triggerAbort()));
  //QObject::connect(progress_manager, SIGNAL(abort()),
  //                 this, SLOT(abort()));
}

//-----------------------------------------------------------------------------
qtCMBGenerateContoursDialog::~qtCMBGenerateContoursDialog()
{
  delete this->InternalWidget;
  if (this->MainDialog)
    {
    delete MainDialog;
    }
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if (this->ContourRepresentation)
    {
    builder->destroy(this->ContourRepresentation);
    }
  if (this->CleanPolyLines)
    {
    builder->destroy(this->CleanPolyLines);
    this->CleanPolyLines = 0;
    }
  if (this->ContourSource)
    {
    builder->destroy(this->ContourSource);
    this->ContourSource = 0;
    }
  if (this->Grid)
    {
    delete Grid;
    }
}
//-----------------------------------------------------------------------------
int qtCMBGenerateContoursDialog::exec()
{
  return this->MainDialog->exec();
}
//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::generateContours()
{
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();
  this->ProgressMessage = "Computing Contours";
  this->updateProgress(this->ProgressMessage, 0);
  this->disableWhileProcessing();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  pqServer *server = this->ParentNode->getTree()->getCurrentServer();

  // save values used for this computation, so can know whether reecute is necessary
  this->ContourValue = this->InternalWidget->contourValue->text().toDouble();
  this->MinimumLineLength = this->InternalWidget->minimumLineLength->text().toDouble();
  this->UseRelativeLineLength = this->InternalWidget->relativeLineLengthCheckbox->isChecked();

  if (!this->ContourSource)
    {
    this->ContourSource = builder->createFilter("filters",
                                                "Contour", this->Grid->getSource());
    }
  vtkSMPropertyHelper(this->ContourSource->getProxy(), "ContourValues").Set(
    this->ContourValue );
  this->ContourSource->getProxy()->UpdateVTKObjects();

  // connects lines up and get rid of short lines (with current hard coded setting, lines
  // less than 5 times the average line length are discarded)
  if (!this->CleanPolyLines)
    {
    this->CleanPolyLines = builder->createFilter("filters",
      "CleanPolylines", this->ContourSource);
    }
  vtkSMPropertyHelper(this->CleanPolyLines->getProxy(), "UseRelativeLineLength").Set(
    this->UseRelativeLineLength );
  vtkSMPropertyHelper(this->CleanPolyLines->getProxy(), "MinimumLineLength").Set(
    this->MinimumLineLength );
  this->CleanPolyLines->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast( this->CleanPolyLines->getProxy() )->
    UpdatePipeline();


  pqPipelineSource *polyDataStatsFilter = builder->createFilter("filters",
    "PolyDataStatsFilter", this->CleanPolyLines);
  vtkSMSourceProxy::SafeDownCast( polyDataStatsFilter->getProxy() )->
    UpdatePipeline();
  polyDataStatsFilter->getProxy()->UpdatePropertyInformation();

  vtkIdType numberOfLines =
    vtkSMPropertyHelper(polyDataStatsFilter->getProxy(), "NumberOfLines").GetAsIdType();

  vtkIdType numberOfPoints =
    vtkSMPropertyHelper(polyDataStatsFilter->getProxy(), "NumberOfPoints").GetAsIdType();

  builder->destroy( polyDataStatsFilter );

  this->InternalWidget->numberOfContoursLabel->setText( QString("Number Of Contours: %1").arg(numberOfLines) );
  this->InternalWidget->numberOfPointsLabel->setText( QString("Number Of Points: %1").arg(numberOfPoints) );

  // add as a single representation.... by default it colors by LineIndex, so
  // with a decent color map, should be able to distinguish many the separate contours
  if (!this->ContourRepresentation)
    {
    this->ContourRepresentation =
      builder->createDataRepresentation(this->CleanPolyLines->getOutputPort(0),
      this->RenderView, "GeometryRepresentation");
    pqSMAdaptor::setElementProperty(
      this->ContourRepresentation->getProxy()->GetProperty("LineWidth"), 2);
    // move contour slightly in front of the image, so no rendering "issues"
    double position[3] = {0, 0, 0.1};
    vtkSMPropertyHelper(this->ContourRepresentation->getProxy(), "Position").Set(position, 3);
    this->ContourRepresentation->getProxy()->UpdateVTKObjects();
    }

  this->RenderView->render();
  this->InternalWidget->createContourNodesButton->setEnabled(true);
  this->ProgressMessage = "";
  this->updateProgress(this->ProgressMessage, 0);
  this->InternalWidget->okCancelBox->setEnabled(true);

  // reanble contour box, but disable contour generation button until parameter change
  this->InternalWidget->generateContoursBox->setEnabled(true);
  this->InternalWidget->loadImageFrame->setEnabled(true);
  this->InternalWidget->generateContoursButton->setEnabled(false);
  delete this->Progress;
  this->Progress = NULL;
}

//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::onCreateContourNodes()
{
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();
  this->disableWhileProcessing();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqServer *server = this->ParentNode->getTree()->getCurrentServer();

  pqPipelineSource *polyDataStatsFilter = builder->createFilter("filters",
    "PolyDataStatsFilter", this->CleanPolyLines);
  vtkSMSourceProxy::SafeDownCast( polyDataStatsFilter->getProxy() )->
    UpdatePipeline();
  polyDataStatsFilter->getProxy()->UpdatePropertyInformation();
  int numberOfLines = pqSMAdaptor::getElementProperty(
    polyDataStatsFilter->getProxy()->GetProperty("NumberOfLines")).toInt();
  builder->destroy( polyDataStatsFilter );

  QFileInfo info(this->ImageNodeName);
  this->ProgressMessage = "Creating Nodes";

  cmbSceneNodeReplaceEvent *event = NULL;
  // Are we recording events?
  if (this->ParentNode->getTree()->recordingEvents())
    {
    event = new cmbSceneNodeReplaceEvent(numberOfLines, 0);
    this->ParentNode->getTree()->insertEvent(event);
    }

  for (int i = 0; i < numberOfLines; i++)
    {
    this->updateProgress(this->ProgressMessage, 100 * (static_cast<double>(i) / static_cast<double>(numberOfLines)));
    pqPipelineSource *extractLine = builder->createFilter("filters",
      "ExtractLine", this->CleanPolyLines);
    vtkSMPropertyHelper(extractLine->getProxy(), "LineId").Set(i);
    extractLine->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy *proxy  = vtkSMSourceProxy::SafeDownCast( extractLine->getProxy() );
    proxy->UpdatePipeline();

    //construct a scene arc from the extractLine proxy
    pqCMBArc* obj = new pqCMBArc(proxy);
    builder->destroy(extractLine);


    pqDataRepresentation* repr = obj->getRepresentation();

    QString nodeName = info.baseName();
    nodeName += QString("%1").arg(i);
    this->ParentNode->getTree()->createNode(nodeName.toStdString().c_str(),
                                            this->ParentNode, obj, event);
    repr->getProxy()->UpdateVTKObjects();
    }
  this->MainDialog->done(QDialog::Accepted);
  delete this->Progress;
  this->Progress = NULL;
}

//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::onCancel()
{
  this->MainDialog->done(QDialog::Rejected);

  // disconnect progressManager????
}

//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::updateProgress(const QString& message, int progress)
{
  // Is there any progress being reported?
  if (!this->Progress)
    {
    return;
    }
  this->Progress->setLabelText(message);
  this->Progress->setValue(progress);
  QCoreApplication::processEvents();
}

//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::disableWhileProcessing()
{
  this->InternalWidget->loadImageFrame->setEnabled(false);
  this->InternalWidget->generateContoursBox->setEnabled(false);
  this->InternalWidget->okCancelBox->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::updateContourButtonStatus()
{
  if (this->ContourValue == this->InternalWidget->contourValue->text().toDouble() &&
    this->MinimumLineLength == this->InternalWidget->minimumLineLength->text().toDouble() &&
    this->UseRelativeLineLength == this->InternalWidget->relativeLineLengthCheckbox->isChecked())
    {
    this->InternalWidget->generateContoursButton->setEnabled(false);
    }
  else
    {
    this->InternalWidget->generateContoursButton->setEnabled(true);
    }
}
//-----------------------------------------------------------------------------
void qtCMBGenerateContoursDialog::onOpacityChanged(int opacity)
{
  vtkSMPropertyHelper(this->Grid->getRepresentation()->getProxy(), "Opacity").Set(
    static_cast<double>(opacity) / 100.0 );
  this->Grid->updateRepresentation();
  this->RenderView->render();
}
