/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneObjectFilterDialog.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
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
  virtual ~qtCMBSceneObjectFilterDialog();


  void setBounds(double bounds[6]);
  void getBounds(double bounds[6]);

  void setObjectTypes(QStringList& objTypes);
  void getSelectedObjectTypes(QStringList& objTypes);

  const char* getSceneFile();
  void setSceneFile(const char* );

  bool getUseBoundsConstraint();
  void setUseBoundsConstraint(bool);

public slots:
  virtual void accept();

protected:

  Ui::qtqtCMBSceneObjectFilterDialog *FilterDialog;
  QPointer<QDoubleValidator> BoundsValidator;

};

#endif /* __qtCMBSceneObjectFilterDialog_h */
