//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBImportShapefile.h"
#include "ui_pqCMBImportShapefile.h"

#include "vtkCMBGeometry2DReader.h"

#include "pqFileDialog.h"

class pqCMBImportShapefile::pqInternal : public Ui::ImportShapefile
{
public:
  pqServer* ActiveServer;
};

pqCMBImportShapefile::pqCMBImportShapefile(pqServer* activeServer, QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  this->Internal = new pqInternal;
  this->Internal->ActiveServer = activeServer;
  this->Internal->setupUi(this);

  // Set up radio button groups
  this->Internal->boundaryStyleGroup->setId(
    this->Internal->radioImportAsIs, vtkCMBGeometry2DReader::NONE);
  this->Internal->boundaryStyleGroup->setId(
    this->Internal->radioAddMargin, vtkCMBGeometry2DReader::ABSOLUTE_MARGIN);
  this->Internal->boundaryStyleGroup->setId(
    this->Internal->radioManualBounds, vtkCMBGeometry2DReader::ABSOLUTE_BOUNDS);
  this->Internal->boundaryStyleGroup->setId(
    this->Internal->radioCustomBoundary, vtkCMBGeometry2DReader::IMPORTED_POLYGON);

  this->Internal->marginStyleGroup->setId(
    this->Internal->radioMarginFraction, vtkCMBGeometry2DReader::RELATIVE_MARGIN);
  this->Internal->marginStyleGroup->setId(
    this->Internal->radioManualMargin, vtkCMBGeometry2DReader::ABSOLUTE_MARGIN);

  QObject::connect(
    this->Internal->buttonBox, SIGNAL(accepted()),
    this, SLOT(accept()));
  QObject::connect(
    this->Internal->buttonBox, SIGNAL(rejected()),
    this, SLOT(reject()));
  QObject::connect(
    this->Internal->buttonCustomBoundaryFileChooser, SIGNAL(clicked()),
    this, SLOT(chooseCustomBoundaryFile()));
}

pqCMBImportShapefile::~pqCMBImportShapefile()
{
  delete this->Internal;
}

int pqCMBImportShapefile::boundaryStyle()
{
  return this->Internal->boundaryStyleGroup->checkedId();
}

int pqCMBImportShapefile::marginStyle()
{
  return this->Internal->marginStyleGroup->checkedId();
}

QString pqCMBImportShapefile::marginSpecification()
{
  switch (this->boundaryStyle())
    {
  case vtkCMBGeometry2DReader::ABSOLUTE_BOUNDS:
    return this->Internal->textManualBounds->text();
  case vtkCMBGeometry2DReader::ABSOLUTE_MARGIN:
    switch (this->marginStyle())
      {
    case vtkCMBGeometry2DReader::ABSOLUTE_MARGIN:
      return this->Internal->textManualMargin->text();
    case vtkCMBGeometry2DReader::RELATIVE_MARGIN:
      return this->Internal->textMarginFraction->text();
      }
    }
  return "NaN";
}

QString pqCMBImportShapefile::customBoundaryFilename()
{
  return this->Internal->textCustomBoundaryFileName->text();
}

void pqCMBImportShapefile::chooseCustomBoundaryFile()
{
  QString filters = "Shapefile (*.shp)";

  pqFileDialog file_dialog(
    this->Internal->ActiveServer,
    this, tr("Boundary Shapefile:"), QString(), filters);

  //file_dialog->setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("CustomBoundaryFileDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  file_dialog.setWindowModality(Qt::WindowModal);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    //each string list holds a list of files that represent a file-series
    QStringList files = file_dialog.getSelectedFiles();
    if (!files.empty())
      { // Take only the first file selected.
      this->setCustomBoundaryFile(files[0]);
      this->Internal->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
      }
    }
}

void pqCMBImportShapefile::setCustomBoundaryFile(const QString& fname)
{
  this->Internal->textCustomBoundaryFileName->setText(fname);
}
