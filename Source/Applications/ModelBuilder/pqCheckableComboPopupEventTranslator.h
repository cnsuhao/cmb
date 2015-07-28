//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __pqCheckableComboPopupEventTranslator_h
#define __pqCheckableComboPopupEventTranslator_h

#include "pqWidgetEventTranslator.h"
#include "cmbSystemConfig.h"
#include <QPoint>

/**
Translates low-level Qt events into high-level ParaView events that can be recorded as test cases.

\sa pqEventTranslator
*/

class pqCheckableComboPopupEventTranslator :
  public pqWidgetEventTranslator
{
  Q_OBJECT
  
public:
  pqCheckableComboPopupEventTranslator(QObject* p=0);
  
  virtual bool translateEvent(QObject* Object, QEvent* Event, bool& Error);
 
protected: 
  QPoint LastPos;

private:
  pqCheckableComboPopupEventTranslator(const pqCheckableComboPopupEventTranslator&);
  pqCheckableComboPopupEventTranslator& operator=(const pqCheckableComboPopupEventTranslator&);

};

#endif // !_pqCheckableComboPopupEventTranslator_h

