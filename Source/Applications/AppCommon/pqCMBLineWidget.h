/*=========================================================================

   Program: ParaView
   Module:    pqCMBLineWidget.h

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
  virtual ~pqCMBLineWidget();

  /// Activates the widget. Respects the visibility flag.
  virtual void select();

  /// Deactivates the widget.
  virtual void deselect();

  /// set the ProcessEvents flag
  virtual void setProcessEvents(bool);

  void setColor(double c[3]);
  void getColor(double c[3]) const;
protected:
  /// Update the widget visibility according to the WidgetVisible and Selected flags
  virtual void updateWidgetVisibility();

  /// updates the enable state of the picking shortcut.
  // virtual void updatePickShortcut();
  double originalColor[3];

private:
  pqCMBLineWidget(const pqCMBLineWidget&); // Not implemented.
  void operator=(const pqCMBLineWidget&); // Not implemented.
};

#endif
