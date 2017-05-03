//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBCreateSimpleGeometry.h"

#include "ui_qtCMBCreateSimpleGeometry.h"
#include <QVBoxLayout>

#include <iostream>

//-----------------------------------------------------------------------------
qtCMBCreateSimpleGeometry::qtCMBCreateSimpleGeometry(QWidget* Parent)
  : QDialog(Parent)
  , Ui(new Ui::qtCMBCreateSimpleGeometry())
{
  this->Ui->setupUi(this);
  this->Ui->stackedWidget->setCurrentIndex(0);

  // QVBoxLayout* layout = new QVBoxLayout(this);
  // layout->addWidget(this->Ui);

  connect(this->Ui->geometryType, SIGNAL(currentIndexChanged(int)), this->Ui->stackedWidget,
    SLOT(setCurrentIndex(int)));
}

//-----------------------------------------------------------------------------
qtCMBCreateSimpleGeometry::~qtCMBCreateSimpleGeometry()
{
  delete this->Ui;
}

//-----------------------------------------------------------------------------
int qtCMBCreateSimpleGeometry::getGeometryType()
{
  return this->Ui->geometryType->currentIndex();
}

//-----------------------------------------------------------------------------
void qtCMBCreateSimpleGeometry::getGeometryValues(std::vector<double>& values)
{
  values.clear();
  switch (this->getGeometryType())
  {
    case 0:
    {
      values.resize(4);
      if (this->Ui->xMinimum->value() <= this->Ui->xMaximum->value())
      {
        values[0] = this->Ui->xMinimum->value();
        values[2] = this->Ui->xMaximum->value();
      }
      else
      {
        values[0] = this->Ui->xMaximum->value();
        values[2] = this->Ui->xMinimum->value();
      }
      if (this->Ui->yMinimum->value() <= this->Ui->yMaximum->value())
      {
        values[1] = this->Ui->yMinimum->value();
        values[3] = this->Ui->yMaximum->value();
      }
      else
      {
        values[1] = this->Ui->yMaximum->value();
        values[3] = this->Ui->yMinimum->value();
      }
      break;
    }
    case 1:
    {
      values.resize(4);
      values[0] = this->Ui->xCenter->value();
      values[1] = this->Ui->yCenter->value();
      values[2] = this->Ui->xRadius->value();
      values[3] = this->Ui->yRadius->value();
      break;
    }
    default:
      std::cerr << "qtCMBCreateSimpleGeometry: unknown geometry\n";
  }
}

//-----------------------------------------------------------------------------
void qtCMBCreateSimpleGeometry::getResolutionValues(std::vector<int>& values)
{
  values.clear();
  switch (this->getGeometryType())
  {
    case 0:
    {
      values.push_back(this->Ui->baseResolution->value());
      values.push_back(this->Ui->heightResolution->value());
      break;
    }
    case 1:
    {
      values.push_back(this->Ui->ellipseResolution->value());
      break;
    }
    default:
      std::cerr << "qtCMBCreateSimpleGeometry: unknown geometry\n";
  }
}
