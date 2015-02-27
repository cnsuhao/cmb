/*=========================================================================

  Program:   CMB
  Module:    qtCMBStackedTINDialog.h

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
// .NAME qtCMBStackedTINDialog - creates a stack of TINS.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBStackedTINDialog_h
#define __qtCMBStackedTINDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;

namespace Ui
{
  class  qtSceneGenqtCMBStackedTINDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBStackedTINDialog : public QObject
{
  Q_OBJECT

public:
  static int processTIN(pqCMBSceneNode *node);

protected slots:
  void accept();
  void cancel();
  void setNumberOfLayers(int num);
  void setTotalThickness(double thickness);
  void offsetChanged();

protected:
  qtCMBStackedTINDialog(pqCMBSceneNode *n);
  virtual ~qtCMBStackedTINDialog();
  int exec();
  int Status;
  Ui::qtSceneGenqtCMBStackedTINDialog *StackDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
  double TotalThickness;
  int NumberOfLayers;
};


#endif /* __qtCMBStackedTINDialog_h */
