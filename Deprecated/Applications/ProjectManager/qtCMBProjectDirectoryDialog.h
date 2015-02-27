/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBProjectDirectoryDialog.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __qtCMBProjectDirectoryDialog_h
#define __qtCMBProjectDirectoryDialog_h

#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui { class qtProjectDirectoryDialog; }

class qtCMBProjectDirectoryDialog : public QDialog
{
  Q_OBJECT
  typedef QDialog Superclass;
public:
  qtCMBProjectDirectoryDialog(QString const& programName,
    QString const& dir,
    QWidget* parent=0);
  virtual ~qtCMBProjectDirectoryDialog();

  QString const& getSelectedDirectory() const {return selectedDirectory;}
protected slots:
  void open();

protected:
  QString programName;
  QString defaultDir;
  QString selectedDirectory;

  void updateDialogWithProgramName();
private:
  Q_DISABLE_COPY(qtCMBProjectDirectoryDialog);
  Ui::qtProjectDirectoryDialog* const Ui;
};

#endif
