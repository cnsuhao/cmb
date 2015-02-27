/*=========================================================================

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

#include "pqCMBTabWidgetTranslator.h"

#include <QTabBar>
#include <QEvent>
#include <QString>

pqCMBTabWidgetTranslator::pqCMBTabWidgetTranslator(QObject* p)
  : pqWidgetEventTranslator(p),
  CurrentObject(0)
{
}

bool pqCMBTabWidgetTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QTabBar* const object = qobject_cast<QTabBar*>(Object);
  if(!object)
    return false;

  switch(Event->type())
    {
    case QEvent::Enter:
      if(this->CurrentObject != Object)
        {
        if(this->CurrentObject)
          {
          disconnect(this->CurrentObject, 0, this, 0);
          }

        this->CurrentObject = object;
        connect(object, SIGNAL(currentChanged(int)), this, SLOT(indexChanged(int)));
        }
      break;
    default:
      break;
    }

  return true;
}

void pqCMBTabWidgetTranslator::indexChanged(int which)
{
  QString text = this->CurrentObject->tabText(which);
  if (text.isEmpty())
    { // If tab text is empty, use index instead
    int index = this->CurrentObject->currentIndex();
    emit recordEvent(this->CurrentObject, "set_tab", QString::number(index));
    }
  else
    {
    emit recordEvent(this->CurrentObject, "set_tab_with_text", text);
    }
}
