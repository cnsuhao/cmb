
#include "pqCMBColorEditor.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqProxyWidget.h"
#include "vtkCommand.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkWeakPointer.h"

#include <QPointer>
#include <QVBoxLayout>

class pqCMBColorEditor::pqInternals
{
public:
  QPointer<pqProxyWidget> ProxyWidget;
  QPointer<pqDataRepresentation> ActiveRepresentation;
  unsigned long ObserverId;

  pqInternals(pqCMBColorEditor* self) : ObserverId(0)
    {
    }

  ~pqInternals()
    {
    }
};

//-----------------------------------------------------------------------------
pqCMBColorEditor::pqCMBColorEditor(QWidget* parentObject)
  : Superclass(parentObject),
  Internals(new pqCMBColorEditor::pqInternals(this))
{
//  pqActiveObjects *activeObjects = &pqActiveObjects::instance();
//  this->connect(activeObjects, SIGNAL(representationChanged(pqDataRepresentation*)),
//    this, SLOT(updateActive()));
  new QVBoxLayout(this);
  this->layout()->setMargin(0);
  this->updateRepresentation();
}

//-----------------------------------------------------------------------------
pqCMBColorEditor::~pqCMBColorEditor()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBColorEditor::updatePanel()
{
  if (this->Internals->ProxyWidget)
    {
    this->Internals->ProxyWidget->filterWidgets(true);
    }
}

//-----------------------------------------------------------------------------
void pqCMBColorEditor::updateRepresentation()
{
  //  pqDataRepresentation* repr =
  //    pqActiveObjects::instance().activeRepresentation();

  pqDataRepresentation* repr= this->Internals->ActiveRepresentation;
  // Set the current LUT proxy to edit.
  if (repr && vtkSMPVRepresentationProxy::GetUsingScalarColoring(repr->getProxy()))
    {
    this->setColorTransferFunction(
      vtkSMPropertyHelper(repr->getProxy(), "LookupTable", true).GetAsProxy());
    }
  else
    {
    this->setColorTransferFunction(NULL);
    }
}

//-----------------------------------------------------------------------------
void pqCMBColorEditor::setDataRepresentation(pqDataRepresentation* repr)
{
  // this method sets up hooks to ensure that when the repr's properties are
  // modified, the editor shows the correct LUT.
  if (this->Internals->ActiveRepresentation == repr)
    {
    return;
    }

  if (this->Internals->ActiveRepresentation)
    {
    // disconnect signals.
    if (this->Internals->ObserverId)
      {
      this->Internals->ActiveRepresentation->getProxy()->RemoveObserver(
        this->Internals->ObserverId);
      }
    }

  this->Internals->ObserverId = 0;
  this->Internals->ActiveRepresentation = repr;
  if (repr && repr->getProxy())
    {
    this->Internals->ObserverId = repr->getProxy()->AddObserver(
      vtkCommand::PropertyModifiedEvent, this, &pqCMBColorEditor::updateRepresentation);
    }
  this->updateRepresentation();
}

//-----------------------------------------------------------------------------
void pqCMBColorEditor::setColorTransferFunction(vtkSMProxy* ctf)
{
  if (this->Internals->ProxyWidget == NULL && ctf == NULL)
    {
    return;
    }
  if (this->Internals->ProxyWidget && ctf &&
    this->Internals->ProxyWidget->proxy() == ctf)
    {
    return;
    }

  if ( (ctf==NULL && this->Internals->ProxyWidget) ||
       (this->Internals->ProxyWidget && ctf && this->Internals->ProxyWidget->proxy() != ctf))
    {
    this->layout()->removeWidget(this->Internals->ProxyWidget);
    delete this->Internals->ProxyWidget;
    }

  if (!ctf)
    {
    return;
    }

  pqProxyWidget* widget = new pqProxyWidget(ctf, this);
  widget->setObjectName("Properties");
  widget->setApplyChangesImmediately(true);
  widget->filterWidgets();

  this->layout()->addWidget(widget);

  this->Internals->ProxyWidget = widget;
  this->updatePanel();

  QObject::connect(widget, SIGNAL(changeFinished()), this, SLOT(renderViews()));
}

//-----------------------------------------------------------------------------
void pqCMBColorEditor::renderViews()
{
  if (this->Internals->ActiveRepresentation)
    {
    this->Internals->ActiveRepresentation->renderViewEventually();
    }
}
