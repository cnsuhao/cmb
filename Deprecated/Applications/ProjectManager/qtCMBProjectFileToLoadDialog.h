/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBProjectFileToLoadDialog.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __qtCMBProjectFileToLoadDialog_h
#define __qtCMBProjectFileToLoadDialog_h

#include <QDialog>
#include <QModelIndex>
#include "cmbSystemConfig.h"
class QSortFilterProxyModel;
class pqFileDialogModel;
class QPushButton;

namespace Ui { class qtProjectFileToLoadDialog; }
class qtCMBProjectFileToLoadDialog : public QDialog
{
  Q_OBJECT
  typedef QDialog Superclass;
public:
  qtCMBProjectFileToLoadDialog(QString const& programName,
    QRegExp const& programExt, pqFileDialogModel *model,
    QWidget* parent=0);
  virtual ~qtCMBProjectFileToLoadDialog();

  bool fileChosen() const { return notCanceled; }
  QString const& getFileToLoad() const {return fileToLoad;}
protected slots:
  void openFile();
  void openFile(const QModelIndex&);
  void cancelDialog();
  void newFile();

protected:
  QString programName;
  QString fileToLoad;
  bool notCanceled;

  QPushButton* NewFileBtn;
  pqFileDialogModel *Model;
  QSortFilterProxyModel* Filter;
private:
  Q_DISABLE_COPY(qtCMBProjectFileToLoadDialog);
  Ui::qtProjectFileToLoadDialog* const Ui;
};

#endif
