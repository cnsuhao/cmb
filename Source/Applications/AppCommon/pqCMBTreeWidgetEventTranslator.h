/*=========================================================================

Program:   CMB
Module:    pqCMBSceneObjectBase.cxx

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
  virtual bool translateEvent(QObject* Object, QEvent* Event, bool& Error);

protected slots:

protected:
  QPoint LastPos;

private:
  pqCMBTreeWidgetEventTranslator(const pqCMBTreeWidgetEventTranslator&);
  pqCMBTreeWidgetEventTranslator& operator=(const pqCMBTreeWidgetEventTranslator&);
};

#endif // !_pqCMBTreeWidgetEventTranslator_h
