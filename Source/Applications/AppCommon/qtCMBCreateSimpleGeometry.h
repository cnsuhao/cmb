/*=========================================================================

  Program:   CMB
  Module:    qtCMBAboutDialog.h

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
