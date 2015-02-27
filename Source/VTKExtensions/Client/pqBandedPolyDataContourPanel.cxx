/*=========================================================================

   Program: ParaView
   Module:    pqBandedPolyDataContourPanel.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
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

=========================================================================*/
#include "pqBandedPolyDataContourPanel.h"
#include "ui_pqBandedPolyDataContourControls.h"

#include "pqApplicationCore.h"
#include "pqCollapsedGroup.h"
#include "pqNamedWidgets.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqPropertyManager.h"
#include "pqProxySelectionWidget.h"
#include "pqSampleScalarWidget.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkPVDataInformation.h"
#include "vtkSMStringVectorProperty.h"

#include <QCheckBox>
#include <QPointer>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////////
// pqBandedPolyDataContourPanel::pqImplementation

class pqBandedPolyDataContourPanel::pqImplementation
{
public:
  pqImplementation() :
    SampleScalarWidget(false)
  {
  }

  /// Provides a container for Qt controls
  QWidget ControlsContainer;
  /// Provides the Qt controls for the panel
  Ui::pqBandedPolyDataContourControls Controls;
  /// Controls the number and values of contours
  pqSampleScalarWidget SampleScalarWidget;
  QPointer<pqPipelineSource> PreviousInput;
};

pqBandedPolyDataContourPanel::pqBandedPolyDataContourPanel(pqProxy* object_proxy, QWidget* p) :
  base(object_proxy, p),
  Implementation(new pqImplementation())
{
  this->Implementation->Controls.setupUi(
    &this->Implementation->ControlsContainer);

  pqCollapsedGroup* const group1 = new pqCollapsedGroup(this);
  group1->setTitle(tr("Contour"));
  QVBoxLayout* l = new QVBoxLayout(group1);
  this->Implementation->ControlsContainer.layout()->setMargin(0);
  l->addWidget(&this->Implementation->ControlsContainer);

  pqCollapsedGroup* const group2 = new pqCollapsedGroup(this);
  group2->setTitle(tr(this->proxy()->GetProperty("ContourValues")->GetXMLLabel()));
  l = new QVBoxLayout(group2);
  this->Implementation->SampleScalarWidget.layout()->setMargin(0);
  l->addWidget(&this->Implementation->SampleScalarWidget);

  QVBoxLayout* const panel_layout = new QVBoxLayout(this);
  panel_layout->addWidget(group1);
  panel_layout->addWidget(group2);
  panel_layout->addStretch();

  connect(this->propertyManager(), SIGNAL(accepted()), this, SLOT(onAccepted()));
  connect(this->propertyManager(), SIGNAL(rejected()), this, SLOT(onRejected()));

  // Setup the sample scalar widget ...
  this->Implementation->SampleScalarWidget.setDataSources(
    this->proxy(),
    vtkSMDoubleVectorProperty::SafeDownCast(this->proxy()->GetProperty("ContourValues")),
    this->proxy()->GetProperty("SelectInputScalars"));

  // Link SampleScalarWidget's qProperty to vtkSMProperty
  this->propertyManager()->registerLink(
    &this->Implementation->SampleScalarWidget, "samples",
    SIGNAL(samplesChanged()), this->proxy(),
    this->proxy()->GetProperty("ContourValues"));

  pqNamedWidgets::link(
    &this->Implementation->ControlsContainer, this->proxy(), this->propertyManager());

  // Whenever input changes, we ensure that we update the enable state of the
  // "Compute Normals", "Compute Gradients" and "Compute Scalars" widgets. These
  // should be available only for structured datasets.
  QObject::connect(object_proxy, SIGNAL(producerChanged(const QString&)),
    this, SLOT(updateEnableState()), Qt::QueuedConnection);
  this->updateEnableState();
}

pqBandedPolyDataContourPanel::~pqBandedPolyDataContourPanel()
{
  delete this->Implementation;
}

void pqBandedPolyDataContourPanel::onAccepted()
{
  this->Implementation->SampleScalarWidget.accept();
}

void pqBandedPolyDataContourPanel::onRejected()
{
  this->Implementation->SampleScalarWidget.reset();
}

void pqBandedPolyDataContourPanel::updateEnableState()
{
  // Get the current input and ensure that we update the filter status when the
  // input pipeline updates (in-case the data-type changes).
  // Refer to BUG #11622.
  pqPipelineFilter* filter = qobject_cast<pqPipelineFilter*>(
    this->referenceProxy());
  pqOutputPort* cur_input = NULL;
  if (filter)
    {
    QList<pqOutputPort*> ports = filter->getAllInputs();
    cur_input = ports.size() > 0? ports[0] : NULL;
    }

  if (this->Implementation->PreviousInput != cur_input->getSource())
    {
    if (this->Implementation->PreviousInput)
      {
      QObject::disconnect(this->Implementation->PreviousInput,
        SIGNAL(dataUpdated(pqPipelineSource*)),
        this, SLOT(updateEnableState()));
      }
    this->Implementation->PreviousInput = cur_input->getSource();
    if (this->Implementation->PreviousInput)
      {
      QObject::connect(this->Implementation->PreviousInput,
        SIGNAL(dataUpdated(pqPipelineSource*)),
        this, SLOT(updateEnableState()), Qt::QueuedConnection);
      }
    }
}
