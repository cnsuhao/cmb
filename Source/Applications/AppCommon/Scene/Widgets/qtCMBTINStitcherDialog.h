/*=========================================================================

  Program:   CMB
  Module:    qtCMBTINStitcherDialog.h

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
// .NAME qtCMBTINStitcherDialog - .
// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBTINStitcherDialog_h
#define __qtCMBTINStitcherDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui
{
  class qtCMBTINStitcherDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBTINStitcherDialog : public QDialog
{
  Q_OBJECT
public:
  qtCMBTINStitcherDialog(QWidget *parent = NULL, Qt::WindowFlags flags= 0);
  virtual ~qtCMBTINStitcherDialog();

  // minimum angle for use inside Triangle
  void setMinimumAngle(double angle);
  double getMinimumAngle() const;

  // if Type I, should quads be used?
  void setUseQuads(bool useQuads);
  bool getUseQuads() const;

  // allow point insertion?
  void setAllowInteriorPointInsertion(bool allowInteriorPointInsertion);
  bool getAllowInteriorPointInsertion() const;

  // Tolerance used to calculate max distance for equality/line
  void setTolerance(double angle);
  double getTolerance() const;

  // User Specified TIN type setting
  void setUserSpecifiedTINType(int TINType);
  int getUserSpecifiedTINType() const;

protected slots:
  void tinTypeChanged();
  void useQuadsChanged();
  void allowPointInsertionChanged();

protected:
  Ui::qtCMBTINStitcherDialog *InternalWidget;

};
#endif
