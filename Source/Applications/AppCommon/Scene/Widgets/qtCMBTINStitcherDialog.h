//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
  ~qtCMBTINStitcherDialog() override;

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
