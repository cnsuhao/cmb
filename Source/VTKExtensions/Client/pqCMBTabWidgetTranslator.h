//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBTabWidgetTranslator_h
#define _pqCMBTabWidgetTranslator_h

#include "pqWidgetEventTranslator.h"
#include "vtkCMBClientModule.h" // For export macro
#include <QPointer>
class QTabBar;

/**
Translates low-level Qt events into high-level ParaView events that can be recorded as test cases.

\sa pqEventTranslator
*/

class VTKCMBCLIENT_EXPORT pqCMBTabWidgetTranslator :
  public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  pqCMBTabWidgetTranslator(QObject* p=0);

  virtual bool translateEvent(QObject* Object, QEvent* Event, bool& Error);

protected slots:
  void indexChanged(int);

private:
  pqCMBTabWidgetTranslator(const pqCMBTabWidgetTranslator&);
  pqCMBTabWidgetTranslator& operator=(const pqCMBTabWidgetTranslator&);

  QPointer<QTabBar> CurrentObject;

};

#endif // !_pqCMBTabWidgetTranslator_h
