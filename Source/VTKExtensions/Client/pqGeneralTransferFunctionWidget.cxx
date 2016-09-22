//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqGeneralTransferFunctionWidget.h"

#include "pqCoreUtilities.h"
#include "pqTimer.h"
#include "QVTKWidget.h"
#include "vtkAxis.h"
#include "vtkBoundingBox.h"
#include "vtkChartXY.h"
#include "vtkColorTransferControlPointsItem.h"
#include "vtkColorTransferFunction.h"
#include "vtkColorTransferFunctionItem.h"
#include "vtkCompositeControlPointsItem.h"
#include "vtkCompositeTransferFunctionItem.h"
#include <vtkKochanekSpline.h>
#include "vtkSplineFunctionItem.h"
#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPiecewiseControlPointsItem.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPiecewiseFunctionItem.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkSpline.h"

#include <QVBoxLayout>
#include <QPointer>
#include <QColorDialog>
#include <algorithm>
#include <vector>

class CMBControlPointItem : public vtkCompositeControlPointsItem
{
public:
  friend class pqGeneralTransferFunctionWidget;
  static CMBControlPointItem* New()
  {
    CMBControlPointItem* r = new CMBControlPointItem();
    r->init();
    return r;
  }
  CMBControlPointItem* GetPointer()
  {
    return this;
  }
  void init(vtkPiecewiseFunction * pwf = NULL)
  {
    this->SetOpacityFunction(pwf);
    this->SetEndPointsXMovable(false);
    this->SetEndPointsYMovable(true);
    this->SetUseOpacityPointHandles(true);
    this->SetLabelFormat("%.3f: %.3f");
    this->SetPointsFunction(2);
    min_x = -1;
    max_x =  1;
    min_y = -1;
    max_y =  1;
  }
  vtkStdString GetControlPointLabel(vtkIdType pointId) override
  {
    vtkStdString result;
    if (this->LabelFormat)
    {
      char *buffer = new char[1024];
      double point[4];
      this->GetControlPoint(pointId, point);
      double x = point[0];
      double y = point[1];
      if(x < 0)
      {
        x = x*-min_x;
      }
      else
      {
        x = x*max_x;
      }
      y = min_y + y*(max_y-min_y);
      sprintf(buffer, this->LabelFormat, x, y, point[2], point[3]);
      result = buffer;
      delete []buffer;
    }
    return result;
  }
  double min_x;
  double max_x;
  double min_y;
  double max_y;
};

//-----------------------------------------------------------------------------
// We extend vtkChartXY to add logic to reset view bounds automatically. This
// ensures that when LUT changes, we are always showing the complete LUT.
class vtkTransferFunctionChartXY : public vtkChartXY
{
  double XRange[2];
  bool DataValid;
public:
  static vtkTransferFunctionChartXY* New();
  vtkTypeMacro(vtkTransferFunctionChartXY, vtkChartXY);
  vtkWeakPointer<CMBControlPointItem> ControlPointsItem;

  // Description:
  // Perform any updates to the item that may be necessary before rendering.
  // The scene should take care of calling this on all items before their
  // Paint function is invoked.
  void Update() override
    {
    if (this->ControlPointsItem)
      {
      // Reset bounds if the control points' bounds have changed.
      double bounds[4];
      this->ControlPointsItem->GetBounds(bounds);
      this->SetVisible(true);
      if (bounds[0] <= bounds[1] &&
          (bounds[0] != this->XRange[0] || bounds[1] != this->XRange[1]))
        {
        this->XRange[0] = bounds[0];
        this->XRange[1] = bounds[1];
        this->DataValid = ((this->XRange[1] - this->XRange[0]) >= 1e-10);
        this->RecalculateBounds();
        }
      }
    this->Superclass::Update();
    }

  bool PaintChildren(vtkContext2D *painter) override
    {
    if (this->DataValid)
      {
      return this->Superclass::PaintChildren(painter);
      }
    painter->DrawString(5, 5, "Data range too small to render.");
    return true;
    }

  bool MouseEnterEvent(const vtkContextMouseEvent &mouse) override
    {
    return (this->DataValid? this->Superclass::MouseEnterEvent(mouse) : false);
    }
  bool MouseMoveEvent(const vtkContextMouseEvent &mouse) override
    {
    return (this->DataValid? this->Superclass::MouseMoveEvent(mouse) : false);
    }
  bool MouseLeaveEvent(const vtkContextMouseEvent &mouse) override
    {
    return (this->DataValid? this->Superclass::MouseLeaveEvent(mouse) : false);
    }
  bool MouseButtonPressEvent(const vtkContextMouseEvent &mouse) override
    {
    return (this->DataValid? this->Superclass::MouseButtonPressEvent(mouse) : false);
    }
  bool MouseButtonReleaseEvent(const vtkContextMouseEvent &mouse) override
    {
    return (this->DataValid? this->Superclass::MouseButtonReleaseEvent(mouse) : false);
    }
  bool MouseWheelEvent(const vtkContextMouseEvent &mouse, int delta) override
    {
    return (this->DataValid? this->Superclass::MouseWheelEvent(mouse, delta) : false);
    }
  bool KeyPressEvent(const vtkContextKeyEvent &key) override
    {
    return (this->DataValid? this->Superclass::KeyPressEvent(key) : false);
    }

protected:
  vtkTransferFunctionChartXY()
    {
    this->XRange[0] = this->XRange[1] = 0.0;
    this->DataValid = false;
    }
  ~vtkTransferFunctionChartXY() override {}

private:
  vtkTransferFunctionChartXY(const vtkTransferFunctionChartXY&);
  void operator=(const vtkTransferFunctionChartXY&);
};

vtkStandardNewMacro(vtkTransferFunctionChartXY);

//-----------------------------------------------------------------------------

class pqGeneralTransferFunctionWidget::pqInternals
{
public:
  QPointer<QVTKWidget> Widget;
  vtkNew<vtkTransferFunctionChartXY> ChartXY;
  vtkNew<vtkContextView> ContextView;
  vtkNew<vtkEventQtSlotConnect> VTKConnect;

  bool UseSpline;
  double Controls[3];

  pqTimer Timer;

  vtkSmartPointer<vtkSplineFunctionItem> TransferFunctionItem;
  vtkSmartPointer<CMBControlPointItem> ControlPointsItem;
  unsigned long CurrentPointEditEventId;

  pqInternals(pqGeneralTransferFunctionWidget* editor):
    Widget(new QVTKWidget(editor)),
    CurrentPointEditEventId(0)
    {
    this->UseSpline = false;
    Controls[0] = 0;
    Controls[1] = 0;
    Controls[2] = 0;

    this->Timer.setSingleShot(true);
    this->Timer.setInterval(0);

    this->ChartXY->SetAutoSize(true);
    this->ChartXY->SetShowLegend(false);
    this->ChartXY->SetForceAxesToBounds(true);
    this->ContextView->GetScene()->AddItem(this->ChartXY.GetPointer());
    this->ContextView->SetInteractor(this->Widget->GetInteractor());
    this->Widget->SetRenderWindow(this->ContextView->GetRenderWindow());
    this->ContextView->GetRenderWindow()->SetLineSmoothing(true);

    this->ChartXY->SetActionToButton(vtkChart::PAN, -1);
    this->ChartXY->SetActionToButton(vtkChart::ZOOM, -1);
    this->ChartXY->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::RIGHT_BUTTON);
    this->ChartXY->SetActionToButton(vtkChart::SELECT_POLYGON, -1);

    this->Widget->setParent(editor);
    QVBoxLayout* layout = new QVBoxLayout(editor);
    layout->setMargin(0);
    layout->addWidget(this->Widget);

    this->ChartXY->SetAutoAxes(false);
    this->ChartXY->SetHiddenAxisBorder(8);
    for (int cc=0; cc < 4; cc++)
      {
      this->ChartXY->GetAxis(cc)->SetVisible(false);
      this->ChartXY->GetAxis(cc)->SetBehavior(vtkAxis::AUTO);
      }
    }
  ~pqInternals()
    {
    this->cleanup();
    }

  void cleanup()
    {
    this->VTKConnect->Disconnect();
    this->ChartXY->ClearPlots();
    if (this->ControlPointsItem && this->CurrentPointEditEventId)
      {
      this->ControlPointsItem->RemoveObserver(this->CurrentPointEditEventId);
      this->CurrentPointEditEventId = 0;
      }
    //this->ControlPointsItem.clear();
    ControlPointsItem = NULL;
    }
};

//-----------------------------------------------------------------------------
pqGeneralTransferFunctionWidget::pqGeneralTransferFunctionWidget(QWidget* parentObject)
  : Superclass(parentObject),
  Internals(new pqInternals(this))
{
  QObject::connect(&this->Internals->Timer, SIGNAL(timeout()), this,
    SLOT(renderInternal()));
}

//-----------------------------------------------------------------------------
pqGeneralTransferFunctionWidget::~pqGeneralTransferFunctionWidget()
{
  delete this->Internals;
  this->Internals = NULL;
}

void pqGeneralTransferFunctionWidget::clear()
{
  this->Internals->cleanup();
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::addFunction(vtkPiecewiseFunction * pwf, bool pwf_editable)
{
  vtkNew<vtkSplineFunctionItem> item;
  item->SetSplineFunction(pwf);
  this->Internals->TransferFunctionItem = item.GetPointer();
  this->Internals->TransferFunctionItem->SetDrawAsSpline(this->Internals->UseSpline);
  this->Internals->TransferFunctionItem->SetControls(this->Internals->Controls[0],
                                                     this->Internals->Controls[1],
                                                     this->Internals->Controls[2]);
  if (pwf == NULL)
    {
    this->Internals->ControlPointsItem = NULL;
    }
  else
    {
    if (pwf_editable)
      {
      vtkNew<CMBControlPointItem> cpItem;
      cpItem->init(pwf);
      this->Internals->ControlPointsItem = cpItem.GetPointer();
      }
    }

  this->Internals->ChartXY->AddPlot(this->Internals->TransferFunctionItem);

  if (this->Internals->ControlPointsItem)
    {
    this->Internals->ChartXY->ControlPointsItem =
      this->Internals->ControlPointsItem;
    this->Internals->ControlPointsItem->SetEndPointsRemovable(false);
    this->Internals->ControlPointsItem->SetShowLabels(true);
    this->Internals->ChartXY->AddPlot(this->Internals->ControlPointsItem);

    pqCoreUtilities::connect(this->Internals->ControlPointsItem,
      vtkControlPointsItem::CurrentPointChangedEvent,
      this, SLOT(onCurrentChangedEvent()));
    pqCoreUtilities::connect(this->Internals->ControlPointsItem,
      vtkCommand::EndEvent,
      this, SIGNAL(controlPointsModified()));
    }

  // If the transfer functions change, we need to re-render the view. This
  // ensures that.
  if (pwf)
    {
    this->Internals->VTKConnect->Connect(
      pwf, vtkCommand::ModifiedEvent, this, SLOT(render()));
    }
}

std::size_t pqGeneralTransferFunctionWidget::getNumberOfFunctions() const
{
  return (this->Internals->TransferFunctionItem != NULL)? 1 : 0;
}

/// Change function to what is passed
bool pqGeneralTransferFunctionWidget::changeFunction(std::size_t at,
                                                     vtkPiecewiseFunction* pwf,
                                                     bool pwf_editable)
{
  if (this->Internals->TransferFunctionItem == NULL)
    {
    return false;
    }
  if(at != 0) return false;

  //TODO A better prepare function for changing function
  this->Internals->cleanup();

  if (pwf == NULL)
  {
    //vtkNew<vtkPiecewiseFunctionItem> item;
    //item->SetPiecewiseFunction(pwf);
    vtkNew<vtkSplineFunctionItem> item;

    this->Internals->TransferFunctionItem = item.GetPointer();
    this->Internals->TransferFunctionItem->SetDrawAsSpline(this->Internals->UseSpline);
    this->Internals->TransferFunctionItem->SetControls(this->Internals->Controls[0],
                                                       this->Internals->Controls[1],
                                                       this->Internals->Controls[2]);

    this->Internals->ControlPointsItem = NULL;
  }
  else
  {
    vtkNew<vtkSplineFunctionItem> item;

    item->SetSplineFunction(pwf);

    this->Internals->TransferFunctionItem = item.GetPointer();
    this->Internals->TransferFunctionItem->SetDrawAsSpline(this->Internals->UseSpline);
    this->Internals->TransferFunctionItem->SetControls(this->Internals->Controls[0],
                                                       this->Internals->Controls[1],
                                                       this->Internals->Controls[2]);

    if (pwf_editable)
    {
      vtkNew<CMBControlPointItem> cpItem;
      cpItem->init(pwf);
      this->Internals->ControlPointsItem = cpItem.GetPointer();
    }
  }
  //TODO a better finalizing of change point
  this->Internals->ChartXY->AddPlot(this->Internals->TransferFunctionItem);

  if (this->Internals->ControlPointsItem)
  {
    this->Internals->ChartXY->ControlPointsItem =
    this->Internals->ControlPointsItem;
    this->Internals->ControlPointsItem->SetEndPointsRemovable(false);
    this->Internals->ControlPointsItem->SetShowLabels(true);
    this->Internals->ChartXY->AddPlot(this->Internals->ControlPointsItem);

    pqCoreUtilities::connect(this->Internals->ControlPointsItem,
                             vtkControlPointsItem::CurrentPointChangedEvent,
                             this, SLOT(onCurrentChangedEvent()));
    pqCoreUtilities::connect(this->Internals->ControlPointsItem,
                             vtkCommand::EndEvent,
                             this, SIGNAL(controlPointsModified()));
  }

  // If the transfer functions change, we need to re-render the view. This
  // ensures that.
  if (pwf)
  {
    this->Internals->VTKConnect->Connect(pwf, vtkCommand::ModifiedEvent, this, SLOT(render()));
  }


  return true;
}

bool pqGeneralTransferFunctionWidget::changeEditablity(std::size_t at, bool editable)
{
  if (this->Internals->TransferFunctionItem == NULL)
  {
    return false;
  }
  if(at != 0) return false;

  if(this->Internals->TransferFunctionItem->GetPiecewiseFunction())
  {
    return false;
  }

  if(editable)
  {
    vtkNew<CMBControlPointItem> cpItem;
    cpItem->init(this->Internals->TransferFunctionItem->GetPiecewiseFunction());
    this->Internals->ControlPointsItem = cpItem.GetPointer();
  }
  else
  {
    this->Internals->ControlPointsItem = NULL;
  }
  return true;
}

/// Change visablity
bool pqGeneralTransferFunctionWidget::changeVisablity(std::size_t at, bool visable)
{
  //TODO implement me
  return false;
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::onCurrentPointEditEvent()
{
  vtkColorTransferControlPointsItem* cpitem =
    vtkColorTransferControlPointsItem::SafeDownCast(this->Internals->ControlPointsItem);
  if (cpitem == NULL)
    {
    return;
    }

  vtkIdType currentIdx = cpitem->GetCurrentPoint();
  if (currentIdx < 0)
    {
    return;
    }

  vtkColorTransferFunction* ctf = cpitem->GetColorTransferFunction();
  Q_ASSERT(ctf != NULL);

  double xrgbms[6];
  ctf->GetNodeValue(currentIdx, xrgbms);
  QColor color = QColorDialog::getColor(
    QColor::fromRgbF(xrgbms[1], xrgbms[2], xrgbms[3]), this,
    "Select Color", QColorDialog::DontUseNativeDialog);
  if (color.isValid())
    {
    xrgbms[1] = color.redF();
    xrgbms[2] = color.greenF();
    xrgbms[3] = color.blueF();
    ctf->SetNodeValue(currentIdx, xrgbms);

    emit this->controlPointsModified();
    }
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::onCurrentChangedEvent()
{
  if (this->Internals->ControlPointsItem)
    {
    emit this->currentPointChanged(
      this->Internals->ControlPointsItem->GetCurrentPoint());
    }
}

//-----------------------------------------------------------------------------
vtkIdType pqGeneralTransferFunctionWidget::currentPoint() const
{
  if (this->Internals->ControlPointsItem)
    {
    return this->Internals->ControlPointsItem->GetCurrentPoint();
    }

  return -1;
}


//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::setCurrentPoint(vtkIdType index)
{
  if (this->Internals->ControlPointsItem)
    {
    if (index  < -1 || index >=
      this->Internals->ControlPointsItem->GetNumberOfPoints())
      {
      index = -1;
      }
    this->Internals->ControlPointsItem->SetCurrentPoint(index);
    }
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::render()
{
  this->Internals->Timer.start();
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::renderInternal()
{
  if (this->isVisible() &&
      this->Internals->ContextView->GetRenderWindow()->IsDrawable())
    {
    this->Internals->ContextView->GetRenderWindow()->Render();
    }
}

//-----------------------------------------------------------------------------
void pqGeneralTransferFunctionWidget::setCurrentPointPosition(double xpos)
{
  vtkIdType currentPid = this->currentPoint();
  if (currentPid < 0)
    {
    return;
    }

  vtkIdType numPts = this->Internals->ControlPointsItem->GetNumberOfPoints();
  if (currentPid >= 0)
    {
    double start_point[4];
    this->Internals->ControlPointsItem->GetControlPoint(0, start_point);
    xpos = std::max(start_point[0], xpos);
    }
  if (currentPid <= (numPts -1))
    {
    double end_point[4];
    this->Internals->ControlPointsItem->GetControlPoint(numPts-1, end_point);
    xpos = std::min(end_point[0], xpos);
    }

  double point[4];
  this->Internals->ControlPointsItem->GetControlPoint(currentPid, point);
  if (point[0] != xpos)
    {
    point[0] = xpos;
    this->Internals->ControlPointsItem->SetControlPoint(currentPid, point);
    }
}

void pqGeneralTransferFunctionWidget::setMinX(double d)
{
  if(this->Internals->ControlPointsItem != NULL)
    {
    this->Internals->ControlPointsItem->min_x = d;
    emit this->controlPointsModified();
    }
}

void pqGeneralTransferFunctionWidget::setMaxX(double d)
{
  if(this->Internals->ControlPointsItem != NULL)
    {
    this->Internals->ControlPointsItem->max_x = d;
    emit this->controlPointsModified();
    }
}

void pqGeneralTransferFunctionWidget::setMinY(double d)
{
  if(this->Internals->ControlPointsItem != NULL)
    {
    this->Internals->ControlPointsItem->min_y = d;
    emit this->controlPointsModified();
    }
}

void pqGeneralTransferFunctionWidget::setMaxY(double d)
{
  if(this->Internals->ControlPointsItem != NULL)
    {
    this->Internals->ControlPointsItem->max_y = d;
    emit this->controlPointsModified();
    }
}

void pqGeneralTransferFunctionWidget::setFunctionType(bool b)
{
  this->Internals->UseSpline = b;
  if(this->Internals->TransferFunctionItem)
    {
    this->Internals->TransferFunctionItem->SetDrawAsSpline(this->Internals->UseSpline);
    this->Internals->TransferFunctionItem->SetControls(this->Internals->Controls[0],
                                                       this->Internals->Controls[1],
                                                       this->Internals->Controls[2]);
    //this->Internals->TransferFunctionItem->RefreshTexture();
    this->renderInternal();
    }
}

void pqGeneralTransferFunctionWidget::setSplineControl(double tension, double continuity, double bias)
{
  this->Internals->Controls[0] = tension;
  this->Internals->Controls[1] = continuity;
  this->Internals->Controls[2] = bias;
  if(this->Internals->TransferFunctionItem)
    {
    this->Internals->TransferFunctionItem->SetControls(this->Internals->Controls[0],
                                                       this->Internals->Controls[1],
                                                       this->Internals->Controls[2]);
    this->Internals->TransferFunctionItem->RefreshTexture();
    this->renderInternal();
    }
}
