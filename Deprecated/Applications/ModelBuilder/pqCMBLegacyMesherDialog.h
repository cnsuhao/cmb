/*=========================================================================

  Program:   CMB
  Module:    pqCMBLegacyMesherDialog.h

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
