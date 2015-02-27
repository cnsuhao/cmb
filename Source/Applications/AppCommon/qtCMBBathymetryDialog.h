/*=========================================================================

  Program:   CMB
  Module:    qtCMBBathymetryDialog.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
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
  virtual ~qtCMBBathymetryDialog();
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
