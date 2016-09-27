//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBNewSceneUnitsDialog - gets the units for a scene.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBNewSceneUnitsDialog_h
#define __qtCMBNewSceneUnitsDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSceneUnits.h"
#include "cmbSystemConfig.h"
class QDialog;

namespace Ui
{
  class qtNewSceneUnitsDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBNewSceneUnitsDialog : public QObject
{
  Q_OBJECT

public:
  static bool getUnits(cmbSceneUnits::Enum initialUnits,
                       cmbSceneUnits::Enum &newUnits);

protected slots:
  void accept();
  void cancel();

protected:
  qtCMBNewSceneUnitsDialog(cmbSceneUnits::Enum initial);
  qtCMBNewSceneUnitsDialog():
    NewUnitsDialog(NULL), MainDialog(NULL), NewUnits(cmbSceneUnits::Unknown),
    Status(false)
    {}
  ~qtCMBNewSceneUnitsDialog() override;
  bool exec(cmbSceneUnits::Enum &newUnits);

  Ui::qtNewSceneUnitsDialog *NewUnitsDialog;
  QDialog *MainDialog;
  cmbSceneUnits::Enum NewUnits;
  bool Status;
};

#endif /* __qtCMBNewSceneUnitsDialog_h */
