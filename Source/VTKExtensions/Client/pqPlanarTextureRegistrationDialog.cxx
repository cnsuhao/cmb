//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for texturing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "pqPlanarTextureRegistrationDialog.h"

#include "ui_qtPlanarTextureRegistrationDialog.h"

#include "pqServerManagerObserver.h"
#include <pqApplicationCore.h>
#include <pqDataRepresentation.h>
#include <pqFileDialog.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqPropertyLinks.h>
#include <pqRenderView.h>
#include <pqSMAdaptor.h>
#include <pqServer.h>

#include "vtkPVRenderView.h"
#include <vtkAlgorithm.h>
#include <vtkImageData.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkProperty.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMSourceProxy.h>

#include <QDoubleValidator>
#include <QFileInfo>
#include <QGridLayout>
#include <QLineEdit>
#include <QTreeWidget>

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

pqPlanarTextureRegistrationDialog::pqPlanarTextureRegistrationDialog(
  pqServer* server, pqRenderView* view, const QString& strTitle, QWidget* parent)
  : Status(-1)
  , CurrentServer(server)
  , CurrentView(view)
  , PV_PLUGIN_USE(false)
{
  this->MainDialog = new QDialog(parent);
  this->TextureDialog = new Ui::qtPlanarTextureRegistrationDialog;
  this->TextureDialog->setupUi(MainDialog);
  this->MainDialog->setWindowTitle(strTitle);
}

void pqPlanarTextureRegistrationDialog::initializeTexture(const double bounds[6],
  const QStringList& textureFiles, const QString& currentTextureFile, const double regPoints[12],
  int numRegPoints, bool pluginPanel)
{
  this->PV_PLUGIN_USE = pluginPanel;
  this->TextureFiles.append(TextureFiles);

  for (int i = 0; i < 6; i++)
  {
    this->CurrentBounds[i] = bounds[i];
  }
  QDoubleValidator* validator = new QDoubleValidator(this->MainDialog);
  // Prep the line edit widgets
  this->TextureDialog->X1->setValidator(validator);
  this->TextureDialog->X1->setText("0.0");
  this->TextureDialog->Y1->setValidator(validator);
  this->TextureDialog->Y1->setText("0.0");
  this->TextureDialog->S1->setValidator(validator);
  this->TextureDialog->S1->setText("0.0");
  this->TextureDialog->T1->setValidator(validator);
  this->TextureDialog->T1->setText("0.0");

  this->TextureDialog->X2->setValidator(validator);
  this->TextureDialog->X2->setText("1.0");
  this->TextureDialog->Y2->setValidator(validator);
  this->TextureDialog->Y2->setText("0.0");
  this->TextureDialog->S2->setValidator(validator);
  this->TextureDialog->S2->setText("1.0");
  this->TextureDialog->T2->setValidator(validator);
  this->TextureDialog->T2->setText("0.0");

  this->TextureDialog->X3->setValidator(validator);
  this->TextureDialog->X3->setText("0.0");
  this->TextureDialog->Y3->setValidator(validator);
  this->TextureDialog->Y3->setText("1.0");
  this->TextureDialog->S3->setValidator(validator);
  this->TextureDialog->S3->setText("0.0");
  this->TextureDialog->T3->setValidator(validator);
  this->TextureDialog->T3->setText("1.0");
  this->TextureDialog->RegistrationMode->setCurrentIndex(0);
  this->TextureDialog->Point3->setVisible(false);

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  this->textureImageSource = NULL;
  this->imageDataRepresentation = NULL;

  //setup render view
  this->imageRenderView = qobject_cast<pqRenderView*>(
    builder->createView(pqRenderView::renderViewType(), this->CurrentServer));

  this->TextureDialog->imageframe->setVisible(!pluginPanel);
  if (!pluginPanel)
  {
    QGridLayout* gridlayout = new QGridLayout(this->TextureDialog->imageframe);
    gridlayout->setMargin(0);
    gridlayout->addWidget(this->imageRenderView->widget());
  }

  //setup Parallel projection and 2D manipulation
  pqSMAdaptor::setElementProperty(
    this->imageRenderView->getProxy()->GetProperty("CameraParallelProjection"), 1);
  pqSMAdaptor::setElementProperty(
    this->imageRenderView->getProxy()->GetProperty("CenterAxesVisibility"), 0);

  vtkSMProxy* viewproxy = this->imageRenderView->getProxy();
  vtkSMPropertyHelper(viewproxy, "InteractionMode").Set(vtkPVRenderView::INTERACTION_MODE_2D);
  viewproxy->UpdateVTKObjects();

  //Create 6 point widget representation proxies(invisible and disabled)
  this->widgetI1 = this->setupPointWidget(this->CurrentServer, this->imageRenderView);
  this->widgetI2 = this->setupPointWidget(this->CurrentServer, this->imageRenderView);
  this->widgetI3 = this->setupPointWidget(this->CurrentServer, this->imageRenderView);
  this->widgetS1 = this->setupPointWidget(this->CurrentServer, this->CurrentView);
  this->widgetS2 = this->setupPointWidget(this->CurrentServer, this->CurrentView);
  this->widgetS3 = this->setupPointWidget(this->CurrentServer, this->CurrentView);
  vtkPointHandleRepresentation3D* reprI = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetI1->GetRepresentationProxy()->GetClientSideObject());
  vtkPointHandleRepresentation3D* reprS = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetS1->GetRepresentationProxy()->GetClientSideObject());

  //Pair the scene and image point widgets using color
  double color[] = { 1.0, 0.0, 0.0 }; //red
  reprI->GetProperty()->SetColor(color);
  reprS->GetProperty()->SetColor(color);
  reprI->GetProperty()->SetLineWidth(2);
  reprS->GetProperty()->SetLineWidth(2);
  reprI->SetHandleSize(20);
  reprS->SetHandleSize(20);

  reprI = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetI2->GetRepresentationProxy()->GetClientSideObject());
  reprS = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetS2->GetRepresentationProxy()->GetClientSideObject());
  color[0] = 0.0;
  color[1] = 0.0;
  color[2] = 1.0; //blue
  reprI->GetProperty()->SetColor(color);
  reprS->GetProperty()->SetColor(color);
  reprI->GetProperty()->SetLineWidth(2);
  reprS->GetProperty()->SetLineWidth(2);
  reprI->SetHandleSize(20);
  reprS->SetHandleSize(20);

  reprI = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetI3->GetRepresentationProxy()->GetClientSideObject());
  reprS = vtkPointHandleRepresentation3D::SafeDownCast(
    this->widgetS3->GetRepresentationProxy()->GetClientSideObject());
  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.0; //yellow
  reprI->GetProperty()->SetColor(color);
  reprS->GetProperty()->SetColor(color);
  reprI->GetProperty()->SetLineWidth(2);
  reprS->GetProperty()->SetLineWidth(2);
  reprI->SetHandleSize(20);
  reprS->SetHandleSize(20);

  //Link the position of the widgets with the respective position boxes
  this->Links = new pqPropertyLinks();
  this->Links->removeAllPropertyLinks();
  this->Links->addPropertyLink(this->TextureDialog->S1, "text", SIGNAL(editingFinished()),
    this->widgetI1, this->widgetI1->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->S1, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->T1, "text", SIGNAL(editingFinished()),
    this->widgetI1, this->widgetI1->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->T1, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->S2, "text", SIGNAL(editingFinished()),
    this->widgetI2, this->widgetI2->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->S2, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->T2, "text", SIGNAL(editingFinished()),
    this->widgetI2, this->widgetI2->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->T2, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->S3, "text", SIGNAL(editingFinished()),
    this->widgetI3, this->widgetI3->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->S3, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->T3, "text", SIGNAL(editingFinished()),
    this->widgetI3, this->widgetI3->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->T3, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->X1, "text", SIGNAL(editingFinished()),
    this->widgetS1, this->widgetS1->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->X1, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->Y1, "text", SIGNAL(editingFinished()),
    this->widgetS1, this->widgetS1->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->Y1, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->X2, "text", SIGNAL(editingFinished()),
    this->widgetS2, this->widgetS2->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->X2, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->Y2, "text", SIGNAL(editingFinished()),
    this->widgetS2, this->widgetS2->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->Y2, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->X3, "text", SIGNAL(editingFinished()),
    this->widgetS3, this->widgetS3->GetProperty("WorldPosition"), 0);
  QObject::connect(
    this->TextureDialog->X3, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));
  this->Links->addPropertyLink(this->TextureDialog->Y3, "text", SIGNAL(editingFinished()),
    this->widgetS3, this->widgetS3->GetProperty("WorldPosition"), 1);
  QObject::connect(
    this->TextureDialog->Y3, SIGNAL(textChanged(const QString&)), this, SIGNAL(dialogModified()));

  QObject::connect(this->Links, SIGNAL(qtWidgetChanged()), this, SLOT(updateAllViews()));
  this->currentImageFileName = "";

  QObject::connect(this->TextureDialog->TextureFiles, SIGNAL(currentIndexChanged(const QString&)),
    this, SLOT(displayImage(const QString&)));

  // Set up the list of current textures
  this->updateTextureList(textureFiles, currentTextureFile);
  this->displayImage(this->currentImageFileName);
  if (!currentTextureFile.isEmpty() && textureFiles.count())
  {
    int numRegPnts = numRegPoints;
    this->TextureDialog->RegistrationMode->setCurrentIndex(numRegPnts - 2);

    QString num;
    double xy[2], st[2];
    this->getRegistrationPointPair(0, xy, st, regPoints);
    this->TextureDialog->X1->setText(num.setNum(xy[0]));
    this->TextureDialog->Y1->setText(num.setNum(xy[1]));
    this->TextureDialog->S1->setText(num.setNum(st[0]));
    this->TextureDialog->T1->setText(num.setNum(st[1]));

    this->getRegistrationPointPair(1, xy, st, regPoints);
    this->TextureDialog->X2->setText(num.setNum(xy[0]));
    this->TextureDialog->Y2->setText(num.setNum(xy[1]));
    this->TextureDialog->S2->setText(num.setNum(st[0]));
    this->TextureDialog->T2->setText(num.setNum(st[1]));

    if (numRegPnts == 3)
    {
      this->TextureDialog->Point3->setVisible(true);
      this->getRegistrationPointPair(2, xy, st, regPoints);
      this->TextureDialog->X3->setText(num.setNum(xy[0]));
      this->TextureDialog->Y3->setText(num.setNum(xy[1]));
      this->TextureDialog->S3->setText(num.setNum(st[0]));
      this->TextureDialog->T3->setText(num.setNum(st[1]));
    }
    this->PlaceWidgets(false);
    this->TextureDialog->RemoveTextureButton->setEnabled(true);
  }
  QObject::connect(
    this->TextureDialog->RemoveTextureButton, SIGNAL(clicked()), this, SLOT(removeTexture()));

  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(this->TextureDialog->buttonBox->button(QDialogButtonBox::Apply),
    SIGNAL(clicked()), this, SLOT(apply()));
  QObject::connect(
    this->TextureDialog->FileBrowserButton, SIGNAL(clicked()), this, SLOT(displayFileBrowser()));
  QObject::connect(this->TextureDialog->RegistrationMode, SIGNAL(currentIndexChanged(int)), this,
    SLOT(registrationModeChanged(int)));

  this->TextureDialog->buttonBox->setVisible(!pluginPanel);
  this->TextureDialog->RemoveTextureButton->setVisible(!pluginPanel);
  if (pluginPanel)
  {
    QObject::connect(builder, SIGNAL(destroying(pqView*)), this, SLOT(destroyingView(pqView*)));
  }
}

pqPlanarTextureRegistrationDialog::~pqPlanarTextureRegistrationDialog()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  if (this->Links)
  {
    this->Links->removeAllPropertyLinks();
    delete this->Links;
  }
  if (this->CurrentView)
  {
    this->removeSWidgets();
    this->CurrentView->getProxy()->UpdateVTKObjects();
  }

  if (this->imageRenderView)
  {
    this->removeIWidgets();
    pqServerManagerObserver* observer = pqApplicationCore::instance()->getServerManagerObserver();
    if (this->PV_PLUGIN_USE)
    {
      observer->blockSignals(true);
    }
    if (this->imageDataRepresentation)
    {
      builder->destroy(this->imageDataRepresentation);
      this->imageDataRepresentation = NULL;
    }
    if (this->textureImageSource)
    {
      builder->destroy(this->textureImageSource);
      this->textureImageSource = NULL;
    }
    builder->destroy(this->imageRenderView);
    this->imageRenderView = NULL;
    if (this->PV_PLUGIN_USE)
    {
      observer->blockSignals(false);
    }
  }
  if (this->TextureDialog)
  {
    delete TextureDialog;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}

void pqPlanarTextureRegistrationDialog::showButtons(bool showing)
{
  this->TextureDialog->buttonBox->setVisible(showing);
}

void pqPlanarTextureRegistrationDialog::registrationModeChanged(int mode)
{
  this->TextureDialog->Point3->setVisible((mode == 1));
  if (this->textureImageSource)
  {
    this->PlaceWidgets(false);
    this->updateAllViews();
  }
}

QDialog* pqPlanarTextureRegistrationDialog::getMainDialog()
{
  return this->MainDialog;
}

pqRenderView* pqPlanarTextureRegistrationDialog::getImageRenderView()
{
  return this->imageRenderView;
}

pqRenderView* pqPlanarTextureRegistrationDialog::currentView()
{
  return this->CurrentView;
}

pqPropertyLinks* pqPlanarTextureRegistrationDialog::getLinks()
{
  return this->Links;
}

int pqPlanarTextureRegistrationDialog::exec()
{
  this->MainDialog->setModal(false);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void pqPlanarTextureRegistrationDialog::accept()
{
  this->Status = 1;

  QFileInfo finfo(this->currentImageFileName);
  if (!finfo.exists())
  {
    this->Status = 0;
    return;
  }
  this->applyTexture();
}

void pqPlanarTextureRegistrationDialog::apply()
{
  if (this->currentImageFileName.isEmpty())
  {
    return;
  }
  this->applyTexture();
}

void pqPlanarTextureRegistrationDialog::applyTexture()
{
  int num;
  // Get the point information
  num = this->TextureDialog->RegistrationMode->currentIndex() + 2;
  this->RegistrationPoints[0] = this->TextureDialog->X1->text().toDouble();
  this->RegistrationPoints[1] = this->TextureDialog->Y1->text().toDouble();
  this->RegistrationPoints[2] = this->TextureDialog->S1->text().toDouble();
  this->RegistrationPoints[3] = this->TextureDialog->T1->text().toDouble();

  this->RegistrationPoints[4] = this->TextureDialog->X2->text().toDouble();
  this->RegistrationPoints[5] = this->TextureDialog->Y2->text().toDouble();
  this->RegistrationPoints[6] = this->TextureDialog->S2->text().toDouble();
  this->RegistrationPoints[7] = this->TextureDialog->T2->text().toDouble();

  if (num == 3)
  {
    this->RegistrationPoints[8] = this->TextureDialog->X3->text().toDouble();
    this->RegistrationPoints[9] = this->TextureDialog->Y3->text().toDouble();
    this->RegistrationPoints[10] = this->TextureDialog->S3->text().toDouble();
    this->RegistrationPoints[11] = this->TextureDialog->T3->text().toDouble();
  }
  emit this->registerCurrentTexture(this->currentImageFileName, num, this->RegistrationPoints);
  this->updateAllViews();
  this->TextureDialog->RemoveTextureButton->setEnabled(
    !(this->currentImageFileName.isEmpty() || this->currentImageFileName.isNull()));
}

int pqPlanarTextureRegistrationDialog::getNumberOfRegistrationPoints()
{
  // Get the point information
  return this->TextureDialog->RegistrationMode->currentIndex() + 2;
}

void pqPlanarTextureRegistrationDialog::getRegistrationPoints(double pnts[12])
{
  int num = this->getNumberOfRegistrationPoints();

  // Get the point information
  pnts[0] = this->TextureDialog->X1->text().toDouble();
  pnts[1] = this->TextureDialog->Y1->text().toDouble();
  pnts[2] = this->TextureDialog->S1->text().toDouble();
  pnts[3] = this->TextureDialog->T1->text().toDouble();

  pnts[4] = this->TextureDialog->X2->text().toDouble();
  pnts[5] = this->TextureDialog->Y2->text().toDouble();
  pnts[6] = this->TextureDialog->S2->text().toDouble();
  pnts[7] = this->TextureDialog->T2->text().toDouble();

  if (num == 3)
  {
    pnts[8] = this->TextureDialog->X3->text().toDouble();
    pnts[9] = this->TextureDialog->Y3->text().toDouble();
    pnts[10] = this->TextureDialog->S3->text().toDouble();
    pnts[11] = this->TextureDialog->T3->text().toDouble();
  }
}

void pqPlanarTextureRegistrationDialog::cancel()
{
  this->Status = 0;
}

void pqPlanarTextureRegistrationDialog::filesSelected(const QStringList& files)
{
  if (files.size() == 0)
  {
    return;
  }

  this->TextureDialog->TextureFiles->addItem(files[0]);
  this->TextureDialog->TextureFiles->setCurrentIndex(
    this->TextureDialog->TextureFiles->count() - 1);
}

void pqPlanarTextureRegistrationDialog::displayFileBrowser()
{
  QString filters = "All Image Formats (*.tiff, *.tif, *.jpeg, *.jpg);;TIFF (*.tiff, *.tif);;JPEG "
                    "(*.jpeg, *.jpg);;All files (*)";

  pqFileDialog file_dialog(
    this->CurrentServer, this->MainDialog, tr("Open File:"), QString(), filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("FileTextureDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  QObject::connect(&file_dialog, SIGNAL(filesSelected(const QStringList&)), this,
    SLOT(filesSelected(const QStringList&)));
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.exec();
}

void pqPlanarTextureRegistrationDialog::removeTexture()
{
  emit this->removeCurrentTexture();
  if (!this->PV_PLUGIN_USE)
  {
    this->Status = 1;
    this->MainDialog->close();
  }
  else
  {
  }
}

void pqPlanarTextureRegistrationDialog::displayImage(const QString& filename)
{
  if (filename.isEmpty() || filename.isNull())
  {
    return;
  }

  this->currentImageFileName = filename;
  bool defaultPosition = false;
  //Create the image source if it does not exist
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if (!this->textureImageSource)
  {
    this->textureImageSource =
      ReadTextureImage(builder, this->CurrentServer, this->currentImageFileName.toLatin1().data());
    this->imageDataRepresentation = builder->createDataRepresentation(
      this->textureImageSource->getOutputPort(0), this->imageRenderView);
    defaultPosition = true;
  }

  vtkSMPropertyHelper(this->textureImageSource->getProxy(), "FileName")
    .Set(this->currentImageFileName.toLatin1().data());
  vtkSMSourceProxy::SafeDownCast(this->textureImageSource->getProxy())->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->textureImageSource->getProxy())->UpdatePipeline();
  vtkSMPropertyHelper(this->imageDataRepresentation->getProxy(), "MapScalars").Set(0);
  vtkSMSourceProxy::SafeDownCast(this->imageDataRepresentation->getProxy())->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->imageDataRepresentation->getProxy())->UpdatePipeline();

  this->PlaceWidgets(defaultPosition);
  this->updateAllViews();
  this->imageRenderView->resetCamera();
  this->CurrentView->resetCamera();
}

void pqPlanarTextureRegistrationDialog::PlaceWidgets(bool defaultPosition)
{
  if (!this->textureImageSource)
  {
    return;
  }
  //Position the widgets
  //Default positions are the corners
  vtkAlgorithm* source =
    vtkAlgorithm::SafeDownCast(this->textureImageSource->getProxy()->GetClientSideObject());
  vtkImageData* image = vtkImageData::SafeDownCast(source->GetOutputDataObject(0));

  int extents[6];
  image->GetExtent(extents);

  double* bounds = this->CurrentBounds;

  QList<QVariant> center;
  if (defaultPosition)
  {
    QString num;
    this->TextureDialog->S1->setText(num.setNum(extents[0]));
    this->TextureDialog->T1->setText(num.setNum(extents[2]));
    this->TextureDialog->S2->setText(num.setNum(extents[1]));
    this->TextureDialog->T2->setText(num.setNum(extents[3]));
    this->TextureDialog->S3->setText(num.setNum(extents[0]));
    this->TextureDialog->T3->setText(num.setNum(extents[3]));

    this->TextureDialog->X1->setText(num.setNum(bounds[0]));
    this->TextureDialog->Y1->setText(num.setNum(bounds[2]));
    this->TextureDialog->X2->setText(num.setNum(bounds[1]));
    this->TextureDialog->Y2->setText(num.setNum(bounds[3]));
    this->TextureDialog->X3->setText(num.setNum(bounds[0]));
    this->TextureDialog->Y3->setText(num.setNum(bounds[3]));
  }
  center << this->TextureDialog->S1->text().toDouble() << this->TextureDialog->T1->text().toDouble()
         << extents[5];
  this->updatePointWidget(this->widgetI1, center);
  center.clear();
  center << this->TextureDialog->S2->text().toDouble() << this->TextureDialog->T2->text().toDouble()
         << extents[5];
  this->updatePointWidget(this->widgetI2, center);
  center.clear();
  center << this->TextureDialog->X1->text().toDouble() << this->TextureDialog->Y1->text().toDouble()
         << bounds[5];
  this->updatePointWidget(this->widgetS1, center);
  center.clear();
  center << this->TextureDialog->X2->text().toDouble() << this->TextureDialog->Y2->text().toDouble()
         << bounds[5];
  this->updatePointWidget(this->widgetS2, center);
  if (this->TextureDialog->RegistrationMode->currentIndex() == 1)
  {
    center.clear();
    center << this->TextureDialog->S3->text().toDouble()
           << this->TextureDialog->T3->text().toDouble() << extents[5];
    this->updatePointWidget(this->widgetI3, center);
    center.clear();
    center << this->TextureDialog->X3->text().toDouble()
           << this->TextureDialog->Y3->text().toDouble() << bounds[5];
    this->updatePointWidget(this->widgetS3, center);
  }
  else
  {
    center.clear();
    center << 0.0 << 0.0 << 0.0;
    this->updatePointWidget(this->widgetI3, center, false, false);
    this->updatePointWidget(this->widgetS3, center, false, false);
  }
}

vtkSMNewWidgetRepresentationProxy* pqPlanarTextureRegistrationDialog::setupPointWidget(
  pqServer* server, pqRenderView* view)
{
  vtkSMNewWidgetRepresentationProxy* widget;
  QList<QVariant> center;
  center << 0.0 << 0.0 << 0.0;
  widget = dynamic_cast<vtkSMNewWidgetRepresentationProxy*>(
    server->proxyManager()->NewProxy("representations", "HandleWidgetRepresentation"));
  pqSMAdaptor::setElementProperty(widget->GetProperty("Visibility"), false);
  pqSMAdaptor::setElementProperty(widget->GetProperty("Enabled"), false);
  pqSMAdaptor::setElementProperty(widget->GetProperty("NumberOfPoints"), 1);
  pqSMAdaptor::setMultipleElementProperty(widget->GetProperty("WorldPosition"), center);
  widget->UpdateVTKObjects();
  pqSMAdaptor::addProxyProperty(view->getProxy()->GetProperty("Representations"), widget);
  view->getProxy()->UpdateVTKObjects();
  return widget;
}

void pqPlanarTextureRegistrationDialog::updatePointWidget(
  vtkSMNewWidgetRepresentationProxy* widget, QList<QVariant> center, bool visibility, bool enable)
{
  pqSMAdaptor::setElementProperty(widget->GetProperty("Visibility"), visibility);
  pqSMAdaptor::setElementProperty(widget->GetProperty("Enabled"), enable);
  pqSMAdaptor::setMultipleElementProperty(widget->GetProperty("WorldPosition"), center);
  widget->UpdateVTKObjects();
}

void pqPlanarTextureRegistrationDialog::updateAllViews()
{
  if (this->imageRenderView)
  {
    this->imageRenderView->getProxy()->UpdateVTKObjects();
    this->imageRenderView->render();
  }
  if (this->CurrentView)
  {
    this->CurrentView->getProxy()->UpdateVTKObjects();
    this->CurrentView->render();
  }
}

void pqPlanarTextureRegistrationDialog::destroyingView(pqView* view)
{
  if (view == NULL)
  {
    return;
  }
  if (view == this->CurrentView)
  {
    this->removeSWidgets();
    this->CurrentView = NULL;
  }
  else if (view == this->imageRenderView)
  {
    this->removeIWidgets();
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
    if (this->imageDataRepresentation)
    {
      builder->destroy(this->imageDataRepresentation);
      this->imageDataRepresentation = NULL;
    }
    this->imageRenderView = NULL;
    if (this->textureImageSource)
    {
      builder->destroy(this->textureImageSource);
      this->textureImageSource = NULL;
    }
  }
}

void pqPlanarTextureRegistrationDialog::getRegistrationPointPair(
  int i, double xy[2], double st[2], const double regPoints[12]) const
{
  int j = 4 * i;
  xy[0] = regPoints[j++];
  xy[1] = regPoints[j++];
  st[0] = regPoints[j++];
  st[1] = regPoints[j++];
}

void pqPlanarTextureRegistrationDialog::updateTextureList(
  const QStringList& textureFiles, const QString& currentTextureFile)
{
  // Set up the list of current textures
  this->TextureDialog->TextureFiles->blockSignals(true);
  this->TextureDialog->TextureFiles->clear();
  this->TextureDialog->TextureFiles->addItems(textureFiles);
  if (!currentTextureFile.isEmpty() && textureFiles.count())
  {
    int pos = textureFiles.indexOf(currentTextureFile);
    if (pos != -1)
    {
      this->currentImageFileName = currentTextureFile;
      this->TextureDialog->TextureFiles->setCurrentIndex(pos);
    }
  }
  this->TextureDialog->TextureFiles->blockSignals(false);
}

void pqPlanarTextureRegistrationDialog::removeSWidgets()
{
  if (this->widgetS1)
  {
    pqSMAdaptor::removeProxyProperty(
      this->CurrentView->getProxy()->GetProperty("Representations"), this->widgetS1);
    this->widgetS1 = NULL;
  }
  if (this->widgetS2)
  {
    pqSMAdaptor::removeProxyProperty(
      this->CurrentView->getProxy()->GetProperty("Representations"), this->widgetS2);
    this->widgetS2 = NULL;
  }
  if (this->widgetS3)
  {
    pqSMAdaptor::removeProxyProperty(
      this->CurrentView->getProxy()->GetProperty("Representations"), this->widgetS3);
    this->widgetS3 = NULL;
  }
}

void pqPlanarTextureRegistrationDialog::removeIWidgets()
{
  if (this->widgetI1)
  {
    pqSMAdaptor::removeProxyProperty(
      this->imageRenderView->getProxy()->GetProperty("Representations"), this->widgetI1);
    this->widgetI1 = NULL;
  }
  if (this->widgetI2)
  {
    pqSMAdaptor::removeProxyProperty(
      this->imageRenderView->getProxy()->GetProperty("Representations"), this->widgetI2);
    this->widgetI2 = NULL;
  }
  if (this->widgetI3)
  {
    pqSMAdaptor::removeProxyProperty(
      this->imageRenderView->getProxy()->GetProperty("Representations"), this->widgetI3);
    this->widgetI3 = NULL;
  }
}
