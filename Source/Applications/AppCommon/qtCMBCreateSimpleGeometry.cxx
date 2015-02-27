/*=========================================================================

  Program:   CMB
  Module:    qtCMBCreateSimpleGeometry.cxx

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
#include "qtCMBCreateSimpleGeometry.h"

#include "ui_qtCMBCreateSimpleGeometry.h"
#include <QVBoxLayout>

#include <iostream>

//-----------------------------------------------------------------------------
qtCMBCreateSimpleGeometry::qtCMBCreateSimpleGeometry(QWidget* Parent) :
  QDialog(Parent),
  Ui(new Ui::qtCMBCreateSimpleGeometry())
{
  this->Ui->setupUi(this);
  this->Ui->stackedWidget->setCurrentIndex(0);

  // QVBoxLayout* layout = new QVBoxLayout(this);
  // layout->addWidget(this->Ui);

  connect(this->Ui->geometryType, SIGNAL(currentIndexChanged(int)),
          this->Ui->stackedWidget, SLOT(setCurrentIndex(int)));
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
  switch(this->getGeometryType())
    {
    case 0:
      {
      values.resize(4);
      if(this->Ui->xMinimum->value() <= this->Ui->xMaximum->value())
        {
        values[0] = this->Ui->xMinimum->value();
        values[2] = this->Ui->xMaximum->value();
        }
      else
        {
        values[0] = this->Ui->xMaximum->value();
        values[2] = this->Ui->xMinimum->value();
        }
      if(this->Ui->yMinimum->value() <= this->Ui->yMaximum->value())
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
  switch(this->getGeometryType())
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
