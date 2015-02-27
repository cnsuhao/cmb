/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneObjectImporter.cxx

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
// .NAME Represents a dialog for importing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBSceneObjectImporter.h"

#include "ui_qtCMBSceneObjectImportNoUnits.h"
#include "qtCMBPlacementConstraintWidget.h"
#include "pqCMBPoints.h"
#include "pqCMBArc.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBSolidMesh.h"
#include "pqCMBUniformGrid.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include "qtCMBApplicationOptions.h"

#include "pqApplicationCore.h"
#include "pqFileDialog.h"
#include "pqFileDialogModel.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqProgressManager.h"
#include "pqDataRepresentation.h"
#include "pqRenderView.h"

#include "vtkCellType.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMVectorProperty.h"

#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QIntValidator>
#include <QProgressDialog>
#include <QTreeWidget>

//-----------------------------------------------------------------------------
pqCMBSceneNode *
qtCMBSceneObjectImporter::importNode(pqCMBSceneNode *parent,
                                     bool enableRandomOption,
                                     bool allowTextureConstraintPlacement,
                                     bool *randomPlacement,
                                     bool *translateBasedOnView,
                                     int *count,
                                     QMap<pqCMBSceneNode*, int> &constraints,
                                     bool  &okToUseGlyphs,
                                     bool &useTextureConstraint,
                                     bool &useGlyphPlayback,
                                     int &glyphPlaybackOption,
                                     QString &glyphPlaybackFilename)
{
  qtCMBSceneObjectImporter importer(parent);
  importer.setRandomPlacementOption(enableRandomOption);
  return importer.exec(randomPlacement, allowTextureConstraintPlacement,
                       translateBasedOnView, count, constraints,
                       okToUseGlyphs, useTextureConstraint,
                       useGlyphPlayback, glyphPlaybackFilename,
                       glyphPlaybackOption);
}

//-----------------------------------------------------------------------------
qtCMBSceneObjectImporter::qtCMBSceneObjectImporter(pqCMBSceneNode *n) : Node(NULL)
{
  this->Parent = n;
  this->MainDialog = new QDialog(this->Parent->getTree()->getWidget());

  this->ImportDialog = new Ui::qtCMBSceneObjectImport;
  this->ImportDialog->setupUi(MainDialog);
  this->ImportDialog->maxNumberOfLIDARPoints->setValue(
    qtCMBApplicationOptions::instance()->maxNumberOfCloudPoints());

  this->PlacementWidget = new qtCMBPlacementConstraintWidget(this->Parent,
    this->ImportDialog->RandomPlacement);
  this->ImportDialog->RandomPlacement->layout()->addWidget(this->PlacementWidget);

  this->ImportDialog->rawDEMParameters->hide();
  this->ImportDialog->rawDEMParameters->setChecked(false);
  this->MainDialog->adjustSize();
  this->ObjectType = pqCMBSceneObjectBase::Unknown;
  // Set up the ObjectTypes
  this->setupObjectTypes();
  pqCMBSceneTree *tree = n->getTree();
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(this->ImportDialog->FileBrowserButton, SIGNAL(clicked()),
                   this, SLOT(displayFileBrowser()));
  QObject::connect(this->ImportDialog->ObjectTypes, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(changeObjectType()));
  QObject::connect(this->ImportDialog->GeometricTypes, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(changeGeometricType()));


  // DEM read extent validators
  this->ColumnExtents[0] = this->ColumnExtents[1] = -1;
  this->RowExtents[0] = this->RowExtents[1] = -1;
  this->MinRowValidator = new QIntValidator( this->MainDialog );
  this->ImportDialog->minRowExtent->setValidator( this->MinRowValidator  );
  this->MinRowValidator->setBottom( 0 );
  this->MaxRowValidator = new QIntValidator(  this->MainDialog );
  this->ImportDialog->maxRowExtent->setValidator( this->MaxRowValidator  );
  this->MinColumnValidator = new QIntValidator( this->MainDialog );
  this->ImportDialog->minColumnExtent->setValidator( this->MinColumnValidator  );
  this->MinColumnValidator->setBottom( 0 );
  this->MaxColumnValidator = new QIntValidator(  this->MainDialog );
  this->ImportDialog->maxColumnExtent->setValidator( this->MaxColumnValidator  );
  QObject::connect(this->ImportDialog->maxRowExtent, SIGNAL(editingFinished()),
    this, SLOT(updateRowExtents()));
  QObject::connect(this->ImportDialog->minRowExtent, SIGNAL(editingFinished()),
    this, SLOT(updateRowExtents()));
  QObject::connect(this->ImportDialog->maxColumnExtent, SIGNAL(editingFinished()),
    this, SLOT(updateColumnExtents()));
  QObject::connect(this->ImportDialog->minColumnExtent, SIGNAL(editingFinished()),
    this, SLOT(updateColumnExtents()));
  QObject::connect(this->ImportDialog->FileNameText, SIGNAL(editingFinished()),
    this, SLOT(updateDialog()));

  // validator for onRatio
  QIntValidator *onRatioValidator = new QIntValidator(  this->MainDialog );
  this->ImportDialog->onRatio->setValidator( onRatioValidator );
  onRatioValidator->setBottom(1);

  this->FileValidator = new pqFileDialogModel( this->Parent->getTree()->getCurrentServer() );
  this->Progress = NULL;
}

//-----------------------------------------------------------------------------
qtCMBSceneObjectImporter::~qtCMBSceneObjectImporter()
{
  if(this->PlacementWidget)
    {
    delete this->PlacementWidget;
    }
  if (this->ImportDialog)
    {
    delete ImportDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
  if (this->FileValidator)
    {
    delete this->FileValidator;
    }
  if (this->Progress)
    {
    delete this->Progress;
    this->Progress = NULL;
    }
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::assignUnits(pqCMBSceneObjectBase *obj)
{
#if 0
  if (this->ImportDialog->INButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::inches);
    }
  else if (this->ImportDialog->FTButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::feet);
    }
  else if (this->ImportDialog->MMButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::mm);
    }
  else if (this->ImportDialog->CMButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::cm);
    }
  else if (this->ImportDialog->MButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::m);
    }
  else if (this->ImportDialog->KMButton->isChecked())
    {
    obj->setUnits(cmbSceneUnits::km);
    }
  else
    {
    obj->setUnits(cmbSceneUnits::Unknown);
    }
#else
  obj->setUnits(cmbSceneUnits::Unknown);
#endif
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::setRandomPlacementOption(bool mode)
{
  this->ImportDialog->RandomPlacement->setEnabled(mode);
  if (!mode)
    {
    this->ImportDialog->RandomPlacement->hide();
    this->MainDialog->adjustSize();
    return;
    }

  this->ImportDialog->translateBasedOnView->setChecked(true);

}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::setupObjectTypes()
{
  this->ImportDialog->GeometricTypes->addItem("Points", pqCMBSceneObjectBase::Points);
  this->ImportDialog->GeometricTypes->addItem("Image/DEM", pqCMBSceneObjectBase::UniformGrid);
  this->ImportDialog->GeometricTypes->addItem("TIN Surface", pqCMBSceneObjectBase::TIN);
  this->ImportDialog->GeometricTypes->addItem("Closed Shells", pqCMBSceneObjectBase::Solid);
  this->ImportDialog->GeometricTypes->addItem("Solid Mesh", pqCMBSceneObjectBase::SolidMesh);
  this->ImportDialog->GeometricTypes->addItem("Arc", pqCMBSceneObjectBase::Arc);
  this->ImportDialog->GeometricTypes->addItem("Other", pqCMBSceneObjectBase::Other);
  this->ImportDialog->GeometricTypes->setCurrentIndex(3);
  this->ImportDialog->ObjectTypes->
    addItems(this->Parent->getTree()->getUserDefinedObjectTypes());
  this->ImportDialog->GeometricTypes->setCurrentIndex(3);
  this->ImportDialog->ObjectTypes->setCurrentIndex(3);
  this->ImportDialog->ObjectTypes->addItem("Specify New Type");
}
//-----------------------------------------------------------------------------
pqCMBSceneNode *qtCMBSceneObjectImporter::exec(bool *randomPlacement,
                                             bool allowTextureConstraintPlacement,
                                             bool *translateBasedOnView,
                                             int *count,
                                             QMap<pqCMBSceneNode*, int> &constaints,
                                             bool &usingGlyphs,
                                             bool &useTextureConstraint,
                                             bool &useGlyphPlayback,
                                             QString &glyphPlaybackFileName,
                                             int &glyphPlaybackOption)
{
  this->PlacementWidget->updateConstraintTable();
  this->PlacementWidget->enableTextureConstraintOption( allowTextureConstraintPlacement );
  if(useGlyphPlayback)
    {
    this->PlacementWidget->showGlyphPlaybackGroupBox(true);
    }
  else
    {
    this->PlacementWidget->showGlyphPlaybackGroupBox(false);
    }

  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();

  *translateBasedOnView = false;
  if (this->ImportDialog->RandomPlacement->isChecked())
    {
    *randomPlacement = true;
    *count = this->PlacementWidget->getPlacementCount();
    constaints = this->PlacementWidget->getSelectedConstraints();
    useTextureConstraint = this->PlacementWidget->useTextureConstraint();
    }
  else
    {
    useTextureConstraint = false;
    if (this->ImportDialog->translateBasedOnView->isChecked())
      {
      *translateBasedOnView = true;
      }
    *randomPlacement = false;
    *count = 1;
    }
  usingGlyphs = this->PlacementWidget->useGlyphs();

  if(useGlyphPlayback)
    {
    glyphPlaybackOption = this->PlacementWidget->getGlyphPlaybackOption();
    glyphPlaybackFileName = this->PlacementWidget->getGlyphPlaybackFilename();
    }
  else
    {
    glyphPlaybackOption = -1;
    glyphPlaybackFileName = QString("");
    }

  return this->Node;
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::accept()
{
  QString n =  this->ImportDialog->FileNameText->text();
  if (!n.size())
    {
    return;
    }

  QFileInfo finfo(n);
  QString s = finfo.completeSuffix().toLower();

  if (s == "pts" ||
      s == "bin" ||
      s == "bin.pts" )
    {
    // then should be type LIDAR, regardless of how user set????
    this->importLIDARFile(n);
    return;
    }
  else if (s == "las")
    {
    // NOTE: importLASFile could be simpler / shorter, and use
    // pqCMBSceneObjectBase::createLIDARPieceObject, as is done for LAS files
    // when reading an sg file, but more efficient to read all at once as is
    // done in importLASFile()... unfortunately it is convenient to do the same
    // when reading from an sg file
    this->importLASFile(n);
    return;
    }
  else if ((s == "vti") || (s == "flt") || (s == "hdr")|| (s == "ftw") || (s == "dem"))
    {
    this->importUniformGrid(n);
    return;
    }
  else if(s=="shp")
    {
    this->importShapeFile(n);
    return;
    }
  else if(s=="3dm")
    {
    this->importSolidMesh(n);
    return;
    }
  else if(s=="bor")
    {
    this->importBorFile(n);
    return;
    }
  pqCMBSceneTree *tree = this->Parent->getTree();
  // Are we suppose to use Glyphs?
  if (this->PlacementWidget->useGlyphs())
    {
    pqCMBGlyphObject *obj = new pqCMBGlyphObject(n.toStdString().c_str(),
                                                         tree->getCurrentServer(),
                                                         tree->getCurrentView(),
                                                         false);
    double p[3];
    p[0] = p[1] = p[2] = 0.0;
    obj->insertNextPoint(p);
    // If we are here and the geometric type is Points for the time being we need to
    // assume the type is Other
    if (this->ImportDialog->ObjectTypes->currentIndex() == 0)
      {
      obj->setSurfaceType(pqCMBSceneObjectBase::Other);
      }
    else
      {
      obj->setSurfaceType(static_cast<pqCMBSceneObjectBase::enumSurfaceType>(
                            this->ImportDialog->GeometricTypes->itemData(
                              this->ImportDialog->GeometricTypes->currentIndex()).toInt()));
      }
    finfo.setFile(n);
    this->Node = this->createObjectNode(
      obj, finfo.baseName().toStdString().c_str(), this->Parent);
    }
  else
    {
    pqCMBFacetedObject *obj = new pqCMBFacetedObject(n.toStdString().c_str(),
                                                           tree->getCurrentServer(),
                                                           tree->getCurrentView(),
                                                           false);
    // If we are here and the geometric type is Points for the time being we need to
    // assume the type is Other
    if (this->ImportDialog->ObjectTypes->currentIndex() == 0)
      {
      obj->setSurfaceType(pqCMBSceneObjectBase::Other);
      }
    else
      {
      obj->setSurfaceType(static_cast<pqCMBSceneObjectBase::enumSurfaceType>(
                            this->ImportDialog->GeometricTypes->itemData(
                              this->ImportDialog->GeometricTypes->currentIndex()).toInt()));
      }
    finfo.setFile(n);
    this->Node = this->createObjectNode(
      obj, finfo.baseName().toStdString().c_str(), this->Parent);
    }
  tree->getCurrentView()->forceRender();
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::cancel()
{
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::updateDialog()
{
  QString fname = this->ImportDialog->FileNameText->text();
  // Lets see if the string has been changed since the last time update was called
  if ((!this->CurrentFileName.isNull()) && (this->CurrentFileName == fname))
    {
    return; // Nothing needs to be done
    }
  this->CurrentFileName = fname;
  this->ObjectType = pqCMBSceneObjectBase::Unknown;

  // See if the file exists.  If it does make accept active.
  QString fullPath;
  bool fileExists = this->FileValidator->fileExists(fname, fullPath);
  this->ImportDialog->buttonBox->
    button(QDialogButtonBox::Open)->setEnabled(fileExists);

  // if there is no fname set or it doesn't exist turneverthing off
  if ((!fileExists) || (fname == ""))
    {
    this->ImportDialog->translateBasedOnView->setChecked(false);
    this->ImportDialog->maxNumberOfLIDARPoints->setEnabled(false);
    this->ImportDialog->maxPtsLabel->setEnabled(false);
    this->ImportDialog->RandomPlacement->hide();
    this->ImportDialog->rawDEMParameters->hide();
    this->PlacementWidget->enableGlyphOption(false);
    return;
    }

  QFileInfo finfo(fname);
  QString s = finfo.suffix().toLower();

  // Now set up the combo box based on the file suffix
  if ((s == "pts") || (s == "bin") || (s == "las"))
    {
    this->ObjectType = pqCMBSceneObjectBase::Points;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-LIDAR"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Points"));
    this->ImportDialog->rawDEMParameters->hide();
    this->ImportDialog->RandomPlacement->hide();
    this->PlacementWidget->enableGlyphOption(false);
    }
  else if ((s == "vti") || (s == "flt") || (s == "hdr")|| (s == "ftw"))
    {
    this->ObjectType = pqCMBSceneObjectBase::UniformGrid;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-UniformGrid"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Image/DEM"));
    this->ImportDialog->RandomPlacement->hide();

    // See if this is a Raw DEM File
    if (pqCMBUniformGrid::isRawDEM(fname.toAscii().constData()))
      {
      this->ImportDialog->rawDEMParameters->show();
      this->updateDEMExtents();
      }
    else
      {
      this->ImportDialog->rawDEMParameters->hide();
      }
    }
  else if ((s == "dem"))
  {
    this->ObjectType = pqCMBSceneObjectBase::UniformGrid;
    this->ImportDialog->ObjectTypes->
          setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-UniformGrid"));
    this->ImportDialog->GeometricTypes->
          setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Image/DEM"));
  }
  else if ((s == "2dm") /*|| (s == "3dm") */|| (s == "fac")|| (s == "sol"))
    {
    this->ObjectType = pqCMBSceneObjectBase::Faceted;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-Solid"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Closed Shells"));
    this->ImportDialog->rawDEMParameters->hide();
    if (this->ImportDialog->RandomPlacement->isEnabled())
      {
      this->ImportDialog->RandomPlacement->show();
      this->PlacementWidget->enableGlyphOption(true);
      }
    }
  else if (s == "3dm")
    {
    this->ObjectType = pqCMBSceneObjectBase::SolidMesh;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-SolidMesh"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Solid Mesh"));
    this->ImportDialog->rawDEMParameters->hide();
    if (this->ImportDialog->RandomPlacement->isEnabled())
      {
      this->ImportDialog->RandomPlacement->show();
      }
    }
  else if (s == "tin")
    {
    this->ObjectType = pqCMBSceneObjectBase::Faceted;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-TIN"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("TIN Surface"));
    this->ImportDialog->rawDEMParameters->hide();
    if (this->ImportDialog->RandomPlacement->isEnabled())
      {
      this->ImportDialog->RandomPlacement->show();
      this->PlacementWidget->enableGlyphOption(true);
      }
    }
  else if ( s == "shp")
    {
    this->ObjectType = pqCMBSceneObjectBase::Arc;
    this->ImportDialog->ObjectTypes->setCurrentIndex(
      this->ImportDialog->ObjectTypes->findText("-Contour"));
    this->ImportDialog->GeometricTypes->setCurrentIndex(
      this->ImportDialog->GeometricTypes->findText("Arc"));
    this->ImportDialog->rawDEMParameters->hide();
    this->ImportDialog->RandomPlacement->hide();
    }
  else
    {
    this->ObjectType = pqCMBSceneObjectBase::Faceted;
    this->ImportDialog->
      ObjectTypes->
      setCurrentIndex(this->ImportDialog->ObjectTypes->findText("-Other"));
    this->ImportDialog->
      GeometricTypes->
      setCurrentIndex(this->ImportDialog->GeometricTypes->findText("Other"));
    this->ImportDialog->rawDEMParameters->hide();
    if (this->ImportDialog->RandomPlacement->isEnabled())
      {
      this->ImportDialog->RandomPlacement->show();
      this->PlacementWidget->enableGlyphOption(true);
      }
    }

  this->MainDialog->adjustSize();
  bool translateBasedOnView = false;
  // If the scene does contain aleast one data object and we are allowing translation based on view and
  // we are not dealing with LIDAR then check the translateBasedOnView box by default
  if (this->Parent->getTree()->containsDataObjects() && this->ImportDialog->translateBasedOnView->isEnabled() &&
      !((s == "pts") || (s == "bin") || (s == "las") || (s == "vtk") ||
        (s == "vti") || (s == "flt") || (s == "hdr") || (s == "ftw") || (s == "dem")))
    {
    translateBasedOnView = true;
    }
  this->ImportDialog->translateBasedOnView->setChecked(translateBasedOnView);
  if(this->ObjectType == pqCMBSceneObjectBase::Points)
    {
    this->ImportDialog->translateBasedOnView->setChecked(false);
    this->ImportDialog->maxNumberOfLIDARPoints->setEnabled(true);
    this->ImportDialog->maxPtsLabel->setEnabled(true);
    }
  else
    {
    this->ImportDialog->maxNumberOfLIDARPoints->setEnabled(false);
    this->ImportDialog->maxPtsLabel->setEnabled(false);
    }

}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::filesSelected(const QStringList &files)
{
  if (files.size() == 0)
    {
    this->ImportDialog->FileNameText->setText("");
    }

  this->ImportDialog->FileNameText->setText(files[0]);

  this->updateDialog();
}


//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::changeGeometricType()
{
  this->PlacementWidget->
    enableGlyphOption(this->ImportDialog->GeometricTypes->currentIndex() != 0);
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::changeObjectType()
{
  if (this->ImportDialog->ObjectTypes->currentIndex() ==
      (this->ImportDialog->ObjectTypes->count()-1))
    {
    // User has asked to add a new type
    this->ImportDialog->ObjectTypes->blockSignals(true);
    QString newType = QInputDialog::getText(this->MainDialog,
                                            "SceneBuilder - New Object Type",
                                            "Enter New Object Type:");

    if (!newType.isEmpty())
      {
      // See if the type is already in list
      int index = this->Parent->getTree()->
        getUserDefinedObjectTypes().indexOf(newType);
      if (index == -1)
        {
        this->Parent->getTree()->addUserDefinedType(newType.toAscii());
        index = this->Parent->getTree()->
          getUserDefinedObjectTypes().indexOf(newType);
        this->ImportDialog->ObjectTypes->insertItem(index, newType);
        }
      this->ImportDialog->ObjectTypes->setCurrentIndex(index);
      }
    else
      {
      // Set the type to be unknown
      this->ImportDialog->ObjectTypes->setCurrentIndex(0);
      }
    this->ImportDialog->ObjectTypes->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::displayFileBrowser()
{
  QString filters = "All Geometry (*.2dm *.3dm *.bin *.bin.pts *.fac *.las *.obj *.pts *.sol *.tin *.vtk *.vtp *.vti *.dem *.flt *.ftw *.hdr *.shp *.bor);;LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;CUBIT Facet (*.fac);;Wavefront Object (*.obj);;Surface Mesh (*.2dm);;Volume Mesh (*.3dm);;Solid (*.sol);;TIN (*.tin);;XML VTK Geometry (*.vtp);;Legacy VTK Geometry (*.vtk);;Image/DEM (*.vti *.flt *.ftw *.hdr *.dem);;Contour (*.shp);;Boreholes (*.bor);;All files (*)";

  pqCMBSceneTree *tree = this->Parent->getTree();
  pqFileDialog file_dialog(tree->getCurrentServer(),
                                                     this->MainDialog,
                                                     tr("Open File:"),
                                                     QString(),
                                                     filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("FileImportDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  QObject::connect(&file_dialog, SIGNAL(filesSelected(const QStringList&)),
    this, SLOT(filesSelected(const QStringList&)));
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.exec();
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importLIDARFile(const QString &fileName)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqCMBSceneTree *tree = this->Parent->getTree();

  QStringList files;
  files << fileName;
  builder->blockSignals(true);
  pqPipelineSource* source =
    builder->createReader("sources", "LIDARReader", files, tree->getCurrentServer());

  // 1st scan file, to find # of pieces and total # of points
  vtkSMSourceProxy *sourceProxy =
    vtkSMSourceProxy::SafeDownCast( source->getProxy() );
  QList<QVariant> pieceOnRatioList;
  pieceOnRatioList << 0 << VTK_INT_MAX;

  pqSMAdaptor::setMultipleElementProperty(
    sourceProxy->GetProperty("RequestedPiecesForRead"), pieceOnRatioList);
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();
  sourceProxy->UpdatePropertyInformation();

  int numberOfPieces = pqSMAdaptor::getElementProperty(
    sourceProxy->GetProperty("KnownNumberOfPieces")).toInt();

  int totalNumberOfPoints = pqSMAdaptor::getElementProperty(
    sourceProxy->GetProperty("TotalNumberOfPoints")).toInt();

  if (totalNumberOfPoints <= 0)
    {
    // We had a problem with reading the file
    // Destroy the source
    builder->destroy(source);
    QMessageBox::warning(this->MainDialog->parentWidget(),
                         tr("Problem Importing LIDAR Data"),
                         tr("There was a problem encountered when reading the file.  For example the number of points may be missing or corrupted"));
    return;
    }
  int maxNumberOfPoints = this->ImportDialog->maxNumberOfLIDARPoints->value();
  int onRatio;
  if (maxNumberOfPoints)
    {
    onRatio = ceil(static_cast<double>(totalNumberOfPoints) /
      maxNumberOfPoints);
    }
  else
    {
    onRatio = totalNumberOfPoints; // not 0, but should be 1 per piece
    }

  // create parent node
  QFileInfo finfo(fileName);
  this->Node = tree->createNode(finfo.baseName().toStdString().c_str(),
                                this->Parent, NULL, NULL);

  // Turn off event recording since undo'ing the creation of this->Node will
  // also undo the creation of all its children
  bool recordingEventsState = tree->recordingEvents();
  tree->turnOffEventRecording();

  pqCMBPoints *obj;
  vtkBoundingBox bbox;
  double bounds[6];
  for (int pieceId = 0; pieceId < numberOfPieces; pieceId++)
    {
    obj = new pqCMBPoints(
      this->Parent->getTree()->getCurrentServer(),
      this->Parent->getTree()->getCurrentView(),
      source, pieceId, onRatio, false);
    char buffer[32];
    sprintf(buffer, "Piece %d", pieceId);
    this->createObjectNode(obj, buffer, this->Node);

    obj->setFileName(fileName.toAscii().constData());
    obj->getBounds(bounds);
    bbox.AddBounds(bounds);
    }

  bbox.GetBounds(bounds);
  if (this->userRequestsDoubleData(bounds))
    {
    // delete all the children of "this->Node" (we just created them)
    std::vector<pqCMBSceneNode *> children = this->Node->getChildren();
    std::vector<pqCMBSceneNode *>::const_iterator childIter;
    for (childIter = children.begin(); childIter != children.end(); childIter++)
      {
      this->Parent->getTree()->deleteNode(*childIter, NULL);
      }

    // now, reread, but using double precision
    for (int pieceId = 0; pieceId < numberOfPieces; pieceId++)
      {
      obj = new pqCMBPoints(
        this->Parent->getTree()->getCurrentServer(),
        this->Parent->getTree()->getCurrentView(),
        source, pieceId, onRatio, true);
      char buffer[32];
      sprintf(buffer, "Piece %d", pieceId);
      this->createObjectNode(obj, buffer, this->Node);

      obj->setFileName(fileName.toAscii().constData());
      }
    }

  if (recordingEventsState)
    {
    tree->turnOnEventRecording();
    }

  builder->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importLASFile(const QString &fileName)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqCMBSceneTree *tree = this->Parent->getTree();

  QStringList files;
  files << fileName;
  builder->blockSignals(true);

  pqPipelineSource* source =
    builder->createReader("sources", "LASReader", files, tree->getCurrentServer());
  vtkSMSourceProxy *sourceProxy =
    vtkSMSourceProxy::SafeDownCast( source->getProxy() );

  // 1st scan file, to find # of pieces and total # of points
  pqSMAdaptor::setElementProperty(
    sourceProxy->GetProperty("ScanMode"), 1);
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();

  vtkSMOutputPort *outputPort = sourceProxy->GetOutputPort(static_cast<unsigned int>(0));
  vtkPVCompositeDataInformation* compositeInformation =
    outputPort->GetDataInformation()->GetCompositeDataInformation();
  int numBlocks = compositeInformation->GetNumberOfChildren();
  QString classifcationName;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  std::vector<unsigned char> classifications;
  vtkIdType totalNumberOfPoints = 0;
  for(int i = 0; i < numBlocks; i++)
    {
    pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

    vtkNew<vtkPVLASOutputBlockInformation> info;
    extract->getProxy()->GatherInformation(info.GetPointer());
    builder->destroy(extract);

    classifications.push_back(info->GetClassification());
    totalNumberOfPoints += info->GetNumberOfPointsInClassification();
    }

  int maxNumberOfPoints = this->ImportDialog->maxNumberOfLIDARPoints->value();
  int onRatio;
  if (maxNumberOfPoints)
    {
    onRatio = ceil(static_cast<double>(totalNumberOfPoints) /
      maxNumberOfPoints);
    }
  else
    {
    onRatio = totalNumberOfPoints; // not 0, but should be 1 per piece
    }

  std::vector<unsigned char>::const_iterator classificationIter;
  QList<QVariant> pieceOnRatioList;
  for (classificationIter = classifications.begin();
    classificationIter != classifications.end(); classificationIter++)
    {
    pieceOnRatioList << *classificationIter << onRatio;
    }
  pqSMAdaptor::setElementProperty(
    sourceProxy->GetProperty("ScanMode"), 0);

  QList<pqPipelineSource*> pdSources;

  pqSMAdaptor::setMultipleElementProperty(
    sourceProxy->GetProperty("RequestedClassificationsForRead"),
    pieceOnRatioList);
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();

  // compute the bounds of the dataset, so that we can decide if we need to
  // consider loading as double precision instead of float precision
  vtkBoundingBox bbox;
  for(int i = 0; i < numBlocks; i++)
    {
    pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

    vtkNew<vtkPVLASOutputBlockInformation> info;
    extract->getProxy()->GatherInformation(info.GetPointer());
    double *bounds = info->GetBounds();
    bbox.AddBounds(bounds);
    builder->destroy(extract);
    }
  double bounds[6];
  bbox.GetBounds(bounds);
  bool doublePrecision = this->userRequestsDoubleData(bounds);
  if (doublePrecision)
    {
    pqSMAdaptor::setElementProperty(
      sourceProxy->GetProperty("OutputDataTypeIsDouble"), true);
    sourceProxy->UpdateVTKObjects();
    sourceProxy->UpdatePipeline();
    }

  // create parent node
  QFileInfo finfo(fileName);
  this->Node = tree->createNode(finfo.baseName().toStdString().c_str(),
                                this->Parent, NULL, NULL);

  // Turn off event recording since undo'ing the creation of this->Node will
  // also undo the creation of all its children
  bool recordingEventsState = tree->recordingEvents();
  tree->turnOffEventRecording();

  // create a node/dataobj for each block
  for(int i = 0; i < numBlocks; i++)
    {
    pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( extract->getProxy() )->UpdatePipeline();

    pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", tree->getCurrentServer());
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
    pdSource->updatePipeline();
    builder->destroy(extract);

    vtkNew<vtkPVLASOutputBlockInformation> info;
    pdSource->getProxy()->GatherInformation(info.GetPointer());

    const char *classificationName = info->GetClassificationName();
    unsigned char classification = info->GetClassification();
    vtkIdType numberOfPointsInClassification = info->GetNumberOfPointsInClassification();

    pqCMBPoints *obj = new pqCMBPoints(pdSource,
                                             this->Parent->getTree()->getCurrentView(),
                                             this->Parent->getTree()->getCurrentServer(),
                                             false);
    obj->setFileName(fileName.toAscii().constData());

    this->createObjectNode(obj, classificationName, this->Node);

    obj->setPieceTotalNumberOfPoints( numberOfPointsInClassification );
    obj->setReaderSource(source);
    obj->setPieceId(classification);
    obj->setPieceOnRatio(onRatio);
    obj->setDoubleDataPrecision(doublePrecision);
    }

  if (recordingEventsState)
    {
    tree->turnOnEventRecording();
    }
  builder->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importUniformGrid(const QString &fileName)
{
  // pqProgressManager* progress_manager =
  //   pqApplicationCore::instance()->getProgressManager();
  // QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
  //                  this, SLOT(updateProgress(const QString&, int)));
  // if (this->Progress)
  //   {
  //   delete this->Progress;
  //   }
  // this->Progress = new QProgressDialog(this->MainDialog);
  // this->Progress->setWindowTitle(QString("Loading Uniform Grid: ") + fileName);
  // this->Progress->setMaximum(0.0);
  // this->Progress->setMinimum(0.0);

  // this->Progress->show();
  pqCMBUniformGrid *grid =
    new pqCMBUniformGrid(fileName.toAscii().constData(),
                            this->Parent->getTree()->getCurrentServer(),
                            this->Parent->getTree()->getCurrentView(),
                            false);
  if (grid->isRawDEM())
    {
    int onRatio = this->ImportDialog->onRatio->text().toInt();
    grid->setOnRatio(onRatio);
    if (this->ImportDialog->readSetCheckBox->isChecked())
      {
      grid->setReadGroupOfFiles(true);
      }
    vtkIdType rExtents[2];
    vtkIdType cExtents[2];
    int temp;
    rExtents[0] = this->ImportDialog->minRowExtent->text().toInt();
    rExtents[1] = this->ImportDialog->maxRowExtent->text().toInt();
    if (rExtents[1] < rExtents[0])
      {
      temp = rExtents[1];
      rExtents[1] = rExtents[0];
      rExtents[0] = temp;
      }
    cExtents[0] = this->ImportDialog->minColumnExtent->text().toInt();
    cExtents[1] = this->ImportDialog->maxColumnExtent->text().toInt();
    if (cExtents[1] < cExtents[0])
      {
      temp = cExtents[1];
      cExtents[1] = cExtents[0];
      cExtents[0] = temp;
      }
    grid->setExtents(rExtents, cExtents);
    }

  grid->updateRepresentation();

  QFileInfo finfo(fileName);
  this->Node = this->createObjectNode(
    grid, finfo.baseName().toStdString().c_str(), this->Parent);
  vtkSMSourceProxy::SafeDownCast(grid->getSource()->getProxy())->UpdatePipeline();

  // Reset the camera if the tree has no data objects
  if (!this->Parent->getTree()->containsDataObjects())
    {
    this->Parent->getTree()->getCurrentView()->resetCamera();
    }
  this->Parent->getTree()->getCurrentView()->forceRender();
  // delete this->Progress;
  // this->Progress = NULL;
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importBorFile(const QString &fileName)
{
  // Load the file and set up the pipeline to display the dataset.
  QStringList files;
  files << fileName;

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  builder->blockSignals(true);
  pqPipelineSource* reader = builder->createReader("sources", "CmbBorFileReader",
    files, this->Parent->getTree()->getCurrentServer());
  builder->blockSignals(false);

  if(!reader)
    {
    QMessageBox::warning(this->MainDialog->parentWidget(),
      tr("Problem Importing Bor File"),
      tr("There was a problem creating reader for the file: ").append(
      fileName));
    return;
    }
  this->Parent->getTree()->createBorFileObjects(
    fileName, reader);
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importSolidMesh(const QString &fileName)
{
  pqProgressManager* progress_manager =
  pqApplicationCore::instance()->getProgressManager();
  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
                   this, SLOT(updateProgress(const QString&, int)));
  if (this->Progress)
    {
    delete this->Progress;
    }
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setWindowTitle(QString("Loading Solid Mesh: ") + fileName);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();
  pqCMBSolidMesh *obj = new pqCMBSolidMesh(fileName.toStdString().c_str(),
     this->Parent->getTree()->getCurrentServer(),
     this->Parent->getTree()->getCurrentView(),
     false);
  QFileInfo finfo(fileName);
  this->Node = this->createObjectNode(
    obj, finfo.baseName().toStdString().c_str(), this->Parent);
  // Reset the camera if the tree has no data objects
  if (!this->Parent->getTree()->containsDataObjects())
    {
    this->Parent->getTree()->getCurrentView()->resetCamera();
    }
  this->Parent->getTree()->getCurrentView()->forceRender();
  delete this->Progress;
  this->Progress = NULL;
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::importShapeFile(const QString &fileName)
{

 pqProgressManager* progress_manager =
    pqApplicationCore::instance()->getProgressManager();
  QObject::connect(progress_manager, SIGNAL(progress(const QString&, int)),
                   this, SLOT(updateProgress(const QString&, int)));
  if (this->Progress)
    {
    delete this->Progress;
    }
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setWindowTitle(QString("Loading Shape File: ") + fileName);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqCMBSceneTree *tree = this->Parent->getTree();

  QStringList files;
  files << fileName;
  builder->blockSignals(true);

  pqPipelineSource* source =
    builder->createReader("sources", "GDALReader", files, tree->getCurrentServer());
  vtkSMSourceProxy *sourceProxy =
    vtkSMSourceProxy::SafeDownCast( source->getProxy() );
  sourceProxy->UpdateVTKObjects();
  sourceProxy->UpdatePipeline();
  sourceProxy->UpdatePropertyInformation();

  int numLayers = pqSMAdaptor::getElementProperty(
    sourceProxy->GetProperty("NumberOfLayers")).toInt();

  vtkSMProperty* activeLayerProperty = sourceProxy->GetProperty("ActiveLayer");
  vtkSMProperty *activeLayerType = sourceProxy->GetProperty("ActiveLayerType");
  vtkSMProperty *activeLayerFeatureCount = sourceProxy->GetProperty("ActiveLayerFeatureCount");

  //create the filter that extracts each block from the shape file
  pqPipelineSource* extract = builder->createFilter("filters",
      "ExtractLeafBlock", source);

  //create the filter that extracts each contour from the poly data of the block
  pqPipelineSource* contourExtract = builder->createFilter("filters",
        "ExtractCellFromDataSet", extract);

  vtkSMSourceProxy *extractProxy = vtkSMSourceProxy::SafeDownCast(extract->getProxy());
  vtkSMSourceProxy *contourExtractProxy =
    vtkSMSourceProxy::SafeDownCast(contourExtract->getProxy());

  int layerType=0, featureCount=0;

  //create the parent node for all the features of the shape file.
  //the name of the parent node will be the name of the file
  QFileInfo fileInfo(fileName);
  this->Node = tree->createNode(fileInfo.baseName().toStdString().c_str(),
                                this->Parent, NULL, NULL);

  // Turn off event recording since undo'ing the creation of this->Node will
  // also undo the creation of all its children
  bool recordingEventsState = tree->recordingEvents();
  tree->turnOffEventRecording();

  for (int i=0; i < numLayers; ++i)
    {
    pqSMAdaptor::setElementProperty(activeLayerProperty,i);
    sourceProxy->UpdatePropertyInformation();

    //get the layer type and feature count
    layerType = pqSMAdaptor::getElementProperty(activeLayerType).toInt();
    featureCount = pqSMAdaptor::getElementProperty(activeLayerFeatureCount).toInt();

    if ( layerType == VTK_POLY_LINE || layerType == VTK_POLYGON)
      {
      //extract the multiblock
      pqSMAdaptor::setElementProperty(extractProxy->GetProperty("BlockIndex"),i);
      extractProxy->UpdateProperty("BlockIndex");
      extractProxy->UpdatePipeline();

      //extract each contour from the polydata
      for (int j=0; j<featureCount;++j)
        {
        pqSMAdaptor::setElementProperty(contourExtractProxy->GetProperty("CellIndex"), j);
        contourExtractProxy->UpdateProperty("CellIndex");
        contourExtractProxy->UpdatePipeline();

        pqCMBArc* obj = new pqCMBArc(contourExtractProxy);

        QString classificationName =
          "Layer: " + QString::number(i) + " Feature: " + QString::number(j);
        this->createObjectNode(obj, classificationName.toStdString().c_str(), this->Node);
        }
      }
    }
  builder->destroy(contourExtract);
  builder->destroy(extract);
  builder->destroy(source);
  if (recordingEventsState)
    {
    tree->turnOnEventRecording();
    }

  // Reset the camera if the tree has no data objects
  if (!this->Parent->getTree()->containsDataObjects())
    {
    this->Parent->getTree()->getCurrentView()->resetCamera();
    }
  this->Parent->getTree()->getCurrentView()->forceRender();
  delete this->Progress;
  this->Progress = NULL;


}
//----------------------------------------------------------------------------
int qtCMBSceneObjectImporter::computeApproximateRepresentingFloatDigits(double min,
                                                                      double max)
{
  double maxComponent = fabs(min) > max ? fabs(min) : max;
  double logMaxComponent = maxComponent != 0 ? log10(maxComponent) : 0;
  double logRange = max - min != 0 ? log10(max - min) : 0;
  // not rounding, but  throw in 0.2 offset so that can be close to 4 digits and
  // be ok
  return static_cast<int>(7.0 - ceil(logMaxComponent - logRange - 0.2));
}

//-----------------------------------------------------------------------------
bool qtCMBSceneObjectImporter::userRequestsDoubleData(double bounds[6])
{
  // all default to float right now, but need to be able to check at some point
  int xDigits = this->computeApproximateRepresentingFloatDigits(
    bounds[0], bounds[1]);
  int yDigits = this->computeApproximateRepresentingFloatDigits(
    bounds[2], bounds[3]);
  int zDigits = this->computeApproximateRepresentingFloatDigits(
    bounds[4], bounds[5]);

  int minDigits = xDigits < yDigits ? xDigits : yDigits;
  minDigits = zDigits < minDigits ? zDigits : minDigits;

  if (minDigits < 4 && QMessageBox::question(this->MainDialog->parentWidget(),
      "Questionable float precision!",
      tr("Potentially insufficient precision with float representation (< ~").append(
      QString::number(minDigits + 1)).append(" digits).  Use double instead?"),
      QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) ==
      QMessageBox::Yes)
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::updateRowExtents()
{
  int rowExtents[2] = {this->ImportDialog->minRowExtent->text().toInt(),
                       this->ImportDialog->maxRowExtent->text().toInt()};
  if (rowExtents[1] < rowExtents[0])
    {
    int tempExtent = rowExtents[1];
    rowExtents[1] = rowExtents[0];
    rowExtents[0] = tempExtent;
    }
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::updateColumnExtents()
{
  int columnExtents[2] = {this->ImportDialog->minColumnExtent->text().toInt(),
                          this->ImportDialog->maxColumnExtent->text().toInt()};
  if (columnExtents[1] < columnExtents[0])
    {
    int tempExtent = columnExtents[1];
    columnExtents[1] = columnExtents[0];
    columnExtents[0] = tempExtent;
    }
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::updateDEMExtents()
{
  pqPipelineSource *readerSource;
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  QStringList fileList;
  fileList << this->ImportDialog->FileNameText->text();
  builder->blockSignals(true);
  readerSource = builder->createReader(
    "sources", "RawDEMReader", fileList, this->Parent->getTree()->getCurrentServer() );
  builder->blockSignals(false);

  vtkSMPropertyHelper(readerSource->getProxy(), "ReadSetOfFiles").Set(
    this->ImportDialog->readSetCheckBox->isChecked() );
  readerSource->getProxy()->UpdateVTKObjects();
  readerSource->getProxy()->InvokeCommand("GatherDimensions");
  readerSource->getProxy()->UpdatePropertyInformation();
  vtkIdType dimensions[2];
  vtkSMPropertyHelper(readerSource->getProxy(), "Dimensions").Get(dimensions, 2);

  this->MinRowValidator->setRange( 0, dimensions[1] - 1 );
  this->MaxRowValidator->setRange( 0, dimensions[1] - 1 );
  this->MinColumnValidator->setRange( 0, dimensions[0] - 1 );
  this->MaxColumnValidator->setRange( 0, dimensions[0] - 1 );

  this->ImportDialog->minRowExtent->setText("0");
  this->ImportDialog->maxRowExtent->setText( QString::number(dimensions[1] - 1) );
  this->ImportDialog->minColumnExtent->setText("0");
  this->ImportDialog->maxColumnExtent->setText( QString::number(dimensions[0] - 1) );
}

//-----------------------------------------------------------------------------
pqCMBSceneNode *qtCMBSceneObjectImporter::createObjectNode(pqCMBSceneObjectBase *obj,
                                                    const char *name,
                                                    pqCMBSceneNode *parentNode)
{
  obj->setUserDefinedType(this->ImportDialog->ObjectTypes->currentText().toAscii());
  this->assignUnits(obj);

  double data[3];
  if (this->ImportDialog->RotateX->isChecked())
    {
    data[0] = 90.0;
    data[1] = data[2] = 0.0;
    obj->setOrientation(data, false);
    }
  else if (this->ImportDialog->RotateY->isChecked())
    {
    data[1] = 90.0;
    data[0] = data[2] = 0.0;
    obj->setOrientation(data, false);
    }

  data[0] = data[1] = data[2] = this->ImportDialog->InitialScale->value();
  obj->setScale(data);

  // shouldn't be necessary?
  data[0] = data[1] = data[2] = 0.0;
  obj->setPosition(data);
  if(obj->getRepresentation())
    {
    obj->getRepresentation()->getProxy()->UpdateVTKObjects();
    vtkSMRepresentationProxy::SafeDownCast(
      obj->getRepresentation()->getProxy())->UpdatePipeline();
    obj->getRepresentation()->getProxy()->UpdatePropertyInformation();
    }

  return this->Parent->getTree()->createNode(name, parentNode, obj, NULL);
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectImporter::updateProgress(const QString&message, int progress)
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
