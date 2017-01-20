//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBConeDialog - edits a Cone Source.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBConeDialog_h
#define __qtCMBConeDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"

class pqProxyWidget;
class QDialog;
class pqPipelineSource;
class pqRenderView;

class CMBAPPCOMMON_EXPORT qtCMBConeDialog : public QObject
{
  Q_OBJECT

public:
  qtCMBConeDialog(pqPipelineSource* coneSource,
    pqRenderView* view);
  ~qtCMBConeDialog() override;

  int exec();

protected slots:
  void accept();
  void cancel();

protected:
  int Status;
  QDialog *MainDialog;
  pqProxyWidget* ConeSourcePanel;
};

#endif /* __qtCMBConeDialog_h */
