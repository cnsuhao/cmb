//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBRulerDialog - The Ruler dialog for CMB.
// .SECTION Description
// .SECTION Caveats

#ifndef _pqCMBRulerDialog_h
#define _pqCMBRulerDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

#include <QDialog>

class pqDataRepresentation;
class QDialog;
class pqProxyWidget;
class pqPipelineSource;

class CMBAPPCOMMON_EXPORT pqCMBRulerDialog : public QDialog
{
  Q_OBJECT

  public:
    pqCMBRulerDialog(QWidget* p = NULL);
    virtual ~pqCMBRulerDialog();

  protected:
    pqProxyWidget* RulerWidget;
    pqPipelineSource* RulerSource;

};

#endif // !_pqCMBRulerDialog_h
