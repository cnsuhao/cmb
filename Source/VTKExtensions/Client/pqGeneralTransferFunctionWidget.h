//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
