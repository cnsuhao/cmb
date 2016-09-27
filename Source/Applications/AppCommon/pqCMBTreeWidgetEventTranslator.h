//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#ifndef _pqCMBTreeWidgetEventTranslator_h
#define _pqCMBTreeWidgetEventTranslator_h

#include "cmbAppCommonExport.h"

#include "pqWidgetEventTranslator.h"
#include <QPoint>
#include "cmbSystemConfig.h"

/**
Translates low-level Qt events into high-level ParaView events that can be recorded as test cases.

\sa pqEventTranslator
*/

class CMBAPPCOMMON_EXPORT pqCMBTreeWidgetEventTranslator : public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  pqCMBTreeWidgetEventTranslator(QObject* p=0);
  bool translateEvent(QObject* Object, QEvent* Event, bool& Error) override;

protected slots:

protected:
  QPoint LastPos;

private:
  pqCMBTreeWidgetEventTranslator(const pqCMBTreeWidgetEventTranslator&);
  pqCMBTreeWidgetEventTranslator& operator=(const pqCMBTreeWidgetEventTranslator&);
};

#endif // !_pqCMBTreeWidgetEventTranslator_h
