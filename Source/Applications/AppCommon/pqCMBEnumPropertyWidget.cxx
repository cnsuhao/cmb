//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
//    colorbyProperty->UpdateDependentDomains();
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
void pqCMBEnumPropertyWidget::setVisible(bool visible)
{
  this->Internal->comboBox->setVisible(visible);

}
