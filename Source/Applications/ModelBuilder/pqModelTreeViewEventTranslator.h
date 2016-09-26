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

#include "pqTreeViewEventTranslator.h"
#include "cmbSystemConfig.h"
#include <QPoint>

/**\brief Translates low-level Qt events into high-level cmb events that can be recorded as test cases.
* 
* The smtk qtModelView has only one column with icons and text in same column, so in order to
* know which icons are cliked, we have to process the mouse click position, then record
* the icon/actions accordingly
* 
\sa pqTreeViewEventTranslator
*/

class pqModelTreeViewEventTranslator : public pqTreeViewEventTranslator
{
  Q_OBJECT

public:
  pqModelTreeViewEventTranslator(QObject* p=0);
  bool translateEvent(QObject* Object, QEvent* Event, bool& Error) override;
  using pqTreeViewEventTranslator::translateEvent;

private:
  pqModelTreeViewEventTranslator(const pqModelTreeViewEventTranslator&);
  pqModelTreeViewEventTranslator& operator=(const pqModelTreeViewEventTranslator&);
};

#endif // !_pqModelTreeViewEventTranslator_h
