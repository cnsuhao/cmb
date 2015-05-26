//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
