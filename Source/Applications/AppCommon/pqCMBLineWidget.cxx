/*=========================================================================

   Program: ParaView
   Module:    pqCMBLineWidget.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
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
