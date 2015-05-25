//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBGroundPlaneDialog - edits a GroundPlane Node.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBGroundPlaneDialog_h
#define __qtCMBGroundPlaneDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QObject>
#include <QStringList>
#include <vector>

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;
class pqPipelineSource;

namespace Ui
{
  class  qtDefineGroundPlane;
};

class CMBAPPCOMMON_EXPORT qtCMBGroundPlaneDialog : public QObject
{
  Q_OBJECT

public:
  static int manageGroundPlane(pqCMBSceneNode *node);
  static int defineGroundPlane(double p1[3], double p2[3]);

protected slots:
  void accept();
  void cancel();

protected:
  qtCMBGroundPlaneDialog(pqCMBSceneNode *n);
  virtual ~qtCMBGroundPlaneDialog();
  int exec();
  int Status;
  Ui::qtDefineGroundPlane *GroundPlaneDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
  double Point1[3], Point2[3];
};

#endif /* __qtCMBGroundPlaneDialog_h */
