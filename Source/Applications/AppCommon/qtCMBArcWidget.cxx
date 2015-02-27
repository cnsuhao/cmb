/*=========================================================================

   Program: ParaView
   Module:    qtCMBArcWidget.cxx

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
#include "qtCMBArcWidget.h"
#include "ui_qtContourWidget.h"

#include "pq3DWidgetFactory.h"
#include "pqApplicationCore.h"
#include "pqPropertyLinks.h"
#include "pqServerManagerModel.h"
#include "pqSignalAdaptorTreeWidget.h"
#include "pqSMAdaptor.h"
#include "pqView.h"

#include <QDoubleValidator>
#include <QShortcut>
#include <QtDebug>
#include <QMessageBox>

#include "vtkEventQtSlotConnect.h"
#include "vtkSmartPointer.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"

#include "vtkAbstractWidget.h"
#include "vtkCommand.h"

#include "vtkClientServerStream.h"

class qtCMBArcWidget::pqInternals : public Ui::qtContourWidget
{
public:
  vtkSmartPointer<vtkEventQtSlotConnect> ClosedLoopConnect;
};

//-----------------------------------------------------------------------------
qtCMBArcWidget::qtCMBArcWidget(
  vtkSMProxy* _smproxy, vtkSMProxy* pxy, QWidget* p) :
  Superclass(_smproxy, pxy, p)
{
  this->Internals = new pqInternals();
  this->Internals->ClosedLoopConnect =
    vtkSmartPointer<vtkEventQtSlotConnect>::New();

  this->Internals->setupUi(this);

  this->Internals->Visibility->setChecked(this->widgetVisible());
  QObject::connect(this, SIGNAL(widgetVisibilityChanged(bool)),
    this->Internals->Visibility, SLOT(setChecked(bool)));

  QObject::connect(this->Internals->Visibility,
    SIGNAL(toggled(bool)), this, SLOT(setWidgetVisible(bool)));

  QObject::connect(this->Internals->Closed,
    SIGNAL(toggled(bool)), this, SLOT(closeLoop(bool)));

  QObject::connect(this->Internals->Delete, SIGNAL(clicked()),
    this, SLOT(deleteAllNodes()));

  QObject::connect(this->Internals->EditMode, SIGNAL(toggled(bool)),
    this, SLOT(updateMode()));
  QObject::connect(this->Internals->ModifyMode, SIGNAL(toggled(bool)),
    this, SLOT(updateMode()));
  QObject::connect(this->Internals->Finished, SIGNAL(clicked()),
    this, SLOT(finishContour()));
  QObject::connect(this->Internals->buttonRectArc, SIGNAL(clicked()),
    this, SLOT(generateRectangleArc()));


  pqServerManagerModel* smmodel =
    pqApplicationCore::instance()->getServerManagerModel();
  this->createWidget(smmodel->findServer(_smproxy->GetSession()));
}

//-----------------------------------------------------------------------------
qtCMBArcWidget::~qtCMBArcWidget()
{
  this->cleanupWidget();
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::createWidget(pqServer* server)
{
  vtkSMNewWidgetRepresentationProxy* widget = NULL;
  widget = pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("ArcWidgetRepresentation", server);

  this->setWidgetProxy(widget);

  widget->UpdateVTKObjects();
  widget->UpdatePropertyInformation();

  this->Internals->ClosedLoopConnect->Connect(
    widget, vtkCommand::EndInteractionEvent,
    this, SLOT(checkContourLoopClosed()));
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::cleanupWidget()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();

  if (widget)
    {
    widget->InvokeCommand("Initialize");
    pqApplicationCore::instance()->get3DWidgetFactory()->
      free3DWidget(widget);
    }
  this->setWidgetProxy(0);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::select()
{
  this->setWidgetVisible(true);
  this->setLineColor(QColor::fromRgbF(1.0,0.0,1.0));
  this->Superclass::select();
  this->Superclass::updatePickShortcut(true);
  vtkSMPropertyHelper(this->getWidgetProxy(), "Enabled").Set(1);
  this->getWidgetProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::deselect()
{
  this->setLineColor(QColor::fromRgbF(1.0,1.0,1.0));
  this->Superclass::updatePickShortcut(false);
  this->setEnabled(0);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::updateWidgetVisibility()
{
  const bool widget_visible = this->widgetVisible();
  const bool widget_enabled = this->widgetSelected();
  this->updateWidgetState(widget_visible,  widget_enabled);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::deleteAllNodes()
{
  QMessageBox msgBox;
  msgBox.setText("Delete all contour nodes.");
  msgBox.setInformativeText("Do you want to delete everything you have drawn?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
    {
    this->removeAllNodes();
    }
}
//-----------------------------------------------------------------------------
void qtCMBArcWidget::removeAllNodes()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget)
    {
    widget->InvokeCommand("ClearAllNodes");
    widget->InvokeCommand("Initialize");
    this->setModified();
    this->render();
    }

}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::checkContourLoopClosed()
{
  vtkSMProxy* repProxy = this->getWidgetProxy()->GetRepresentationProxy();

  //request from the info the state of the loop not on the property it self
  vtkSMPropertyHelper loopHelper(repProxy,"ClosedLoopInfo");
  loopHelper.UpdateValueFromServer();

  int loopClosed = loopHelper.GetAsInt();
  this->Internals->Closed->setChecked(loopClosed);
  if(loopClosed)
    {
    this->ModifyMode();
    emit this->contourLoopClosed();
    }

}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::closeLoop(bool val)
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget)
    {
    vtkSMProxy* repProxy = widget->GetRepresentationProxy();
    vtkSMPropertyHelper loopHelper(repProxy,"ClosedLoop");
    if(val)
      {
      widget->InvokeCommand("CloseLoop");
      }
    this->Internals->ModifyMode->setChecked(val);
    loopHelper.Set(val);
    repProxy->UpdateVTKObjects();

    this->setModified();
    this->render();
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::ModifyMode()
{
  this->Internals->ModifyMode->setChecked(true);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::checkCanBeEdited()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget)
    {
    vtkSMProxy* repProxy = widget->GetRepresentationProxy();
    vtkSMPropertyHelper canEditHelper(repProxy,"CanEdit");
    canEditHelper.UpdateValueFromServer();
    int canEdit = canEditHelper.GetAsInt();
    if (!canEdit)
      {
      this->Internals->ModifyMode->setChecked(true);
      this->Internals->EditMode->setDisabled(true);
      this->Internals->Closed->setChecked(false);
      this->Internals->Closed->setDisabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::updateMode()
{
  //the text should always be updated to this.
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget)
    {
    if (this->Internals->EditMode->isChecked() )
      {
       pqSMAdaptor::setElementProperty(
        widget->GetProperty("WidgetState"), 1);
      }
    else if (this->Internals->ModifyMode->isChecked() )
      {
      pqSMAdaptor::setElementProperty(
        widget->GetProperty("WidgetState"), 2);
      }
    widget->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::finishContour( )
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  widget->GetWidget()->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  emit this->contourDone();
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::generateRectangleArc( )
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget)
    {
    widget->InvokeCommand("Rectangularize");
    this->setModified();
    this->render();
    }
  emit this->contourDone();
}

//----------------------------------------------------------------------------
void qtCMBArcWidget::setPointPlacer(vtkSMProxy* placerProxy)
{
  this->updateRepProperty(placerProxy, "PointPlacer");
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::setLineInterpolator(vtkSMProxy* interpProxy)
{
  this->updateRepProperty(interpProxy, "LineInterpolator");
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::reset()
{
  this->Superclass::reset();

  //update our mode
  this->Internals->EditMode->setDisabled(false);
  this->Internals->EditMode->setChecked(true);
  this->Internals->Closed->blockSignals(true);
  this->Internals->Closed->setEnabled(true);
  this->Internals->Closed->setChecked(false);
  this->Internals->Closed->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::setLineColor(const QColor& color)
{
  vtkSMProxy* widget = this->getWidgetProxy();
  vtkSMPropertyHelper(widget,
    "LineColor").Set(0, color.redF());
  vtkSMPropertyHelper(widget,
    "LineColor").Set(1,color.greenF());
  vtkSMPropertyHelper(widget,
    "LineColor").Set(2 , color.blueF());
  widget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::updateRepProperty(
  vtkSMProxy* smProxy, const char* propertyName)
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (widget && propertyName && *propertyName)
    {
    vtkSMProxyProperty* proxyProp =
      vtkSMProxyProperty::SafeDownCast(
        widget->GetProperty(propertyName));
    if (proxyProp)
      {
      proxyProp->RemoveAllProxies();
      proxyProp->AddProxy(smProxy);
      widget->UpdateProperty(propertyName);
      }
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidget::useArcEditingUI(bool isWholeArc)
{
  this->Internals->buttonRectArc->setVisible(isWholeArc);
  this->Internals->Delete->setVisible(false);
  this->Internals->Closed->setEnabled(isWholeArc);
}
