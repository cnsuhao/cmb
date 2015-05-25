/*=========================================================================

  
  

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
