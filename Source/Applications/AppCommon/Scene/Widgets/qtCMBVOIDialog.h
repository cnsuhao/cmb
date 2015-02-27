/*=========================================================================

  Program:   CMB
  Module:    qtCMBVOIDialog.h

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
  virtual ~qtCMBVOIDialog();
  int exec();
  int Status;
  Ui::qtDefineVOI *VOIDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
};

#endif /* __qtCMBVOIDialog_h */
