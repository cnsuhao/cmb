/*=========================================================================

  Program:   CMB
  Module:    pqCMBEnumPropertyWidget.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME pqCMBEnumPropertyWidget - a CMB enum property dropdown widget.
// .SECTION Description
//  This class is a convenient widget to modify an enum-property of
//  a representation proxy.
// .SECTION Caveats

#ifndef __pqCMBEnumPropertyWidget_h
#define __pqCMBEnumPropertyWidget_h

#include "cmbAppCommonExport.h"
#include <QWidget>
#include "cmbSystemConfig.h"

class pqCMBEnumPropertyWidgetInternal;
class pqDataRepresentation;

class CMBAPPCOMMON_EXPORT pqCMBEnumPropertyWidget : public QWidget
{
  Q_OBJECT

public:
  pqCMBEnumPropertyWidget(QWidget* parent=0);
  virtual ~pqCMBEnumPropertyWidget();

  // Description:
  // Set the property name of the proxy that will be linked to the widget
  void setPropertyName(const char*);

  // Description:
  // Set the label text of the widget
  void setLabelText(const char*);

  // Description:
  // Set Enable/Visible property
  void setEnabled(int);
  void setVisible(int);

signals:
  // Description:
  // Fired when the text in the dropdown box is changed
  void currentTextChanged(const QString&);

  // Description:
  // Fired when the widget begins an undo-able change.
  void beginUndo(const QString&);

  // Description:
  // Fired when the widget is finished with an undo-able change.
  void endUndo();

public slots:
  void setRepresentation(pqDataRepresentation* display);
  void reloadGUI();

private slots:
  // Description:
  // Called when the text in the dropdown box is changed
  void onCurrentTextChanged(const QString&);

  // Description:
  // Called when the qt widget changes, we mark undo set
  // and push the widget changes to the property.
  void onQtWidgetChanged();

  // Description:
  // Called whenever the linked property is changed.
  void updateLinks();

private:
  pqCMBEnumPropertyWidgetInternal* Internal;

  QString RepPropertyName;
};
#endif
