//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBConeNodeDialog - edits a Cone Node.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBConeNodeDialog_h
#define __qtCMBConeNodeDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;
class pqPipelineSource;

namespace Ui
{
  class  qtCMBConicalSourceDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBConeNodeDialog : public QObject
{
  Q_OBJECT

public:
  static int manageCone(pqCMBSceneNode *node);

protected slots:
  void accept();
  void cancel();

protected:
  qtCMBConeNodeDialog(pqCMBSceneNode *n);
  virtual ~qtCMBConeNodeDialog();
  int exec();
  int Status;
  Ui::qtCMBConicalSourceDialog *ConeDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
};

#endif /* __qtCMBConeNodeDialog_h */
