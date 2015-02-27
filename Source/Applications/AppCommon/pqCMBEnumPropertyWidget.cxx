/*=========================================================================

  Program:   CMB
  Module:    pqCMBEnumPropertyWidget.cxx

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

#include "pqCMBEnumPropertyWidget.h"
#include "ui_qtEnumPropertyWidget.h"

#include "vtkSMEnumerationDomain.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMRepresentationProxy.h"

#include <QLabel>
#include <QPointer>

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPropertyLinks.h"
#include "pqSignalAdaptors.h"
#include "pqSMAdaptor.h"
#include "pqUndoStack.h"

class pqCMBEnumPropertyWidgetInternal :
  public Ui::qtEnumPropertyWidget
{
public:
  QPointer<pqDataRepresentation> Display;
  pqPropertyLinks Links;
  pqSignalAdaptorComboBox* Adaptor;
};

//-----------------------------------------------------------------------------
pqCMBEnumPropertyWidget::pqCMBEnumPropertyWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internal = new pqCMBEnumPropertyWidgetInternal;
  this->Internal->setupUi(this);
  this->Internal->Links.setUseUncheckedProperties(true);

  this->Internal->Adaptor = new pqSignalAdaptorComboBox(
    this->Internal->comboBox);
  this->Internal->Adaptor->setObjectName("adaptor");

  QObject::connect(this->Internal->Adaptor,
    SIGNAL(currentTextChanged(const QString&)),
    this, SLOT(onCurrentTextChanged(const QString&)), Qt::QueuedConnection);

  QObject::connect(this->Internal->Adaptor,
    SIGNAL(currentTextChanged(const QString&)),
    this, SIGNAL(currentTextChanged(const QString&)), Qt::QueuedConnection);

  QObject::connect(&this->Internal->Links,
    SIGNAL(qtWidgetChanged()),
    this, SLOT(onQtWidgetChanged()));

  pqUndoStack* ustack = pqApplicationCore::instance()->getUndoStack();
  if (ustack)
    {
    QObject::connect(this, SIGNAL(beginUndo(const QString&)),
      ustack, SLOT(beginUndoSet(const QString&)));
    QObject::connect(this, SIGNAL(endUndo()),
      ustack, SLOT(endUndoSet()));
    }
}

//-----------------------------------------------------------------------------
pqCMBEnumPropertyWidget::~pqCMBEnumPropertyWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::setRepresentation(pqDataRepresentation* display)
{
  if(display != this->Internal->Display)
    {
    this->Internal->Display = qobject_cast<pqDataRepresentation*>(display);
    this->updateLinks();
    }
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::setPropertyName(
  const char* propName)
{
  this->RepPropertyName = propName;
  this->updateLinks();
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::setLabelText(
  const char* labelText)
{
  //this->Internal->label->setText(labelText);
  this->Internal->comboBox->setToolTip(labelText);
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::updateLinks()
{
  // break old links.
  this->Internal->Links.removeAllPropertyLinks();

  this->Internal->comboBox->setEnabled(this->Internal->Display != 0);
  this->Internal->comboBox->blockSignals(true);
  this->Internal->comboBox->clear();
  if (!this->Internal->Display)
    {
    //this->Internal->comboBox->addItem("None");
    this->Internal->comboBox->blockSignals(false);
    return;
    }

  vtkSMProxy* displayProxy = this->Internal->Display->getProxy();
  vtkSMProperty* colorbyProperty =displayProxy->
    GetProperty(this->RepPropertyName.toAscii().data());
  if (colorbyProperty)
    {
    colorbyProperty->UpdateDependentDomains();
    QList<QVariant> items =
      pqSMAdaptor::getEnumerationPropertyDomain(colorbyProperty);
    foreach(QVariant item, items)
      {
      this->Internal->comboBox->addItem(item.toString());
      }
    this->Internal->comboBox->setEnabled(true);
    this->Internal->comboBox->blockSignals(false);

    this->Internal->Links.addPropertyLink(
      this->Internal->Adaptor, "currentText",
      SIGNAL(currentTextChanged(const QString&)),
      displayProxy, colorbyProperty);
    }
  else
    {
    this->Internal->comboBox->setEnabled(false);
    this->Internal->comboBox->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::reloadGUI()
{
  this->updateLinks();
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::onQtWidgetChanged()
{
  emit this->beginUndo("Changed 'ModelFaceColorMode'");

  QString text = this->Internal->Adaptor->currentText();

  vtkSMProperty* colorbyProperty =
      this->Internal->Display->getProxy()->
      GetProperty(this->RepPropertyName.toAscii().data());
  QList<QVariant> domainStrings =
    pqSMAdaptor::getEnumerationPropertyDomain(colorbyProperty);

  if (domainStrings.contains(text))
    {
    vtkSMEnumerationDomain* ed = vtkSMEnumerationDomain::SafeDownCast(
      colorbyProperty->GetDomain("enum"));
    if(ed)
      {
      int valid;
      int colorby = ed->GetEntryValue(text.toAscii().data(), valid);
      pqSMAdaptor::setElementProperty(colorbyProperty, colorby);
      }
    else
      {
      pqSMAdaptor::setElementProperty(colorbyProperty, text);
      }
    this->Internal->Display->getProxy()->UpdateVTKObjects();
    }
  emit this->endUndo();
}

//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::onCurrentTextChanged(const QString&)
{
  if (this->Internal->Display)
    {
    this->Internal->Display->renderViewEventually();
    }
}
//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::setEnabled(int enable)
{
  this->Internal->comboBox->setEnabled(enable);
}
//-----------------------------------------------------------------------------
void pqCMBEnumPropertyWidget::setVisible(int visible)
{
  this->Internal->comboBox->setVisible(visible);

}
