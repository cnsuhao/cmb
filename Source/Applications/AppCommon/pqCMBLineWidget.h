//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqCMBLineWidget_h
#define __pqCMBLineWidget_h

#include "cmbAppCommonExport.h"
#include "pqLineWidget.h"
#include "cmbSystemConfig.h"

class CMBAPPCOMMON_EXPORT pqCMBLineWidget : public pqLineWidget
{
  Q_OBJECT
  typedef pqLineWidget Superclass;
public:
  pqCMBLineWidget(vtkSMProxy* o, vtkSMProxy* pxy, QWidget* p = 0);
  ~pqCMBLineWidget() override;

  /// Activates the widget. Respects the visibility flag.
  void select() override;

  /// Deactivates the widget.
  void deselect() override;

  /// set the ProcessEvents flag
  virtual void setProcessEvents(bool);

  void setColor(double c[3]);
  void getColor(double c[3]) const;
protected:
  /// Update the widget visibility according to the WidgetVisible and Selected flags
  void updateWidgetVisibility() override;

  /// updates the enable state of the picking shortcut.
  // virtual void updatePickShortcut();
  double originalColor[3];

private:
  pqCMBLineWidget(const pqCMBLineWidget&); // Not implemented.
  void operator=(const pqCMBLineWidget&); // Not implemented.
};

#endif
