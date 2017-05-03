//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Dialog for exporting DEM data.
// .SECTION Description
// .SECTION Caveats

#include "pqDEMExporterDialog.h"

#include "ui_qtDEMExporter.h"

int pqDEMExportDialog::exportToDem(QWidget* parent, pqServer* server, double* min, double* max,
  int* outRastSize, double* spacing, int* zone, bool* isnorth, double* scale)
{
  pqDEMExportDialog dialog(parent, server, min, max, spacing, *zone, *isnorth, *scale);
  int status = dialog.exec();
  if (status)
  {
    outRastSize[0] = dialog.width;
    outRastSize[1] = dialog.height;
    *zone = dialog.ExportDialog->Zone->value();
    *isnorth = dialog.ExportDialog->IsNorth->isChecked();
    *scale = dialog.ExportDialog->Scale->value();
    if (dialog.ExportDialog->DifferentFromSpacing->isChecked())
    {
      spacing[0] = dialog.ExportDialog->SearchRadius->value();
      spacing[1] = dialog.ExportDialog->SearchRadius->value();
    }
    else
    {
      spacing[0] = dialog.ExportDialog->SpacingX->value();
      spacing[1] = dialog.ExportDialog->SpacingY->value();
    }
  }
  return status;
}

void pqDEMExportDialog::accept()
{
  this->Status = 1;
}

void pqDEMExportDialog::cancel()
{
  this->Status = 0;
}

pqDEMExportDialog::pqDEMExportDialog(QWidget* parent, pqServer* server, double* min, double* max,
  double* spacing, int zone, bool isnorth, double scale)
  : Status(-1)
  , Server(server)
  , dist_w(max[0] - min[0])
  , dist_h(max[1] - min[1])
{
  this->MainDialog = new QDialog(parent);
  this->ExportDialog = new Ui::qtDEMExporter;
  this->ExportDialog->setupUi(MainDialog);
  this->width = dist_w / spacing[0];
  this->height = dist_h / spacing[1];

  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(
    this->ExportDialog->SpacingX, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
  QObject::connect(
    this->ExportDialog->SpacingY, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()));
  QObject::connect(this->ExportDialog->DifferentFromSpacing, SIGNAL(toggled(bool)),
    this->ExportDialog->SearchRadius, SLOT(setEnabled(bool)));

  this->ExportDialog->SearchRadius->setEnabled(false);
  this->ExportDialog->SearchRadius->setValue((spacing[0] + spacing[1]) * 0.5);

  this->ExportDialog->SpacingX->setValue(spacing[0]);
  this->ExportDialog->SpacingY->setValue(spacing[1]);
  this->ExportDialog->Scale->setValue(scale);

  this->ExportDialog->Zone->setValue(zone);
  this->ExportDialog->IsNorth->setChecked(isnorth);
}

pqDEMExportDialog::~pqDEMExportDialog()
{
  if (this->ExportDialog)
  {
    delete ExportDialog;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}

int pqDEMExportDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void pqDEMExportDialog::valueChanged()
{
  //compute the width and height and display them
  this->width = dist_w / this->ExportDialog->SpacingX->value();
  this->height = dist_h / this->ExportDialog->SpacingY->value();
  QString str =
    "The Raster Size: (" + QString::number(this->width) + "x" + QString::number(this->height) + ")";

  this->ExportDialog->RasterSize->setText(str);
}
