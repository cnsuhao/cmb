//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqScalarBarWidget - a CMB color map Dialog object.
// .SECTION Description
//  This class provides a color editor dialog for the representation
// .SECTION Caveats

#ifndef _qtScalarBarWidget_h
#define _qtScalarBarWidget_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QColor>
#include "cmbSystemConfig.h"

class pqDataRepresentation;
class QDialog;

class CMBAPPCOMMON_EXPORT pqScalarBarWidget : public QObject
{
  Q_OBJECT

public:
  pqScalarBarWidget(pqDataRepresentation* display, QWidget* p = NULL);
  virtual ~pqScalarBarWidget();

public:

  void setIndexedColors( const QList<QColor>& colors );
  void setAnnotations(
    const QList<QVariant>& annotations );
  void setVisible(bool visible);

  void setTitle(const QString& title);
  void setPositionToLeft();
  void setPositionToRight();
protected:
  void init();

private:
  class cmbInternals;
  cmbInternals* Internals;
};

#endif
