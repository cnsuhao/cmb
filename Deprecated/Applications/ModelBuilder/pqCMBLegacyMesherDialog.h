//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLegacyMesherDialog - provides a dialog to define meshing parameters.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBLegacyMesherDialog_h
#define __pqCMBLegacyMesherDialog_h

#include <QDialog>
#include "cmbSystemConfig.h"

class pqServer;

namespace Ui
{
  class qtCMBMesherDialog;
};


class pqCMBLegacyMesherDialog : public QDialog
{
  Q_OBJECT
public:

  pqCMBLegacyMesherDialog(pqServer* server,
                      double modelBounds[6],
                      QWidget *parent = NULL,
                      Qt::WindowFlags flags= 0);
  virtual ~pqCMBLegacyMesherDialog();

  // Sets the bounds of the model - needed when dealing with
  // relative mesh sizes
  void setModelBounds(const double bounds[6]);
  double getMinModelBounds() const;

  void setFileName(QString name);
  QString getFileName() const;

  double getMeshLength(bool &isRelative) const;

  QString getActiveMesher() const;
  QString getTetGenOptions() const;

protected slots:
  void displayFileBrowser();
  void filesSelected(const QStringList &files);
  void displayAdvanceOptions(bool);
  void mesherSelectionChanged(int);
  bool selectVolumeMesher( const QList<QStringList> &files );
private:
  void setMeshLength(double c, bool isRelative);

  Ui::qtCMBMesherDialog *InternalWidget;
  pqServer *CurrentServer;
  double ModelBounds[6];
  QString DefaultVolumeMesher;
  int LastMesherPathIndex;
};

#endif /* __pqCMBLegacyMesherDialog_h */
