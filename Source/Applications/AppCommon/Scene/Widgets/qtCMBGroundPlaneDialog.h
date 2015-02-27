/*=========================================================================

  Program:   CMB
  Module:    qtCMBGroundPlaneDialog.h

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
