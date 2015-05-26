//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBCreateSimpleGeometry - Widget for creating simple geometries for CMB.
// .SECTION Description
// .SECTION Caveats
#ifndef _qtCMBCreateSimpleGeometry_h
#define _qtCMBCreateSimpleGeometry_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include <vector>
#include "cmbSystemConfig.h"

namespace Ui { class qtCMBCreateSimpleGeometry; }

class QPixmap;

/// Provides an about dialog
class CMBAPPCOMMON_EXPORT qtCMBCreateSimpleGeometry : public QDialog
{
  Q_OBJECT

public:
  qtCMBCreateSimpleGeometry(QWidget* Parent);
  virtual ~qtCMBCreateSimpleGeometry();

  int getGeometryType();
  void getGeometryValues(std::vector<double>& values);
  void getResolutionValues(std::vector<int>& values);

private:
  qtCMBCreateSimpleGeometry(const qtCMBCreateSimpleGeometry&);
  qtCMBCreateSimpleGeometry& operator=(const qtCMBCreateSimpleGeometry&);

  Ui::qtCMBCreateSimpleGeometry* Ui;
};

#endif // !_qtCMBCreateSimpleGeometry_h
