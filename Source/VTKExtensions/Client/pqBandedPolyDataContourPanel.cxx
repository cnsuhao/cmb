//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

  if (
    (!cur_input && this->Implementation->PreviousInput) ||
    this->Implementation->PreviousInput != cur_input->getSource())
    {
    if (this->Implementation->PreviousInput)
      {
      QObject::disconnect(this->Implementation->PreviousInput,
        SIGNAL(dataUpdated(pqPipelineSource*)),
        this, SLOT(updateEnableState()));
      }
    if (cur_input)
      this->Implementation->PreviousInput = cur_input->getSource();
    else
      this->Implementation->PreviousInput = static_cast<pqPipelineSource*>(NULL);
    if (this->Implementation->PreviousInput)
      {
      QObject::connect(this->Implementation->PreviousInput,
        SIGNAL(dataUpdated(pqPipelineSource*)),
        this, SLOT(updateEnableState()), Qt::QueuedConnection);
      }
    }
}
