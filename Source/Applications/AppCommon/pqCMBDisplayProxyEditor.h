//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBBoundaryConditionTree - a CMB display properties editor object.
// .SECTION Description
//  This class provides an editor for the properties of a representation display
// .SECTION Caveats

#ifndef _pqCMBDisplayProxyEditor_h
#define _pqCMBDisplayProxyEditor_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqProxyWidget.h"
#include <QPointer>

class pqDataRepresentation;

class CMBAPPCOMMON_EXPORT pqCMBDisplayProxyEditor : public pqProxyWidget
{
  Q_OBJECT

public:
  pqCMBDisplayProxyEditor(pqDataRepresentation* display, QWidget* p = NULL);
  ~pqCMBDisplayProxyEditor() override;

public:
  // Description:
  // get the display whose properties.
  virtual pqDataRepresentation* displayRepresentation() { return this->m_rep; }

protected:
  QPointer<pqDataRepresentation> m_rep;
};

#endif
