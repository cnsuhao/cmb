/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneMesherDialog.h

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
  virtual ~qtCMBSceneMesherDialog();

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
