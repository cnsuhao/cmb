//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBBathymetryDialog - manages the node's Bathymetry information.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBBathymetryDialog_h
#define __qtCMBBathymetryDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;
class pqServer;
class pqCMBSceneTree;
class qtCMBSceneObjectImporter;

namespace Ui
{
  class  qtCMBBathymetryDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBBathymetryDialog : public QObject
{
  Q_OBJECT

public:
  static int manageBathymetry(pqCMBSceneNode *node);

  qtCMBBathymetryDialog(pqCMBSceneTree* sceneTree);
  ~qtCMBBathymetryDialog() override;
  pqCMBSceneObjectBase* bathymetrySourceObject() const;
  double elevationRadius();
  double highElevationLimit();
  double lowElevationLimit();
  bool useHighElevationLimit();
  bool useLowElevationLimit();
  bool applyOnlyToVisibleMeshes();
  void setMeshAndModelMode(bool);
  int exec();

protected slots:
  void accept();
  void cancel();
  void displaySourceImporter();
  void removeBathymetry();

protected:
  qtCMBBathymetryDialog(pqCMBSceneNode *n);

  void initConnections();
  void initUI();

  int Status; // 0, cancel; 1, accept; 2, reverse/remove
  bool ModelAndMeshMode;
  Ui::qtCMBBathymetryDialog *BathymetryDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node;
  pqCMBSceneTree *SceneTree;
  qtCMBSceneObjectImporter* ObjectImporter;
};

#endif /* __qtCMBBathymetryDialog_h */
