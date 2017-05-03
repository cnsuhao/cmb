//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBEnumPropertyWidget - a CMB enum property dropdown widget.
// .SECTION Description
//  This class is a convenient widget to modify an enum-property of
//  a representation proxy.
// .SECTION Caveats

#ifndef __pqCMBEnumPropertyWidget_h
#define __pqCMBEnumPropertyWidget_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QWidget>

class pqCMBEnumPropertyWidgetInternal;
class pqDataRepresentation;

class CMBAPPCOMMON_EXPORT pqCMBEnumPropertyWidget : public QWidget
{
  Q_OBJECT

public:
  pqCMBEnumPropertyWidget(QWidget* parent = 0);
  ~pqCMBEnumPropertyWidget() override;

  // Description:
  // Set the property name of the proxy that will be linked to the widget
  void setPropertyName(const char*);

  // Description:
  // Set the label text of the widget
  void setLabelText(const char*);

  // Description:
  // Set Enable/Visible property
  void setEnabled(int);

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
