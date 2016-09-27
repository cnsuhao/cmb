//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef _pqCMBFileDialogEventTranslator_h
#define _pqCMBFileDialogEventTranslator_h

#include <pqWidgetEventTranslator.h>
#include "cmbAppCommonExport.h"
#include <QPointer>

class pqCMBFileDialog;

/**
Translates low-level Qt events into high-level ParaView events that can be recorded as test cases.

\sa pqEventTranslator
*/

class CMBAPPCOMMON_EXPORT pqCMBFileDialogEventTranslator :
  public pqWidgetEventTranslator
{
  Q_OBJECT
  
public:
  pqCMBFileDialogEventTranslator(QObject* p=0);
  
  bool translateEvent(QObject* Object, QEvent* Event, bool& Error) override;

private:
  pqCMBFileDialogEventTranslator(const pqCMBFileDialogEventTranslator&);
  pqCMBFileDialogEventTranslator& operator=(const pqCMBFileDialogEventTranslator&);

  QPointer<pqCMBFileDialog> CurrentObject;

private slots:
  void onFilesSelected(const QString&);
  void onCancelled();
};

#endif // !_pqCMBFileDialogEventTranslator_h

