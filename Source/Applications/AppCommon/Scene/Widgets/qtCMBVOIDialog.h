//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBVOIDialog - edits a VOI Node.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBVOIDialog_h
#define __qtCMBVOIDialog_h

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
  class  qtDefineVOI;
};

class CMBAPPCOMMON_EXPORT qtCMBVOIDialog : public QObject
{
  Q_OBJECT

public:
  static int manageVOI(pqCMBSceneNode *node);

protected slots:
  void accept();
  void cancel();

protected:
  qtCMBVOIDialog(pqCMBSceneNode *n);
  ~qtCMBVOIDialog() override;
  int exec();
  int Status;
  Ui::qtDefineVOI *VOIDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
};

#endif /* __qtCMBVOIDialog_h */
