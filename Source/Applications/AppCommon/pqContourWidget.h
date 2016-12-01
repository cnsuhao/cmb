//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef pqContourWidget_h
#define pqContourWidget_h

#include "cmbAppCommonExport.h"
#include "smtk/extension/paraview/widgets/pq3DWidget.h"
#include <QColor>

class pqServer;
class vtkSMProxy;

/// GUI for ContourWidgetRepresentation. This is a 3D widget that edits a Contour.
class CMBAPPCOMMON_EXPORT pqContourWidget : public pq3DWidget
{
  Q_OBJECT
  typedef pq3DWidget Superclass;
public:
  pqContourWidget(vtkSMProxy* refProxy, vtkSMProxy* proxy, QWidget* parent);
  ~pqContourWidget() override;

  /// Resets the bounds of the 3D widget to the reference proxy bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget
  /// with reference proxy bounds.
  /// This should be explicitly called after the panel is created
  /// and the widget is initialized i.e. the reference proxy, controlled proxy
  /// and hints have been set.
  void resetBounds(double /*bounds*/[6]) override {}
  void resetBounds() override
    { return this->Superclass::resetBounds(); }

  // Some convenient methods
  // Set the point placer/line interpolator
  virtual void setPointPlacer(vtkSMProxy*);
  virtual void setLineInterpolator(vtkSMProxy*);

  /// Activates the widget. Respects the visibility flag.
  void select() override;
  /// Deactivates the widget.
  void deselect() override;

  /// Set the line color
  virtual void setLineColor(const QColor& color);

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

  ///Move to the next mode ( Drawing, Editing, Done )
  void updateMode( );

  ///Toggle the edit mode, which will switch between Edit/Modify mode
  void toggleEditMode( );

  ///Finish editing the contour
  void finishContour( );

  /// Resets pending changes. Default implementation
  /// pushes the property values of the controlled widget to the
  /// 3D widget properties.
  /// The correspondence is determined from the <Hints />
  /// associated with the controlled proxy.
  void reset() override;

protected:
  /// Internal method to create the widget.
  virtual void createWidget(pqServer*);

  /// Update the widget visibility according to the WidgetVisible and Selected flags
  void updateWidgetVisibility() override;

protected slots:
  void deleteAllNodes();

private:
  pqContourWidget(const pqContourWidget&); // Not implemented.
  void operator=(const pqContourWidget&); // Not implemented.

  void updateRepProperty(vtkSMProxy* smProxy,
    const char* propertyName);

  class pqInternals;
  pqInternals* Internals;
};

#endif
