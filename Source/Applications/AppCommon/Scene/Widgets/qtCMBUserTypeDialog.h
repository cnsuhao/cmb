/*=========================================================================

  Program:   CMB
  Module:    qtCMBUserTypeDialog.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME qtCMBUserTypeDialog - changes the user defined type of an object.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBUserTypeDialog_h
#define __qtCMBUserTypeDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"
class QDialog;
class pqCMBSceneNode;
namespace Ui
{
  class qtObjectTypeDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBUserTypeDialog : public QObject
{
  Q_OBJECT

public:
  static void updateUserType(pqCMBSceneNode *node);

protected slots:
  void accept();
  void cancel();
  void changeObjectType();

protected:
  qtCMBUserTypeDialog(pqCMBSceneNode *node);
  virtual ~qtCMBUserTypeDialog();
  void exec();

  Ui::qtObjectTypeDialog *TypeDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
};

#endif /* __qtCMBUserTypeDialog_h */
