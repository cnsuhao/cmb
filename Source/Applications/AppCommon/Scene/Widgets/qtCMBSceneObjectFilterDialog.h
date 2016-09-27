//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBSceneObjectFilterDialog - changes the user defined type of an object.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBSceneObjectFilterDialog_h
#define __qtCMBSceneObjectFilterDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include <QPointer>
#include "cmbSystemConfig.h"

class QDoubleValidator;

namespace Ui
{
  class qtqtCMBSceneObjectFilterDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBSceneObjectFilterDialog : public QDialog
{
  Q_OBJECT
  typedef QDialog Superclass;

public:
  qtCMBSceneObjectFilterDialog(QWidget* parent=0);
  ~qtCMBSceneObjectFilterDialog() override;


  void setBounds(double bounds[6]);
  void getBounds(double bounds[6]);

  void setObjectTypes(QStringList& objTypes);
  void getSelectedObjectTypes(QStringList& objTypes);

  const char* getSceneFile();
  void setSceneFile(const char* );

  bool getUseBoundsConstraint();
  void setUseBoundsConstraint(bool);

public slots:
  void accept() override;

protected:

  Ui::qtqtCMBSceneObjectFilterDialog *FilterDialog;
  QPointer<QDoubleValidator> BoundsValidator;

};

#endif /* __qtCMBSceneObjectFilterDialog_h */
