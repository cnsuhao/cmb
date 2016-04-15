//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __qtCMBArcWidget_h
#define __qtCMBArcWidget_h

#include "cmbAppCommonExport.h"
#include "pq3DWidget.h"
#include <QColor>
#include "cmbSystemConfig.h"
#include "vtkContourWidget.h"
#include "vtkCMBArcWidgetRepresentation.h"

class pqServer;
class vtkSMProxy;

/// GUI for ArcWidgetRepresentation. This is a 3D widget that edits an Arc.
class CMBAPPCOMMON_EXPORT qtCMBArcWidget : public pq3DWidget
{
  Q_OBJECT
  typedef pq3DWidget Superclass;
public:
  qtCMBArcWidget(vtkSMProxy* refProxy, vtkSMProxy* proxy, QWidget* parent);
  virtual ~qtCMBArcWidget();

  /// Resets the bounds of the 3D widget to the reference proxy bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget
  /// with reference proxy bounds.
  /// This should be explicitly called after the panel is created
  /// and the widget is initialized i.e. the reference proxy, controlled proxy
  /// and hints have been set.
  virtual void resetBounds(double /*bounds*/[6]) {}
  virtual void resetBounds()
    { return this->Superclass::resetBounds(); }

  // Some convenient methods
  // Set the point placer/line interpolator
  virtual void setPointPlacer(vtkSMProxy*);
  virtual void setLineInterpolator(vtkSMProxy*);

  /// Activates the widget. Respects the visibility flag.
  virtual void select();
  /// Deactivates the widget.
  virtual void deselect();

  /// Set the line color
  virtual void setLineColor(const QColor& color);

  /// Update the UI to be in the Arc Editing mode.
  /// In this mode, for whole arc, the Visibility, Closed, Delete, buttonRectArc
  /// are all invisible; and for sub-arc, we only allow Modify for now.
  virtual void useArcEditingUI(bool isWholeArc);

  void highlightPoint(int);

signals:
  /// Signal emitted when the representation proxy's "ClosedLoop" property
  /// is modified.
  void contourLoopClosed();
  void contourDone();

public slots:
  void removeAllNodes();
  void checkContourLoopClosed();

  /// Close the contour loop
  void closeLoop(bool);

  /// Move to modify mode
  void ModifyMode( );

  /// Check if the loop can even go to edit mode
  void checkCanBeEdited();

  /// Move to the next mode ( Drawing, Editing, Done )
  void updateMode( );

  /// Finish editing the contour
  void finishContour( );

  /// Generate a rectangle arc if possible from available
  /// points in the contour
  void generateRectangleArc( );

  /// Resets pending changes. Default implementation
  /// pushes the property values of the controlled widget to the
  /// 3D widget properties.
  /// The correspondence is determined from the <Hints />
  /// associated with the controlled proxy.
  virtual void reset();

protected:
  /// Internal method to create the widget.
  virtual void createWidget(pqServer*);

  /// Update the widget visibility according to the WidgetVisible and Selected flags
  virtual void updateWidgetVisibility();

  /// Internal method to cleanup widget.
  void cleanupWidget();

protected slots:
  void deleteAllNodes();

private:
  qtCMBArcWidget(const qtCMBArcWidget&); // Not implemented.
  void operator=(const qtCMBArcWidget&); // Not implemented.

  void updateRepProperty(vtkSMProxy* smProxy,
    const char* propertyName);

  class pqInternals;
  pqInternals* Internals;
};

#endif
