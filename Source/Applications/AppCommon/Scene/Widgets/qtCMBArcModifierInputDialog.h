//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBSceneMesherDialog - provides a dialog to define meshing parameters.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBArcModifierInputDialog_h
#define __qtCMBArcModifierInputDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"
#include "pqCMBSceneObjectBase.h"

namespace Ui
{
  class qtCMBArcModifierInputDialog;
};

class pqCMBSceneTree;

class CMBAPPCOMMON_EXPORT qtCMBArcModifierInputDialog : public QDialog
{
  Q_OBJECT
public:
  qtCMBArcModifierInputDialog(pqCMBSceneTree *tree, QWidget *parent = NULL, Qt::WindowFlags flags= 0);
  virtual ~qtCMBArcModifierInputDialog();

  void insertSourceName(int i, const char *vname);
  void removeAllSourceNames();
  void setSelectedSourceNames(QList<int> &currentIndices);
  void getSelectedSourceNames(QStringList &selectedNames) const;
  int getNumberOfSourceNames() const;

  void insertArcName(int i, const char *vname);
  void removeAllArcNames();
  void setSelectedArcNames(QList<int> &currentIndices);
  void getSelectedArcNames(QStringList &selectedNames) const;
  int getNumberOfArcNames() const;

  void setUseNormal(bool);
  bool getUseNormal() const;

  pqCMBSceneObjectBase::enumObjectType GetSourceType();

signals:
  void sourceTypeChanged();

private slots:
  void selectedSourceChanged();

signals:
  void selectedSourceChanged(int);

protected:
  Ui::qtCMBArcModifierInputDialog *InternalWidget;
  pqCMBSceneTree *Tree;

};

#endif /* __qtCMBSceneMesherDialog_h */
