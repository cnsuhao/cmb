//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLineWidget.h"

// Qt Includes.
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QColor>

#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "pqSMAdaptor.h"
#include <vtkSMPropertyHelper.h>
#include "vtkPVDataInformation.h"
#include "vtkSMRepresentationProxy.h"
//-----------------------------------------------------------------------------
pqCMBLineWidget::pqCMBLineWidget(vtkSMProxy* o, vtkSMProxy* pxy, QWidget* p)
  :Superclass(o, pxy, p)
{
  QVBoxLayout* l = qobject_cast<QVBoxLayout*>(this->layout());
  if (l)
    {
    QSpacerItem* spacer = new QSpacerItem(
      20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    l->addItem(spacer);
    }

   pqSMAdaptor::setElementProperty(this->getWidgetProxy()->GetProperty("Visibility"), 1);
   this->getWidgetProxy()->UpdateVTKObjects();
   vtkSMPropertyHelper(this->getWidgetProxy(), "LineColor").Get(this->originalColor, 3);
}

//-----------------------------------------------------------------------------
pqCMBLineWidget::~pqCMBLineWidget()
{
}

//-----------------------------------------------------------------------------
void pqCMBLineWidget::setColor(double c[3])
{
  this->originalColor[0] = c[0];
  this->originalColor[1] = c[1];
  this->originalColor[2] = c[2];
}
//-----------------------------------------------------------------------------
void pqCMBLineWidget::getColor(double c[3]) const
{
  c[0] = this->originalColor[0];
  c[1] = this->originalColor[1];
  c[2] = this->originalColor[2];
}
//-----------------------------------------------------------------------------
void pqCMBLineWidget::select()
{
  this->setWidgetVisible(true);
  this->setVisible(true);
  this->setLineColor(QColor::fromRgbF(1.0,0.0,1.0));
  this->Superclass::select();
  this->Superclass::updatePickShortcut(true);
}

//-----------------------------------------------------------------------------
void pqCMBLineWidget::setProcessEvents(bool val)
{
  if(vtkSMIntVectorProperty* const processProp =
    vtkSMIntVectorProperty::SafeDownCast(
    this->getWidgetProxy()->GetProperty("ProcessEvents")))
    {
    processProp->SetElement(0, val);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLineWidget::deselect()
{
  // this->Superclass::deselect();
  this->setVisible(0);
  this->setLineColor(QColor::fromRgbF(this->originalColor[0],
                                      this->originalColor[1],
                                      this->originalColor[2]));
  this->Superclass::updatePickShortcut(false);
}

//-----------------------------------------------------------------------------
void pqCMBLineWidget::updateWidgetVisibility()
{
  const bool widget_visible = this->widgetVisible();
  const bool widget_enabled = this->widgetSelected();
  this->updateWidgetState(widget_visible,  widget_enabled);
}
