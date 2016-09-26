//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBSceneSurfaceMesherDialog - provides a dialog to define meshing parameters.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBSceneSurfaceMesherDialog_h
#define __qtCMBSceneSurfaceMesherDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui
{
  class qtCMBSceneSurfaceMesherDialog;
};

class pqCMBSceneTree;

class CMBAPPCOMMON_EXPORT qtCMBSceneSurfaceMesherDialog : public QDialog
{
  Q_OBJECT
public:
  qtCMBSceneSurfaceMesherDialog(pqCMBSceneTree *tree, QWidget *parent = NULL, Qt::WindowFlags flags= 0);
  ~qtCMBSceneSurfaceMesherDialog() override;

  void insertSurfaceName(int i, const char *vname);
  void removeAllSurfaceNames();
  void setSelectedSurfaceNames(QList<int> &currentIndices);
  void getSelectedSurfaceNames(QStringList &selectedNames) const;
  int getNumberOfSurfaceNames() const;

  void insertVOIName(int i, const char *vname);
  void removeVOIName(int i);
  void removeAllVOINames();
  QString getVOIName(int i) const;
  void setCurrentVOINameIndex(int i);
  int getCurrentVOINameIndex() const;
  QString getCurrentVOIName() const;
  int getNumberOfVOINames() const;

  double getElevationWeightRadius();
  bool getMeshVisibleArcSets();

private slots:
  void surfaceSelectionChanged();
protected:
  Ui::qtCMBSceneSurfaceMesherDialog *InternalWidget;
  pqCMBSceneTree *Tree;

};




#endif /* __qtCMBSceneSurfaceMesherDialog_h */
