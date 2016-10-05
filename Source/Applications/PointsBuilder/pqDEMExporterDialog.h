//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqDEMExportDialog - Controls the export of DEM files.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqDEMExportDialog_h
#define __pqDEMExportDialog_h

#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class QDialog;
class pqPipelineSource;
class pqServer;

namespace Ui
{
  class  qtDEMExporter;
};

class pqDEMExportDialog : public QObject
{
  Q_OBJECT

public:
  static int exportToDem(QWidget *parent, pqServer *server,
                         double * min, double * max,
                         int * outRastSize, double * spacing,
                         int* zone, bool * isnorth, double * scale);

protected slots:
  void accept();
  void cancel();
  void valueChanged();

protected:
  pqDEMExportDialog(QWidget *parent, pqServer *server, double *min, double * max,
                    double * spacing, int zone, bool isnorth, double scale);
  ~pqDEMExportDialog() override;
  int exec();
  int Status;
  Ui::qtDEMExporter *ExportDialog;
  QDialog *MainDialog;
  QString FileName;
  pqServer *Server;
  int width, height;
  double dist_w, dist_h;
};

#endif /* __pqCMBLIDARSaveDialog_h */
