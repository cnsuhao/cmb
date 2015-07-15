//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqModelTreeViewEventTranslator - The event translator for smtk::attribute::qtModelView
// .SECTION Description
// .SECTION Caveats

#ifndef __pqModelTreeViewEventTranslator_h
#define __pqModelTreeViewEventTranslator_h

#include "pqWidgetEventTranslator.h"
#include <QPoint>
#include "cmbSystemConfig.h"

/**
Translates low-level Qt events into high-level cmb events that can be recorded as test cases.

\sa pqEventTranslator
*/

class pqModelTreeViewEventTranslator : public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  pqModelTreeViewEventTranslator(QObject* p=0);
  virtual bool translateEvent(QObject* Object, QEvent* Event, bool& Error);

protected slots:

protected:
  QPoint LastPos;

private:
  pqModelTreeViewEventTranslator(const pqModelTreeViewEventTranslator&);
  pqModelTreeViewEventTranslator& operator=(const pqModelTreeViewEventTranslator&);
};

#endif // !_pqModelTreeViewEventTranslator_h
