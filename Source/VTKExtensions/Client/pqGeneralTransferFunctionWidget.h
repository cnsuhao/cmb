/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

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
#ifndef __pqGeneralTransferFunctionWidget_h
#define __pqGeneralTransferFunctionWidget_h

#include <QWidget>
#include "pqComponentsModule.h"
#include "vtkType.h"
#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h"

class vtkScalarsToColors;
class vtkPiecewiseFunction;

/// pqGeneralTransferFunctionWidget provides a widget that can edit the control points
/// in a vtkPiecewiseFunction (or subclass).
class VTKCMBCLIENT_EXPORT pqGeneralTransferFunctionWidget : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;
public:
  pqGeneralTransferFunctionWidget(QWidget* parent=0);
  virtual ~pqGeneralTransferFunctionWidget();

  /// Clears the functions
  void clear();

  /// Add a function to the widget.
  void addFunction(vtkPiecewiseFunction* pwf, bool pwf_editable);

  /// Returns the number of functions
  std::size_t getNumberOfFunctions() const;

  /// Change function to what is passed
  bool changeFunction(std::size_t at, vtkPiecewiseFunction* pwf, bool pwf_editable);

  /// Change editablity
  bool changeEditablity(std::size_t at, bool editable);

  /// Change visablity
  bool changeVisablity(std::size_t at, bool visable);

public slots:
  /// Set the current point. Set to -1 clear the current point.
  void setCurrentPoint(vtkIdType index);

  /// Set the X-position for the current point (without changing the Y position,
  /// if applicable. We ensure that the point doesn't move past neighbouring
  /// points. Note this will not change the end points i.e. start and end points.
  void setCurrentPointPosition(double xpos);

  /// re-renders the transfer function view. This doesn't render immediately,
  /// schedules a render.
  void render();

  void setMinX(double d);
  void setMaxX(double d);
  void setMinY(double d);
  void setMaxY(double d);

  void setFunctionType(bool);

  void setSplineControl(double, double, double);

signals:
  /// signal fired when the \c current selected control point changes.
  void currentPointChanged(vtkIdType index);

  /// signal fired to indicate that the control points changed i.e. either they
  /// were moved, orone was added/deleted, or edited to change color, etc.
  void controlPointsModified();

protected slots:
  /// slot called when the internal vtkControlPointsItem fires
  /// vtkControlPointsItem::CurrentPointChangedEvent
  void onCurrentChangedEvent();

  void renderInternal();

protected:
  /// callback called when vtkControlPointsItem fires
  /// vtkControlPointsItem::CurrentPointEditEvent.
  void onCurrentPointEditEvent();

  vtkIdType currentPoint() const;

private:
  Q_DISABLE_COPY(pqGeneralTransferFunctionWidget)

  class pqInternals;
  pqInternals* Internals;
};

#endif
