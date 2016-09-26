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


#ifndef __qtCMBSceneMesherDialog_h
#define __qtCMBSceneMesherDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui
{
  class qtCMBSceneMesherDialog;
};

class pqCMBSceneTree;

class CMBAPPCOMMON_EXPORT qtCMBSceneMesherDialog : public QDialog
{
  Q_OBJECT
public:
  qtCMBSceneMesherDialog(pqCMBSceneTree *tree, QWidget *parent = NULL, Qt::WindowFlags flags= 0);
  ~qtCMBSceneMesherDialog() override;

  void insertMesherPath(int i, const char *mpath);
  void removeMesherPath(int i);
  void removeAllMesherPaths();
  QString getMesherPath(int i) const;
  void setCurrentMesherPathIndex(int i);
  int getCurrentMesherPathIndex() const;
  QString getCurrentMesherPath() const;
  int getNumberOfMesherPaths() const;

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

  void setTemporaryPtsFileName(const char *name);
  QString getTemporaryPtsFileName();

  void setMeshLength(double c, bool isRelative);
  double getMeshLength(bool &isRelative) const;

  void setInterpolatingRadius(double c);
  double getInterpolatingRadius() const;

  bool getDeleteCreatedPtsFile();

  void setOmicronBaseName(const char *name);
  QString getOmicronBaseName();

private slots:
  void surfaceSelectionChanged();
  void displayFileBrowser();
  void filesSelected(const QStringList &files);

signals:
  void mesherSelectionChanged(int);

protected:
  Ui::qtCMBSceneMesherDialog *InternalWidget;
  pqCMBSceneTree *Tree;

};




#endif /* __qtCMBSceneMesherDialog_h */
